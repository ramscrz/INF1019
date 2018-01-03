#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/wait.h>
#include<time.h>

int ongoingCall = 0;
clock_t start, end;

void startCall(int signal) {

	if(ongoingCall == 0) {
		start = clock();
		ongoingCall = 1;
	} else {
		printf("\n\nBusy! There is already an ongoing call. Please wait for this call to end.\n\n");
	}

	return;
}

void finishCall(int signal) {
	
	if(ongoingCall == 1) {
		end = clock();

		double time = ((double)(end - start)) / CLOCKS_PER_SEC;

		int cents = 0;

		if(time > 60) {
			cents = (time - 60) + 120;
		} else {
			cents = time * 2;
		}

		int reais = cents / 100;
		int centavos = cents % 100;

		printf("\n\nCall ended! Time: %lf seconds", time);
		printf("\nPrice of the Call: R$%d,%d\n\n", reais, centavos);
		ongoingCall = 0;
	} else {

		printf("\n\nPlease, make a call first! There is no ongoing call at the moment.\n\n");
	}

	return;
}

int main() {

	signal(SIGUSR1, startCall);	
	signal(SIGUSR2, finishCall);

	pid_t pid = 0;

	start = 0;
	end = 0;

	pid = fork();

	if(pid < 0) {

		printf("\n\nfork() call error!\n\n");

		exit(1);

	} else if(pid > 0) {

	} else {

		while(1) {
			
		}

	}
	
	return 0;
}
