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
		printf("\nERRO 3 %d\n", segmentSharedData);
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

		printf("\nCONSUMER WAITING TO ACCESS CRITICAL REGION!\n");		

		semaforoP(semId);

		printf("\nCONSUMER ENTERED CRITICAL REGION!\n");

		(*data)++;

		printf("\nConsumer reads (+1 performed): %d\n", *data);

		semaforoV(semId);

	}

	shmdt(data);

	printf("\n\nCONSUMER ENDED!\n\n");

	return 0;

}
