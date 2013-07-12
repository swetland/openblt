/* Copyright 1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <blt/syscall.h>
#include <blt/namer.h>
#include <blt/qsem.h>
#include <blt/fdl.h>


int main (void)
{
#if 0
	int i, j, k;
	msg_hdr_t mh;
	qsem_t *sem;
	DIR *dir1, *dir2;
	struct dirent *dirent;
	struct stat buf;
	char c;
	void *ptr;
	int (*fn1)(char *), (*fn2)(void);

/*
	dir1 = opendir ("/");
	while ((dirent = readdir (dir1)) != NULL)
		printf ("readdir says %d %s\n", dirent->d_fileno, dirent->d_name);
	closedir (dir1);
	dir2 = opendir ("/");
	while ((dirent = readdir (dir2)) != NULL)
		printf ("readdir says %d %s\n", dirent->d_fileno, dirent->d_name);
	closedir (dir2);
	dir1 = opendir ("/boot");
	while ((dirent = readdir (dir1)) != NULL)
		printf ("readdir says %d %s\n", dirent->d_fileno, dirent->d_name);
	closedir (dir1);
*/
	i = open ("/boot/rc.boot", O_RDONLY, 0);
	printf ("got fd %d\n", i);
/*
	if (i >= 0)
		while (read (i, &c, 1))
			printf ("%c", c);
*/
	close (i);
	i = open ("/boot/rc.boot", O_RDONLY, 0);
	printf ("got fd %d\n", i);
/*
	if (i >= 0)
		while (read (i, &c, 1))
			printf ("%c", c);
*/
	close (i);
/*
	dir1 = opendir ("/portal");
	if (dir1 != NULL)
		while ((dirent = readdir (dir1)) != NULL)
			printf ("readdir says %d %s\n", dirent->d_fileno, dirent->d_name);
	closedir (dir1);
*/
	//i = stat ("/boot/rc.boot", &buf);
/*
	if (i)
		printf ("stat failed\n");
	else
	{
		printf ("stat is good\n");
		printf ("size is %d\n", buf.st_size);
	}
*/
/*
	printf ("going for input...\n");
	for (;;)
	{
		c = getchar ();
		printf ("we got %d\n", c);
	}
*/
/*
	printf ("opening /boot/foo.so\n");
	ptr = dlopen ("/boot/foo.so", 0);
	if (ptr != NULL)
	{
		fn2 = dlsym (ptr, "bar");
		(*fn2) ();
		printf ("closing\n");
		dlclose (ptr);
	}
	else
		printf ("error: %s\n", dlerror ());
*/
#endif

	int *ptr;

	ptr = 1;
	printf ("hello %d\n", *ptr);

	return 0;
}

