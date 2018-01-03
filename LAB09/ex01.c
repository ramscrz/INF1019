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
#define MAXPROD 64
#define MAXQUEUE 8
#define MSGRD 0400
#define MSGWRT 0200

struct msgbuf {
	long mtype;								/* message type, integer (must be greater than zero). */
	char mtext[MESSAGE_SIZE];	/* message body array. */
};

typedef struct msgbuf message;

int producer(int msgid, int *counter) {

	int item;
	message m;

	item = rand()%100;
	sprintf(m.mtext, "%d", item);
	m.mtype = 1;

	if(msgsnd(msgid, &m, sizeof(m.mtext), 0) == -1) {
		printf("\nError (at producer): message could not be sent! \n%d: %s\n", errno, strerror(errno));
		return -1;
	}

	(*counter)++;

	return item;
}

int consumer(int msgid, int *counter) {

	int item;
	message m;

	if(msgrcv(msgid, &m, sizeof(m.mtext), 1, IPC_NOWAIT) == -1) {

		if(errno == ENOMSG) {
			sleep(10);

			if(msgrcv(msgid, &m, sizeof(m.mtext), 1, IPC_NOWAIT) == -1) {

				if(errno == ENOMSG) {

					printf("\nEmpty Queue!\n");

					exit(1);

				} else {

					printf("\nError (at consumer): message could not be received! \n%d: %s\n", errno, strerror(errno));
					exit(1);
				}
			}

		} else {
			printf("\nError (at consumer): message could not be received! \n%d: %s\n", errno, strerror(errno));
			exit(1);
		}
	}

	item = (int) strtol(m.mtext, NULL, 10);

	if(item < 0) {
		printf("\nError (at consumer): strtol failed!\n");
		exit(1);
	}

	(*counter)++;

	return item;
}

int main(int argc, char **argv) {

	int pid = 0, pidO = 0, counter = 0, queueId = 0, value = 0;

	if(argc == 2) {
		queueId =  msgget(strtol(argv[1], NULL, 10), MSGRD | MSGWRT | IPC_CREAT | IPC_EXCL);
	} else {
		queueId =  msgget(IPC_PRIVATE, MSGRD | MSGWRT | IPC_CREAT | IPC_EXCL);
	}

	struct msqid_ds msgbuf;

	if(queueId == -1) {
		printf("\nError (at main): message queue could be created/allocated.\n");
		exit(-1);
	}

	msgctl(queueId, IPC_SET, &msgbuf);

	msgbuf.msg_qbytes = (MAXQUEUE * MESSAGE_SIZE);

	pidO = fork();

	if(pidO > 0) {

	} else if(pidO == 0) {

		while(counter < MAXPROD) {
			sleep(2);
			printf("\n\tConsumer 1 reads: %d \t=>\t Reading value #%d...\n", consumer(queueId, &counter), counter + 1);
		}

		exit(1);

	} else {
		printf("\nError (at main): child process could not be created.\n");
		msgctl(queueId, IPC_RMID, NULL);
		exit(-1);
	}

	pid = fork();

	if(pid > 0) {

		while(counter < MAXPROD) {
			sleep(1);
			printf("\nProducer writes: %d \t=>\t Writing value #%d...\n", producer(queueId, &counter), counter + 1);
		}

		waitpid(pid, NULL, 0);
		waitpid(pidO, NULL, 0);

	} else if(pid == 0) {

		while(counter < MAXPROD) {

			sleep(2);
			printf("\n\t\tConsumer 2 reads: %d \t=>\t Reading value #%d...\n", consumer(queueId, &counter), counter + 1);
		}

		exit(1);

	} else {
		printf("\nError (at main): child process could not be created.\n");
		msgctl(queueId, IPC_RMID, NULL);
		exit(-1);
	}

	msgctl(queueId, IPC_RMID, NULL);

	return 0;
}
