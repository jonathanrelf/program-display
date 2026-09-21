// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "output_controller.hpp"
#include "output_request_policy.hpp"

#include "qt/display_metadata.hpp"

#include <QGuiApplication>
#include <QScreen>

#include <algorithm>
#include <sstream>

namespace program_display {
namespace {

template<typename Displays> std::vector<domain::DisplayMetadata> metadataOnly(const Displays &displays)
{
	std::vector<domain::DisplayMetadata> result;
	result.reserve(displays.size());
	for (const auto &display : displays) {
		result.push_back(display.metadata);
	}
	return result;
}

std::string configuredLabel(const std::optional<domain::DisplayMetadata> &configured)
{
	if (!configured) {
		return "Not configured";
	}
	if (!configured->name.empty()) {
		return configured->name;
	}
	if (!configured->model.empty()) {
		return configured->manufacturer + " " + configured->model;
	}
	return "Saved destination";
}

std::string requestedOutputMessage(const std::optional<domain::DisplayMetadata> &configured)
{
	return "Should now be visible on " + configuredLabel(configured) +
	       ". OBS cannot verify that it remains open.";
}

} // namespace

OutputController::OutputController(ProgramDisplayDock &dock, DestinationStore &store, ObsGateway &gateway)
	: dock_(dock),
	  store_(store),
	  gateway_(gateway)
{
	domain::DisplayMetadata destination;
	switch (store_.load(destination)) {
	case DestinationStore::LoadResult::Loaded:
		configured_ = std::move(destination);
		break;
	case DestinationStore::LoadResult::Error:
		startup_error_ = store_.lastError();
		break;
	case DestinationStore::LoadResult::NotConfigured:
		break;
	}

	dock_.setStartHandler([this](QScreen *screen) { selectAndStart(screen); });
	const auto displays = connectedDisplays();
	refreshDestinations(displays);
	present(configured_ ? "STARTING" : "UNCONFIGURED",
		configured_ ? "Waiting for OBS to finish loading." : "Choose a Program destination.");
}

void OutputController::finishedLoading()
{
	if (stopping_) {
		return;
	}
	ready_ = true;
	reconcile();
}

void OutputController::refreshOperationalDetails()
{
	if (!stopping_) {
		reconcile();
	}
}

void OutputController::screenAdded(QScreen *)
{
	if (stopping_) {
		return;
	}
}

void OutputController::screenRemoved(QScreen *screen)
{
	if (stopping_) {
		return;
	}
	if (outstanding_attachment_ == screen) {
		outstanding_attachment_.clear();
	}
}

void OutputController::selectAndStart(QScreen *screen)
{
	if (stopping_ || !ready_ || !screen) {
		return;
	}
	if (outstanding_attachment_) {
		present("ERROR", "A projector request is already outstanding. Restart OBS before reassignment.");
		return;
	}

	const domain::DisplayMetadata metadata = qt::displayMetadataFor(screen);
	const domain::IdentityStrength strength = domain::identity_strength(metadata);
	if (strength == domain::IdentityStrength::Insufficient) {
		present("ERROR", "This display exposes insufficient identity metadata for safe selection.", screen);
		return;
	}
	if (!store_.save(metadata)) {
		present("ERROR", store_.lastError(), screen);
		return;
	}

	startup_error_.clear();
	configured_ = metadata;
	reconcile();
}

void OutputController::stop()
{
	stopping_ = true;
	ready_ = false;
	dock_.setStartHandler({});
}

std::vector<OutputController::ConnectedDisplay> OutputController::connectedDisplays() const
{
	std::vector<ConnectedDisplay> result;
	const QList<QScreen *> screens = QGuiApplication::screens();
	result.reserve(static_cast<std::size_t>(screens.size()));
	for (QScreen *screen : screens) {
		const domain::DisplayMetadata metadata = qt::displayMetadataFor(screen);
		result.push_back({screen, metadata, displayLabel(metadata, screen)});
	}
	return result;
}

void OutputController::refreshDestinations(const std::vector<ConnectedDisplay> &displays)
{
	QScreen *selected = nullptr;
	std::optional<domain::Resolution> configured_resolution;
	if (configured_) {
		configured_resolution = domain::resolve_display_identity(*configured_, metadataOnly(displays));
		if (configured_resolution->matched_display_index >= 0) {
			selected =
				displays[static_cast<std::size_t>(configured_resolution->matched_display_index)].screen;
		}
	}

	std::vector<DestinationOption> options;
	options.reserve(displays.size() + (configured_ && !selected ? 1U : 0U));
	if (configured_ && !selected && configured_resolution) {
		const std::string name = configuredLabel(configured_);
		switch (configured_resolution->outcome) {
		case domain::ResolutionOutcome::Missing:
			options.push_back({nullptr, name + " — Not connected",
					   "Saved destination. Connect it again, or deliberately choose another display to "
					   "replace the association.",
					   false, false});
			break;
		case domain::ResolutionOutcome::Ambiguous:
			options.push_back({nullptr, name + " — Multiple matches",
					   "More than one connected display matches the saved destination. Choose a specific "
					   "display only to replace the association.",
					   false, false});
			break;
		case domain::ResolutionOutcome::InsufficientMetadata:
			options.push_back({nullptr, name + " — Cannot resolve",
					   "The saved destination cannot be resolved safely. Choose another display only to "
					   "replace the association.",
					   false, false});
			break;
		case domain::ResolutionOutcome::UniqueStrong:
		case domain::ResolutionOutcome::UniqueWeak:
			break;
		}
	}
	for (const auto &display : displays) {
		const domain::IdentityStrength strength = domain::identity_strength(display.metadata);
		const std::string name = display.metadata.name.empty() ? "Unnamed display" : display.metadata.name;
		switch (strength) {
		case domain::IdentityStrength::Strong:
			options.push_back({display.screen, name + " — Restores automatically",
					   display.label + "\nStable identity; safe to restore automatically.", true,
					   false});
			break;
		case domain::IdentityStrength::Weak:
			options.push_back(
				{display.screen, name + " — Remember this display",
				 display.label +
					 "\nSerial-less profile. Remember it once; automatic restore occurs only when "
					 "exactly one matching display is connected.",
				 true, true});
			break;
		case domain::IdentityStrength::Insufficient:
			options.push_back(
				{display.screen, name + " — Identity unavailable",
				 display.label +
					 "\nConnected, but missing identity metadata required for safe restoration.",
				 false, false});
			break;
		}
	}
	dock_.setDestinations(options, selected);
}

void OutputController::reconcile()
{
	if (stopping_) {
		return;
	}
	const auto displays = connectedDisplays();
	refreshDestinations(displays);

	if (!startup_error_.empty()) {
		present("ERROR", startup_error_);
		return;
	}
	if (!configured_) {
		present("UNCONFIGURED", "Choose a Program destination.");
		return;
	}
	if (!ready_) {
		present("STARTING", "Waiting for OBS to finish loading.");
		return;
	}
	if (gateway_.projectorPersistenceEnabled()) {
		present("ERROR", "OBS 'Save projectors on exit' is enabled. Disable it and restart OBS before using "
				 "Program Display.");
		return;
	}

	const domain::Resolution resolution = domain::resolve_display_identity(*configured_, metadataOnly(displays));
	if (resolution.outcome == domain::ResolutionOutcome::Missing) {
		present("MISSING", "The configured destination is not connected.");
		return;
	}
	if (resolution.outcome == domain::ResolutionOutcome::Ambiguous) {
		present("ERROR", "More than one connected display matches the saved identity.");
		return;
	}
	if (resolution.outcome == domain::ResolutionOutcome::InsufficientMetadata) {
		present("ERROR", "The saved identity is insufficient for safe restoration.");
		return;
	}

	QScreen *resolved = displays[static_cast<std::size_t>(resolution.matched_display_index)].screen;
	if (!shouldRequestOutput(resolution.outcome, !outstanding_attachment_.isNull())) {
		if (!outstanding_attachment_) {
			present("ERROR", "The resolved destination is not eligible for output.", resolved);
			return;
		}
		if (outstanding_attachment_ == resolved) {
			present("DEGRADED", requestedOutputMessage(configured_), resolved);
		} else {
			present("ERROR", "A previous projector may still be open on another display. Restart OBS.",
				resolved);
		}
		return;
	}

	requestOutput(resolved);
}

void OutputController::requestOutput(QScreen *screen)
{
	// Re-enumerate and re-resolve on the UI thread immediately before converting
	// the identity-bound choice into OBS's transient monitor index.
	const auto displays = connectedDisplays();
	const auto resolution = domain::resolve_display_identity(*configured_, metadataOnly(displays));
	if (resolution.outcome != domain::ResolutionOutcome::UniqueStrong &&
	    resolution.outcome != domain::ResolutionOutcome::UniqueWeak) {
		present("ERROR", "The destination changed before the projector could be opened.");
		return;
	}

	QScreen *resolved = displays[static_cast<std::size_t>(resolution.matched_display_index)].screen;
	if (resolved != screen) {
		present("ERROR", "The destination attachment changed before the projector could be opened.");
		return;
	}

	const int current_index = QGuiApplication::screens().indexOf(screen);
	if (current_index < 0) {
		present("MISSING", "The destination disappeared before the projector could be opened.");
		return;
	}

	gateway_.requestProgramProjector(current_index);
	outstanding_attachment_ = screen;
	present("DEGRADED", requestedOutputMessage(configured_), screen);
}

void OutputController::present(std::string status, std::string reason, QScreen *resolved)
{
	DockViewModel view_model;
	view_model.status = std::move(status);
	view_model.reason = std::move(reason);
	view_model.configured_destination = configuredLabel(configured_);
	view_model.resolved_destination = resolved ? displayLabel(qt::displayMetadataFor(resolved), resolved) : "None";
	view_model.program_scene = ready_ ? gateway_.currentProgramScene() : "Waiting for OBS";
	view_model.video_format = ready_ ? gateway_.currentVideoFormat() : "Waiting for OBS";
	view_model.audio_summary = ready_ ? gateway_.currentAudioSummary() : "Waiting for OBS";
	view_model.request_outstanding = view_model.status == "DEGRADED" && !outstanding_attachment_.isNull();
	view_model.controls_enabled = ready_ && !stopping_ && !outstanding_attachment_ &&
				      !gateway_.projectorPersistenceEnabled();
	dock_.setViewModel(view_model);
}

std::string OutputController::displayLabel(const domain::DisplayMetadata &metadata, QScreen *screen)
{
	std::ostringstream label;
	label << (metadata.name.empty() ? "Unnamed display" : metadata.name);
	if (screen) {
		const QSize pixels = screen->size() * screen->devicePixelRatio();
		label << " — " << pixels.width() << 'x' << pixels.height();
	}
	switch (domain::identity_strength(metadata)) {
	case domain::IdentityStrength::Strong:
		label << " (strong identity)";
		break;
	case domain::IdentityStrength::Weak:
		label << " (weak identity)";
		break;
	case domain::IdentityStrength::Insufficient:
		label << " (insufficient identity)";
		break;
	}
	return label.str();
}

} // namespace program_display
