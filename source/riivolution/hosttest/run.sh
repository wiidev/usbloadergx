#!/bin/sh
# GX -> Friivolution contract test. Runs on the host with the normal C++
# compiler; no Wii toolchain needed.
#
# Usage: sh run.sh [FRIIV_SRC]
#   FRIIV_SRC defaults to ../../../../friivolution (this workspace layout).
#   When the Friivolution tree is absent the consumer half is skipped and the
#   script reports that platform boundary instead of failing.
set -eu
cd "$(dirname "$0")"
FRIIV_SRC="${1:-../../../../friivolution}"

HAVE_CONSUMER=0
if [ -f "$FRIIV_SRC/launcher/source/bootcfg.cpp" ] && \
   [ -f "$FRIIV_SRC/launcher/include/FriivConfig.h" ] && \
   [ -f "$FRIIV_SRC/launcher/include/bootcfg.h" ]; then
	HAVE_CONSUMER=1
	mkdir -p .tmp-contract
	sed -n '/^\/\/ --- bootcfg (host-tested/,/^\/\/ --- end bootcfg/p' \
		"$FRIIV_SRC/launcher/source/bootcfg.cpp" | sed '$d' > .tmp-contract/bootcfg-body.inc
	mkdir -p .tmp-contract/launcher-include
	cp "$FRIIV_SRC/launcher/include/FriivConfig.h" .tmp-contract/launcher-include/
	cp "$FRIIV_SRC/launcher/include/bootcfg.h" .tmp-contract/launcher-include/
fi

FLAGS="-std=gnu++98 -g -O1 -fno-strict-aliasing -Wall -Wextra"
if [ "$HAVE_CONSUMER" = 1 ]; then
	FLAGS="$FLAGS -DHAVE_FRIIV_CONSUMER -I.tmp-contract -I.tmp-contract/launcher-include"
	echo "contract: testing adapter builder against real Friivolution consumer ($FRIIV_SRC)"
else
	echo "contract: Friivolution tree not found at $FRIIV_SRC; adapter-only checks (consumer boundary NOT exercised)"
fi

g++ $FLAGS test_friiv_contract.cpp ../FriivAdapter.cpp -o /tmp/friiv-contract
/tmp/friiv-contract
rm -rf .tmp-contract
