# OBS Community resource draft

This is a working draft, not text to submit unchanged. The maintainer should
rewrite it in their own voice, verify every claim against the release, insert
the final release URL and retain the complete AI-assistance disclosure. The OBS
forum policy says descriptions written with LLM assistance are discouraged and
resources created largely with AI coding tools may be denied even when their use
is disclosed.

## Suggested resource fields

**Name:** Program Display

**Tagline:** Remember and safely restore a fullscreen Program destination on macOS.

**Version:** 0.1.0

**Category:** OBS Studio Plugins

**Supported platform:** macOS

**Source and documentation:**
<https://github.com/jonathanrelf/program-display>

**Download:** Replace this line with the final GitHub Release URL. Do not submit
the resource while the only installation route is an unsigned development build.

## Draft description

Program Display is a small macOS plugin for people who regularly send the OBS
Studio Program feed to a dedicated external display or capture device.

The problem it addresses is simple: display numbering can change after a Mac,
dock, adapter or capture device is reconnected. Restoring a fullscreen projector
by a saved monitor number can therefore target the wrong screen.

Program Display remembers descriptive hardware identity instead. Each time OBS
starts—or the selected display returns—it resolves the saved identity against
the displays connected now and requests the fullscreen Program projector only
when the match is safe and unique. If the destination is missing, ambiguous or
cannot be identified confidently, it fails closed instead of falling back to
another display.

### What 0.1 includes

- A compact native dock for selecting and inspecting the Program destination.
- Automatic restoration after OBS restart or display reconnection.
- Strong and explicitly confirmed weak display-identity matching on macOS.
- Universal Apple Silicon and Intel binaries; the initial physical validation
  has been performed on Apple Silicon.
- Clear `OUTPUT REQUESTED`, `DISPLAY MISSING` and error states.
- No private OBS frontend headers or implementation types.

### Important limitation

The public OBS frontend API can request a projector, but it does not return a
handle that a plugin can query or close. Program Display therefore says
**Output requested** rather than claiming that the projector is independently
verified. Changing the destination after a request currently requires restarting
OBS.

Program Display does not route audio. Configure audio monitoring or operating
system routing separately.

### Requirements

- macOS 13 or later
- OBS Studio 32.2.2
- A destination exposed by macOS as an extended display

Windows and Linux are not supported in version 0.1. Platform support will be
claimed only after platform-specific identity work and physical restart,
hot-plug, display-order and ambiguity testing.

Installation, operating instructions, limitations, architecture, source code
and the issue tracker are available in the GitHub repository. Bug reports should
include the OBS and macOS versions, Mac model, display or capture hardware, and
the complete adapter/cable path. Please review logs before sharing them because
diagnostic display hashes are persistent pseudonymous identifiers.

Program Display is an independent third-party community plugin. It is not part
of, affiliated with or endorsed by the OBS Project.

### Development and artwork disclosure

OpenAI Codex generated and revised portions of the C++, Objective-C++, CMake,
tests and documentation during the initial development of Program Display, and
was also used for targeted research and code review. OpenAI image generation
created the project logo from a maintainer-reviewed brief. The maintainer made
the product and architecture decisions, reviewed the source changes and visual
output, ran the automated test suite, and performed the documented hardware
checks. AI-assisted work was not treated as a substitute for understanding the
code, maintainer review or physical testing.

Program Display is open-source software licensed under GPL-2.0-or-later.

## Suggested first update/comment

I built Program Display for a small but awkward real-world workflow: a Mac sends
OBS Program over HDMI to a capture device, and that destination needs to come
back reliably after restarts and cable changes.

For 0.1 I have deliberately kept the scope narrow and macOS-only. I would be
particularly interested in feedback from people using capture devices, USB-C
docks or adapters where display order changes between sessions. Please include
the full connection path in reports; it is often as important as the display
model itself.

The most useful early feedback would be:

1. whether the saved destination restores after restarting OBS;
2. what happens after physically disconnecting and reconnecting it;
3. whether the dock explains missing or unsafe matches clearly; and
4. whether the source-first installation instructions are reproducible.
