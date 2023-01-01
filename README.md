<p align="center"><a href="https://github.com/wiidev/usbloadergx/" title="USB Loader GX"><img src="data/web/logo.png"></a></p>
<p align="center">
<a href="https://github.com/wiidev/usbloadergx/releases" title="Releases"><img src="https://img.shields.io/github/v/release/wiidev/usbloadergx?logo=github"></a>
<a href="https://github.com/wiidev/usbloadergx/actions" title="Actions"><img src="https://img.shields.io/github/actions/workflow/status/wiidev/usbloadergx/main.yml?branch=enhanced&logo=github"></a>
</p>

## Description
USB Loader GX allows you to play Wii and GameCube games from a USB storage device or an SD card, launch other homebrew apps, create backups, use cheats in games and a whole lot more.

## Installation
1. Extract the apps folder to the root of your SD card and replace any existing files.
2. Install the [d2x v11 cIOS](https://github.com/wiidev/d2x-cios/releases).
3. Optional: Download wiitdb.xml by selecting the update option within the loaders settings menu.
4. Optional: Install the loaders forwarder channel ([Wii](https://github.com/wiidev/usbloadergx/raw/updates/USBLoaderGX_forwarder%5BUNEO%5D_Wii.wad) or [vWii](https://github.com/wiidev/usbloadergx/raw/updates/USBLoaderGX_forwarder%5BUNEO%5D_vWii.wad)) and then set the return to setting to `UNEO`.

## cIOS guide
The first configuration is the optimal one for the Wii, but the second configuration should improve compatibility.

**For Wii**
````
Slot 249 base 56
Slot 250 base 57
Slot 251 base 38
````
**For vWii and Wii**
````
Slot 248 base 38 (Wii only)
Slot 249 base 56
Slot 250 base 57
Slot 251 base 58
````
