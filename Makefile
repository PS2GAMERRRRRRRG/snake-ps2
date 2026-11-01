EE_BIN   = snake_ps2.elf
EE_OBJS  = main.o game.o render.o input.o font.o sound.o save.o
EE_LIBS  = -lpad -laudsrv -lmc -lgsKit -ldraw -lgraph -ldma -lmath3d -lm -lc

EE_CFLAGS = -O2 -G0 -Wall -D_USE_MATH_DEFINES -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include

all: $(EE_BIN)

clean:
	rm -f *.o *.elf

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
