# Yeti3D PVR example makefile
# 
# Based on:
# Basic KallistiOS skeleton / test program
# (c)2001 Dan Potter
#   

# Put the filename of the output binary here
TARGET = yetipvr.elf

# List all of your C files here, but change the extension to ".o"
OBJS = main.o pvr_texture.o pvr_sprite.o ../../src/data.o ../../src/draw.o ../../src/extra.o ../../src/font.o \
 ../../src/game.o ../../src/maps.o ../../src/model.o ../../src/sprites.o ../../src/yeti.o

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(TARGET) $(OBJS)

rm-elf:
	-rm -f $(TARGET)

KOS_CFLAGS += -D__PATCH_DRAW_SPRITE__ -D__PATCH_DRAW_CLIPPED_POLY__

$(TARGET): $(OBJS) 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) \
		$(OBJS) $(OBJEXTRA) -lm -lparallax $(KOS_LIBS)

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist:
	-sh-elf-objcopy -R .stack -O binary yetipvr.elf yetipvr.bin
	-./scramble yetipvr.bin 1st_read.bin
	-rm -f $(TARGET)
	-rm -f yetipvr.bin
	-rm -f $(OBJS)

test:	
	-sh-elf-objcopy -R .stack -O binary yetipvr.elf yetipvr.bin
	-./scramble yetipvr.bin 1st_read.bin
	-rm -f $(TARGET)
	-rm -f yetipvr.bin
