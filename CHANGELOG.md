# Changelog

All notable changes to Program Display are documented here. The project follows
[Semantic Versioning](https://semver.org/) once a public release is tagged.

## [0.1.0] - 2026-09-21

### Added

- Compact native dock for choosing one OBS Program display destination.
- Descriptive display identity persistence with fail-closed resolution.
- Automatic restoration after OBS restart and display reconnect.
- Explicit handling for unique serial-less display profiles.
- Missing, ambiguous, insufficient-identity and unverified-request states.
- Optional privacy-conscious macOS display identity diagnostics.
- Dependency-free identity and output-request policy tests, plus a documented
  hardware validation checklist.
- An original Program Display project mark and an OBS Community resource draft
  with explicit development and artwork provenance.
- A wider floating size, responsive stacked controls for narrow dock layouts,
  and concise status copy that remains readable at the minimum size.
- A saved-destination placeholder that prevents a connected display from
  appearing to replace a missing destination implicitly.
- A bounded, scrollable Details panel with more compact audio diagnostics.

### Compatibility and validation

- OBS Studio 32.2.2 with a macOS 13 deployment target.
- Runtime behaviour physically validated only on macOS 27 and Apple Silicon.
- Universal macOS build (`arm64` and `x86_64`); Intel and macOS 13 through 26
  remain untested runtime targets.
- Source builds require Xcode 26.5 or newer and therefore a macOS version that
  can run that toolchain; Xcode 26.5 requires macOS 26.2 or newer.

### Known limitations

- The public OBS API cannot verify or close a requested projector.
- Audio is not routed by the plugin.
- Windows and Linux are not supported in this release.
- An identical replacement for a serial-less display cannot be distinguished
  when it is the sole matching device.
