#include<pthread.h>
#include<stdio.h>
#include <unistd.h>
#include<stdlib.h>

struct params {
	int threadid;
	int value;
};

typedef struct params Params;

void * countUp(void * arguments) {

	Params* args = arguments;
	int i = 0;
	int value = args->value;
	int threadid = args->threadid;
	for(i = 0; i <= value; i++) {
		printf("\nUP Thread %d says: %d", threadid, i);
		sleep(1);
	}

}

void * countDown(void * arguments) {

    Params* args = arguments;
    int t = args->value;
    int threadid = args->threadid;
    for(; t >= 0; t--) {
        printf("\nDOWN Thread %d says: %d", threadid, t);
        sleep(2);
    }

}

int main() {

	pthread_t threads[2];
	int t;
	Params* args1 = (Params*) malloc(sizeof(Params));
	Params* args2 = (Params*) malloc(sizeof(Params));
	args1->threadid = 1;
	args1->value = 20;

	printf("\n\nHERE 1 %d %d\n", args1->threadid, args1->value);
	pthread_create(&threads[0], NULL, countUp, (void*)args1);

	args1->threadid = 2;
	args2->value = 20;
	pthread_create(&threads[1], NULL, countDown, (void*)args1);

	for(t = 0; t < 2; t++) {
		pthread_join(threads[t], NULL);
	}

	return 0;
}
