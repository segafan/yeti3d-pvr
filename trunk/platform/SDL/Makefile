#
# Basic KallistiOS skeleton / test program
# (c)2001 Dan Potter
#   

# Put the filename of the output binary here
TARGET = yetiSDL

# You may need to change the paths in these next two lines to reflect locations
# on your own system--this is what works on mine.
SDL_INC_DIR = /usr/include/SDL
SDL_LIB_DIR = /usr/lib/SDL
# List all of your C files here, but change the extension to ".o"
OBJS = main.o ../../src/data.o ../../src/draw.o ../../src/extra.o \
       ../../src/font.o  ../../src/game.o ../../src/maps.o ../../src/model.o \
       ../../src/sprites.o ../../src/yeti.o

all: rm-elf $(TARGET)


clean:
	-rm -f $(TARGET) $(OBJS)

rm-elf:
	-rm -f $(TARGET)

CFLAGS = -g -Wall -I$(SDL_INC_DIR) -D__YETI_SDL__

LDFLAGS = -L$(SDL_LIB_DIR) -lSDL
$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) \
		$(OBJS)

run: $(TARGET)
	./$(TARGET)

dist:
	rm -f $(OBJS)
	$(STRIP) $(TARGET)
