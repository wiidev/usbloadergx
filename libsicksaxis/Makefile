.SUFFIXES:

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules


TARGET	:=	libsicksaxis.a
CFLAGS	= -g -O2 -Wall $(MACHDEP) -I$(LIBOGC_INC)


all: sicksaxis.o
	powerpc-eabi-ar rcs $(TARGET) -c sicksaxis.o
	@echo "Type 'make install' to install"

sicksaxis.o:
	powerpc-eabi-gcc $(CFLAGS) -c sicksaxis.c

install: all
	@echo install ...
	cp $(TARGET) $(LIBOGC_LIB)
	cp sicksaxis.h $(LIBOGC_INC)

clean:
	@echo clean ...
	@rm -fr sicksaxis.o $(TARGET)
