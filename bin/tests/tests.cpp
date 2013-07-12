#include <stdio.h>
#include <stdlib.h>
#include <blt/syscall.h>
#include <blt/os.h>

#define LOOK_ITS_A_RACE 1

#if LOOK_ITS_A_RACE
	int malloc_sem = -1;
#endif

//
//	Test malloc()
//
int malloc_thread(void*)
{
	for (int i = 0; i < 50000; i++){
#if LOOK_ITS_A_RACE
		sem_acquire(malloc_sem);
#endif
		free(malloc(10000));

#if LOOK_ITS_A_RACE
		sem_release(malloc_sem);
#endif
	}

	printf("malloc thread finished\n");		
	os_terminate(0);
	return 0;
}

void malloc_test(void)
{
	printf("starting malloc tests\n");
#if LOOK_ITS_A_RACE
	malloc_sem = sem_create(1,"malloc lock");
#endif

	for (int i = 0; i < 5; i++)
		thr_create(malloc_thread, 0, "malloc_thread");	
}


//
//	producer/consumer
//
struct cl_info {
	char name[32];
	int port;
};

int consumer(void *_p)
{
	int port = ((cl_info*) _p)->port;
	char buffer[10];

	for (;;) {
		msg_hdr_t header;
		header.src = 0;
		header.dst = port;
		header.data = buffer;
		header.size = 10;
		old_port_recv(&header);
		printf("%s receive\n", ((cl_info*) _p)->name);
	}
}

int producer(void *_p)
{
	int port = ((cl_info*) _p)->port;
	char buffer[10];

	for (;;) {
		msg_hdr_t header;
		header.src = port;
		header.dst = port;
		header.data = buffer;
		header.size = 10;
		old_port_send(&header);
		printf("%s send\n", ((cl_info*) _p)->name);
	}
}

void prodcons(int num_producers, int num_consumers)
{
	int port = port_create(0, "prodcons");
	printf("%d producers, %d consumers\n", num_producers, num_consumers);
	for (int i = 0; i < num_producers; i++) {
		cl_info *scinfo = new cl_info;
		snprintf(scinfo->name, 32, "producer %d", i + 1);
		scinfo->port = port;
		thr_create(producer, scinfo, scinfo->name);
	}

	for (int i = 0; i < num_consumers; i++) {
		cl_info *scinfo = new cl_info;
		snprintf(scinfo->name, 32, "consumer %d", i + 1);
		scinfo->port = port;
		thr_create(consumer, scinfo, scinfo->name);
	}
}


int main()
{
	// Case 1: malloc tests.
	//	This crashes the kernel when more than 1 thread is doing malloc/
	//	free operations.  Using only one thread, or using my own locks,
	//	reduces the problem, although I still see a reboot once in a while.
	malloc_test();

	// Case 2: single producer, single consumer
	//	The system quickly wedges when I do this.
//	prodcons(1, 1);
	
	// case 3: multi-producer, single consumer
	//	The kernel code looks like it might not handle this case properly.
//	prodcons(2, 1);
	
	// Case 4: multi-consumer, single producer
//	prodcons(1, 2);

	return 0;	
}
