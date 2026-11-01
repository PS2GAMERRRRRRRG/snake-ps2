EE_BIN   = snake_ps2.elf
EE_OBJS  = main.o game.o render.o input.o font.o sound.o save.o

# مسیرهای جستجوی هدرها
EE_CFLAGS = -O2 -G0 -Wall -D_USE_MATH_DEFINES -I$(PS2DEV)/gsKit/include -I$(PS2SDK)/ports/include -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include

# مسیرهای جستجوی کتابخانه‌ها
EE_LDFLAGS = -L$(PS2DEV)/gsKit/lib -L$(PS2SDK)/ports/lib -L$(PS2SDK)/ee/lib

# نام صحیح کتابخانه dmaKit با حروف کوچک
EE_LIBS  = -lpad -laudsrv -lmc -lgskit -ldraw -lgraph -ldma -ldmakit -lmath3d -lm -lc

all: $(EE_BIN)

clean:
	rm -f *.o *.elf *.a

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
