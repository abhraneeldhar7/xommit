# Publishing

## One version source

`cli/package.json` holds the version. Nothing else stores it.

`cli/scripts/build.js` reads that version and passes it to the compiler:

```
-DXOMMIT_VERSION="<version>"
```

`src/version.h` falls back to `"dev"` when the macro is missing, so a plain `g++` build still works.

## Local build

```sh
cd cli
node scripts/build.js
```

The script collects every `src/*.cpp` file, runs `g++` with C++20, static runtime, and `-lwinhttp`, and writes `cli/bin/xommit.exe`. `cli/bin` is git ignored.

## Tag and release

```sh
cd cli
npm run release
```

`scripts/release.js`:

1. Fails if the working tree is not clean.
2. Fails if the tag `v<version>` already exists.
3. Builds locally to catch compile errors before tagging.
4. Creates the tag and pushes it.

The tag starts the GitHub Actions workflow. Nothing is published from a developer machine.

## Workflow

`.github/workflows/release.yml` runs on `v*` tags on `windows-latest`.

1. Checkout and Node 22.
2. Compare the tag with `cli/package.json`. A mismatch stops the job. This prevents publishing a tag that does not match the npm version.
3. Build `cli/bin/xommit.exe`.
4. Create the GitHub Release and upload `xommit.exe` as the only asset.
5. Update npm, then `npm publish --provenance --access public` from `cli/`.

The release asset name is fixed: `xommit.exe`. The installers and the npm shim build the download URL as:

```
https://github.com/abhraneeldhar7/xommit/releases/download/v<version>/xommit.exe
```

Renaming the asset or the tag format breaks all three install routes.

## npm trusted publishing

The workflow has `id-token: write` and publishes without a token. npm exchanges the GitHub OIDC identity for a short lived publish credential.

One time setup on npmjs.com, for `@abhraneeldhar7/xommit`:

- Repository: `abhraneeldhar7/xommit`
- Workflow file: `release.yml`
- Environment: leave empty

No `NPM_TOKEN` secret is needed. If the trusted publisher entry is missing, `npm publish` fails with an authentication error.

Provenance is enabled by `--provenance`, which also uses the OIDC token. The published package page shows the build source.

## What npm receives

The `files` field limits the package to:

```
scripts/index.js
scripts/install.ps1
scripts/install.sh
README.md
```

The exe is never part of the npm package. It is downloaded from the GitHub Release on first run, pinned to the package version.

## Checklist

1. Bump `version` in `cli/package.json`.
2. Commit.
3. `cd cli && npm run release`.
4. Watch the workflow: build, GitHub Release, npm publish.
5. `npm install -g @abhraneeldhar7/xommit` in a clean shell to verify the download path.
