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
				source/libwiigui \
				source/images \
				source/fonts \
				source/sounds \
				source/system \
				source/libwbfs \
				source/language \
				source/mload \
				source/patches \
				source/usbloader \
				source/xml \
				source/network \
				source/settings \
				source/prompts \
				source/wad \
				source/banner \
				source/cheats \
				source/homebrewboot \
				source/themes \
				source/menu \
				source/libfat \
				source/memory \
				source/libntfs \
				source/FileOperations \
				source/ImageOperations \
				source/utils \
				source/utils/minizip \
				source/usbloader/wbfs
DATA		:=	data
INCLUDES	:=	source

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS		=	-g -O4 -Wall $(MACHDEP) $(INCLUDE) -DHAVE_CONFIG_H
CXXFLAGS	=	-Xassembler -aln=$@.lst $(CFLAGS)
LDFLAGS		=	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map,--section-start,.init=0x80B00000,-wrap,malloc,-wrap,free,-wrap,memalign,-wrap,calloc,-wrap,realloc,-wrap,malloc_usable_size
-include $(PROJECTDIR)/Make.config

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS :=  -lpngu -lpng -lgd -lm -lz -lwiiuse -lbte -lasnd -logc -lfreetype -lvorbisidec -lmad -lmxml -ljpeg -lunzip
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
export CFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
ELFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.elf)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))
TTFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ttf)))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))
OGGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ogg)))
PCMFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.pcm)))
DOLFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.dol)))
MP3FILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.mp3)))
	
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o) \
					$(TTFFILES:.ttf=.ttf.o) $(PNGFILES:.png=.png.o) $(addsuffix .o,$(DOLFILES))\
					$(OGGFILES:.ogg=.ogg.o) $(PCMFILES:.pcm=.pcm.o) $(MP3FILES:.mp3=.mp3.o) \
					$(addsuffix .o,$(ELFFILES)) $(CURDIR)/data/magic_patcher.o

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) lang all clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@/bin/bash ./buildtype.sh
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
#	@echo debug...
#	start geckoreader.exe

channel:
	@[ -d build ] || mkdir -p build
	@/bin/bash ./buildtype.sh FULLCHANNEL
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
lang:
	@[ -d build ] || mkdir -p build
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile language

#---------------------------------------------------------------------------------
all:
	@[ -d build ] || mkdir -p build
	@./buildtype.sh
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile language

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

language: $(wildcard $(PROJECTDIR)/Languages/*.lang)
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



export PATH		:=	$(PROJECTDIR)/gettext-bin:$(PATH)

%.pot: $(CFILES) $(CPPFILES)
	@echo Update Language-Files ...
	@xgettext -C -cTRANSLATORS --from-code=utf-8 --sort-output --no-wrap --no-location -k -ktr -ktrNOOP -o $@ $^

%.lang: $(PROJECTDIR)/Languages/$(TARGET).pot
	@msgmerge -U -N --no-wrap --no-location --backup=none -q $@ $<
	@touch $@



-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
