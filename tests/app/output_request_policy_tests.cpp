// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "app/output_request_policy.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void expect(bool condition, const char *message)
{
	if (!condition) {
		std::cerr << "FAILED: " << message << '\n';
		std::exit(1);
	}
}

} // namespace

int main()
{
	using program_display::domain::ResolutionOutcome;
	using program_display::shouldRequestOutput;

	expect(shouldRequestOutput(ResolutionOutcome::UniqueStrong, false),
	       "a unique strong identity should request output");
	expect(shouldRequestOutput(ResolutionOutcome::UniqueWeak, false),
	       "an explicitly saved unique weak identity should request output");
	expect(!shouldRequestOutput(ResolutionOutcome::UniqueWeak, true),
	       "an outstanding weak-identity request must not be duplicated");
	expect(!shouldRequestOutput(ResolutionOutcome::UniqueStrong, true),
	       "an outstanding strong-identity request must not be duplicated");
	expect(!shouldRequestOutput(ResolutionOutcome::Missing, false),
	       "a missing identity must not request output");
	expect(!shouldRequestOutput(ResolutionOutcome::Ambiguous, false),
	       "an ambiguous identity must not request output");
	expect(!shouldRequestOutput(ResolutionOutcome::InsufficientMetadata, false),
	       "insufficient identity metadata must not request output");

	std::cout << "output request policy tests passed\n";
	return 0;
}
