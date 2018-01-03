#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>

int main(int argc, char** argv) {

	int file;  

  if(argc != 3) {
    printf("\nInvalid number of parameters! Run: %s <PATH_OF_FILE> <TEXT>\n\n", argv[0]);
    exit(-1);
  }

	file = open(argv[1], O_WRONLY | O_CREAT | O_SYNC, 0700);

	if(file <= 0) {
		printf("\nError: file %s could not be opened.\n\n", argv[1]);
		exit(-1);
	}

	if(write(file, argv[2], strlen(argv[2]) * sizeof(char)) < 0) {
		printf("\nError: write() failed to write to file %s.\n", argv[1]);
		exit(-1);
	}

  return 0;

}
