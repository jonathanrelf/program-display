# Program Display roadmap

This roadmap is sequenced to prove a narrow operational slice before broadening
the routing model. Status values are `planned`, `in progress`, `blocked`, or
`done`.

## v0.1 — macOS preview (`release candidate`)

Goal: prove a native plugin can restore one deliberate OBS Studio Program
display destination without silently following a monitor index.

- [x] Verify current official plugin template and frontend projector path.
- [x] Define strict strong/weak display matching and visible output states.
- [x] Pass initial architecture review and reconcile findings.
- [x] Create template-based CMake/plugin skeleton and project-local Codex roles.
- [x] Add a native Program Display dock and frontend lifecycle integration.
- [x] Persist a selected destination outside scene collections.
- [x] Open `StudioProgram` through `obs_frontend_open_projector()` only after a
      safe resolution.
- [x] Reconcile display removal/addition without numeric-index fallback.
- [x] Supplement missing Qt identity fields with public macOS display metadata
      while keeping the transient CoreGraphics display ID non-authoritative.
- [x] Add domain unit tests for unique, missing, ambiguous and weak matches.
- [x] Add application-policy tests proving unique strong/weak restoration and
      duplicate-request suppression without an OBS or Qt dependency.
- [x] Build and run automated tests locally. Domain tests pass, and the macOS
      preset produces an ad-hoc-signed universal plugin against OBS 32.2.2.
- [x] Document a macOS/Elgato/OBS manual integration test.
- [x] Add optional pseudonymous macOS diagnostic probes for recording identity
      evidence across restart, cable hot-plug and changed display order.
      The observed unique Elgato serial-less profile restores automatically;
      identical replacement hardware remains an explicit limitation.
- [x] Restore a uniquely matching, explicitly saved weak display profile without
      monitor-index fallback or per-session reconfirmation.
- [x] Pass the second mandatory architecture review and correct material issues.
- [x] Remove the development working title from publishable source and rename
      the plugin, module, dock and settings to Program Display.
- [x] Restrict build presets and continuous integration to the supported macOS
      target.
- [x] Add release, contribution, security, issue-reporting and AI-assistance
      documentation.
- [x] Make operator terminology explicit about remembered displays, unverified
      projector requests and audio not being routed by the plugin.

Exit criteria: source and tests exist; the local build either passes or has one
reproducible, external prerequisite; the dock exposes configured/output health;
and limitations are stated without implying unavailable safety guarantees. This
closes the source milestone only. Supported operation additionally requires a
recorded successful build plus the physical-hardware integration procedure and
results.

## v0.2 — Output lifecycle and observability (`planned`)

- Broaden Qt/native identity validation beyond the initial Mac/Elgato target.
- Pursue upstream projector handles/query/close and stable monitor tokens.
- Add active-output verification when a supported API exists.
- Define deliberate black/fallback behaviour only after Program Display can own and
  observe projector lifecycle.

## v0.3 — Operator controls (`planned`)

- Expand the dock around Program Video, Program Audio, current scene, format,
  destination and actionable health.
- Add clear recovery actions and operator event history.
- Keep configuration secondary to at-a-glance confidence.

## v0.4 — Audio intent (`planned`)

- Map existing OBS program/monitor semantics into Program Display vocabulary.
- Inventory monitoring devices and per-source monitoring state.
- Prototype routes using public libobs APIs; do not add a parallel mixer.
- Specify general upstream primitives needed for true AUX buses, if any.

## v0.5 — Safe Output hardening (`planned`)

- Specify output-lock semantics against APIs that can actually enforce them.
- Handle format mismatch, sink capability changes and controlled fallback.
- Add fault injection and restart/hot-plug test matrices.
- Prove Windows/Linux providers before claiming support.

## Public release follow-up

- Complete and record the final renamed-plugin restart/reconnect checklist.
- Run the first GitHub-hosted macOS build and retain its result with the release.
- Add Developer ID signing and notarisation before presenting a downloadable
  package as a one-click installation for general users.
- Add a second platform only after its identity provider and physical validation
  matrix meet the same fail-closed standard as macOS.
