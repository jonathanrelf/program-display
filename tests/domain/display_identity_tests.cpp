// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "display_identity.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using program_display::domain::DisplayMetadata;
using program_display::domain::IdentityStrength;
using program_display::domain::ResolutionOutcome;

namespace {

void expect(bool condition, const std::string &message)
{
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
  }
}

DisplayMetadata strong_display()
{
  return {.manufacturer = "Elgato", .model = "HD60 X", .serial = "SN-42",
          .name = "Stage Left", .physical_width_mm = 600, .physical_height_mm = 340};
}

DisplayMetadata weak_display()
{
  return {.manufacturer = "Elgato", .model = "HD60 X", .name = "Stage Left",
          .physical_width_mm = 600, .physical_height_mm = 340};
}

void test_normalization()
{
  expect(program_display::domain::normalize_identity_text(" \tElGaTo\r\n") == "elgato",
         "normalization trims ASCII whitespace and lowercases ASCII letters");
}

void test_unique_strong_ignores_name_and_geometry()
{
  auto persisted = strong_display();
  auto connected = strong_display();
  connected.name = "Different Qt Screen Name";
  connected.physical_width_mm = 1;
  connected.physical_height_mm = 1;
  connected.manufacturer = "  ELGATO ";
  connected.model = "hd60 x";
  connected.serial = " sn-42 ";

  const auto result = program_display::domain::resolve_display_identity(persisted, {connected});
  expect(result.outcome == ResolutionOutcome::UniqueStrong, "strong identity resolves uniquely");
  expect(result.matched_display_index == 0, "strong match returns current transient index");
}

void test_missing_strong()
{
  auto persisted = strong_display();
  auto other = strong_display();
  other.serial = "SN-other";
  expect(program_display::domain::resolve_display_identity(persisted, {other}).outcome ==
             ResolutionOutcome::Missing,
         "no matching strong identity is missing");
}

void test_ambiguous_strong()
{
  const auto saved = strong_display();
  auto duplicate = saved;
  duplicate.name = "Other connector";
  expect(program_display::domain::resolve_display_identity(saved, {saved, duplicate}).outcome ==
             ResolutionOutcome::Ambiguous,
         "duplicate serial/manufacturer/model is ambiguous");
}

void test_unique_weak()
{
  const auto saved = weak_display();
  auto matched = saved;
  matched.manufacturer = " elgato ";
  matched.model = "HD60 x";
  expect(program_display::domain::identity_strength(saved) == IdentityStrength::Weak,
         "complete serial-free fingerprint is weak");
  expect(program_display::domain::resolve_display_identity(saved, {matched}).outcome ==
             ResolutionOutcome::UniqueWeak,
         "unique weak fingerprint is identified as weak");
}

void test_whitespace_serial_is_weak()
{
  auto saved = weak_display();
  saved.serial = " \t\r\n ";
  expect(program_display::domain::identity_strength(saved) == IdentityStrength::Weak,
         "whitespace-only serial does not become strong");
}

void test_weak_identity_does_not_match_new_serial()
{
  const auto saved = weak_display();
  auto connected = saved;
  connected.serial = "newly-visible-serial";
  expect(program_display::domain::resolve_display_identity(saved, {connected}).outcome ==
             ResolutionOutcome::Missing,
         "weak identity does not authorize a display whose serial evidence changed");
}

void test_empty_connected_set_is_missing()
{
  expect(program_display::domain::resolve_display_identity(strong_display(), {}).outcome ==
             ResolutionOutcome::Missing,
         "empty connected display set is missing");
}

void test_duplicate_after_nonmatches_is_ambiguous()
{
  const auto saved = strong_display();
  auto other = saved;
  other.serial = "other";
  expect(program_display::domain::resolve_display_identity(saved, {other, saved, other, saved}).outcome ==
             ResolutionOutcome::Ambiguous,
         "duplicates remain ambiguous when separated by nonmatches");
}

void test_ambiguous_weak()
{
  const auto saved = weak_display();
  expect(program_display::domain::resolve_display_identity(saved, {saved, saved}).outcome ==
             ResolutionOutcome::Ambiguous,
         "duplicate weak fingerprints are ambiguous");
}

void test_insufficient_metadata()
{
  auto incomplete = weak_display();
  incomplete.physical_width_mm = 0;
  expect(program_display::domain::identity_strength(incomplete) == IdentityStrength::Insufficient,
         "weak identity needs physical size");
  expect(program_display::domain::resolve_display_identity(incomplete, {weak_display()}).outcome ==
             ResolutionOutcome::InsufficientMetadata,
         "insufficient identity never resolves");
}

} // namespace

int main()
{
  test_normalization();
  test_unique_strong_ignores_name_and_geometry();
  test_missing_strong();
  test_ambiguous_strong();
  test_unique_weak();
  test_whitespace_serial_is_weak();
  test_weak_identity_does_not_match_new_serial();
  test_empty_connected_set_is_missing();
  test_duplicate_after_nonmatches_is_ambiguous();
  test_ambiguous_weak();
  test_insufficient_metadata();
  std::cout << "display identity tests passed\n";
}
