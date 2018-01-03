#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<sys/sem.h>
#include<ctype.h>
#include<string.h>
#include"semF.h"
#include<errno.h>

int main() {

	pid_t pid = -1, pidC = -1;

	char **arg;

	int index = 0, semId = 0, segmentSharedData = 0;

	semId = semget(8744, 1, 0666 | IPC_CREAT);
	setSemValue(semId);

	printf("\n\n===== Producer-Consumer Algorithm =====\n\n");

	arg = (char**) malloc(3 * sizeof(char*));

	if(arg == NULL) {
		printf("\nERRO 1\n");
		exit(-1);
	}

	for(index = 0; index < 3; index++) {
		arg[index] = (char*) malloc (30 * sizeof(char));
	}

	sprintf(arg[1], "%d", semId);

	sprintf(arg[2], "%d", 2765 + rand()%50);

	segmentSharedData = shmget(strtol(arg[2], NULL, 10), sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	if(segmentSharedData < 0) {
		printf("Something went wrong! %s\n", strerror(errno));
		exit(-1);
	}

	arg[3] = NULL;

	pidC = fork();

	if(pidC > 0) {

		pid = fork();

		if(pid > 0) {

			waitpid(pid, NULL, 0);

		} else if(pid == 0) {

			strcpy(arg[0], "ex03P");

			execv(arg[0], arg);

			printf("\nERRO PRODUTOR: %s %s\n", arg[0], arg[1]);
		} else {

			exit(1);

		}

		waitpid(pidC, NULL, 0);

	} else if(pidC == 0) {

		strcpy(arg[0], "ex03C");

		execv(arg[0], arg);

		printf("\nERRO CONSUMIDOR: %s %s\n", arg[0], arg[1]);

	} else {

		exit(1);

	}

	printf("\n\n ===== END OF EXECUTION! ===== \n\n");

	shmctl(segmentSharedData, IPC_RMID, 0);

	return 0;

}
