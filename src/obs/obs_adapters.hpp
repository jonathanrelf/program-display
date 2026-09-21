// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "domain/display_identity.hpp"

#include <optional>
#include <string>

namespace program_display {

class DestinationStore {
public:
  enum class LoadResult { NotConfigured, Loaded, Error };

  LoadResult load(domain::DisplayMetadata &destination);
  bool save(const domain::DisplayMetadata &destination);
  const std::string &lastError() const { return last_error_; }

private:
  std::string last_error_;
};

class ObsGateway {
public:
  bool projectorPersistenceEnabled() const;
  void requestProgramProjector(int monitor_index) const;

  std::string currentProgramScene() const;
  std::string currentVideoFormat() const;
  std::string currentAudioSummary() const;
};

} // namespace program_display
