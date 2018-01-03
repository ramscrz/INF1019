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

  if(argc != 2) {
    printf("\nInvalid number of parameters! Run: %s <PATH_OF_DIRECTORY/FILE>\n\n", argv[0]);
    exit(-1);
  }

	if(stat(argv[1], &fileData) == -1) {
		printf("\nError: file %s does not exist.\n\n", argv[1]);
		exit(-1);
	}

	printf("\n==============================================\n");
	printf("\n\tAttributes of File: %s\n", argv[1]);
	printf("\n==============================================\n");

	printf("\nID of device containing file: %ld", fileData.st_dev);
	printf("\ninode number: %ld", fileData.st_ino);
	printf("\nProtection: %ld", fileData.st_mode);
	printf("\nNumber of hard links: %ld", fileData.st_nlink);
	printf("\nUser ID of owner: %ld", fileData.st_uid);
	printf("\nGroup ID of owner: %ld", fileData.st_gid);
	printf("\nDevice ID (if special file): %ld", fileData.st_rdev);
	printf("\nTotal size, in bytes: %ld", fileData.st_size);
	printf("\nBlocksize for file system I/O: %ld", fileData.st_blksize);
	printf("\nNumber of 512B blocks allocated: %ld", fileData.st_blocks);
	printf("\nTime of last access: %ld", fileData.st_atime);
	printf("\nTime of last modification: %ld", fileData.st_mtime);
	printf("\nTime of last status change: %ld", fileData.st_ctime);

	printf("\n\n==============================================\n\n");

  return 0;
}
