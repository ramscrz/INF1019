#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

struct stat fileData;

int main(int argc, char** argv) {
  
  if(argc != 2) {
    printf("\nInvalid number of parameters! Run: %s <NAME _OF_DIRECTORY>\n\n", argv[0]);
    exit(-1);
  }

  if(stat(argv[1], &fileData) == -1) {
    if(mkdir(argv[1], S_IRWXU|S_IRWXG|S_IRWXO) == 0) {
      printf("\nDirectory %s created successfuly!\n\n", argv[1]);
    } else {
      printf("\nError creating directory %s => %s\n\n", argv[1], strerror(errno));
    }
  } else {
    printf("\nDirectory already exists\n\n");
  }

  return 0;

}
