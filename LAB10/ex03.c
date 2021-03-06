#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<assert.h>

#define PRODUCER_MAX 64
#define CONSUMER_MAX 64
#define PRODUCER_THREAD_INDEX 0
#define CONSUMER_THREAD_INDEX 1

// Struct representing a FIFO.
struct storage {

	unsigned int *buffer;	// Buffer storing FIFO's elements
	unsigned int max;			// Maximum number of stored elements
	unsigned int nelem;		// Current number of stored elements
	unsigned int head;		// Head of the FIFO.

};

typedef struct storage Storage;

static Storage data;
static unsigned int produced[PRODUCER_MAX];
static unsigned int consumed[CONSUMER_MAX];
static pthread_mutex_t accesslock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t can_produce = PTHREAD_COND_INITIALIZER;
static pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;

// Function for the Consumer's activities execution.
void * consumer() {

	int demand = CONSUMER_MAX - 1;
	int index = 0;

	printf("\nCONSUMER STARTED!\n");

	while(demand >= 0) {
		
		sleep(2);

		pthread_mutex_lock(&accesslock);

		if(data.nelem == 0) {
			//printf("\n WAIT CONSUMER\n");
			pthread_mutex_lock(&lock);
			pthread_mutex_unlock(&accesslock);
			pthread_cond_wait(&can_consume, &lock);
			pthread_mutex_unlock(&lock);
			pthread_mutex_lock(&accesslock);
		}

		index = data.head - data.nelem;

		if(index >= 0) {
			consumed[demand] = data.buffer[index];
		} else {
			consumed[demand] = data.buffer[data.max + index];
			index = data.max + index;
		}

		data.nelem--;

		printf("\n\tConsumer reading: %d", consumed[demand]);

		if(data.nelem == data.max - 1) {
			//printf("\n LIBERA PRODUCER\n");
			pthread_cond_signal(&can_produce);
		}

		demand--;

		pthread_mutex_unlock(&accesslock);

	}

	printf("\n\nCONSUMER ENDED!\n\n");

	pthread_exit(NULL);

}

// Function for the Producer's activities execution.
void * producer() {

	int demand = PRODUCER_MAX - 1;
	int index = 0;

	printf("\nPRODUCER STARTED!\n");

	while(demand >= 0) {	

		sleep(1);

		pthread_mutex_lock(&accesslock);

		if(data.nelem == data.max) {
			//printf("\n WAIT PRODUCER\n");
			pthread_mutex_lock(&lock);
			pthread_mutex_unlock(&accesslock);
			pthread_cond_wait(&can_produce, &lock);
			pthread_mutex_unlock(&lock);
			pthread_mutex_lock(&accesslock);
		}

		produced[demand] = rand() % 65;	

		data.buffer[data.head] = produced[demand];
		
		if(data.head == data.max - 1) {
			data.head = 0;
		} else {
			data.head++;
		}

		data.nelem++;

		printf("\nProducer writing: %d", produced[demand]);

		if(data.nelem == 1 ) {//&& demand < PRODUCER_MAX - 1) {
			//printf("\nLIBERA CONSUMER\n");
			pthread_cond_signal(&can_consume);
		}

		demand--;

		pthread_mutex_unlock(&accesslock);
	}

	printf("\n\nPRODUCER FINISHED!\n\n");

	pthread_exit(NULL);

}


int main() {

	pid_t parent = getpid();
	unsigned int i = 0;
	pthread_t threads[2];

	if(CONSUMER_MAX > PRODUCER_MAX){
		printf("\n\nERROR: MORE CONSUMPTION THAN PRODUCTION!\n\n");
	}
	
	for(i = 0; i < PRODUCER_MAX; i++) {
		produced[i] = 0;
	}

	for(i = 0; i < CONSUMER_MAX; i++) {
		consumed[i] = 0;
	}

	data.max 	= 8;
	data.buffer = (int*) malloc (data.max * sizeof(int));	
	data.nelem = 0;
	data.head 	= 0;

	printf("\n\n===== Producer-Consumer Algorithm =====\n\n");

	pthread_create(&threads[PRODUCER_THREAD_INDEX], NULL, producer, NULL);
	pthread_create(&threads[CONSUMER_THREAD_INDEX], NULL, consumer, NULL);

	pthread_join(threads[CONSUMER_THREAD_INDEX],NULL);
	pthread_join(threads[PRODUCER_THREAD_INDEX],NULL);

	printf("\n\nProduction Order:\n");
	for(i = 0; i < PRODUCER_MAX; i++) {
		printf("\t%d\n", produced[i]);
	}

	printf("\n\nConsumption Order:\n");
	for(i = 0; i < CONSUMER_MAX; i++) {
		printf("\t%d\n", consumed[i]);
		assert(consumed[i] == produced[i]);
	}

	printf("\n\n ===== END OF EXECUTION! ===== \n\n");

	return 0;

}
