# $Id: //depot/blt/etc/rc.boot#2 $
#
# List of servers to run in order to bootstrap the system.  Each
# server must be in the boot filesystem and statically linked since
# the VFS isn't running yet.
#
# Order is important!  These programs don't get run concurrently like
# they do when you run them from the kernel.  If you get the order
# wrong, or if a bug causes one of these servers to crash or not detach
# correctly, your system is probably wedged.

# namer
console
vfs

