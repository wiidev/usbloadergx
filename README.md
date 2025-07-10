<p align="center"><a href="https://github.com/wiidev/usbloadergx/" title="USB Loader GX"><img src="data/web/logo.png"></a></p>
<p align="center">
<a href="https://github.com/wiidev/usbloadergx/releases" title="Releases"><img src="https://img.shields.io/github/v/release/wiidev/usbloadergx?logo=github"></a>
<a href="https://github.com/wiidev/usbloadergx/actions" title="Actions"><img src="https://img.shields.io/github/actions/workflow/status/wiidev/usbloadergx/main.yml?branch=enhanced&logo=github"></a>
</p>

## Description
USB Loader GX allows you to play Wii and GameCube games from a USB storage device or an SD card, launch other homebrew apps, create backups, use cheats in games, and a whole lot more.

## Installation
1. Extract the apps folder to the root of your SD card and replace any existing files.
2. Install the [latest d2x cIOS](https://github.com/wiidev/d2x-cios/releases).
3. Optional: Update wiitdb.xml by selecting the update option within the loaders settings menu.
4. Optional: Install the loaders [forwarder channel](https://raw.githubusercontent.com/wiidev/usbloadergx/updates/USBLoaderGX_forwarder%5BUNEO%5D.wad), then go into `Loader Settings` and set `Return To` to `UNEO`.

## d2x cIOS
1. Use the correct cIOS package for your console — e.g., `d2x-v11-beta3` for the Wii and `d2x-v11-beta3-vWii` for the Wii U.
2. When using the d2x cIOS installer, set the cIOS to the version that you downloaded — e.g., `d2x-v11-beta3`.
3. Install the cIOS into each slot with the following settings.

````
Slot 248 base 38
Slot 249 base 56
Slot 250 base 57
Slot 251 base 58
````
