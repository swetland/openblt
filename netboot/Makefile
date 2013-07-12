BLTHOME := ../
include $(BLTHOME)make.conf

CFLAGS 		:= -I$(BLTHOME)include -fno-pic -O2
LIBS 		:= -L$(BLTHOME)lib/obj -lconsole -lkern
ST			:= strip

all: netrom.hex 

load: netrom.hex
	./sersend -b 115200 /dev/ports/serial1 < netrom.hex 

makerom: makerom.c
	gcc -o makerom makerom.c

netrom: netrom.asm
	nasm netrom.asm

netrom.hex: netrom netboot.raw makerom
	cat netrom netboot.raw > temp.raw
	./makerom temp.raw netrom.hex

OBJS = crt0.o netboot.o pci.o ne2000.o 

netboot.raw: $(OBJS)
	$(LD) -dN -Ttext 0x90000 -o netboot.bin  $(OBJS) $(LIBS)
	$(NM) -nC netboot.bin > netboot.map
	objcopy -O binary netboot.bin netboot.raw

download: download.c
	cc -o download download.c

install: download netrom.hex
	./download < netrom.hex

clean:
	rm -f *.o *~ os.* core *.core *.s *.map netboot.bin temp.raw netboot.raw netrom.hex *.boot makerom download
