# Yeti3D PVR example makefile
# 
# Based on:
# Basic KallistiOS skeleton / test program
# (c)2001 Dan Potter
#   

# Put the filename of the output binary here
TARGET = yetipvr.elf
BIN = yetipvr.bin
FIRST_READ = 1st_read.bin
CDRECORD_DEV = /dev/scd0

# List all of your C files here, but change the extension to ".o"
OBJS = main.o pvr_texture.o pvr_sprite.o ../../src/data.o ../../src/draw.o ../../src/extra.o ../../src/font.o \
 ../../src/game.o ../../src/maps.o ../../src/model.o ../../src/sprites.o ../../src/yeti.o

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(TARGET) $(OBJS) $(BIN) scramble $(FIRST_READ)
	
scramble: 
	gcc -o scramble ../../tools/scramble.c	

rm-elf:
	-rm -f $(TARGET)

KOS_CFLAGS += -D__PATCH_DRAW_SPRITE__ -D__PATCH_DRAW_CLIPPED_POLY__ -D__PATCH_MD2__

$(TARGET): $(OBJS) 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) \
		$(OBJS) $(OBJEXTRA) -lm -lparallax -ljpeg $(KOS_LIBS)

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist:   $(TARGET) $(BIN) scramble $(FIRST_READ) 
	-rm -f $(TARGET)
	-rm -f $(BIN)
	-rm -f $(OBJS)
	rm -f scramble

$(BIN):	$(TARGET)
	-sh-elf-objcopy -R .stack -O binary $(TARGET) $(BIN)
	
$(FIRST_READ):
	-./scramble $(BIN) $(FIRST_READ)
	-rm -f $(TARGET)
	-rm -f $(BIN)
	
# cdrecord support taken from Makefile from Dreamcast port of Genesis Plus by Stuart Dalton.
# http://genplus-dc.googlecode.com
newcd: $(TARGET) $(BIN) scramble $(FIRST_READ)
	mv $(FIRST_READ) disc/data/1st_read.bin
	mkisofs -l -J -R -o disc/session1.iso disc/data
	rm disc/data/1st_read.bin
	cdrecord dev=$(CDRECORD_DEV) speed=4 gracetime=2 -tao -multi -eject -xa disc/session1.iso
	rm disc/session?.iso

mscd: $(TARGET) $(BIN) scramble $(FIRST_READ)
	mv $(FIRST_READ) disc/data/1st_read.bin
	mkisofs -C `cdrecord dev=$(CDRECORD_DEV) -msinfo` -M $(CDRECORD_DEV) -l -J -R -o disc/session2.iso disc/data
	rm disc/data/1st_read.bin
	cdrecord dev=$(CDRECORD_DEV) speed=4 gracetime=2 -tao -multi -eject -xa disc/session2.iso
	rm disc/session?.iso
	
archives:   $(TARGET) $(BIN) scramble $(FIRST_READ) 
	-rm -f $(TARGET)
	-rm -f $(BIN)
	-rm -f $(OBJS)
	rm -f scramble
	mv $(FIRST_READ) disc/data/1st_read.bin
	tar -czf ~/yeti3dpvr_md2.tgz --exclude-vcs disc/data
	rm disc/data/1st_read.bin
	tar -czf ~/yeti3dpvr_md2_source.tgz --exclude-vcs ../../../yeti3d-pvr
	
