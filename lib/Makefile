BLTHOME := ../
include $(BLTHOME)make.conf

PRETARGETS	:= obj crt0.o crtb.o c++rt0.o version.o
TARGETS		:= subdirs

SUBDIRS		:= libc libblt libposix libconsole libdl libwin

obj:
	mkdir obj
	ln -fs ../libblt/libblt.a obj
	ln -fs ../libblt/libblt.so obj
	ln -fs ../libc/libc.a obj
	ln -fs ../libc/libc.so obj
	ln -fs ../libc/libkern.a obj
	ln -fs ../libconsole/libconsole.a obj
	ln -fs ../libconsole/libconsole.so obj
	ln -fs ../libposix/libposix.a obj
	ln -fs ../libposix/libposix.so obj
	ln -fs ../libdl/libdl.a obj
	ln -fs ../libdl/libdl.so obj
	ln -fs ../libwin/libwin.a obj
	ln -fs ../libwin/libwin.so obj

clean::
	@rm -rf obj

include $(BLTHOME)make.actions
