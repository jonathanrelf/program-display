# Program Display architecture

Status: current v0.1 architecture
Evidence baseline: OBS Studio `caaa0223401f2195128f9998265a0f66d84c9a02`, OBS
Plugin Template `3e7d7ac3b5342cd7d9b88890b9c70b472d1520fc`

## Purpose and boundary

Program Display adds output intent, persistence, lifecycle coordination, health and
operator controls to OBS. It does not replace libobs rendering, scenes,
Preview/Program, mixing, or normal output infrastructure.

For v0.1, Program Display means one configured relationship:

```text
OBS Studio Program -> one deliberately selected physical display
```

The first implementation uses the supported
`obs_frontend_open_projector("StudioProgram", monitorIndex, nullptr, nullptr)`
mechanism. It does not render Program independently.

## Component boundaries

```text
ProgramDisplayDock (operator view)
        |
OutputController (lifecycle/state coordination + Qt screen enumeration)
        |                 |                         |
Display metadata    DestinationStore          ObsGateway
  Qt + macOS               |                         |
     APIs             module JSON        public OBS Frontend/libobs APIs
```

- `domain/` contains display identity and destination resolution. It has no OBS
  or Qt dependency and is covered by unit tests. v0.1 output lifecycle state is
  held by the concrete coordinator (see ADR-0004).
- `obs/` adapts the OBS configuration path, current Program/video/audio
  information, and Program projector call.
- `plugin-main.cpp` owns the OBS module lifecycle and frontend/Qt signal wiring.
- `qt/` owns the dock widgets and the platform-neutral Qt metadata adapter.
- `platform/macos/` supplements empty Qt identity fields through public AppKit
  and CoreGraphics APIs. It does not make output or lifecycle decisions.
- In v0.1, `OutputController` directly enumerates `QScreen` and consumes concrete
  `ProgramDisplayDock`, `DestinationStore`, and `ObsGateway` collaborators. OBS calls
  remain confined to `obs/`, and all identity decisions remain in `domain/`.
  ADR-0004 records this narrow exception; a display-provider/view interface is
  required before controller lifecycle tests or another platform provider.

The plugin owns one controller and one dock. Creation and destruction happen on
the OBS UI thread. OBS frontend callbacks are unregistered before Qt objects are
destroyed.

## Display identity and matching

A persisted destination is descriptive, not an OBS monitor index. It contains a
schema version and the best metadata the display adapter exposes: manufacturer,
model, serial number, screen name, physical size and last observed pixel size.
Qt supplies the baseline. On macOS, public AppKit and CoreGraphics APIs
supplement manufacturer, model and non-zero serial fields that Qt leaves empty.
The transient index is diagnostic only and is never authoritative after the
enumeration in which it was produced.

The optional macOS display identity probe is disabled by default and is enabled
only with the `ENABLE_DISPLAY_IDENTITY_PROBE` build option. When enabled, it
logs diagnostic-only evidence when the plugin starts and whenever Qt reports a
display topology change. It records current Qt metadata, a hash of that
descriptive metadata, and public CoreGraphics display facts such as the
transient display ID, vendor and product. Serial values are represented only as
present/absent. It also emits unlinked public I/O Registry
`IOMobileFramebuffer` records, including a SHA-256 hash of an EDID-derived UUID
when exposed. These hashes are persistent pseudonymous diagnostic evidence, not
privacy-safe anonymisation. The records are separate because macOS provides no
supported one-to-one QScreen-to-IOKit association. The probe neither persists
any of these values nor gives them matching authority. It does not call the
deprecated `CGDisplayIOServicePort` API, does not treat an I/O Registry record
as a QScreen match, and never dumps raw EDID.

Identity strength is explicit, but always treated as evidence rather than proof:

- **strong**: a normalized, non-empty serial plus normalized manufacturer and
  model. All three are required, and the connected set must contain exactly one
  match;
- **weak**: normalized manufacturer/model/name/physical-size fingerprint where
  no serial is exposed;
- **insufficient**: metadata is too empty to distinguish a destination;
- **ambiguous**: more than one connected display matches the saved identity.

Normalization trims whitespace and compares text case-insensitively. Strong
matching requires exact normalized manufacturer, model and serial values; pixel
geometry and screen name may change without changing identity. A serial is not
assumed globally trustworthy: uniqueness is checked against the full current
display set every time.

Resolution rules are fail-closed:

1. A unique strong match may be opened or reacquired automatically.
2. No match is `MISSING`; Program Display never substitutes the primary display or the
   same numeric index.
3. Multiple matches are `ERROR`; the operator must select again.
4. A unique weak match for an explicitly saved destination may be opened or
   reacquired automatically. It remains serial-less evidence: an identical
   replacement device with the same manufacturer/model/name/physical-size
   profile cannot be distinguished when it is the sole match.
5. Insufficient metadata cannot be selected in v0.1. A future explicitly
   confirmed, non-persistent attachment mode may support it, but the first slice
   fails closed.

`QScreen::name()` is display text, not a unique key. On macOS, the adapter uses
Qt's public Cocoa native-screen interface to associate the current `QScreen`
with its current CoreGraphics display. The numeric `CGDirectDisplayID` is a
transient lookup handle and is never persisted. The adapter persists only the
descriptive vendor, product and non-zero serial values returned for that
display. A zero serial remains absent, so the identity remains weak. An
explicitly saved unique weak profile is restored automatically under the
full-set uniqueness rules; it never falls back to a primary display or monitor
index.

## Output state model

The controller tracks destination resolution and whether a projector request is
outstanding for the resolved attachment. Externally visible health is derived
from those facts:

- `ACTIVE`: reserved for a future API that can verify a live, identity-bound
  output. v0.1 never claims this state.
- `MISSING`: the configured identity has no connected match. No output is
  opened elsewhere.
- `DEGRADED`: a projector request was issued but cannot be observed. The dock
  uses the operator-facing label `OUTPUT REQUESTED` after calling the frontend API. It
  explains that Program should now be live, while stating that OBS cannot
  confirm the projector remains open. Internally the controller retains the
  `DEGRADED` state and supplies the dock with a human-readable reason.
- `ERROR`: settings are invalid, matching is ambiguous, OBS projector restore is
  enabled, or an integration step failed.

The dock also uses `UNCONFIGURED` before a destination is chosen. Each status is
paired with a human-readable reason; colour alone is never the contract.

Transitions are event driven:

```text
UNCONFIGURED --select--> DEGRADED
configured --no match---------> MISSING
DEGRADED/requested --matching screen removed--> MISSING
MISSING --strong unique add---> DEGRADED/requested-unverified
MISSING --weak unique add-----> DEGRADED/requested-unverified
any --ambiguous/invalid-------> ERROR
```

Once a request is issued, the controller records the `QScreen` attachment object
and will not issue another request for it in response to repeated frontend or
topology events. Only a
`screenRemoved` event for that exact object clears the outstanding request,
because OBS's current projector implementation closes a fullscreen projector on
that event. Loss of confidence for any other reason does not prove closure.

Changing destination is disabled whenever any projector request is outstanding,
including in `DEGRADED`, because the public API cannot close the old projector.
The dock explains that OBS must be restarted before reassignment. This is an
intentional safe limitation, not a simulated output lock.

Resolution and opening both run on the OBS UI thread. The current `QScreen` is
resolved and its current index is derived from `QGuiApplication::screens()`
immediately before `obs_frontend_open_projector()`. An index is never cached
across queued work. The public API still cannot make identity resolution and
projector creation atomic, so even this narrow handoff has a documented race if
Qt's screen list changes inside the synchronous frontend call.

## Startup, persistence and shutdown

Settings live in a versioned JSON file below `obs_module_config_path()` and are
written atomically through libobs safe JSON persistence. They are independent
of the current scene collection because output intent describes equipment.

The plugin creates the dock during module load in a not-ready state and waits for
`OBS_FRONTEND_EVENT_FINISHED_LOADING` before automatic restore. Frontend
callbacks schedule reconciliation onto the UI thread; a monotonically increasing
generation cancels stale scheduled work. It subscribes to Qt `screenAdded` and
`screenRemoved` only after the dock/controller exists. On
`OBS_FRONTEND_EVENT_EXIT` or module unload it marks the controller stopping,
increments the cancellation generation, disconnects Qt signals, unregisters
frontend callbacks, removes the dock, then destroys Qt objects. No queued task
may call the frontend after stopping begins.

OBS itself can optionally persist all open projectors in a scene collection,
but saves fullscreen destinations only as integer monitor indexes. If OBS's
`BasicWindow/SaveProjectors` setting is enabled, Program Display blocks all automatic
and manual opening, reports `ERROR`, and gives an actionable instruction to
disable that setting and restart OBS. Program Display does not silently change the
user's global OBS preference, cannot prevent an OBS-restored projector that was
opened before plugin readiness, and cannot close one through the public API.

## Dock contract

The compact dock prioritises state over setup. Its default view keeps only the
current status, actionable reason, destination picker and primary action
visible. A native disclosure control reveals diagnostics without permanently
consuming OBS workspace space. It shows:

- an outstanding, unobservable projector request as the calm informational
  label `OUTPUT REQUESTED`, rather than exposing the internal `DEGRADED` health term;

- Program Video status and reason;
- configured destination and current resolved display;
- current Program scene;
- OBS base/output video format;
- audio as explicitly informational, plus the current OBS monitor device for
  visibility only. The video projector does not route audio to that display;
- destination picker and a one-time remember/start action for a new
  serial-less profile.

Audio controls are deliberately read-only in v0.1.

## Audio boundary

v0.1 exposes no audio control. The Details row states that audio is not routed
by this plugin and reports the current OBS monitor device for context only. Any
future audio work must remain an intent layer over OBS rather than introducing a
parallel mixer. Missing bus or routing primitives should be proposed as general
libobs capabilities before product-specific workarounds.

## Safety claims and non-claims

v0.1 guarantees that its own resolver never deliberately falls back from a saved
identity to an unrelated display or a stale numeric index. The final index-based
frontend handoff is not atomic with display identity. It does not guarantee that:

- OBS rendered frames reached the sink;
- a user did not close the projector;
- another plugin or OBS opened a projector;
- the sink accepted the intended timing/colour format;
- black/fallback can be forced after the OBS projector has disappeared.

A unique weak profile identifies the sole connected display with the saved
serial-less descriptive fingerprint. It cannot prove that an identical
replacement unit is not connected instead.

Those guarantees need projector lifecycle/control and output telemetry not
present in the public frontend API. They are tracked in `docs/UPSTREAM.md`.

## Compatibility direction

The first supported target is current OBS Studio on Apple Silicon macOS. Qt and
OBS calls stay behind interfaces so Windows and Linux providers can be added
without changing the resolver. Platform support is a tested claim, not an
assumption inferred from compilation.
