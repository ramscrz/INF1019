#include<stdio.h>
#include<stdlib.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>

#define MESSAGE_SIZE 2
#define MAXCONSONE 32
#define MAXCONSTWO 32
#define MAXQUEUE 8
#define MSGRD 0400
#define MSGWRT 0200

struct msgbuf {
	long mtype;						/* message type, integer (must be greater than zero). */
	char mtext[MESSAGE_SIZE];	/* message body array. */
};

typedef struct msgbuf message;

int producer(int msgid, int msgtype, int *counter) {

	int item;
	message m;

	m.mtype = msgtype;
	item = rand()%100;
	sprintf(m.mtext, "%d", item);

	if(msgsnd(msgid, &m, sizeof(m.mtext), 0) < 0) {
		printf("\nError (at producer): message could not be sent! \n%d: %s\n", errno, strerror(errno));
		exit(-1);
	}

	(*counter)++;

	return item;
}

int consumer(int msgid, int msgtype, int *counter) {

	int item;
	message m;

	if(msgrcv(msgid, &m, sizeof(m.mtext), msgtype, 0) < 0) {
		printf("\nError (at consumer): message could not be received! \n%d: %s\n", errno, strerror(errno));
	exit(-1);
	}

	item = (int) strtol(m.mtext, NULL, 10);

	(*counter)++;

	return item;
}

int main(int argc, char **argv) {

	int pid = 0, pidSecondChild, counter = 0, queueIdOne = 0, queueIdTwo = 0;
	int value = 0, counterOne = 0, counterTwo = 0;

	if(argc == 2) {
		queueIdOne = msgget(strtol(argv[1], NULL, 10), IPC_CREAT | IPC_EXCL | MSGRD | MSGWRT );
	} else {
		queueIdOne =  msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | MSGRD | MSGWRT );
	}

	struct msqid_ds msgbuf;

	if(queueIdOne == -1) {
		printf("\nError (at main): message queue could be created/allocated. Queue One = %d; Queue Two = %d\n", queueIdOne, queueIdOne);
		exit(-1);
	}

	msgctl(queueIdOne, IPC_SET, &msgbuf);

	msgbuf.msg_qbytes = MAXQUEUE * MESSAGE_SIZE;
	
	pid = fork();

	if(pid > 0) {

	} else if(pid == 0) {

		while(counter < MAXCONSONE) {
			sleep(2);
			printf("\n\t\tConsumer 1 <PID %d> reads: %d \t=>\t Reading value #%d...\n", getpid(), consumer(queueIdOne, 1, &counter), counter + 1);
		}

		exit(1);

	} else {
		printf("\nError (at main): child process could not be created.\n");
		msgctl(queueIdOne, IPC_RMID, NULL);
		exit(-1);
	}

	pidSecondChild = fork();

	if(pidSecondChild > 0) {

		while(counterOne < MAXCONSONE || counterTwo < MAXCONSTWO) {
			if(counterOne < MAXCONSONE) {
         	sleep(1);
				printf("\nProducer writes on Queue 1 <Consumer = Process PID %d>: %d \t=>\t Writing value #%d...\n", pid, producer(queueIdOne, 1, &counterOne), counterOne + 1);
			}

			if(counterTwo < MAXCONSTWO) {
				sleep(1);
				printf("\nProducer writes on Queue 2 <Consumer = Process PID %d>: %d \t=>\t Writing value #%d...\n", pidSecondChild, producer(queueIdOne, 2, &counterTwo), counterTwo + 1);
			}
		}

      waitpid(pidSecondChild, NULL, 0);
		waitpid(pid, NULL, 0);

	} else if(pidSecondChild == 0) {

		while(counter < MAXCONSTWO) {
			sleep(2);
			printf("\n\tConsumer 2 <PID %d> reads: %d \t=>\t Reading value #%d...\n", getpid(), consumer(queueIdOne, 2, &counter), counter + 1);
		}

		exit(1);

	} else {
		printf("\nError (at main): child process could not be created.\n");
		msgctl(queueIdOne, IPC_RMID, NULL);
		exit(-1);
	}

	msgctl(queueIdOne, IPC_RMID, NULL);

	return 0;
}
