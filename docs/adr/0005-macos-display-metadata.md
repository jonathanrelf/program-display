# ADR-0005: Supplement Qt display identity on macOS

Status: accepted
Date: 2026-09-18

## Context

On the initial Iiyama/Elgato hardware path, Qt enumerates both connected
displays and provides their names, sizes and geometry, but leaves manufacturer,
model and serial empty. CoreGraphics exposes vendor and product identifiers for
both displays and a non-zero serial for the Iiyama. Treating the displays as
disconnected hides usable identity evidence; weakening the resolver would risk
routing Program to the wrong display.

## Decision

Keep `QScreen` as the live attachment used for OBS monitor-index handoff. On
macOS, associate it with the current `NSScreen` through Qt's public Cocoa native
interface and use public CoreGraphics display metadata to supplement only empty
manufacturer, model and serial fields. Treat serial zero as absent. Never
persist `CGDirectDisplayID`; it is used only as a transient lookup handle.

The existing strong, weak, ambiguous and insufficient resolver rules remain
unchanged. Platform code stays behind the Qt metadata adapter and outside the
domain.

## Consequences

The tested Iiyama becomes a strong identity. The tested Elgato remains weak
because it reports serial zero. Under ADR-0006, its deliberately saved profile
may restore when it is the uniquely matching connected display; an identical
replacement device remains indistinguishable. Other adapters that expose
neither useful Qt nor CoreGraphics metadata still fail closed.
