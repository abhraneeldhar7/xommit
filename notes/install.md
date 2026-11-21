# Install

xommit ships as a single Windows x64 executable. There are three install routes and they are independent. Pick one.

Every route ends with the same binary at:

```
%LOCALAPPDATA%\xommit\bin\xommit.exe
%LOCALAPPDATA%\xommit\bin\version.txt
```

`version.txt` records which release the binary came from. The npm shim and `--update` use it to decide whether a download is needed.

The binary is not code signed. Windows SmartScreen may show "Windows protected your PC" on first run. Choose "More info" then "Run anyway". A signing certificate is not part of this project.

## npm

```sh
npm install -g @abhraneeldhar7/xommit
```

The published package contains only the Node shim and the install scripts. It does not contain `xommit.exe`.

What npm does:

1. It installs the package into the global prefix. On Windows that is usually `%APPDATA%\npm\node_modules\@abhraneeldhar7\xommit`.
2. It reads the `bin` field from `package.json`:

   ```json
   "bin": { "xommit": "scripts/index.js" }
   ```

3. It creates launchers in the global prefix, next to `node_modules`. On Windows you get `xommit.cmd` for cmd and PowerShell, `xommit.ps1` for PowerShell, and a plain `xommit` shim for Git Bash. Each one runs `node "...\scripts\index.js"`.
4. The global prefix is `npm prefix -g`, usually `%APPDATA%\npm`. npm puts that folder on `PATH` during install. If `xommit` is not found, add the folder to `PATH` and open a new terminal.

What the shim does on every run:

1. Read `version` from the package `package.json`.
2. Read `%LOCALAPPDATA%\xommit\bin\version.txt`.
3. If the exe is missing, or the recorded version is older than the package version, download:

   ```
   https://github.com/abhraneeldhar7/xommit/releases/download/v<version>/xommit.exe
   ```

   The download goes to a temporary file first. Files under 100 KB are rejected. The temporary file is then renamed over `xommit.exe` and `version.txt` is written.
4. Spawn the exe with `stdio: "inherit"`, so prompts, colors, and the picker talk to your terminal directly.
5. Exit with the exe exit code.

The shim never downgrades. If you ran `xommit --update` and the binary is newer than the npm package, the shim runs it as is.

Because the shim spawns the exe as a child process, pressing keys works normally. The Node process only forwards the exit code.

## PowerShell

```powershell
irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.ps1 | iex
```

Or with a pinned version:

```powershell
.\install.ps1 -Version 0.1.0
```

The script:

1. Asks the GitHub API for the latest release tag when no version is given.
2. Creates `%LOCALAPPDATA%\xommit\bin`.
3. Downloads `xommit.exe` into a temporary file.
4. Checks the size, moves it into place, writes `version.txt`.
5. Adds the bin folder to the user `PATH` if it is not already there.

Open a new terminal after the script finishes so the new `PATH` is picked up.

## Git Bash

```sh
curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.sh | bash
```

The script does the same steps as the PowerShell route, using `curl`. It converts `LOCALAPPDATA` with `cygpath` when available, appends an `export PATH` line to `~/.bashrc`, and prints the installed path.

This route only changes the Git Bash `PATH`. If you also want `xommit` in cmd or PowerShell, use the npm or PowerShell route.

## Updating

```sh
xommit --update
```

This asks the GitHub API for the latest release, compares it with the compiled in version, and if it is newer downloads `xommit.exe` into the same folder.

A running exe cannot be overwritten on Windows, so the update renames the current binary to `xommit.old.exe`, moves the new one into place, and writes `version.txt`. The next start deletes `xommit.old.exe`. If anything fails, the old binary is restored.

The npm shim respects this: it compares versions and leaves a newer binary alone.
