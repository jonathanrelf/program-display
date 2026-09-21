# ADR-0006: Restore explicitly saved unique weak profiles

Status: accepted
Date: 2026-09-18

## Context

The Elgato 4K S exposes no usable serial number, so its saved destination is a
weak manufacturer/model/name/physical-size profile. Requiring a fresh
attachment-specific confirmation after every OBS restart or HDMI reconnect
preserved a conservative theoretical boundary, but did not fulfil Program Display's
purpose of restoring a deliberately selected output relationship.

The macOS diagnostic probe recorded stable descriptive and EDID-derived profile
evidence across the observed restart and reconnect path. OBS still accepts only
a transient monitor index, so that evidence cannot create an atomic or
unit-verified output handle.

## Decision

An explicitly saved weak destination, including an existing saved v0.1 weak
setting, authorizes automatic Program projector requests whenever the resolver
finds exactly one matching connected display. It does not require a migration
click or attachment-specific confirmation state.

All existing fail-closed controls remain: resolution considers the complete
connected set; missing, ambiguous, and insufficient identities open nothing;
the monitor index is derived again immediately before the frontend call; and an
outstanding request is deduplicated per current attachment.

## Consequences

Program Display can restore the observed Elgato setup after an OBS restart or HDMI
reconnect without an operator action. It cannot distinguish the original device
from an identical replacement unit with the same serial-less profile when that
replacement is the only matching display. The dock describes weak selections as
a one-time association rather than a recurring confirmation. Projector health
remains `DEGRADED — REQUESTED_UNVERIFIED` internally and `OUTPUT REQUESTED` to the
operator after the request.
