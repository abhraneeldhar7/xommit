# Security

## Reporting a vulnerability

Do not open a public issue for a security problem.

Email `abhraneeldhar@gmail.com` with a description, the affected version, and
the steps to reproduce. I will reply as soon as I can and credit you in the
fix if you want that.

## How your API key is handled

This is what xommit actually does with your credentials, so you can judge the
risk yourself.

- Your API key is stored in plain text at `%APPDATA%\xommit\config.json`.
  It is not encrypted.
- The key is sent only to the provider you chose, over HTTPS:
  `generativelanguage.googleapis.com` for Google AI Studio,
  `ai-gateway.vercel.sh` for Vercel AI Gateway.
- The key is never sent to me, and there is no telemetry, no account, and no
  server in the middle.
- `xommit --reset` deletes the config file.

Because the file is unencrypted, it is readable by anything running as your
user. On a shared or managed machine, treat it like any other credential file
and keep `%APPDATA%` out of backups and version control.

## What xommit reads

- `git diff --cached`, which covers staged changes only
- `.xommitignore` at the repository root, if it exists
- `%APPDATA%\xommit\config.json`

Nothing else from your repository is read, and your index is never modified.

## What leaves your machine

- Your staged diffs, trimmed to fit the model context window
- Your hint, if you pass one
- Your API key, to the provider only

## Known limits

- The self-update downloads a release binary from GitHub and swaps it in place.
  It checks the download size but does not verify a signature.
- The npm shim downloads the same binary from the same source on first run.
