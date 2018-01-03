#include<stdio.h>
#include<unistd.h>
#include<wait.h>
#include<stdlib.h>
#include<signal.h>

volatile static int mutex;

void schedulerFeedbackHandler(int signo);

int main(int argc, char **argv) {

	int i = 0, k = 0;
	int cycles = 0;
	int schedulerPid = strtol(argv[argc - 1], NULL, 10);
	mutex = 1;

	signal(SIGUSR2, schedulerFeedbackHandler);

	for(i = 1; i < (argc - 2); i++) {

		cycles = strtol(argv[i], NULL, 10);

		for(k = 0; k < cycles; k++) {
			printf("%d\n", getpid());
			sleep(1);
		}
	
		mutex = 0;
		kill(schedulerPid, SIGUSR1);
		while(mutex == 0);
	}

	cycles = strtol(argv[i], NULL, 10);

	for(k = 0; k < cycles; k++) {
		printf("%d\n", getpid());
		sleep(1);
	}

	return 0;
}

void schedulerFeedbackHandler(int signo) {

	mutex = 1;

}
