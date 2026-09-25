#!/usr/bin/env bash
# Builds with the Weev engine's pinned toolchain (cmake, ninja, clang),
# so nothing has to be installed. Point WEEV_TOOLCHAIN_ENV elsewhere if
# the weev checkout is not at C:/weev/weev.
#
#   ./build.sh [debug|release] [-- args passed to the binary]

set -euo pipefail

CONFIG="${1:-debug}"
shift || true

case "$CONFIG" in
	debug) CMAKE_CONFIG="Debug" ;;
	release) CMAKE_CONFIG="Release" ;;
	*)
		echo "usage: ./build.sh [debug|release] [-- args]" >&2
		exit 2
		;;
esac

TOOLCHAIN_ENV="${WEEV_TOOLCHAIN_ENV:-/c/weev/weev/engine/external/_toolchain/env.sh}"

if [ ! -f "$TOOLCHAIN_ENV" ]; then
	echo "error: no toolchain at $TOOLCHAIN_ENV" >&2
	echo "run ./engine/scripts/setup.sh in the weev checkout first" >&2
	exit 1
fi

# shellcheck disable=SC1090
. "$TOOLCHAIN_ENV"

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT/build/$CONFIG"

cmake -S "$ROOT" -B "$BUILD_DIR" -G Ninja \
	-DCMAKE_BUILD_TYPE="$CMAKE_CONFIG" \
	"-DCMAKE_CXX_COMPILER:FILEPATH=$WEEV_CXX" >/dev/null

cmake --build "$BUILD_DIR"

if [ "${1:-}" = "--" ]; then
	shift
	"$BUILD_DIR/bc_input_test.exe" "$@"
fi
