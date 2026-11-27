# Tests for "Create Wii Menu Channel"

`source/channel/` builds a per-game shortcut channel and installs it through ES:

| Content | What |
|---|---|
| 0 | The game's `opening.bnr` (from the banner cache, else from the game image). Layout and animation files named `Icon…`/`Banner…` are renamed to lower case, because the Wii Menu looks them up case-sensitively. |
| 1 (boot) | The nandloader of the installed UNEO forwarder channel, read through ISFS. A libogc DOL as boot content gives a black screen when the Wii Menu starts the channel on a real console. |
| 2 | The forwarder DOL (`data/binary/fwd_forwarder.bin`, source in `tools/forwarder_dol/`). It starts `sd:` (else `usb1:`) `/apps/usbloader_gx/boot.dol` with argv = path, the `<arguments>` of its meta.xml, and the game ID, so `StartUpProcess::ParseArguments` boots the game right away. |

**GameCube games** have no channel banner, only a BNR1/BNR2 `opening.bnr` (a 96x32 RGB5A3
picture and texts). Their channel banner is built from `data/binary/custom_banner.bnr`, the
template the loader already uses to draw GameCube games: the game picture in the icon and the
banner, the GameCube logo and background (`fwd_gc_logo.bin`, `fwd_gc_bg.bin`, TPLs made from
`data/images/gc_icon_bg.png` and `gc_banner_bg.png` by `make_gc_tpls.py`), the title, developer
and description. The loader starts GameCube games by ID the same way (`QuickGameBoot`), as long
as GameCube games are among its game sources.

Ticket, TMD fields (IOS 58, AHB access) and certificates come from the 3-content UNEO forwarder
(`data/binary/fwd_template.bin`); ticket and TMD are fakesigned. The channel ID is the game ID
with its first letter replaced: Wii R -> T, S -> K (`RMCP01` -> `00010001-TMCP`), GameCube
G -> Y, D -> Z (`GALE01` -> `00010001-YALE`). G is avoided on purpose: System Menu 4.3U does not
show 00010001 titles starting with G (Y, Z, T, K, Q, V, U and A are shown).

## Tests

- `host_test.sh` builds `source/channel/fwdchannel.c` for a PC and writes the channel as a WAD.
  Output is deterministic; it was compared byte for byte with an independent Python
  implementation for all 49 Wii games of a banner cache.
- `wii/` (`build.sh`, Docker devkitPPC) is a Wii program: for each `sd:/fwdtest/<GAMEID>.bnr`
  it builds the channel, saves the WAD, installs it via ES and logs to `sd:/fwdtest/result.txt`;
  with a file `sd:/fwdtest/delete` it removes those channels instead. It also logs which
  installed channels count as shortcuts.

## Checked on a console (Wii RVL-001, System Menu 4.3U, d2x cIOS 249)

"Create Wii Menu Channel" for Kirby's Return to Dream Land installed `00010001-KUKE`; the
channel shows the game's banner on the Wii Menu and boots the game through the loader;
"Remove Wii Menu Channel" deleted it. Shortcuts no longer show up next to their game in the
list. Erase in Data Management also removes such a channel.
