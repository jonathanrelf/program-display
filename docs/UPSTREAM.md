# Upstream OBS candidates

Evidence baseline: OBS Studio commit
[`caaa022`](https://github.com/obsproject/obs-studio/tree/caaa0223401f2195128f9998265a0f66d84c9a02)
and current public frontend documentation reviewed on 2026-09-16.

This file separates general OBS/plugin API gaps from Program Display product behavior.
No OBS fork or private-frontend dependency is planned.

## Identity-bound projector creation

**Limitation.** The public API accepts only an integer monitor index:
[`obs_frontend_open_projector`](https://github.com/obsproject/obs-studio/blob/caaa0223401f2195128f9998265a0f66d84c9a02/frontend/api/obs-frontend-api.h#L201).
OBS builds its own monitor menu from `QGuiApplication::screens()` and display
text, while fullscreen creation indexes that list in
[`OBSBasic_Projectors.cpp`](https://github.com/obsproject/obs-studio/blob/caaa0223401f2195128f9998265a0f66d84c9a02/frontend/widgets/OBSBasic_Projectors.cpp).
Identity resolution in a plugin therefore cannot be atomic with creation.

**General capability.** Enumerate destinations with a platform-backed stable
identity/token and open a projector against that token in one frontend-owned
operation.

**Smallest reasonable change.** Add a versioned screen descriptor/list API and
an `open_projector_ex` request that accepts an opaque descriptor token, validates
it on the UI thread, and returns an explicit result. Do not expose `QScreen *` in
the C ABI.

**Tests.** Reorder a fake screen list between enumeration and creation; reject a
missing token; reject ambiguous durable identities; prove no fallback to primary
or same index.

**Worth pursuing?** Yes, first as an issue/RFC because cross-platform durable
identity semantics need maintainer agreement before a pull request.

## Projector handle, query, close and lifecycle

**Limitation.** The implementation converts the request to a private
`SavedProjectorInfo` and opens it through the frontend, but the public call
returns no handle. Private `OBSProjector` objects close on screen removal, with
no public creation/destruction event.

**General capability.** Plugins need to distinguish creation failure, query the
current destination/type, close only the projector they created, and observe
destruction.

**Smallest reasonable change.** Return an opaque reference-counted projector
handle from an extended open call; add status/destination query, close, and
destroyed callback functions. Define ownership and UI-thread behavior in the
public header.

**Tests.** Invalid type/destination result; explicit close; Escape/manual close;
screen removal; frontend shutdown; double-close/idempotence; no callback after
subscriber removal.

**Worth pursuing?** Yes. An issue can be filed independently, but the handle and
identity-bound creation designs should be reviewed together.

## Safe projector persistence

**Limitation.** OBS serializes projector `monitor`, `type`, and geometry in
[`SaveProjectors()`](https://github.com/obsproject/obs-studio/blob/caaa0223401f2195128f9998265a0f66d84c9a02/frontend/widgets/OBSBasic_Projectors.cpp#L23-L63).
Fullscreen restore uses the saved integer index. Plugins cannot opt their own
projectors out or intercept restoration.

**General capability.** OBS should restore fullscreen projectors only to the
same intended display, and plugin-owned projectors should have explicit
persistence policy.

**Smallest reasonable change.** Extend saved projector data with the general
screen token above and a persistence/owner field. Preserve old index data for
migration, but require confirmation rather than unrelated-display fallback when
identity cannot resolve.

**Tests.** Missing display, changed ordering, duplicated monitors, legacy data,
plugin-owned nonpersistent projector, and restore after reconnect.

**Worth pursuing?** Yes, after the screen-token contract is agreed. This is a
frontend safety improvement independent of Program Display.

## Screen-topology notification

**Limitation.** There is no public frontend event for display topology changes.
A Qt-linked plugin can subscribe to `QGuiApplication`, but non-Qt consumers and
future frontend implementations cannot.

**General capability.** Notify plugins when the available projector destination
set changes.

**Smallest reasonable change.** Add a frontend event carrying no private Qt
objects; consumers re-enumerate the public destination list.

**Tests.** Add/remove/coalesce events during startup and shutdown; ensure no
event after callback removal.

**Worth pursuing?** Yes, as part of the destination enumeration proposal.

## Audio buses

Current libobs exposes one global monitoring device and per-source monitoring
enable state. That is enough for a clearer presentation of existing PROGRAM and
MONITOR behavior, but it is not a general AUX-bus model. Program Display will research
actual routing use cases before proposing an API; no concrete upstream change is
recommended yet.
