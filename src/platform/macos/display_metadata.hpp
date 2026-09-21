// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "domain/display_identity.hpp"

#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
#include <string>
#include <vector>
#endif

class QScreen;

namespace program_display::platform::macos {

// Supplements fields that Qt's Cocoa integration leaves empty. The transient
// CoreGraphics display ID used for the lookup is deliberately not persisted.
void supplementDisplayMetadata(QScreen *screen, domain::DisplayMetadata &metadata);

#if defined(PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE)
// Returns diagnostic-only evidence for the current QScreen attachment. None of
// these values are persisted or used by the resolver.
std::string displayDiagnosticFor(QScreen *screen);

// I/O Registry display records are intentionally returned separately because
// macOS exposes no supported one-to-one association with QScreen.
std::vector<std::string> displayRegistryDiagnostics();
#endif

} // namespace program_display::platform::macos
