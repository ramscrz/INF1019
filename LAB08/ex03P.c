#include<stdio.h>
#include"semF.h"
#include<unistd.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<string.h>
#include<errno.h>

int main(int argc, char** argv) {

	int segmentSharedData = 0;
	int *data = NULL;

	int index = 0, semId = 0;

	semId = strtol(argv[1], NULL, 10);

	segmentSharedData = shmget(strtol(argv[2], NULL, 10), sizeof(int), S_IRUSR | S_IWUSR);

	if(segmentSharedData < 0) {
		printf("\nERRO 4\n");
		printf("Something went wrong! %s\n", strerror(errno));
		exit(-1);
	}

	data = (int*) shmat(segmentSharedData, 0, 0);

	if(data == NULL) {

		if(data != 0) {
			shmdt(data);
		}

		exit(-1);
	}

	for(index = 0; index < 3; index++) {

		printf("\nPRODUCER WAITING TO ACCESS CRITICAL REGION!\n");

		semaforoP(semId);

		printf("\nPRODUCER ENTERED CRITICAL REGION!\n");

		(*data) += 5;

		printf("\nProducer reads (+5 performed): %d\n", *data);

		semaforoV(semId);

	}

	shmdt(data);

	return 0;

}
