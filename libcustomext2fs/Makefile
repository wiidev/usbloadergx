
default: cube-release wii-release

all: debug release

debug: cube-debug wii-debug

release: cube-release wii-release

cube-debug:
	$(MAKE) -C source PLATFORM=cube BUILD=cube_debug

wii-debug:
	$(MAKE) -C source  PLATFORM=wii BUILD=wii_debug

cube-release:
	$(MAKE) -C source  PLATFORM=cube BUILD=cube_release

wii-release:
	$(MAKE) -C source  PLATFORM=wii BUILD=wii_release

clean: 
	$(MAKE) -C source clean

install: cube-release wii-release
	$(MAKE) -C source install

run: install
	$(MAKE) -C example
	$(MAKE) -C example run

