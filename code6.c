#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
const int size = 8192;

void generate(char* buffer, ssize_t len){
  for(int i = 0; i < len;i++) {
    if(buffer[i] >= 'a' && buffer[i] <= 'z' && buffer[i] != 'a' && buffer[i] != 'e' && buffer[i] != 'i' && buffer[i] != 'o' && buffer[i] != 'u') { 
       buffer[i] = (char)(buffer[i] - 32);
    } 
  }
}

int main(int argc, char* argv[]) {
  char buffer[8192];
  int fdOne[2];
  int fdTwo[2];
  if (argc != 3) {
    printf("Wrong number of input arguments: %d instead of 2\n", argc - 1);
    return 0;
  }
  if(pipe(fdOne) < 0) {
    printf("Can't open pipe #1\n");
    return 0;
  }

  if(pipe(fdTwo) < 0) {
    printf("Can't open pipe #2\n");
    return 0;
  }

  int result1 = fork();
  if (result1 < 0){
      printf("Can't create child #1\n");
      return 0;
  }
  if (result1 > 0) {
    sleep(1);
    return 0;
  }
  if (result1 == 0) { //переходим в ребенка
    int result2 = fork();
    if (result2 < 0){
      printf("Child 2: Can't create child #2\n");
      return 0;
    }
    if (result2 > 0) { //первый процесс
    int fd_read, fd_write;
    ssize_t  read_bytes, res;
    if((fd_read = open(argv[1], O_RDONLY)) < 0){ // проверка, что файл с входным текстом получилось открыть
        printf("Child 1: Can't open input file\n");
        return 0;
    }
    read_bytes = read(fd_read, buffer, size);
    if(read_bytes == -1){ // проверка, что размера буфера хватило для записи
        printf("Child 1: Input file is too big\n");
        return 0;
    }
    res = write(fdOne[1], buffer, read_bytes);
    if(res != read_bytes) {
      printf("Child 1: Can't write all string to pipe #1\n");
      return 0;
    }
    if(close(fd_read) < 0){
        printf("Child 1: Can't close input file\n");
        return 0;
    }
    if(close(fdOne[1]) < 0){
      printf("Child 1: Can't close writing side of pipe #1\n"); 
      return 0;
    }
    read_bytes = read(fdTwo[0], buffer, size);
      if(read_bytes == -1){
        printf("Child 1: Can't read from pipe #2\n");
        return 0;
      }
      if((fd_write = open(argv[2], O_WRONLY | O_TRUNC)) < 0){ // проверка, что файл для вывода получилось открыть
        printf("Child 1: Can't open output file\n");
        return 0;
      }
      res = write(fd_write, buffer, read_bytes);
      if(res == -1) {
        printf("Child 1: Can't write all string to output file\n");
        return 0;
      }
      if(close(fd_write) < 0){
        printf("Child 1: Can't close output file\n");
        return 0;
      }
      if(close(fdTwo[0]) < 0){
        printf("Child 1: Can't close reading side of pipe #2\n"); 
        return 0;
      }
      if(close(fdTwo[1]) < 0){
        printf("Child 1: Can't close writing side of pipe #2\n"); 
        return 0;
      }
        printf("OK\n");
        return 0;
    }
    if (result2 == 0) { //второй процесс анализирует и передает первому
      ssize_t  rb, ans;
      rb = read(fdOne[0], buffer, size);
      if(rb == -1){
        printf("Child 2: Can't read from pipe #1\n");
        return 0;
      }
      generate(buffer, rb);
      ans = write(fdTwo[1], buffer, rb);
      if(ans != rb) {
        printf("Child 2: Can't write all string to pipe #2\n");
        return 0;
      }
      if(close(fdOne[0]) < 0){
        printf("Child 2: Can't close reading side of pipe #1\n"); 
        return 0;
      }
      if(close(fdTwo[1]) < 0){
        printf("Child 2: Can't close writing side of pipe #2\n"); 
        return 0;
    }
    return 0;
    }
  }
}
