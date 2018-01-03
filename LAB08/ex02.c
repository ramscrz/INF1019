#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<sys/sem.h>
#include<ctype.h>

#define MAX 16

union semun {
	int val;
	struct semid_ds *buf;
	ushort * array;
};

// inicializa o valor do semáforo
int setSemValue(int semId);

// remove o semáforo
void delSemValue(int semId);

// operação P
int semaforoP(int semId);

//operação V
int semaforoV(int semId);

// Function for the Consumer's activities execution.
void consumer(char *data, int semId) {

	int index = 0;

	while(1) {

		printf("\nCONSUMER WAITING TO ACESS CRITICAL REGION!\n");		

		semaforoP(semId);

		printf("\nCONSUMER ENTERED CRITICAL REGION!\n");

		for(index = 0; index < MAX; index++) {
			printf("\nConsumer reading: %c\n", data[index]);
		}

		semaforoV(semId);

		sleep(2);

	}

	printf("\n\nCONSUMER ENDED!\n\n");

}

// Function for the Producer's activities execution.
void producer(char *data, int semId) {

	int index = 0;

	while(1) {		

		printf("\nPRODUCER WAITING TO ACESS CRITICAL REGION!\n");		

		semaforoP(semId);

		printf("\nPRODUCER ENTERED CRITICAL REGION!\n");

		printf("\n\nEnter %d characters: \n\n", MAX);
		for(index = 0; index < MAX; index++) {
			scanf(" %c", &(data[index]));
			fflush(stdout);
			//printf("\nREAD CHAR = %c\n", data[index]);
		}

		semaforoV(semId);

		printf("\n\nPRODUCER ENDED!\n\n");

		sleep(1);

	}

}


int main() {

	pid_t pid = -1;

	int segmentSharedData = 0;
	char *data;

	int i = 0, semId = 0;

	segmentSharedData = shmget(IPC_PRIVATE, MAX * sizeof(char), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	if(segmentSharedData < 0) {
		exit(-1);
	}

	data = (char*) shmat(segmentSharedData, 0, 0);

	shmctl(segmentSharedData, IPC_RMID, 0);

	if(data == NULL) {

		if(data != 0) {
			shmdt(data);
		}

		exit(-1);
	}

	for(i = 0; i < MAX; i++) {

		data[i] = '0';

	}

	semId = semget(8744, 1, 0666 | IPC_CREAT);
	setSemValue(semId);

	printf("\n\n===== Producer-Consumer Algorithm =====\n\n");

	pid = fork();

	if(pid > 0) {

		producer(data, semId);

	} else if(pid == 0) {

		sleep(1);

		consumer(data, semId);

	} else {

		if(data != NULL) {
			shmdt(data);
		}

		exit(1);

	}

	printf("\n\n ===== END OF EXECUTION! ===== \n\n");

	if(data != 0) {
		shmdt(data);
	}

	return 0;

}

int setSemValue(int semId) {
	union semun semUnion;

	semUnion.val = 1;

	return semctl(semId, 0, SETVAL, semUnion);
}

void delSemValue(int semId) {

	union semun semUnion;

	semctl(semId, 0, IPC_RMID, semUnion);
}

int semaforoP(int semId) {

	struct sembuf semB;
	semB.sem_num = 0;
	semB.sem_op = -1;
	semB.sem_flg = SEM_UNDO;
	semop(semId, &semB, 1);

	return 0;
}

int semaforoV(int semId) {

	struct sembuf semB;
	semB.sem_num = 0;
	semB.sem_op = 1;
	semB.sem_flg = SEM_UNDO;
	semop(semId, &semB, 1);

	return 0;
}

