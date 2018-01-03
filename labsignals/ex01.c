#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/wait.h>

#define TIME 3

// flags
int turn = 1;
int changes = 0;

// pid of the children (processes)
pid_t pidOne = 0, pidTwo = 0;

void changeContext(int signal) {

	if(changes == 10) {

		printf("\n\n======== Contexto foi trocado 10 vezes! ========\n\n");
	
		kill(pidOne, SIGKILL);
		kill(pidTwo, SIGKILL);

		return;
	
	} else {

		if(turn == 1) {
			kill(pidOne, SIGSTOP);
			printf("\n=== Change from process PID<%d> to process PID<%d>\n", pidOne, pidTwo);
			kill(pidTwo, SIGCONT);
			turn = 2;
		} else if(turn == 2) {
			kill(pidTwo, SIGSTOP);
			printf("\n=== Change from process PID<%d> to process PID<%d>\n", pidTwo, pidOne);
			kill(pidOne, SIGCONT);
			turn = 1;
		} else {
			turn = 1;
			kill(pidTwo, SIGSTOP);
			kill(pidOne, SIGCONT);
		}
	}

	alarm(TIME);

	changes++;

	return;

}

int main() {

	signal(SIGALRM, changeContext);	

	pid_t pid = 0;

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

			alarm(TIME);

			waitpid(pidOne, 0, 0);
			waitpid(pidTwo, 0, 0);

		} else {

			kill(getpid(), SIGSTOP);

			while(1) {
				printf("\n\tProcesso 2 sendo executado!\n");
				sleep(TIME + 1);
			}

			exit(0);
		}

	} else {

		while(1) {
			printf("\nProcesso 1 sendo executado!\n");
			sleep(TIME + 1);
		}

		exit(0);

	}
	
	return 0;
}
