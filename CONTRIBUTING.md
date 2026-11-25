# Contributing to xommit

Thanks for wanting to help. Issues and pull requests are welcome.

## Before you start

Open an issue first for anything that is not a small bug fix. That covers
new providers, changes to the CLI surface, and anything that touches how the
prompt or the diff pipeline works. It saves you from writing code I will not
merge.

## What I will usually merge

- Bug fixes with a clear reproduction
- Support for a new AI provider
- Better error messages
- Test coverage
- Documentation corrections

## What I will usually reject

- Rebranding, new logos, or renaming the command
- Rewrites of the prompt without evidence the output got better
- Dependencies that are not strictly needed
- Changes that touch files outside the problem being fixed

## Setting up

You need MinGW-w64 UCRT64 GCC on PATH (`g++` must resolve) and Node.js 18
or newer.

```sh
cd cli
npm install
npm run build
```

The build writes `cli/bin/xommit.exe`.

## Testing

```sh
cd cli
npm test        # unit tests and smoke tests, no API calls
```

Live tests hit a real provider and spend your own credits. They are not run
in CI and you should not run them in a pull request.

## Pull requests

- One problem per pull request
- Keep the diff small and explain the change in the description
- Do not reformat files you did not otherwise touch
- Do not commit across unrelated areas of the repo

## Code style

The CLI is plain C++ with no framework. Keep it that way.

- Match the file you are editing
- Comments explain why, not what
- No em dashes in comments or docs, use plain punctuation
- Do not add a dependency for something in the standard library

## Reporting bugs

Include your version (`xommit --version`), your OS, the exact command you ran,
and what you expected versus what happened. If it involves a provider, say
which one. Never paste your API key into an issue.

## Security

Do not open a public issue for a security problem. See
[SECURITY.md](SECURITY.md).

## License

By contributing you agree your work is distributed under the same
[PolyForm Noncommercial License 1.0.0](LICENSE) that covers the project.
