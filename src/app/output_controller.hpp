// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "domain/display_identity.hpp"
#include "obs/obs_adapters.hpp"
#include "qt/program_display_dock.hpp"

#include <QPointer>

#include <optional>
#include <string>
#include <vector>

class QScreen;

namespace program_display {

class OutputController final {
public:
	struct ConnectedDisplay {
		QPointer<QScreen> screen;
		domain::DisplayMetadata metadata;
		std::string label;
	};

	OutputController(ProgramDisplayDock &dock, DestinationStore &store, ObsGateway &gateway);

	void finishedLoading();
	void refreshOperationalDetails();
	void screenAdded(QScreen *screen);
	void screenRemoved(QScreen *screen);
	void selectAndStart(QScreen *screen);
	void stop();

private:
	std::vector<ConnectedDisplay> connectedDisplays() const;
	void refreshDestinations(const std::vector<ConnectedDisplay> &displays);
	void reconcile();
	void requestOutput(QScreen *screen);
	void present(std::string status, std::string reason, QScreen *resolved = nullptr);
	static std::string displayLabel(const domain::DisplayMetadata &metadata, QScreen *screen);

	ProgramDisplayDock &dock_;
	DestinationStore &store_;
	ObsGateway &gateway_;
	std::optional<domain::DisplayMetadata> configured_;
	QPointer<QScreen> outstanding_attachment_;
	bool ready_ = false;
	bool stopping_ = false;
	std::string startup_error_;
};

} // namespace program_display
