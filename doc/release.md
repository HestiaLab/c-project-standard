# Release

This template does not prescribe a release tool. A typical release should:

1. Update version metadata.
2. Run `./scripts/check.sh release`.
3. Run shared-library, sanitizer, or coverage presets where supported.
4. Update release notes.
5. Run the package smoke test for installable libraries.
6. Tag the release.
7. Publish artifacts through CI or a documented local packaging command.

Do not publish releases from AI-assisted changes without maintainer approval.
