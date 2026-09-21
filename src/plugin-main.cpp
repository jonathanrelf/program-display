// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "app/output_controller.hpp"
#include "obs/obs_adapters.hpp"
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
#include "qt/display_metadata.hpp"
#endif
#include "plugin-support.h"
#include "qt/program_display_dock.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QGuiApplication>
#include <QMetaObject>
#include <QPointer>
#include <QScreen>

#include <cstdint>
#include <memory>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

namespace {

constexpr const char *kDockId = "program-display-dock";

class ProgramDisplayPlugin final {
public:
	bool initialize()
	{
		auto *dock = new program_display::ProgramDisplayDock();
		dock_ = dock;
		controller_ = std::make_unique<program_display::OutputController>(*dock, store_, gateway_);

		if (!obs_frontend_add_dock_by_id(kDockId, "Program Display", dock)) {
			controller_.reset();
			delete dock;
			dock_.clear();
			obs_log(LOG_ERROR, "could not register the Program Display dock");
			return false;
		}

		obs_frontend_add_event_callback(frontendEvent, this);
		auto *application = qGuiApp;
		screen_added_ =
			QObject::connect(application, &QGuiApplication::screenAdded, dock, [this](QScreen *screen) {
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
				logDisplayTopology("screen-added");
#endif
				if (controller_) {
					controller_->screenAdded(screen);
					queueOperationalRefresh();
				}
			});
		screen_removed_ =
			QObject::connect(application, &QGuiApplication::screenRemoved, dock, [this](QScreen *screen) {
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
				logDisplayTopology("screen-removed");
#endif
				if (controller_) {
					controller_->screenRemoved(screen);
					// OBS projectors also consume screenRemoved and close
					// themselves. Reconcile on the next UI turn so their
					// close handler runs before Program Display can reacquire.
					queueOperationalRefresh();
				}
			});
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
		logDisplayTopology("plugin-startup");
#endif
		return true;
	}

	void shutdown()
	{
		if (shutdown_) {
			return;
		}
		shutdown_ = true;
		++lifecycle_generation_;
		QObject::disconnect(screen_added_);
		QObject::disconnect(screen_removed_);

		if (controller_) {
			controller_->stop();
			controller_.reset();
		}

		// OBS_FRONTEND_EVENT_EXIT is explicitly the final point where frontend API
		// calls are permitted. Unregister and remove the dock there as well as on a
		// normal module unload so no callback or queued receiver outlives the DLL.
		obs_frontend_remove_event_callback(frontendEvent, this);
		if (dock_) {
			obs_frontend_remove_dock(kDockId);
		}
		dock_.clear();
	}

private:
#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
	void logDisplayTopology(const char *cause) const
	{
		int index = 0;
		for (QScreen *screen : QGuiApplication::screens()) {
			const std::string diagnostic = program_display::qt::displayDiagnosticFor(screen);
			obs_log(LOG_INFO, "display-probe cause=%s screen=%d %s", cause, index++, diagnostic.c_str());
		}
		for (const std::string &diagnostic : program_display::qt::displayRegistryDiagnostics()) {
			obs_log(LOG_INFO, "display-probe-registry cause=%s %s", cause, diagnostic.c_str());
		}
	}
#endif

	void queueOperationalRefresh()
	{
		if (!dock_ || shutdown_) {
			return;
		}
		const std::uint64_t generation = lifecycle_generation_;
		QMetaObject::invokeMethod(
			dock_,
			[this, generation]() {
				if (generation == lifecycle_generation_ && controller_) {
					controller_->refreshOperationalDetails();
				}
			},
			Qt::QueuedConnection);
	}

	static void frontendEvent(obs_frontend_event event, void *private_data)
	{
		auto *plugin = static_cast<ProgramDisplayPlugin *>(private_data);
		if (event == OBS_FRONTEND_EVENT_EXIT) {
			plugin->shutdown();
			return;
		}

		if (!plugin->dock_ || plugin->shutdown_) {
			return;
		}
		const std::uint64_t generation = plugin->lifecycle_generation_;
		QMetaObject::invokeMethod(
			plugin->dock_,
			[plugin, event, generation]() {
				if (generation != plugin->lifecycle_generation_ || !plugin->controller_) {
					return;
				}
				if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
					plugin->controller_->finishedLoading();
				} else if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED ||
					   event == OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED ||
					   event == OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED) {
					plugin->controller_->refreshOperationalDetails();
				}
			},
			Qt::QueuedConnection);
	}

	program_display::DestinationStore store_;
	program_display::ObsGateway gateway_;
	QPointer<program_display::ProgramDisplayDock> dock_;
	std::unique_ptr<program_display::OutputController> controller_;
	QMetaObject::Connection screen_added_;
	QMetaObject::Connection screen_removed_;
	std::uint64_t lifecycle_generation_ = 0;
	bool shutdown_ = false;
};

std::unique_ptr<ProgramDisplayPlugin> plugin;

} // namespace

bool obs_module_load(void)
{
	plugin = std::make_unique<ProgramDisplayPlugin>();
	if (!plugin->initialize()) {
		plugin.reset();
		return false;
	}
	obs_log(LOG_INFO, "loaded version %s", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	if (plugin) {
		plugin->shutdown();
		plugin.reset();
	}
	obs_log(LOG_INFO, "unloaded");
}
