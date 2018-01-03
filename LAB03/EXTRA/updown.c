#include<pthread.h>
#include<stdio.h>
#include <unistd.h>
#include<stdlib.h>

struct params {
	int threadid;
	int *value;
};

typedef struct params Params;

int shared = 10;

void * countUp(void * arguments) {

	Params* args = arguments;
	int i = 0;
	while(1) {
		*(args->value) += 1;
		printf("\nUP Thread %d says: %d", args->threadid, *(args->value));
		sleep(1);
	}

}

void * countDown(void * arguments) {

	Params* args = arguments;
	
	while(1) {
		*(args->value) -= 1;
		printf("\nDOWN Thread %d says: %d", args->threadid, *(args->value));
		sleep(2);
	}

}

int main() {

	pthread_t threads[2];
	int t;
	Params * args1 = (Params*) malloc(sizeof(Params));
	Params * args2 = (Params*) malloc(sizeof(Params));
	args1->threadid = 1;
	args1->value = &shared;
	args2->threadid = 2;
   args2->value = &shared;

	pthread_create(&threads[0], NULL, countUp, (void*)args1);
	pthread_create(&threads[1], NULL, countDown, (void*)args2);

	for(t = 0; t < 2; t++) {
		pthread_join(threads[t], NULL);
	}

	return 0;
}
