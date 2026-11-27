#!/bin/sh
# Build the forwarder DOL used by "Create Wii Menu Channel" in the devkitPPC Docker image
# and install it as data/binary/fwd_forwarder.bin. Its app_booter is the loader's own.
set -eu
cd "$(dirname "$0")"
ROOT="$(cd ../.. && pwd)"
mkdir -p data
cp "$ROOT/data/binary/app_booter.bin" data/
docker run --rm --platform linux/amd64 -v "$PWD":/src -w /src devkitpro/devkitppc:20250527 sh -c 'make clean >/dev/null; make'
cp forwarder.dol "$ROOT/data/binary/fwd_forwarder.bin"
rm -rf ./build ./data ./forwarder.dol ./forwarder.elf ./forwarder.elf.map
shasum "$ROOT/data/binary/fwd_forwarder.bin"
