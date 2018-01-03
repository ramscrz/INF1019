#include<stdio.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>

static void printMatrix(int rows, int cols, double* M) {

	int r, c = 0;
	int n = rows * cols;

	for(r = 0; r < rows; r++) {
		printf("\n| ");
		for(c = r*cols; c < r*cols + cols - 1; c++) {
			printf("\t %.5f", M[c]);
		}
		printf("\t %.5f \t|", M[c]);
	}
}


double* somaMatrizes(int row, int col, double* A, double* B) {

	int i = 0;
	int sum = 0;

	pid_t pidparent = getpid();
	pid_t pidsibling = 0;
	pid_t pid = 0;

	int segmentR, segmentA, segmentB, j, k;
	double *p, *a, *b;
	int n = row * col;
	
	double *result = (double*) malloc(n * sizeof(double));

	segmentR = shmget(IPC_PRIVATE, n * sizeof(double), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	segmentA = shmget(IPC_PRIVATE, n * sizeof(double), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	segmentB = shmget(IPC_PRIVATE, n * sizeof(double), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	
	if(segmentR < 0) {
		exit(1);
	}

	p = (double *) shmat(segmentR, 0, 0);

	if(p < 0) {
		shmctl(segmentR, IPC_RMID, 0);
		exit(1);
	}

	if(segmentA < 0) {
		shmdt(p);
		shmctl(segmentR, IPC_RMID, 0);
		exit(1);
	}

	a = (double *) shmat(segmentA, 0, 0);

	if(a < 0) {
		shmdt(p);
		shmctl(segmentR, IPC_RMID, 0);
		shmctl(segmentA, IPC_RMID, 0);
		exit(1);
	}

	if(segmentB < 0) {
		shmdt(p);
		shmctl(segmentR, IPC_RMID, 0);
		shmdt(a);
		shmctl(segmentA, IPC_RMID, 0);
		exit(1);
	}

	b = (double *) shmat(segmentB, 0, 0);

	if(b < 0) {
		shmdt(p);
		shmctl(segmentR, IPC_RMID, 0);
		shmdt(a);
		shmctl(segmentA, IPC_RMID, 0);
		shmctl(segmentB, IPC_RMID, 0);
		exit(1);
	}

	for(k = 0; k < n; k++) {
		a[k] = A[k];
		b[k] = B[k];
	}


	if(row >= 0) {

		for(k = 0; k < row; k++) {

			pid = fork();

			if(pid > 0) {
				// Parent
				if(k == row - 1) {
					waitpid(pid, 0, 0);
				
					printf("\nPAI <PID %d>: Resultado da Matriz Soma LINHA 0 a LINHA %d :\n", getpid()
		 																												, row);
					for(j = 0; j <= k; j++) {
						printf("| ");
						for(i = j*col; i < (j*col) + col; i++) {
							printf("\t %.5f", p[i]);

							result[i] = p[i];

						}
						printf("\t |\n");
					}
				}

				pidsibling = pid;

			} else if(pid == 0) {
				// Child

				if(k > 0) {
					waitpid(pidsibling, 0, 0);
				}

				printf("\nFILHO <PID %d> de PAI <PID %d> e Irmão <PID %d>: Matriz Soma LINHA %d :\n | ", 
							getpid(), 
							pidparent, 
							pidsibling,
							k + 1);

				for(i = k*col; i < (k*col) + col; i++) {
					p[i] = a[i] + b[i];
					printf("\t %.5f, %d", p[i], i);
				}

				printf("\t|\n");
				shmdt(p);
				shmdt(a);
				shmdt(b);
				exit(0);
			} else {
				shmdt(p);
				shmctl(segmentR, IPC_RMID, 0);
				shmdt(a);
				shmctl(segmentA, IPC_RMID, 0);
				shmdt(b);
				shmctl(segmentB, IPC_RMID, 0);
				exit(2);

			}
		
		}

	}

	shmdt(p);
	shmctl(segmentR, IPC_RMID, 0);
	shmdt(a);
	shmctl(segmentA, IPC_RMID, 0);
	shmdt(b);
	shmctl(segmentB, IPC_RMID, 0);

	return result;
}

int main() {

	double M[] = {1, 3.9, 2, 2, -3, 2.5, 90, -11, 3.6};

	double N[] = {23, -45, 8.2, 2, 6, 2.9, -9.5, 34, 83.5};

	double A[] = {1, 3.9, 2, 5, 2, -3, 2.5, 5.4, -8, 11, 16.7, -52, 90, -11, 3.6, 22};

	double B[] = {23, -45, 8.2, 75.5, 2, 6, 2.9, 3, -8.9, 1.1, 167, -5.2, -9.5, 34, 83.5, 11};

	double C[] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};

	double D[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	double *result;

	printMatrix(3, 3, M);
	printf("\n\n + \n\n");
	printMatrix(3, 3, N);
	printf("\n\n");
	result = somaMatrizes(3, 3, M, N);

	printf("\n\nM + N = R\n");
	printMatrix(3, 3, result);
	printf("\n\n");

	printf("\n\n");
	printMatrix(4, 4, C);
	printf("\n\n + \n\n");
	printMatrix(4, 4, C);
	printf("\n\n");
	result = somaMatrizes(4, 4, C, C);
	
	printf("\n\nC + C = R\n");
	printMatrix(4, 4, result);
	printf("\n\n");

	printf("\n\n");
	printMatrix(4, 4, D);
	printf("\n\n + \n\n");
	printMatrix(4, 4, C);
	printf("\n\n");
	result = somaMatrizes(4, 4, D, C);

	printf("\n\nD + C = R\n");
	printMatrix(4, 4, result);
	printf("\n\n");

	printf("\n\n");
	printMatrix(4, 4, A);
	printf("\n\n + \n\n");
	printMatrix(4, 4, B);
	printf("\n\n");
	result = somaMatrizes(4, 4, A, B);
	printf("\n\n");

	printf("\n\nA + B = R\n");
	printMatrix(4, 4, result);
	printf("\n\n");

	free(result);
	result = NULL;

	return 0;
}
