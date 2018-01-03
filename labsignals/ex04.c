#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/wait.h>

#define TIME_PROCESS_1 1
#define TIME_PROCESS_2 2
#define TIME_PROCESS_3 2

// flags
int turn = 1;

// pid of the children (processes)
pid_t pidOne = 0, pidTwo = 0, pidThree = 0;

void scheduleProcess(int signal) {

	if(turn == 1) {
		kill(pidOne, SIGSTOP);
		printf("\n\n=== Change from process %d (PID<%d>) to process %d (PID<%d>)\n\n", turn, pidOne, 2, pidTwo);
		kill(pidTwo, SIGCONT);
		turn = 2;
		alarm(TIME_PROCESS_2);
	} else if(turn == 2) {
		kill(pidTwo, SIGSTOP);
		printf("\n\n=== Change from process %d (PID<%d>) to process %d (PID<%d>)\n\n", turn, pidTwo, 3, pidThree);
		kill(pidThree, SIGCONT);
		turn = 3;
		alarm(TIME_PROCESS_3);
	} else if(turn == 3) {
		kill(pidThree, SIGSTOP);
      printf("\n\n=== Change from process %d (PID<%d>) to process %d (PID<%d>)\n\n", turn, pidThree, 1, pidOne);
      kill(pidOne, SIGCONT);
      turn = 1;
		alarm(TIME_PROCESS_1);
	} else {
		turn = 1;
		kill(pidTwo, SIGSTOP);
		kill(pidThree, SIGSTOP);
		kill(pidOne, SIGCONT);
		alarm(TIME_PROCESS_1);
	}

}

int main() {

	signal(SIGALRM, scheduleProcess);	

	pid_t pid = 0;

	int a = 0;

	char *parameters[] = {NULL};

	pid = fork();

	if(pid < 0) {

		printf("\n\nfork() call error!\n\n");

		exit(1);

	} else if(pid > 0) {

		pidOne = pid;

		pid = fork();

		if(pid < 0) {

			printf("\n\nfork() call error!\n\n");

			exit(1);

		} else if(pid > 0) {

			pidTwo = pid;

			pid = fork();

			if(pid < 0) {

				printf("\n\nfork() call error!\n\n");
				exit(1);

			} else if(pid > 0) {

				pidThree = pid;

				alarm(TIME_PROCESS_1);

				waitpid(pidOne, 0, 0);
				waitpid(pidTwo, 0, 0);
				waitpid(pidThree, 0, 0);

			} else {

				kill(getpid(), SIGSTOP);

				execv("programThree", parameters);

         	exit(0);

			}

		} else {

			kill(getpid(), SIGSTOP);

			execv("programTwo", parameters);
			
			exit(0);
		}

	} else {

		execv("programOne", parameters);

		exit(0);

	}
	
	return 0;
}
