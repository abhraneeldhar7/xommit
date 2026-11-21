#!/usr/bin/env bash
set -euo pipefail

repo="abhraneeldhar7/xommit"
version="${1:-}"

if [ -z "$version" ]; then
  version="$(curl -fsSL -H "User-Agent: xommit-installer" \
    "https://api.github.com/repos/$repo/releases/latest" |
    sed -n 's/.*"tag_name": *"v\([^"]*\)".*/\1/p')"
fi

if [ -z "$version" ]; then
  echo "error: could not find the latest release" >&2
  exit 1
fi

local_appdata="${LOCALAPPDATA:-$HOME/AppData/Local}"
if command -v cygpath >/dev/null 2>&1; then
  dir="$(cygpath -u "$local_appdata")/xommit/bin"
else
  dir="$local_appdata/xommit/bin"
fi

mkdir -p "$dir"
temporary="$dir/xommit.download.exe"

echo "downloading xommit $version"
curl -fL -H "User-Agent: xommit-installer" \
  "https://github.com/$repo/releases/download/v$version/xommit.exe" \
  -o "$temporary"

if [ "$(wc -c <"$temporary")" -lt 100000 ]; then
  rm -f "$temporary"
  echo "error: download looks invalid" >&2
  exit 1
fi

mv -f "$temporary" "$dir/xommit.exe"
printf '%s\n' "$version" >"$dir/version.txt"

if ! grep -q "xommit/bin" "$HOME/.bashrc" 2>/dev/null; then
  printf '\nexport PATH="%s:$PATH"\n' "$dir" >>"$HOME/.bashrc"
  echo "added $dir to PATH in ~/.bashrc, open a new shell"
fi

echo "installed xommit $version to $dir/xommit.exe"
