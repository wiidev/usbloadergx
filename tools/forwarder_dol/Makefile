# Forwarder DOL for "Create Wii Menu Channel". Build with build.sh (Docker).
.SUFFIXES:
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

TARGET   := forwarder
BUILD    := build
SOURCES  := source
DATA     := data
INCLUDES :=

CFLAGS   = -Os -Wall $(MACHDEP) $(INCLUDE)
LDFLAGS  = $(MACHDEP) -Wl,-Map,$(notdir $@).map
LIBS     := -lfat -logc -lm
LIBDIRS  := $(PORTLIBS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT  := $(CURDIR)/$(TARGET)
export VPATH   := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export LD := $(CC)
export OFILES_BIN     := $(addsuffix .o,$(BINFILES))
export OFILES_SOURCES := $(CFILES:.c=.o)
export OFILES         := $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES         := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBOGC_LIB)

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
$(OFILES_SOURCES): $(HFILES)

%.bin.o %_bin.h : %.bin
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
