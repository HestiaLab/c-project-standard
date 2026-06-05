# Release

This template does not prescribe a release tool. A typical release should:

1. Update version metadata.
2. Run `./scripts/check.sh release`.
3. Run sanitizer or coverage presets where supported.
4. Update release notes.
5. Tag the release.
6. Publish artifacts through CI or a documented local packaging command.

Do not publish releases from AI-assisted changes without maintainer approval.
