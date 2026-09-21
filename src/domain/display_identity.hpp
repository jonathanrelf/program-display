// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <string>
#include <vector>

namespace program_display::domain {

// Metadata available for a physical display.  All text is intentionally kept
// in its source form here; comparisons are made on normalized values.
struct DisplayMetadata {
  std::string manufacturer;
  std::string model;
  std::string serial;
  std::string name;
  int physical_width_mm = 0;
  int physical_height_mm = 0;
  int pixel_width = 0;
  int pixel_height = 0;
};

enum class IdentityStrength {
  Strong,
  Weak,
  Insufficient,
};

enum class ResolutionOutcome {
  UniqueStrong,
  UniqueWeak,
  Missing,
  Ambiguous,
  InsufficientMetadata,
};

struct Resolution {
  ResolutionOutcome outcome;
  // Set only for unique matches. This is an index into the supplied connected
  // display list and is diagnostic/transient, never a persisted identity.
  int matched_display_index = -1;
};

// Trims ASCII whitespace and lowercases ASCII letters. It deliberately does
// not perform locale-sensitive or Unicode transformations.
std::string normalize_identity_text(const std::string &value);

// A strong identity needs manufacturer, model, and serial. A weak identity
// needs manufacturer, model, name, and both physical dimensions, and may only
// be used when the serial is absent.
IdentityStrength identity_strength(const DisplayMetadata &display);

// Resolves a persisted identity against the currently connected set. Strong
// comparison uses only manufacturer/model/serial. Weak comparison uses only
// manufacturer/model/name/physical size. Both require exactly one match.
Resolution resolve_display_identity(
    const DisplayMetadata &persisted,
    const std::vector<DisplayMetadata> &connected_displays);

} // namespace program_display::domain
