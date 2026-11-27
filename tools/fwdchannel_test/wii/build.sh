#!/bin/sh
# Build fwdinstall_test.dol in the devkitPPC Docker image (resources come from data/binary).
set -eu
cd "$(dirname "$0")"
ROOT="$(cd ../../.. && pwd)"
mkdir -p data
cp "$ROOT/data/binary/fwd_template.bin" "$ROOT/data/binary/fwd_forwarder.bin" data/
docker run --rm --platform linux/amd64 -v "$ROOT":/src -w /src/tools/fwdchannel_test/wii devkitpro/devkitppc:20250527 sh -c 'make clean >/dev/null; make'
rm -rf ./build ./data ./fwdinstall_test.elf.map ./fwdinstall_test.elf
ls -l fwdinstall_test.dol
