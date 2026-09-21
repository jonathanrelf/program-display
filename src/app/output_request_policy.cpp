// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "output_request_policy.hpp"

namespace program_display {

bool shouldRequestOutput(domain::ResolutionOutcome outcome, bool request_outstanding)
{
	if (request_outstanding) {
		return false;
	}
	return outcome == domain::ResolutionOutcome::UniqueStrong ||
	       outcome == domain::ResolutionOutcome::UniqueWeak;
}

} // namespace program_display
