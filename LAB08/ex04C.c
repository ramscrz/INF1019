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
	char *data = NULL;

	int index = 0, semId = 0;

	semId = strtol(argv[1], NULL, 10);

	segmentSharedData = shmget(strtol(argv[2], NULL, 10), 200 * sizeof(char), S_IRUSR | S_IWUSR);

	if(segmentSharedData < 0) {
		printf("Something went wrong! %s\n", strerror(errno));
		exit(-1);
	}

	data = (char*) shmat(segmentSharedData, 0, 0);

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

		printf("\nReceived Message: %s\n", data);

		if(index < 2) {
			printf("\nEnter a message for the producer:\n");
			scanf(" %[^\n]s", data);
		}

		semaforoV(semId);

		sleep(1);

	}

	shmdt(data);

	printf("\n\nCONSUMER ENDED!\n\n");

	return 0;

}
