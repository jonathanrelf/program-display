# ADR-0003: Treat projector requests as unverified

Status: accepted
Date: 2026-09-16

## Context

`obs_frontend_open_projector()` is synchronous but returns `void`; public APIs do
not expose projector creation, lifetime, destination, or frame-delivery state.

## Decision

After a request, v0.1 reports `DEGRADED` with reason
`REQUESTED_UNVERIFIED`. `ACTIVE` is reserved for a future observable contract.
Only removal of the exact `QScreen` attachment clears the outstanding request;
manual closure cannot be detected. Reassignment remains disabled while a
request may exist. The dock translates that diagnostic state into the
operator-facing label `OUTPUT REQUESTED` and plain language: Program should now be
live, but OBS cannot confirm that the projector remains open.

## Consequences

The UI is deliberately conservative. Operational confidence can improve only
with general upstream lifecycle and output-observability APIs, not inference or
private-widget inspection.
