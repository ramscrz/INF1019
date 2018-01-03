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

// Struct representing a FIFO.
struct storage {

	int *buffer;	// Buffer storing FIFO's elements
	int max;			// Maximum number of stored elements
	int nelem;		// Current number of stored elements
	int head;		// Head of the FIFO.

};

typedef struct storage Storage;

// Function for the Consumer's activities execution.
void consumer(Storage *data, int *producerPid, int* artefacts) {

	int demand = CONSUMER_MAX - 1;
	int index = 0;

	printf("\nCONSUMER STARTED!\n");

	while(demand >= 0) {

		if(data->nelem == 0) {
			kill(getpid(), SIGSTOP);
		}

		sleep(2);

		index = data->head - data->nelem; 

		if(index >= 0) {
			artefacts[demand] = data->buffer[index];
		} else {
			artefacts[demand] = data->buffer[data->max + index];
			index = data->max + index;
		}

		data->nelem--;

		printf("\nConsumer reading: %d", artefacts[demand]);

		if(data->nelem == data->max - 1) {
			kill(*producerPid, SIGCONT);
		}

		demand--;
	}

	printf("\n\nCONSUMER ENDED!\n\n");

	return;

}

// Function for the Producer's activities execution.
void producer(Storage *data, int *consumerPid, int *artefacts) {

	int demand = PRODUCER_MAX - 1;
	int index = 0;

	printf("\nPRODUCER STARTED!\n");

	while(demand >= 0) {

		artefacts[demand] = rand() % 65;		

		if(data->nelem == data->max) { 
			kill(getpid(), SIGSTOP);
		}

		sleep(1);

		data->buffer[data->head] = artefacts[demand];
		
		if(data->head == data->max - 1) {
			data->head = 0;
		} else {
			data->head++;
		}

		data->nelem++;

		printf("\nProducer writing: %d", artefacts[demand]);

		if(data->nelem == 1) {
			kill(*consumerPid, SIGCONT);
		}

		demand--;
	}

	printf("\n\nPRODUCER FINISHED!\n\n");

	return;

}


int main() {

	pid_t parent = getpid();
	pid_t consumerPid = 0, producerPid = 0;

	int segment = 0, segmentConsumer = 0, segmentPid = 0, segmentProducer = 0, segmentBuffer = 0;
	int segmentConsumed = 0, segmentProduced = 0;
	int *consumerCreated, *pid, *producerId;
	Storage *data;

	int i = 0;

	int *consumed, *produced;

	if(CONSUMER_MAX > PRODUCER_MAX){
		printf("\n\nERROR: MORE CONSUMPTION THAN PRODUCTION!\n\n");
	}

	segment 				= shmget(IPC_PRIVATE, sizeof(Storage), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	segmentPid 			= shmget(IPC_PRIVATE, sizeof(pid_t), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	segmentConsumer 	= shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	segmentProducer 	= shmget(IPC_PRIVATE, sizeof(pid_t), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	segmentBuffer 		= shmget(IPC_PRIVATE, 8 * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	segmentConsumed	= shmget(IPC_PRIVATE, CONSUMER_MAX * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	segmentProduced   = shmget(IPC_PRIVATE, PRODUCER_MAX * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	if(segmentBuffer < 0 || segment < 0 || segmentPid < 0 || segmentConsumer < 0 
		|| segmentProducer < 0 || segmentProduced < 0 || segmentConsumed < 0) {
		
		shmctl(segment, IPC_RMID, 0);
		shmctl(segmentPid, IPC_RMID, 0);
		shmctl(segmentConsumer, IPC_RMID, 0);
		shmctl(segmentBuffer, IPC_RMID, 0);
		shmctl(segmentConsumed, IPC_RMID, 0);
		shmctl(segmentProduced, IPC_RMID, 0);
		printf("\nERRO 1\n");

		exit(1);
	}

	data 						= (Storage*) 	shmat(segment, 0, 0);
	pid 						= (pid_t*) 		shmat(segmentPid, 0, 0);
	consumerCreated = (int*) 		shmat(segmentConsumer, 0, 0);
	producerId 			= (pid_t*) 		shmat(segmentProducer, 0, 0);
	consumed				= (int*)			shmat(segmentConsumed, 0, 0);
	produced				= (int*)			shmat(segmentProduced, 0, 0);

	if(data < 0 || pid < 0 || consumerCreated < 0 || producerId < 0) {
		printf("\nERRO 2\n");
		shmdt(data);
		shmctl(segment, IPC_RMID, 0);
		shmdt(pid);
		shmctl(segmentPid, IPC_RMID, 0);
		shmdt(consumerCreated);
		shmctl(segmentConsumer, IPC_RMID, 0);
		shmdt(producerId);
		shmctl(segmentProducer, IPC_RMID, 0);
		shmdt(consumed);
		shmctl(segmentConsumed, IPC_RMID, 0);
		shmdt(produced);
		shmctl(segmentProduced, IPC_RMID, 0);

		exit(1);
	}

	data->buffer = (int*) shmat(segmentBuffer, 0, 0);

	if(data->buffer < 0) {
		shmdt(data);
		shmctl(segment, IPC_RMID, 0);
		shmdt(pid);
		shmctl(segmentPid, IPC_RMID, 0);
		shmdt(consumerCreated);
		shmctl(segmentConsumer, IPC_RMID, 0);
		shmdt(producerId);
		shmctl(segmentProducer, IPC_RMID, 0);
		shmctl(segmentBuffer, IPC_RMID, 0);
		shmdt(consumed);
		shmctl(segmentConsumed, IPC_RMID, 0);
		shmdt(produced);
		shmctl(segmentProduced, IPC_RMID, 0);

		exit(1);
	}

	data->max 	= 8;
	data->nelem = 0;
	data->head 	= 0;

	*consumerCreated = 0;

	printf("\n\n===== Producer-Consumer Algorithm =====\n\n");

	producerPid = fork();

	if(producerPid > 0) {

		*producerId = producerPid;

	} else if(producerPid == 0) {

		// Wait for the flag ConsumerCreated to be set.
		while(consumerCreated == 0);
		
		producer(data, pid, produced);

		shmdt(data->buffer);
		shmdt(data);
		shmdt(pid);
		shmdt(consumerCreated);
		shmdt(producerId);
		
		exit(0);

	} else {
		shmdt(data->buffer);
		shmctl(segmentBuffer, IPC_RMID, 0);
		shmdt(data);
		shmctl(segment, IPC_RMID, 0);
		shmdt(pid);
		shmctl(segmentPid, IPC_RMID, 0);
		shmdt(consumerCreated);
		shmctl(segmentConsumer, IPC_RMID, 0);
		shmdt(producerId);
		shmctl(segmentProducer, IPC_RMID, 0);
		shmdt(consumed);
		shmctl(segmentConsumed, IPC_RMID, 0);
		shmdt(produced);
		shmctl(segmentProduced, IPC_RMID, 0);
		
		exit(1);
	}

	consumerPid = fork();

	if(consumerPid > 0) {

		*pid = consumerPid;
		*consumerCreated = 1;
		waitpid(consumerPid, NULL, 0);
		waitpid(*producerId, NULL, 0);

		printf("\n\nProduction Order:\n");
		for(i = 0; i < PRODUCER_MAX; i++) {
			printf("\t%d\n", produced[i]);
		}

		printf("\n\nConsumption Order:\n");
		for(i = 0; i < CONSUMER_MAX; i++) {
			printf("\t%d\n", consumed[i]);
			assert(consumed[i] == produced[i]);
		}

	} else if(consumerPid == 0) {

		consumer(data, &producerPid, consumed);
		
		shmdt(data->buffer);
		shmdt(data);
		shmdt(pid);
		shmdt(consumerCreated);
		shmdt(producerId);
		exit(0);

	} else {
		shmdt(data->buffer);
		shmctl(segmentBuffer, IPC_RMID, 0);
		shmdt(data);
		shmctl(segment, IPC_RMID, 0);
		shmdt(pid);
		shmctl(segmentPid, IPC_RMID, 0);
		shmdt(consumerCreated);
		shmctl(segmentConsumer, IPC_RMID, 0);
		shmdt(producerId);
		shmctl(segmentProducer, IPC_RMID, 0);
		shmdt(consumed);
		shmctl(segmentConsumed, IPC_RMID, 0);
		shmdt(produced);
		shmctl(segmentProduced, IPC_RMID, 0);

		exit(1);
	}

	printf("\n\n ===== END OF EXECUTION! ===== \n\n");

	shmdt(data->buffer);
	shmctl(segmentBuffer, IPC_RMID, 0);
	shmdt(data);
	shmctl(segment, IPC_RMID, 0);
	shmdt(pid);
	shmctl(segmentPid, IPC_RMID, 0);
	shmdt(consumerCreated);
	shmctl(segmentConsumer, IPC_RMID, 0);
	shmdt(producerId);
	shmctl(segmentProducer, IPC_RMID, 0);
	shmdt(consumed);
	shmctl(segmentConsumed, IPC_RMID, 0);
	shmdt(produced);
	shmctl(segmentProduced, IPC_RMID, 0);

	return 0;

}


