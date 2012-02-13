ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro)
endif
 
export TOPDIR	:=	$(CURDIR)
 
export LIBFAT_MAJOR	:= 1
export LIBFAT_MINOR	:= 0
export LIBFAT_PATCH	:= 10

export VERSTRING	:=	$(LIBFAT_MAJOR).$(LIBFAT_MINOR).$(LIBFAT_PATCH)


default: release

all: release

release: include/libfatversion.h wii-release

wii-release:
	$(MAKE) -C libogc PLATFORM=wii BUILD=wii_release

clean: ogc-clean

ogc-clean:
	$(MAKE) -C libogc clean

install: ogc-install

ogc-install: cube-release wii-release
	$(MAKE) -C libogc install
