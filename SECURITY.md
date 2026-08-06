# Security

The project supports the current state of `main` and the latest release.

Do not publish credentials, tokens, personal data, or exploitable details in issues.
Use [GitHub private vulnerability reporting](https://github.com/aldomigge/mu-097k/security/advisories/new).

Include the affected component, reproduction steps, impact, and a possible
mitigation. Do not submit real server or player data. The documented environment is
intended for local use; do not expose the Compose stack, web panel, or game server to
the internet without a dedicated security review.

## Client credentials

- Never persist credentials in plaintext files, logs, crash reports, console
  output, or debug messages.
- Use Windows Credential Manager or DPAPI for local credential persistence.
- Minimize the lifetime and number of plaintext copies in memory, and clear
  temporary buffers when they are no longer required.
- Keep visual placeholders and masks separate from native input buffers and
  protocol data.
- Document the threat model and lifecycle of any credential-handling feature.

Credential Manager protects storage for the Windows user profile; it does not
protect secrets from malware, a debugger, or code running as the same user. The
native client may also require plaintext credentials briefly during submission.
