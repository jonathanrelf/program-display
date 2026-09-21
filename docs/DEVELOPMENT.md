# Development

## Development and compatibility targets

- Runtime-tested configuration: Apple Silicon on macOS 27
- Runtime deployment target: macOS 13 or later; macOS 13 through 26 and Intel
  runtime behaviour are not yet physically tested
- OBS Studio 32.2.2
- full Xcode 26.5 or newer, providing the macOS 26.5 SDK or newer
- CMake 3.28 or later

Apple's
[Xcode system requirements](https://developer.apple.com/xcode/system-requirements/)
list macOS 26.2 or newer as the host requirement for Xcode 26.5. The source-first
v0.1 release therefore needs a newer build host even though the generated plugin
declares macOS 13 as its deployment target. A deployment target is not evidence
that the plugin has been exercised on that OS version.

The repository follows the official
[`obsproject/obs-plugintemplate`](https://github.com/obsproject/obs-plugintemplate)
layout, pinned during project creation at `3e7d7ac`. `buildspec.json` pins OBS,
Qt and OBS dependency archives with SHA-256 hashes.

The v0.1 build intentionally exposes only a macOS preset. Windows and Linux need
platform-specific identity implementations and physical validation before build
presets or release artifacts are added.

## Domain tests

These require only a C++20 compiler and CMake:

```sh
cmake -S tests -B build-tests
cmake --build build-tests --parallel
ctest --test-dir build-tests --output-on-failure
```

## Plugin build and install

```sh
cmake --preset macos
cmake --build --preset macos --parallel
cmake --install build_macos --config RelWithDebInfo
```

The template defaults installation to the current user's OBS plugin directory.
Launch OBS from Terminal while developing so plugin log messages are visible.

## macOS display identity probe

The probe is off by default. Configure a local diagnostic build with:

```sh
cmake --preset macos -DENABLE_DISPLAY_IDENTITY_PROBE=ON
```

Program Display then writes one `display-probe` line per screen at plugin
startup and after each display add/remove notification. These lines are
diagnostic only:
they do not alter saved destination matching or automatic restore.

For the Elgato investigation, capture the `display-probe` lines for the same
physical setup after: an OBS restart, a Mac restart, an HDMI reconnect, and a
different hub/adapter port. Compare `identity-sha256`, CoreGraphics vendor and
product, and the presence of a serial. `identity-sha256` and the EDID-derived
UUID hash are persistent pseudonymous evidence, not anonymised values; treat
logs as sensitive. `cg-display-id` is transient corroborating evidence, not a
persistent ID. Do not share complete OBS logs publicly without reviewing them
first.

The probe also writes separate `display-probe-registry` records for public
`IOMobileFramebuffer` I/O Registry services. When macOS exposes an
EDID-derived UUID, only its SHA-256 hash is logged alongside advisory transport,
product, manufacture, physical-size and native-format fields. Compare those
records, but do not infer a QScreen association from them: macOS offers no
supported one-to-one QScreen-to-IOKit mapping. The historical API that maps a
CoreGraphics display to an IOKit service is deprecated as unsupported, so
Program Display will not use it to attach registry data to a destination.
Registry fields are implementation-specific diagnostic evidence, not a supported
identity contract. Raw EDID and serial values are never logged. Before retaining
or sharing a log, verify that every `display-probe` and
`display-probe-registry` line has `serial-present=` rather than a serial value
and only `*-sha256=` hashes rather than raw EDID or EDID UUID text.

The macOS preset and universal `RelWithDebInfo` plugin build have been verified
against OBS 32.2.2 and its pinned Qt 6.11.1 dependency runtime using local Xcode
27 and GitHub-hosted Xcode 26.6 builds. CMake requires that exact Qt version.
The generated module contains both `arm64` and `x86_64` slices, declares macOS
13 as its deployment target and is ad-hoc signed for local development. Full
Xcode must be installed and selected:

```sh
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
xcodebuild -version
```

The post-build rule removes extended attributes before signing. This is needed
when the checkout is in a macOS File Provider-managed directory such as a
synced Documents folder. File Provider may reattach metadata to the build-tree
directory after signing; the installed copy outside that directory retains a
valid signature.

## Manual v0.1 integration test

Record OBS version, macOS version, Mac model, adapter/cable path, capture-device
model/firmware, and each result. A source-complete v0.1 is not operationally
validated until this procedure passes on the initial MacBook-to-Elgato target.

1. In OBS Settings, disable **Save projectors on exit**, then restart OBS.
2. Attach the Elgato/capture destination as a macOS display. Open the Program
   Display dock and record its identity strength and the saved
   `program-display.json` metadata.
3. Choose the Elgato destination and start. Confirm the dock says
   `OUTPUT REQUESTED`, says Program should now be visible, and clearly states that OBS
   cannot confirm the projector remains open. Visually confirm the sink shows
   OBS Studio Program, not Preview.
4. Change Preview while Studio Mode is enabled. Confirm the sink remains Program
   until an OBS transition changes Program.
5. Disconnect the destination. Confirm the projector closes and Program Display
   reports `DISPLAY MISSING`; confirm no projector appears on the built-in display.
6. Reconnect it. A unique strong or explicitly saved weak identity should be
   requested automatically. Confirm no duplicate projector appears.
7. Repeat disconnect/reconnect with display ordering changed and with another
   same-model display if available. Ambiguity must report `ERROR` and open
   nowhere.
8. Restart OBS with the destination attached, then absent. Automatic restore is
   accepted when the saved strong or weak profile remains uniquely matched
   across the restart. Absent hardware must remain `MISSING`. A serial-less
   weak profile cannot distinguish an identical replacement device.
9. With a request outstanding, confirm destination controls are disabled. To
   close the projector manually, move the pointer onto the destination display,
   click the fullscreen projector to focus it and press **Escape**; alternatively,
   right-click inside the projector and choose **Close**. Opening Program in a
   new window or on another display creates an additional projector and is not a
   close test. After closing the original, Program Display must remain unverified
   rather than claiming to detect closure.
10. Enable OBS **Save projectors on exit** and restart. Confirm Program Display
    blocks opening and explains the conflict. OBS may already have restored an
    older index-based projector; record that separately because Program Display
    cannot close it.
11. Confirm the Audio row says the plugin does not route audio and does not imply
    audio is sent through the selected video display.

## Code boundaries and checks

- Domain changes: run the standalone tests.
- OBS/Qt changes: build the plugin and exercise the lifecycle checklist above.
- Never include private OBS frontend headers.
- Run `git diff --check` before review. Use the repository `.clang-format` when
  `clang-format` is installed.
