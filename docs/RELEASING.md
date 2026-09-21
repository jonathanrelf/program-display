# Release procedure

Program Display uses semantic versions. Version 0.1 is a macOS-only,
source-first preview.

## Prepare

1. Confirm `buildspec.json` pins the intended OBS Studio, Qt and dependency
   versions and hashes.
2. Set the release date in `CHANGELOG.md` and ensure every known limitation is
   represented in the README.
3. Search the publishable tree case-insensitively for obsolete product names,
   private paths, credentials and unreviewed hardware identifiers.
4. Configure with `ENABLE_DISPLAY_IDENTITY_PROBE=OFF`.
5. Run the dependency-free tests and the universal macOS build.
6. Verify both `arm64` and `x86_64` slices and verify the installed bundle's
   code signature.
7. Complete and record the manual integration checklist in `DEVELOPMENT.md`
   using the release-candidate bundle.
8. Confirm the GitHub-hosted macOS CI run succeeds from a clean checkout.

## Publish

1. Create an annotated `v0.1.0` tag from the reviewed release commit.
2. Create a GitHub release using the corresponding changelog section.
3. Keep the release labelled as a preview and source-first.
4. Do not advertise a package as one-click installation until it is Developer
   ID signed, notarised and tested after download on a separate Mac account.
5. Before submitting, re-read the OBS
   [Forum Resource and IP Policy](https://obsproject.com/forum/threads/forum-resource-and-ip-policy.178569/).
   Use the product name **Program Display**, use no OBS logo, state clearly that
   it is an independent plugin, include the AI-assistance disclosure, and
   provide screenshots and support instructions.
6. Follow the forum's
   [resource-submission procedure](https://obsproject.com/forum/threads/how-to-post-a-plugin-read-here.23124/):
   use **Resources -> Add Resource** and select **OBS Studio Plugins**. Do not
   create a separate announcement thread: the Resources system creates the
   plugin thread automatically. Two-step verification must be enabled on the
   forum account, and a newly enabled or new account may need time before the
   Add Resource control appears.
7. Treat `docs/OBS-COMMUNITY-ANNOUNCEMENT.md` as a fact-checked starting point,
   not submission-ready prose. Rewrite it in the maintainer's own voice and
   re-check the current Forum Resource and IP Policy immediately before posting.

## After publication

- Install from the published artifact or source archive rather than the local
  checkout and repeat the startup/reconnect smoke test.
- Keep the previous release available until the replacement has passed that
  test.
- Record compatibility regressions against exact OBS and macOS versions.
