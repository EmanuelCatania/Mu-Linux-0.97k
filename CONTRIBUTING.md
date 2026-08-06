# Contributing

The normal workflow uses a short-lived branch, a pull request targeting `main`, and
a squash merge. Before changing code, read the [development guide](docs/development.md)
and search for related issues.

## Workflow

1. Update `main` with `git pull --ff-only`.
2. Create a branch with a `feat/`, `fix/`, `refactor/`, `docs/`, or `chore/` prefix.
3. Make small, signed commits with messages that explain the change.
4. Run the validation appropriate to the affected component and risk.
5. Open a pull request to `aldomigge/mu-097k:main` describing the change and the
   validation performed.

Do not push directly to `main`. Pull requests are squash-merged and must not combine
unrelated changes.

Use the [validation strategy](docs/testing.md) to select the required checks. If a
change affects a protocol, configuration, encoder, or runtime contract, document its
compatibility with every affected component in the pull request. Do not include
credentials, generated binaries, or third-party material without verifiable origin
and authorization.

## Upstream and licensing

This fork follows an independent development line. Do not open automated pull
requests against upstream projects; external fixes are evaluated and imported
individually while preserving authorship and references.

No repository-wide license has been identified for the legacy material. Review
[`NOTICE.md`](NOTICE.md) before copying, redistributing, or adding third-party
content.

## Releases

GitHub Releases are the official release history. Use titles in the form
`MU 0.97k — fork vX.Y.Z`, highlight only the most relevant changes, and call out
compatibility requirements or limitations that matter to users.
