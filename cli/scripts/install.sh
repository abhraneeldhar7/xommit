#!/bin/bash
# for linux people without nodejs. downloads the binary, stores in ~/.local/bin -> adds this to PATH

set -euo pipefail

VERSION="${1:-}"
REPO="abhraneeldhar7/xommit"

if [ -z "$VERSION" ]; then
    VERSION=$(curl -fsSL -H "User-Agent: xommit-installer" "https://api.github.com/repos/$REPO/releases/latest" | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')
fi

ARCH="$(uname -m)"
case "$ARCH" in
    x86_64) ASSET="xommit-linux-x64" ;;
    aarch64) ASSET="xommit-linux-arm64" ;;
    *) echo "Unsupported arch: $ARCH"; exit 1 ;;
esac

INSTALL_DIR="${XDG_BIN_HOME:-$HOME/.local/bin}"
mkdir -p "$INSTALL_DIR"

TMP_FILE=$(mktemp)
trap 'rm -f "$TMP_FILE"' EXIT

echo "downloading xommit $VERSION ($ASSET)"
curl -fsSL -H "User-Agent: xommit-installer" \
    "https://github.com/$REPO/releases/download/v$VERSION/$ASSET" \
    -o "$TMP_FILE"

SIZE=$(stat -c%s "$TMP_FILE" 2>/dev/null || stat -f%z "$TMP_FILE" 2>/dev/null)
if [ "$SIZE" -lt 100000 ]; then
    echo "download looks invalid"
    exit 1
fi

mv "$TMP_FILE" "$INSTALL_DIR/xommit"
chmod +x "$INSTALL_DIR/xommit"

# Check if install dir is in PATH
case ":$PATH:" in
    *":$INSTALL_DIR:"*) ;;
    *)
        echo "added $INSTALL_DIR to your PATH - add this to your shell config:"
        echo "  export PATH=\"\$PATH:$INSTALL_DIR\""
        ;;
esac

echo "installed xommit $VERSION to $INSTALL_DIR/xommit"