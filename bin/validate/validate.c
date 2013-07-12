
#include <stdio.h>
#include <stdlib.h>
#include <blt/syscall.h>
#include <blt/namer.h>

int send_port, recv_port;


void sender(void)
{
	char buffer[32];
	msg_hdr_t mh;

	os_sleep(20);
		
	for(;;){
		mh.src = send_port;
		mh.dst = recv_port;
		mh.size = 32;
		mh.flags = 0;
		mh.data = buffer;
		
		old_port_send(&mh);
	}
}

void receiver(void)
{
	char buffer[32];
	msg_hdr_t mh;
	
	recv_port = port_create(0,"recv port");
	os_sleep(20);
	
	for(;;){
		mh.src = 0;
		mh.dst = recv_port;
		mh.size = 32;
		mh.flags = 0;
		mh.data = buffer;
		
		old_port_recv(&mh);
	}
}

int port_test(void)
{
	int s,c;
	printf("port_test: starting\n");
	for(c=0;c<1000000;c++){
		if(!(c % 100000)) printf("port_test: %dth port\n",c);
		if((s = port_create(0,"port test")) < 1) {
			printf("port_test: failed (in create) - iteration %d\n",c);
			return 1;
		}
		if(port_destroy(s)){
			printf("port_test: failed (in destroy) - iteration %d\n",c);
			return 1;
		}
	}
	printf("port_test: passed\n");
	return 0;
}

int semid;

void tt_thread(void *data)
{
	sem_release(semid);
	os_terminate(0);
}

int thread_test(void)
{
	int n;
	semid = sem_create(0, "thread_test_step");
	
	for(n=0;n<100000;n++){
		if(!(n % 1000)) printf("thread_test: %dth thread\n",n);
		thr_create(tt_thread, NULL, "thread test");
		sem_acquire(semid);
	}
	
}

int sem_test(void)
{
	int s,c;
	printf("sem_test: starting\n");
	for(c=0;c<1000000;c++){
		if(!(c % 100000)) printf("sem_test: %dth semaphore\n",c);
		if((s = sem_create(1,"sem test")) < 1) {
			printf("sem_test: failed (in create) - iteration %d\n",c);
			return 1;
		}
		if(sem_destroy(s)){
			printf("sem_test: failed (in destroy) - iteration %d\n",c);
			return 1;
		}
	}
	printf("sem_test: passed\n");
	return 0;
}

int main (int argc, char **argv)
{
	int s,c;

#if 0
	send_port = port_create(0,"xmit port");
	os_thread(sender);
	receiver();
#endif

#if 0
	if(sem_test()) return 0;
	if(port_test()) return 0;
#endif

	if(thread_test()) return 0;
	
	return 0;	
}

