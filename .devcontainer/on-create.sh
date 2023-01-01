#!/bin/bash

# Install some essentials
apt-get update && apt-get install make xz-utils -y

# Download and extract
cd /opt
wget "https://wii.leseratte10.de/devkitPro/file.php/devkitPPC-r41-2-linux_x86_64.pkg.tar.xz"
wget "https://wii.leseratte10.de/devkitPro/file.php/devkitppc-rules-1.1.1-1-any.pkg.tar.xz"
wget "https://wii.leseratte10.de/devkitPro/file.php/libogc-2.3.1-1-any.pkg.tar.xz"
wget "https://wii.leseratte10.de/devkitPro/file.php/general-tools-1.2.0-2-linux_x86_64.pkg.tar.xz"
wget "https://wii.leseratte10.de/devkitPro/file.php/gamecube-tools-1.0.3-1-linux_x86_64.pkg.tar.xz"
for f in *.pkg.tar.xz; do tar xf "$f" --strip-components=1; done

# Clean up
rm *.pkg.tar.xz

# Set environment variables
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=/opt/devkitpro/devkitPPC
