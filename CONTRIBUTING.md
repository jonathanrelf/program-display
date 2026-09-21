# Contributing to Program Display

Thank you for considering a contribution. Program Display is deliberately
narrow: OBS owns scenes, rendering, mixing and its projector implementation;
this plugin owns destination intent, descriptive identity, restoration policy,
operator health and a compact dock.

## Before opening a change

- Search existing issues and read `docs/ARCHITECTURE.md`.
- Check `docs/DECISIONS.md` for accepted architectural decisions.
- Discuss changes that broaden platform support, output ownership or persisted
  identity before implementing them.
- Keep unrelated refactors out of focused fixes.

## Supported development target

Version 0.1 supports macOS only. Windows and Linux contributions need more than
a successful build: they require a public-API identity provider, fail-closed
matching and recorded physical validation across restart, hot-plug, changed
display order, missing hardware and ambiguous same-model displays.

The plugin declares a macOS 13 deployment target, but the tested runtime is
macOS 27 on Apple Silicon. Plugin builds require Xcode 26.5 or newer and its
macOS 26.5 SDK; see `docs/DEVELOPMENT.md` for the distinct build-host,
deployment-target and runtime-validation statements.

## Build and test

Run the dependency-free tests for every identity or state-policy change:

```sh
cmake -S tests -B build-tests
cmake --build build-tests --parallel
ctest --test-dir build-tests --output-on-failure
```

For plugin or UI changes, also run:

```sh
cmake --preset macos
cmake --build --preset macos --parallel
```

Then complete the relevant manual checks in `docs/DEVELOPMENT.md`. Pull requests
should state exactly which automated and physical checks were performed.

## Design rules

- Use public libobs and OBS Frontend APIs only.
- Keep domain policy independent of OBS and Qt.
- Never restore from a saved numeric monitor index.
- Resolve the current display attachment immediately before requesting output.
- Missing, ambiguous or insufficient identity must open nowhere.
- A void projector request is unverified; do not present it as active output.
- Do not imply that the Program projector routes audio.
- Keep user-facing language concise and put diagnostics behind Details.

Use the repository formatting configuration and run `git diff --check` before
submitting a pull request.

## Bug reports

Useful reports include:

- OBS Studio and macOS versions
- Mac model and architecture
- display or capture-device model
- hub, adapter and cable path
- expected and observed behaviour
- exact restart or reconnect sequence
- a reviewed OBS log when relevant

Do not publish raw EDID data, hardware serials or complete unreviewed OBS logs.
Diagnostic hashes are pseudonymous identifiers, not anonymised data.

## AI-assisted contributions

Disclose material use of generative AI in the pull-request description. The
author remains responsible for understanding the change, verifying generated
claims, reviewing every submitted line and providing appropriate tests. An AI
tool's output is not evidence that hardware behaviour has been validated.

## Licence

By contributing, you agree that your contribution is licensed under
GPL-2.0-or-later, the licence used by this project.
