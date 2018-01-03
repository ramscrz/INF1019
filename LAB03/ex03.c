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


double* transposta(int rows, int cols, double* A) {

	int i, j, row;
	int n = rows * cols;
	pid_t parentpid = getpid();
	pid_t pid = 0;
	double *T, *a;
	int segment = 0;
	int segmentA = 0;
	pid_t *children = (pid_t*) malloc(rows * sizeof(pid_t));

	double *R = (double*) malloc(n * sizeof(double));

	segment  = shmget(IPC_PRIVATE, sizeof(R), IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR);
	segmentA = shmget(IPC_PRIVATE, sizeof(R), IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR);

	if(segment < 0) {
		exit(1);
	}

	T = (double*) shmat(segment, 0, 0);

	if(T < 0) {
   	shmctl(segment, 0, 0);
		exit(2);
	}

	if(segmentA < 0) {
		shmdt(T);
		shmctl(segment, 0, 0);
		exit(1);
	}

	a = (double*) shmat(segmentA, 0, 0);

	if(a < 0) {
		shmdt(T);
		shmctl(segment, 0, 0);
		shmctl(segmentA, 0, 0);
		exit(2);
	}

	for(i = 0; i < n; i++) {
		a[i] = A[i];
	}

	for(row = 0; row < rows; row++) {

		pid = fork();

		if(pid > 0) {
			//Parent			
			printf("\nPAI <PID %d> atribuindo transposição da linha %d para FILHO <PID %d>...\n", 
						parentpid,
						row,
						pid);
			
			children[row] = pid;

			if(row == rows - 1) {

				for(j = 0; j < rows; j++) {
					waitpid(children[j], 0, 0);
				}

				printf("\n\nMatriz Original:\n");
				printMatrix(rows, cols, a);
				printf("\n\nMatriz Transposta:\n");
				printMatrix(cols, rows, T);
				printf("\n\n");

				free(children);
				children = NULL;

			}

		} else if(pid == 0) {

			printf("\n\tFILHO <PID %d> calculando linha %d da transposta de A...\n", getpid(), row);

			for(i = 0; i < cols; i++) {
				T[i*rows + row] = a[i + row*cols];
			}

			shmdt(T);
			shmdt(a);
			exit(0);
		} else {
			shmdt(T);
			shmctl(segment, IPC_RMID, 0);
			shmdt(a);
			shmctl(segmentA, 0, 0);
			exit(2);
		}
	}

	if(rows > 0) {

		for(row = 0; row < rows; row++) {
			for(i = row*cols; i < row*cols + cols; i++) {
				R[i] = T[i];
			}
		}

	}

	shmdt(T);
	shmctl(segment, 0, 0);
	shmdt(a);
	shmctl(segmentA, 0, 0);

	return R;
}

int main() {

	// Matriz 3x3
	double M[] = {1, 3.9, 2, 2, -3, 2.5, 90, -11, 3.6};

	// Mtriz 4x4
	double A[] = {1, 3.9, 2, 5, 2, -3, 2.5, 5.4, -8, 11, 16.7, -52, 90, -11, 3.6, 22};

	// Matriz 3x5
	double B[] = {23, -45, 8.2, 75.5, 2, 6, 2.9, 3, -8.9, 1.1, 167, -5.2, -9.5, 34, 83.5};

	// Matriz 5x4
	double C[] = {1, 1, 1, 1, 10, 10, 10, 10, 1, 1, 1, 1, 10, 10, 10, 10, 1, 1, 1, 1};

	double *result;

	result = transposta(3, 3, M);

	printf("\n\nT(M) = \n");
	printMatrix(3, 3, result);
	printf("\n\n");

	result = transposta(4, 4, A);

	printf("\n\nT(A) = \n");
	printMatrix(4, 4, result);
	printf("\n\n");

	result = transposta(3, 5, B);

	printf("\n\nT(B) = \n");
	printMatrix(5, 3, result);
	printf("\n\n");

	result = transposta(5, 4, C);

	printf("\n\nT(C) = \n");
	printMatrix(4, 5, result);
	printf("\n\n");

	free(result);
	result = NULL;

	return 0;

}
