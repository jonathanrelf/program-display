// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "obs_adapters.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <util/bmem.h>
#include <util/config-file.h>
#include <util/platform.h>

#include <filesystem>
#include <iomanip>
#include <sstream>

namespace program_display {
namespace {

constexpr long long kSettingsSchemaVersion = 1;

std::string settingsPath()
{
  char *path = obs_module_config_path("program-display.json");
  if (!path) {
    return {};
  }
  std::string result(path);
  bfree(path);
  return result;
}

} // namespace

DestinationStore::LoadResult DestinationStore::load(domain::DisplayMetadata &destination)
{
  last_error_.clear();
  const std::string path = settingsPath();
  if (path.empty()) {
    last_error_ = "OBS did not provide a module configuration path.";
    return LoadResult::Error;
  }
  if (!os_file_exists(path.c_str())) {
    return LoadResult::NotConfigured;
  }

  obs_data_t *data = obs_data_create_from_json_file_safe(path.c_str(), "bak");
  if (!data) {
    last_error_ = "Program Display settings could not be parsed.";
    return LoadResult::Error;
  }

  const long long schema = obs_data_get_int(data, "schema_version");
  if (schema != kSettingsSchemaVersion) {
    last_error_ = "Program Display settings use an unsupported schema version.";
    obs_data_release(data);
    return LoadResult::Error;
  }

  destination.manufacturer = obs_data_get_string(data, "manufacturer");
  destination.model = obs_data_get_string(data, "model");
  destination.serial = obs_data_get_string(data, "serial");
  destination.name = obs_data_get_string(data, "name");
  destination.physical_width_mm = static_cast<int>(obs_data_get_int(data, "physical_width_mm"));
  destination.physical_height_mm = static_cast<int>(obs_data_get_int(data, "physical_height_mm"));
  destination.pixel_width = static_cast<int>(obs_data_get_int(data, "pixel_width"));
  destination.pixel_height = static_cast<int>(obs_data_get_int(data, "pixel_height"));
  obs_data_release(data);

  if (domain::identity_strength(destination) == domain::IdentityStrength::Insufficient) {
    last_error_ = "Saved destination metadata is insufficient for safe restoration.";
    return LoadResult::Error;
  }
  return LoadResult::Loaded;
}

bool DestinationStore::save(const domain::DisplayMetadata &destination)
{
  last_error_.clear();
  const std::string path = settingsPath();
  if (path.empty()) {
    last_error_ = "OBS did not provide a module configuration path.";
    return false;
  }

  const std::filesystem::path parent = std::filesystem::path(path).parent_path();
  if (!parent.empty() && os_mkdirs(parent.string().c_str()) != MKDIR_SUCCESS &&
      !os_file_exists(parent.string().c_str())) {
    last_error_ = "Program Display could not create its configuration directory.";
    return false;
  }

  obs_data_t *data = obs_data_create();
  obs_data_set_int(data, "schema_version", kSettingsSchemaVersion);
  obs_data_set_string(data, "manufacturer", destination.manufacturer.c_str());
  obs_data_set_string(data, "model", destination.model.c_str());
  obs_data_set_string(data, "serial", destination.serial.c_str());
  obs_data_set_string(data, "name", destination.name.c_str());
  obs_data_set_int(data, "physical_width_mm", destination.physical_width_mm);
  obs_data_set_int(data, "physical_height_mm", destination.physical_height_mm);
  obs_data_set_int(data, "pixel_width", destination.pixel_width);
  obs_data_set_int(data, "pixel_height", destination.pixel_height);
  const bool saved = obs_data_save_json_safe(data, path.c_str(), "tmp", "bak");
  obs_data_release(data);

  if (!saved) {
    last_error_ = "Program Display could not save its destination settings.";
  }
  return saved;
}

bool ObsGateway::projectorPersistenceEnabled() const
{
  config_t *config = obs_frontend_get_user_config();
  return config && config_get_bool(config, "BasicWindow", "SaveProjectors");
}

void ObsGateway::requestProgramProjector(int monitor_index) const
{
  obs_frontend_open_projector("StudioProgram", monitor_index, nullptr, nullptr);
}

std::string ObsGateway::currentProgramScene() const
{
  obs_source_t *scene = obs_frontend_get_current_scene();
  if (!scene) {
    return "Unavailable";
  }
  const char *name = obs_source_get_name(scene);
  std::string result = name ? name : "Unnamed scene";
  obs_source_release(scene);
  return result;
}

std::string ObsGateway::currentVideoFormat() const
{
  obs_video_info info{};
  if (!obs_get_video_info(&info) || info.fps_den == 0) {
    return "Unavailable";
  }

  const double fps = static_cast<double>(info.fps_num) / static_cast<double>(info.fps_den);
  std::ostringstream stream;
  if (info.base_width == info.output_width && info.base_height == info.output_height) {
    stream << info.base_width << 'x' << info.base_height << " base/output";
  } else {
    stream << "base " << info.base_width << 'x' << info.base_height << " -> output "
           << info.output_width << 'x' << info.output_height;
  }
  stream << " @ " << std::fixed << std::setprecision(fps == static_cast<int>(fps) ? 0 : 2) << fps
         << " fps";
  return stream.str();
}

std::string ObsGateway::currentAudioSummary() const
{
  const char *name = nullptr;
  const char *id = nullptr;
  obs_get_audio_monitoring_device(&name, &id);
  if (name && *name) {
    return std::string("Not routed · Monitor: ") + name;
  }
  return "Not routed · Monitor unavailable";
}

} // namespace program_display
