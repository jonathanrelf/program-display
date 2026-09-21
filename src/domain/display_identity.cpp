// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "display_identity.hpp"

#include <cctype>

namespace program_display::domain {
namespace {

bool is_ascii_whitespace(unsigned char character)
{
  return character == ' ' || character == '\t' || character == '\n' ||
         character == '\r' || character == '\f' || character == '\v';
}

bool has_text(const std::string &value)
{
  return !normalize_identity_text(value).empty();
}

bool has_physical_size(const DisplayMetadata &display)
{
  return display.physical_width_mm > 0 && display.physical_height_mm > 0;
}

bool strong_match(const DisplayMetadata &left, const DisplayMetadata &right)
{
  return normalize_identity_text(left.manufacturer) ==
             normalize_identity_text(right.manufacturer) &&
         normalize_identity_text(left.model) == normalize_identity_text(right.model) &&
         normalize_identity_text(left.serial) == normalize_identity_text(right.serial);
}

bool weak_match(const DisplayMetadata &left, const DisplayMetadata &right)
{
  return !has_text(right.serial) &&
         normalize_identity_text(left.manufacturer) ==
             normalize_identity_text(right.manufacturer) &&
         normalize_identity_text(left.model) == normalize_identity_text(right.model) &&
         normalize_identity_text(left.name) == normalize_identity_text(right.name) &&
         left.physical_width_mm == right.physical_width_mm &&
         left.physical_height_mm == right.physical_height_mm;
}

} // namespace

std::string normalize_identity_text(const std::string &value)
{
  std::size_t first = 0;
  while (first < value.size() && is_ascii_whitespace(static_cast<unsigned char>(value[first]))) {
    ++first;
  }

  std::size_t last = value.size();
  while (last > first && is_ascii_whitespace(static_cast<unsigned char>(value[last - 1]))) {
    --last;
  }

  std::string normalized;
  normalized.reserve(last - first);
  for (std::size_t index = first; index < last; ++index) {
    const unsigned char character = static_cast<unsigned char>(value[index]);
    normalized.push_back(character >= 'A' && character <= 'Z'
                             ? static_cast<char>(character - 'A' + 'a')
                             : static_cast<char>(character));
  }
  return normalized;
}

IdentityStrength identity_strength(const DisplayMetadata &display)
{
  if (has_text(display.manufacturer) && has_text(display.model) && has_text(display.serial)) {
    return IdentityStrength::Strong;
  }

  if (!has_text(display.serial) && has_text(display.manufacturer) && has_text(display.model) &&
      has_text(display.name) && has_physical_size(display)) {
    return IdentityStrength::Weak;
  }

  return IdentityStrength::Insufficient;
}

Resolution resolve_display_identity(const DisplayMetadata &persisted,
                                    const std::vector<DisplayMetadata> &connected_displays)
{
  const IdentityStrength strength = identity_strength(persisted);
  if (strength == IdentityStrength::Insufficient) {
    return {ResolutionOutcome::InsufficientMetadata};
  }

  int matched_index = -1;
  for (std::size_t index = 0; index < connected_displays.size(); ++index) {
    const bool matches = strength == IdentityStrength::Strong
                             ? strong_match(persisted, connected_displays[index])
                             : weak_match(persisted, connected_displays[index]);
    if (!matches) {
      continue;
    }
    if (matched_index != -1) {
      return {ResolutionOutcome::Ambiguous};
    }
    matched_index = static_cast<int>(index);
  }

  if (matched_index == -1) {
    return {ResolutionOutcome::Missing};
  }
  return {strength == IdentityStrength::Strong ? ResolutionOutcome::UniqueStrong
                                                : ResolutionOutcome::UniqueWeak,
          matched_index};
}

} // namespace program_display::domain
