# ADR-0002: Persist descriptive display identity and fail closed

Status: superseded in part by ADR-0006
Date: 2026-09-16

## Context

OBS's projector call accepts a transient monitor index. Screen ordering changes
across disconnects and restarts, and Qt states that a screen name is not unique.

## Decision

Persist the observed manufacturer/model/serial values when all are available and
normalize them for comparison. Otherwise use a weaker
manufacturer/model/name/physical-size fingerprint. Every match must be unique.
Missing, ambiguous, or insufficient identities never fall back to another
display. ADR-0006 supersedes the original attachment-specific confirmation rule
for an explicitly saved unique weak profile.

The current index is derived on the UI thread immediately before the projector
call and is never persisted as authority.

## Consequences

Some adapters will not qualify for automatic restore. Strong identity is still
conditional evidence and must be verified on the initial Mac/Elgato hardware.
The final identity-to-index handoff cannot be atomic without an OBS API change.
