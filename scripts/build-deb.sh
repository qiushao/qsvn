#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

PACKAGE_NAME="qsvn"
VERSION="${VERSION:-$(sed -nE 's/^project\(qsvn VERSION ([^ )]+).*/\1/p' "$REPO_ROOT/CMakeLists.txt" | head -n1)}"
MAINTAINER="${MAINTAINER:-qsvn maintainers <qsvn@example.invalid>}"
BUILD_ROOT="${BUILD_ROOT:-$REPO_ROOT/build/deb}"
BUILD_DIR="$BUILD_ROOT/build"
PACKAGE_ROOT="$BUILD_ROOT/package"
DIST_DIR="${DIST_DIR:-$REPO_ROOT/dist}"

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Missing required command: $1" >&2
        exit 1
    fi
}

if [[ -z "$VERSION" ]]; then
    echo "Could not read project version from CMakeLists.txt" >&2
    exit 1
fi

if [[ ! "$VERSION" =~ ^[0-9][A-Za-z0-9.+:~-]*$ ]]; then
    echo "Invalid Debian package version: $VERSION" >&2
    exit 1
fi

require_command cmake
require_command dpkg
require_command dpkg-deb

ARCH="$(dpkg --print-architecture)"
OUTPUT="$DIST_DIR/${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"

rm -rf "$BUILD_DIR" "$PACKAGE_ROOT"
mkdir -p "$BUILD_DIR" "$PACKAGE_ROOT" "$DIST_DIR"

CMAKE_ARGS=(
    -S "$REPO_ROOT"
    -B "$BUILD_DIR"
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_PREFIX=/usr
    -DBUILD_TESTING=OFF
)

if command -v ninja >/dev/null 2>&1; then
    CMAKE_ARGS+=(-G Ninja)
fi

cmake "${CMAKE_ARGS[@]}"
cmake --build "$BUILD_DIR" --parallel
DESTDIR="$PACKAGE_ROOT" cmake --install "$BUILD_DIR"

mkdir -p "$PACKAGE_ROOT/DEBIAN"

SHLIBS_DEPENDS=""
if command -v dpkg-shlibdeps >/dev/null 2>&1; then
    mkdir -p "$BUILD_ROOT/debian"
    cat > "$BUILD_ROOT/debian/control" <<EOF
Source: $PACKAGE_NAME
Package: $PACKAGE_NAME
Architecture: any
EOF
    if SHLIBS_OUTPUT="$(cd "$BUILD_ROOT" && dpkg-shlibdeps -O "$PACKAGE_ROOT/usr/bin/qsvn" 2>/dev/null)"; then
        SHLIBS_DEPENDS="${SHLIBS_OUTPUT#shlibs:Depends=}"
    fi
fi

if [[ -n "$SHLIBS_DEPENDS" ]]; then
    DEPENDS="$SHLIBS_DEPENDS, subversion, meld"
else
    DEPENDS="libqt5widgets5, subversion, meld"
fi

INSTALLED_SIZE="$(du -sk "$PACKAGE_ROOT" | cut -f1)"

cat > "$PACKAGE_ROOT/DEBIAN/control" <<EOF
Package: $PACKAGE_NAME
Version: $VERSION
Section: vcs
Priority: optional
Architecture: $ARCH
Maintainer: $MAINTAINER
Depends: $DEPENDS
Installed-Size: $INSTALLED_SIZE
Description: Small Qt 5 SVN desktop client
 qsvn is a small Qt 5 desktop client for common Subversion workflows.
 It uses the system svn executable as its backend.
EOF

find "$PACKAGE_ROOT" -type d -exec chmod 755 {} +
find "$PACKAGE_ROOT/DEBIAN" -type f -exec chmod 644 {} +

rm -f "$OUTPUT"
dpkg-deb --build "$PACKAGE_ROOT" "$OUTPUT"

echo "$OUTPUT"
