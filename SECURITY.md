# Security

The project supports the current state of `main` and the latest release.

Do not publish credentials, tokens, personal data, or exploitable details in issues.
Use [GitHub private vulnerability reporting](https://github.com/aldomigge/mu-097k/security/advisories/new).

Include the affected component, reproduction steps, impact, and a possible
mitigation. Do not submit real server or player data. The documented environment is
intended for local use; do not expose the Compose stack, web panel, or game server to
the internet without a dedicated security review.

## Client credential rules

Client features that handle login credentials must use Windows Credential
Manager or DPAPI rather than plaintext files. Never place a password, credential
blob, or transient snapshot in `Config.ini`, logs, console output, crash reports,
or debug messages. Keep placeholders and masks strictly visual; native input
buffers must contain only the real values required by the existing protocol.

Clear temporary copies with `SecureZeroMemory` as soon as their work is complete.
Persist credentials only after the user submits them and authentication succeeds.
Invalid credentials must be removed according to the feature's defined state
machine, while reconnect and logout paths must not accidentally overwrite them.

Credential Manager protects storage for the Windows user profile; it does not
protect secrets from malware, a debugger, or code running as the same user. The
password may also exist briefly in the `main.exe` process because the native
client still requires it for submission.
