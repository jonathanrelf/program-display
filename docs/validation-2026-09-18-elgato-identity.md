# Elgato 4K S identity validation — 2026-09-18

## Setup

- OBS Studio 32.2.2 on macOS, with Program Display's optional identity probe enabled.
- Elgato 4K S connected as an external macOS display through the observed
  DisplayPort-to-HDMI path.

## Observations

- An OBS restart preserved the Elgato descriptive and EDID-derived profile.
- HDMI removal at 15:14:30 and addition at 15:14:35 preserved the descriptive
  identity hash prefix `459e…` and EDID-derived profile hash prefix `c500…`.
- The advisory registry record remained manufacturer `EGA`, product `175`, week
  `18`, year `2025`, physical size `60x34 cm`, transport `DP-to-HDMI`, and
  serial absent.

## Release-candidate confirmation — 2026-09-21

After the public-name migration, restarting OBS required the renamed **Program
Display** dock to be opened once because the previous dock layout referred to
the old dock identity. The saved Elgato association itself migrated correctly:
Program output was requested automatically, the destination controls were
locked without another weak-identity confirmation, and the Elgato 4K S was
visually confirmed to receive OBS Program output.

Changing Preview in Studio Mode left the Elgato feed unchanged, confirming that
the requested projector was Program rather than Preview. Disconnecting the
Elgato produced `DISPLAY MISSING`; reconnecting it returned directly to
`OUTPUT REQUESTED`. Quitting OBS while the Elgato was missing, reconnecting it,
and then launching OBS also restored Program output automatically. No fallback
projector was opened on the remaining display. The missing state did reveal a
presentation problem—the remaining display appeared selected in the picker—so
the release candidate now keeps a disabled saved-destination placeholder selected
until the Elgato returns or the operator deliberately chooses a replacement.

The corrected missing state was then verified with the Elgato disconnected: the
picker retained **Elgato 4K S — Not connected** rather than implying that the
remaining display had replaced it. At the normal dock height, expanding
**Details** allocated additional space and exposed a bounded vertical scrollbar;
all configured, resolved, scene, video and audio diagnostics were reachable
without forcing the surrounding OBS dock layout to grow further.

Enabling OBS **Save projectors on exit** and restarting produced the expected
`ERROR`; Program Display did not issue another projector request and explained
that the setting must be disabled before use. Opening Program in a new window or
on the other connected display left the Elgato projector uninterrupted, which is
consistent with OBS allowing multiple independent projector windows rather than
treating a new projector as a reassignment of the existing one.

Finally, manually closing the Elgato fullscreen projector left Program Display
at `OUTPUT REQUESTED`, as required: the public frontend API exposes no projector
handle or destruction event to the plugin. Restarting OBS resolved the saved
Elgato identity again and restored Program output. This completed the v0.1
hardware acceptance pass for the initial MacBook-to-Elgato configuration.

## Interpretation and limits

The observed evidence supports automatic restoration for this deliberately
saved, uniquely matched serial-less profile. The hashes are persistent
pseudonymous diagnostic evidence, not anonymisation. CoreGraphics display IDs
and I/O Registry entry IDs are transient. No raw serial or raw EDID UUID is
recorded here. An identical replacement device with the same serial-less
profile cannot be distinguished if it is the sole matching connected display.
