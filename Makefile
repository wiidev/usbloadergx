#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	boot
BUILD		:=	build
SOURCES		:=	source \
				source/GUI \
				source/Controls \
				source/system \
				source/libs/libwbfs \
				source/language \
				source/mload \
				source/mload/modules \
				source/patches \
				source/usbloader \
				source/xml \
				source/network \
				source/settings \
				source/settings/menus \
				source/prompts \
				source/wad \
				source/banner \
				source/Channels \
				source/BoxCover \
				source/GameCube \
				source/cheats \
				source/homebrewboot \
				source/themes \
				source/menu \
				source/memory \
				source/FileOperations \
				source/ImageOperations \
				source/SoundOperations \
				source/SystemMenu \
				source/utils \
				source/utils/minizip \
				source/usbloader/wbfs
DATA		:=	data \
				data/images \
				data/fonts \
				data/sounds \
				data/binary
INCLUDES	:=	source

#---------------------------------------------------------------------------------
# Default cIOS to load into to load the settings
#---------------------------------------------------------------------------------
ifndef $(IOS)
IOS = 249
endif

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CFLAGS		=	-g -ggdb -O3 -Wall -Wno-multichar -Wno-unused-parameter -Wextra $(MACHDEP) $(INCLUDE) -DBUILD_IOS=$(IOS)
CXXFLAGS	=	$(CFLAGS)
LDFLAGS		=	-g -ggdb $(MACHDEP) -Wl,-Map,$(notdir $@).map,--section-start,.init=0x80B00000,-wrap,malloc,-wrap,free,-wrap,memalign,-wrap,calloc,-wrap,realloc,-wrap,malloc_usable_size,-wrap,wiiuse_register

ifeq ($(BUILDMODE),channel)
CFLAGS += -DFULLCHANNEL
CXXFLAGS += -DFULLCHANNEL
endif

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS := -lcustomfat -lcustomntfs -lcustomext2fs -lvorbisidec -lmad -lfreetype \
		-lgd -ljpeg -lpng -lzip -lm -lz -lwiiuse -lwupc -lbte -lasnd -logc
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(DEVKITPPC)/lib $(CURDIR)
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export PROJECTDIR := $(CURDIR)
export OUTPUT	:=	$(CURDIR)/$(TARGETDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
SVNREV		:=	$(shell bash ./svnrev.sh)
IMPORTFILES	:=  $(shell bash ./filelist.sh)
export CFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
ELFFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.elf)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))
TTFFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.ttf)))
PNGFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.png)))
OGGFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.ogg)))
PCMFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.pcm)))
WAVFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.wav)))
DOLFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.dol)))
MP3FILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.mp3)))
BNRFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bnr)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o) \
					$(TTFFILES:.ttf=.ttf.o) $(PNGFILES:.png=.png.o) $(addsuffix .o,$(DOLFILES)) \
					$(OGGFILES:.ogg=.ogg.o) $(PCMFILES:.pcm=.pcm.o) $(MP3FILES:.mp3=.mp3.o) \
					$(WAVFILES:.wav=.wav.o) $(addsuffix .o,$(ELFFILES)) $(addsuffix .o,$(BINFILES)) \
					$(BNRFILES:.bnr=.bnr.o) $(CURDIR)/data/magic_patcher.o

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC) \
					-I$(PORTLIBS)/include -I$(PORTLIBS)/include/freetype2

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(CURDIR)/source/libs/libfat/ \
					-L$(CURDIR)/source/libs/libntfs/ -L$(CURDIR)/source/libs/libext2fs/ \
					-L$(LIBOGC_LIB) -L$(PORTLIBS)/lib

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) lang all clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
ifneq ($(IOS),249)
	@rm -f $(BUILD)/CSettings.o
endif
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

channel:
	@[ -d build ] || mkdir -p build
	@$(MAKE) BUILDMODE=channel --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
lang:
	@[ -d build ] || mkdir -p build
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile language

#---------------------------------------------------------------------------------
theme:
	@[ -d build ] || mkdir -p build
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile language

#---------------------------------------------------------------------------------
all:
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile lang

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol
#---------------------------------------------------------------------------------
run:
	$(MAKE)
	@echo Done building ...
	@echo Now Run That Shit ...

	wiiload $(OUTPUT).dol

#---------------------------------------------------------------------------------
reload:
	wiiload -r $(OUTPUT).dol

#---------------------------------------------------------------------------------
release:
	$(MAKE)
	cp boot.dol ./hbc/boot.dol


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

language: $(wildcard $(PROJECTDIR)/Languages/*.lang) $(wildcard $(PROJECTDIR)/Themes/*.them)
#---------------------------------------------------------------------------------
# This rule links in binary data with .ttf, .png, and .mp3 extensions
#---------------------------------------------------------------------------------

%.elf.o : %.elf
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.dol.o : %.dol
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.ttf.o : %.ttf
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.png.o : %.png
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.ogg.o : %.ogg
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.pcm.o : %.pcm
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.wav.o : %.wav
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.mp3.o : %.mp3
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.certs.o	:	%.certs
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.dat.o	:	%.dat
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.bin.o	:	%.bin
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.tik.o	:	%.tik
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.tmd.o	:	%.tmd
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)
	
%.bnr.o	:	%.bnr
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

export PATH		:=	$(PROJECTDIR)/gettext-bin:$(PATH)

%.pot: $(CFILES) $(CPPFILES)
	@echo Updating Languagefiles ...
	@touch $(PROJECTDIR)/Languages/$(TARGET).pot
	@xgettext -C -cTRANSLATORS --from-code=utf-8 --sort-output --no-wrap --no-location -ktr -ktrNOOP -o$(PROJECTDIR)/Languages/$(TARGET).pot -p $@ $^
	@echo Updating Themefiles ...
	@touch $(PROJECTDIR)/Themes/$(TARGET).pot
	@xgettext -C -cTRANSLATORS --from-code=utf-8 -F --no-wrap --add-location -kthInt -kthFloat -kthColor -kthAlign -o$(PROJECTDIR)/Themes/$(TARGET).pot -p $@ $^

%.lang: $(PROJECTDIR)/Languages/$(TARGET).pot
	@msgmerge -U -N --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

%.them: $(PROJECTDIR)/Themes/$(TARGET).pot
	@msgmerge -U -N --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
