# OBS Community resource submission copy

This document maps Program Display 0.1.0 to the OBS Forums resource fields. It
is ready for maintainer review, adaptation into the maintainer's own voice and
submission. Re-check every claim against the release and retain the complete
AI-assistance disclosure: the OBS Forum Resource and IP Policy requires that
disclosure and warns that substantially AI-produced resources may be rejected.

## Resource fields

**Category:** OBS Studio Plugins

**Name:** Program Display

**Tagline:** Remember and safely restore a fullscreen Program destination on macOS.

**Version:** 0.1.0

**Supported platform:** macOS

**Resource type:** External download

**Download URL:**
<https://github.com/jonathanrelf/program-display/releases/tag/v0.1.0>

**Source and documentation URL:**
<https://github.com/jonathanrelf/program-display>

**Support URL:**
<https://github.com/jonathanrelf/program-display/issues>

**Licence:** GPL-2.0-or-later

**Price:** Free

**Resource icon:** `assets/program-display-logo.png`

**Screenshot:** `assets/program-display-dock.png`

## Description

Paste the following into the resource description field after reviewing and
adapting it into the maintainer's own voice. It uses XenForo BBCode.

```bbcode
Program Display is an independent, open-source macOS plugin for people who regularly send the OBS Studio Program feed to a dedicated external display or capture device.

Display numbering can change after a Mac, dock, adapter or capture device is reconnected. Restoring a fullscreen projector by a saved monitor number can therefore target the wrong screen. Program Display remembers descriptive hardware identity instead. When OBS starts—or the selected display returns—it resolves that identity against the displays connected now and requests the fullscreen Program projector only when the match is safe and unique.

If the destination is missing, ambiguous or cannot be identified confidently, Program Display fails closed instead of falling back to another display.

[B]What version 0.1 includes[/B]

[LIST]
[*]A compact native dock for selecting and inspecting the Program destination.
[*]Automatic restoration after OBS restart or display reconnection.
[*]Strong and explicitly confirmed weak display-identity matching on macOS.
[*]Clear Output requested, Display missing and Error states.
[*]A universal arm64 and x86_64 build configuration.
[*]No private OBS frontend headers or implementation types.
[/LIST]

[B]Release status and compatibility[/B]

Version 0.1.0 is a source-first preview. Runtime behaviour has been tested only with OBS Studio 32.2.2 on macOS 27, Apple Silicon and an Elgato 4K S display/capture path.

The generated plugin declares a macOS 13 deployment target and contains arm64 and x86_64 slices. Those are build properties, not tested-support claims: macOS 13 through 26 and Intel runtime behaviour have not yet been physically tested.

There is no signed and notarised one-click package in this release. Building from source requires Xcode 26.5 or newer, providing the macOS 26.5 SDK or newer, and CMake 3.28 or newer. Apple's [URL='https://developer.apple.com/xcode/system-requirements/']Xcode system requirements[/URL] list macOS 26.2 or newer as the host requirement for Xcode 26.5, so the current source-first release does not provide a practical installation path for older macOS versions.

[B]Important OBS API limitation[/B]

The public OBS frontend API can request a projector, but it does not return a handle that a plugin can query or close. Program Display therefore reports [B]Output requested[/B] rather than claiming the projector is independently verified. Changing the destination after a request currently requires restarting OBS.

Program Display does not route audio. Configure OBS monitoring or operating-system audio routing separately.

[B]Before using it[/B]

OBS [B]Save projectors on exit[/B] must be disabled before Program Display is used. OBS persists projectors by numeric monitor position, while Program Display resolves a remembered physical destination. Allowing both mechanisms to restore output could create duplicate projectors or target the wrong display.

[B]Links[/B]

[URL='https://github.com/jonathanrelf/program-display/releases/tag/v0.1.0']Version 0.1.0 release and source download[/URL]
[URL='https://github.com/jonathanrelf/program-display']Documentation and source code[/URL]
[URL='https://github.com/jonathanrelf/program-display/issues']Bug reports and support[/URL]

Useful bug reports include the OBS and macOS versions, Mac model, display or capture-device model, complete hub/adapter/cable path, and the exact restart or reconnect sequence. Please review logs before sharing them because diagnostic display hashes are persistent pseudonymous identifiers.

[B]Independent project[/B]

Program Display is a third-party community plugin. It is not part of, affiliated with or endorsed by the OBS Project. Its name, logo and other artwork do not use OBS Project branding.

[B]AI-assisted development and artwork disclosure[/B]

OpenAI Codex generated and revised portions of the C++, Objective-C++, CMake, tests and documentation during the initial development of Program Display, and was also used for targeted research and code review. OpenAI image generation created the project logo from a maintainer-reviewed brief. The maintainer made the product and architecture decisions, reviewed the source changes and visual output, ran the automated test suite, and performed the documented hardware checks. AI-assisted work was not treated as a substitute for understanding the code, maintainer review or physical testing.

Program Display is free software licensed under GPL-2.0-or-later.
```

## First resource update/comment

After the resource is approved, the following can be used as its first update
or discussion comment after maintainer review.

```bbcode
I built Program Display for a small but awkward real-world workflow: a Mac sends OBS Program over HDMI to a capture device, and that destination needs to come back reliably after restarts and cable changes.

For 0.1 I have deliberately kept the scope narrow and macOS-only. The tested configuration is macOS 27 on Apple Silicon; the macOS 13 deployment target and universal build are compatibility targets rather than evidence of testing on earlier macOS releases or Intel hardware.

I would be particularly interested in feedback from people using capture devices, USB-C docks or adapters where display order changes between sessions. Please include the full connection path in reports; it is often as important as the display model itself.

The most useful early feedback would be:

[LIST=1]
[*]whether the saved destination restores after restarting OBS;
[*]what happens after physically disconnecting and reconnecting it;
[*]whether the dock explains missing or unsafe matches clearly; and
[*]whether the source-first build and installation instructions are reproducible.
[/LIST]
```

## Submission check

- Enable two-step verification on the OBS Forums account and allow time for the
  **Add Resource** control to appear on a new account.
- Re-read the current Forum Resource and IP Policy immediately before posting.
- Use **Resources -> Add Resource -> OBS Studio Plugins**; do not create a
  separate announcement thread.
- Upload the original Program Display icon and dock screenshot listed above.
- Preserve the independent-project statement and full AI-assistance disclosure.
- Confirm the public release, source, documentation and issue links while
  signed out.
- Do not describe macOS 13 through 26 or Intel as tested or supported.
