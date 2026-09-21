// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "domain/display_identity.hpp"

namespace program_display {

bool shouldRequestOutput(domain::ResolutionOutcome outcome, bool request_outstanding);

} // namespace program_display
