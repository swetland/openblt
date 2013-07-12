BLTHOME := ../../

all: version build image

version:
	echo "0" > version

build:
	$(BLTHOME)/lib/newvers
	mv -f version.c $(BLTHOME)lib
	@cd $(BLTHOME) ; make subdirs

image:
	@( DIR=`pwd` ; cd $(BLTHOME) ; util/bootmaker $$DIR/openblt.ini $$DIR/openblt )

clean:
	rm -f openblt
	@cd $(BLTHOME) ; make clean

