# Decision index

`docs/ARCHITECTURE.md` is the current architectural truth. These ADRs preserve
why the important choices were made without restating the full design.

- [ADR-0001: Use the OBS Studio Program projector](adr/0001-obs-program-projector.md)
- [ADR-0002: Persist descriptive display identity and fail closed](adr/0002-display-identity.md)
- [ADR-0003: Treat projector requests as unverified](adr/0003-unverified-output-health.md)
- [ADR-0004: Keep the v0.1 coordinator concrete](adr/0004-v01-concrete-coordinator.md)
- [ADR-0005: Supplement Qt display identity on macOS](adr/0005-macos-display-metadata.md)
- [ADR-0006: Restore explicitly saved unique weak profiles](adr/0006-restore-unique-weak-profiles.md)

## Initial architecture gate

The first mandatory review was performed after the initial architecture and
roadmap drafts and before implementation. Accepted corrections were:

- separate destination resolution, attachment confirmation and request state;
- reserve `ACTIVE` for observable output and report v0.1 requests as unverified;
- suppress duplicate requests per attachment and block reassignment while a
  request may still exist;
- derive the monitor index only at the final UI-thread handoff;
- initially expire weak confirmation on topology changes (superseded for
  explicitly saved unique weak profiles by ADR-0006 after hardware validation);
- require manufacturer/model/serial for strong matching and test uniqueness;
- block Program Display opening when OBS projector persistence is enabled;
- queue reacquisition until OBS has processed display removal;
- state explicitly that the Program projector does not route audio;
- distinguish source completion from physical-hardware validation.

No material gate-1 recommendation was rejected. Projector lifecycle APIs,
atomic identity-bound opening, pixel/sink verification, safe black, broader
platform identity providers and AUX buses were accepted as real needs but
deferred beyond v0.1.

## Post-implementation architecture gate

The second mandatory review accepted the display-routing safety properties and
required four bounded corrections before the source milestone: make the current
component diagram truthful, clear a repaired settings-load error, show the
restart-before-reassignment instruction in the normal unverified state, and
display both base/output video dimensions. All four were accepted and applied.

The reviewer accepted concrete controller dependencies as a documented v0.1
exception. Interface extraction and automated controller lifecycle tests are
deferred until v0.2, before a second platform provider is added. The full Xcode
build now passes; operational approval remains blocked on the physical
Mac/Elgato test.
