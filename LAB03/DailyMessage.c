#include<stdio.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/stat.h>

int main(int argc, char **argv) {

	int segment;
	char* p;

	segment = shmget(strtol(argv[1], NULL, 10), 500 * sizeof(char), S_IRUSR);
	
	if(segment < 0) {
		exit(2);
	}

	p = (char*) shmat(segment, 0, 0);

	if(p < 0) {
		exit(2);
	}

	printf("\n\n%sDailyMessage says: %s\n\n", argv[0], p);

	shmdt(p);

	return 0;
}

