#!/bin/sh
# Build the host test and build a channel WAD on the PC with source/channel/fwdchannel.c:
#   ./host_test.sh nandloader.app banner.bnr GAMEID TITLE keys.bin out.wad
# nandloader.app = boot content (index 1) of the 3-content UNEO forwarder WAD
set -eu
cd "$(dirname "$0")"
ROOT="$(cd ../.. && pwd)"
cc -O2 -Wall -Wextra -o ./fwdtest main.c "$ROOT/source/channel/fwdchannel.c"
./fwdtest "$ROOT/data/binary/fwd_template.bin" "$1" "$ROOT/data/binary/fwd_forwarder.bin" "$2" "$3" "$4" /apps/usbloader_gx/boot.dol "$5" "$6"
