#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>

struct stat fileData;

int main(int argc, char** argv) {

	int file;  

  if(argc != 3) {
    printf("\nInvalid number of parameters! Run: %s <PATH_OF_ORIGIN_FILE> <PATH_OF_DEST_FILE>\n\n", argv[0]);
    exit(-1);
  }

	if(stat(argv[1], &fileData) == -1) {
		printf("\nError: origin file %s does not exist.\n\n", argv[1]);
		exit(-1);
	}

	file = open(argv[1], O_WRONLY | O_CREAT | O_SYNC, 0700);

	if(file <= 0) {
		printf("\nError: file %s could not be found in this directory.\n\n", argv[1]);
		exit(-1);
	}

	destFile = open(argv[2], O_WRONLY | O_CREAT | O_SYNC, 0700);

	if(destFile <= 0) {
		printf("\nError: file %s could not be opened in this directory.\n\n", argv[2]);
		close(file);
		exit(-1);
	}

	do {

		if(read(file, &r, 1) < 0) {
			printf("\nError: %s\n", strerror(errno));
			close(file);
			close(destFile);
			exit(-1);
		}

		write(destFile, &r, 1);

	} while(r != '\n');

	close(file);
	close(destFile);

  return 0;

}
