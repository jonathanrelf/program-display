# ADR-0004: Keep the v0.1 coordinator concrete

Status: accepted exception
Date: 2026-09-16

## Context

The initial design diagram placed display enumeration, settings, output gateway
and operator view behind interfaces. The implemented v0.1 keeps identity and
resolution pure, and confines OBS calls to `obs/`, but `OutputController`
directly enumerates Qt screens and uses concrete dock/store/gateway classes.

## Decision

Accept the concrete coordinator for the first source slice rather than perform a
late abstraction refactor that cannot be compiled against Qt on the current
host. Keep this exception visible in `ARCHITECTURE.md`.

## Consequences

Display resolution remains independently tested, but controller lifecycle and
request suppression currently require OBS/Qt integration testing. Extract
display-provider, output-gateway, settings-store and operator-view ports before
adding Windows/Linux providers or automated controller lifecycle tests. No OBS
API call may move out of `obs/` while this exception exists.
