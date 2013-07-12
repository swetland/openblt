BLTHOME := .
include make.conf

ifeq ($(BUILDLOG),)
BUILDLOG := `pwd`/build.log
endif

SUBDIRS		:= lib kernel srv boot bin util

image: log subdirs
	./util/bootmaker $(INIFILES) $(LOCALINIFILES) -o boot/openblt.boot

floppy: subdirs
	./util/bootmaker $(INIFILES) $(LOCALINIFILES) -o boot/openblt.boot --floppy

fimage: floppy
	dd if=/dev/zero of=fd.img bs=1024 count=1440
	dd if=boot/openblt.boot of=fd.img conv=notrunc

bfloppy: floppy
	dd if=boot/openblt.boot of=/dev/disk/floppy/raw bs=18k
	
lfloppy: floppy
	dd if=boot/openblt.boot of=/dev/fd0 bs=18k

run: image
	./util/netboot boot/openblt.boot $(IP)

go:
	./util/bootmaker $(INIFILES) $(LOCALINIFILES) -o boot/openblt.boot
	./util/netboot boot/openblt.boot $(IP)

all:: log

log::
	@echo "### `date`" > $(BUILDLOG)

clean::
	@rm -f boot/openblt.boot

include make.actions
