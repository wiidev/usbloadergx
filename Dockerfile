# Build: 
# DOCKER_BUILDKIT=1 docker build -o output .
# for Windows, use 
# { "features": { "buildkit": true } }
# instead of the environment variable

# Build a Debian base container
FROM debian:buster as usbloader
ENV DEBIAN_FRONTEND="noninteractive" TZ="Europe/London"
RUN apt-get update -y && apt-get install -y \ 
    xz-utils make git

ADD https://wii.leseratte10.de/devkitPro/file.php/devkitPPC-r41-2-linux_x86_64.pkg.tar.xz /
ADD https://wii.leseratte10.de/devkitPro/file.php/libogc-2.3.1-1-any.pkg.tar.xz /
ADD https://wii.leseratte10.de/devkitPro/file.php/devkitppc-rules-1.1.1-1-any.pkg.tar.xz /
ADD https://wii.leseratte10.de/devkitPro/file.php/general-tools-1.2.0-2-linux_x86_64.pkg.tar.xz /
ADD https://wii.leseratte10.de/devkitPro/file.php/gamecube-tools-1.0.3-1-linux_x86_64.pkg.tar.xz /


RUN tar -xf /devkitPPC-r41-2-linux_x86_64.pkg.tar.xz opt/devkitpro/devkitPPC --strip-components=1 && \
    tar -xf /libogc-2.3.1-1-any.pkg.tar.xz opt/devkitpro/libogc --strip-components=1 && \
    tar -xf /devkitppc-rules-1.1.1-1-any.pkg.tar.xz opt/devkitpro/devkitPPC --strip-components=1 && \
    tar -C /usr/local/bin -xf /general-tools-1.2.0-2-linux_x86_64.pkg.tar.xz opt/devkitpro/tools/bin/bin2s --strip-components=4 && \
    tar -C /usr/local/bin -xf /gamecube-tools-1.0.3-1-linux_x86_64.pkg.tar.xz opt/devkitpro/tools/bin/elf2dol --strip-components=4 && \
    mkdir /projectroot 

ENV DEVKITPRO=/devkitpro
ENV DEVKITPPC=/devkitpro/devkitPPC


# Now we have a container that has the dev environment set up. 
# Copy current folder into container, then compile
COPY . /projectroot/
RUN cd /projectroot && make


# Copy the DOL and ELF out of the container
FROM scratch AS export-stage
COPY --from=usbloader /projectroot/boot.* /
