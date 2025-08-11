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
				source/libs/libruntimeiospatch \
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
				source/usbloader/wbfs \
				source/Riivolution \
				source/Riivolution/rawksd \
				source/Riivolution/libxml++ \
				source/cache
DATA		:=	data/images \
				data/fonts \
				data/sounds \
				data/binary
INCLUDES	:=	source

#---------------------------------------------------------------------------------
# Options for code generation
#---------------------------------------------------------------------------------
CFLAGS		=	-ggdb -Os -Wall -Wno-multichar -Wno-unused-parameter -Wextra $(MACHDEP) $(INCLUDE) -D_GNU_SOURCE -DNDEBUG -DWOLFSSL_USER_SETTINGS
CXXFLAGS	=	$(CFLAGS)
LDFLAGS		=	-ggdb $(MACHDEP) -Wl,-Map,$(notdir $@).map,--section-start,.init=0x80B00000,-wrap,malloc,-wrap,free,-wrap,memalign,-wrap,calloc,-wrap,realloc,-wrap,malloc_usable_size

ifeq ($(BUILDMODE),channel)
	CFLAGS += -DFULLCHANNEL
	CXXFLAGS += -DFULLCHANNEL
else ifeq ($(BUILDMODE),release)
# Unofficial builds should be tagged as such
	GIT_ORIGIN_URL := $(shell git remote get-url origin 2>/dev/null)
	ifneq (,$(findstring wiidev/usbloadergx,$(GIT_ORIGIN_URL)))
		CFLAGS += -DGITRELEASE
		CXXFLAGS += -DGITRELEASE
	endif
endif

#---------------------------------------------------------------------------------
# Any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS := -lwolfssl -lcustomfat -lcustomntfs -lcustomext2fs -lvorbisidec -logg \
		-lmad -lfreetype -lgd -ljpeg -lpng -lm -lz -lwiiuse -lwiidrc \
		-lbte -lasnd -logc
#---------------------------------------------------------------------------------
# List of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CURDIR)/portlibs
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export PROJECTDIR	:=	$(CURDIR)
export OUTPUT		:=	$(CURDIR)/$(TARGET)
export VPATH		:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
						$(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR		:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# Automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CREATEFILES		:=	$(shell bash ./makexml.sh)
IMPORTFILES		:=	$(shell bash ./filelist.sh)
export CFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(sort $(wildcard $(dir)/*.c))))
export CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(sort $(wildcard $(dir)/*.cpp))))
sFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(sort $(wildcard $(dir)/*.s))))
SFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(sort $(wildcard $(dir)/*.S))))
DATAFILES		:=	$(foreach dir,$(DATA),$(notdir $(sort $(wildcard $(dir)/*.*))))

#---------------------------------------------------------------------------------
# Use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(DATAFILES)) \
					$(CURDIR)/data/magic_patcher.o \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# Build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# Build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(CURDIR)/source/libs/libdrc/ \
					-L$(CURDIR)/source/libs/libext2fs -L$(CURDIR)/source/libs/libfat \
					-L$(CURDIR)/source/libs/libntfs \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) channel lang theme all clean deploy zip reload release

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
channel:
	@[ -d build ] || mkdir -p build
	@$(MAKE) BUILDMODE=channel --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
release:
	@[ -d build ] || mkdir -p build
	@$(MAKE) BUILDMODE=release --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

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
	@echo Cleaning...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol usbloader_gx.zip usbloader_gx

#---------------------------------------------------------------------------------
deploy:
	$(MAKE)
	@echo Deploying...
	@[ -d usbloader_gx ] || mkdir -p usbloader_gx
	@cp $(TARGET).dol usbloader_gx/
	@cp HBC/icon.png usbloader_gx/
	@cp HBC/meta.xml usbloader_gx/
	@zip usbloader_gx.zip usbloader_gx/*
	@wiiload usbloader_gx.zip

#---------------------------------------------------------------------------------
zip:
	$(MAKE)
	@echo Creating zip file...
	@[ -d usbloader_gx ] || mkdir -p usbloader_gx
	@cp $(TARGET).dol usbloader_gx/
	@cp $(TARGET).elf usbloader_gx/
	@cp HBC/icon.png usbloader_gx/
	@cp HBC/meta.xml usbloader_gx/
	@zip usbloader_gx.zip usbloader_gx/*

#---------------------------------------------------------------------------------
reload:
	@wiiload -r $(OUTPUT).dol

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# Main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

language: $(wildcard $(PROJECTDIR)/Languages/*.lang) $(wildcard $(PROJECTDIR)/Themes/*.them)
#---------------------------------------------------------------------------------
# This rule links in binary data with .ttf, .png, and .mp3 extensions
#---------------------------------------------------------------------------------

%.elf.o : %.elf
	@echo $(notdir $<)
	$(bin2o)

%.dol.o : %.dol
	@echo $(notdir $<)
	$(bin2o)

%.ttf.o : %.ttf
	@echo $(notdir $<)
	$(bin2o)

%.png.o : %.png
	@echo $(notdir $<)
	$(bin2o)

%.ogg.o : %.ogg
	@echo $(notdir $<)
	$(bin2o)

%.bin.o	:	%.bin
	@echo $(notdir $<)
	$(bin2o)
	
%.bnr.o	:	%.bnr
	@echo $(notdir $<)
	$(bin2o)

export PATH		:=	$(PROJECTDIR)/gettext-bin:$(PATH)

%.pot: $(CFILES) $(CPPFILES)
	@echo Updating language files...
	@touch $(PROJECTDIR)/Languages/$(TARGET).pot
	@xgettext -C -cTRANSLATORS --from-code=utf-8 --sort-output --no-wrap --no-location -ktr -ktrNOOP -o$(PROJECTDIR)/Languages/$(TARGET).pot -p $@ $^
	@echo Updating theme files...
	@touch $(PROJECTDIR)/Themes/$(TARGET).pot
	@xgettext -C -cTRANSLATORS --from-code=utf-8 -F --no-wrap --add-location -kthInt -kthFloat -kthColor -kthAlign -o$(PROJECTDIR)/Themes/$(TARGET).pot -p $@ $^

%.lang: $(PROJECTDIR)/Languages/$(TARGET).pot
	@msgmerge -U -N --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

%.them: $(PROJECTDIR)/Themes/$(TARGET).pot
	@msgmerge -U -N --no-wrap --no-location --backup=none -q $@ $<
	@touch $@
	@bash $(PROJECTDIR)/updatetheme.sh $(PROJECTDIR)/source/themes/gettheme.c $(PROJECTDIR)/Themes/Default.them

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
