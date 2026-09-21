// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "domain/display_identity.hpp"

#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
#include <string>
#include <vector>
#endif

class QScreen;

namespace program_display::qt {

// Collects the descriptive identity exposed by Qt and, where supported,
// supplements missing fields with public platform display metadata.
domain::DisplayMetadata displayMetadataFor(QScreen *screen);

#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
// A log-ready description of current display evidence. It is diagnostic only
// and must not be stored or used for identity matching.
std::string displayDiagnosticFor(QScreen *screen);

std::vector<std::string> displayRegistryDiagnostics();
#endif

} // namespace program_display::qt
