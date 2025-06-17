#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <math.h>
#include <semaphore.h>
#include <time.h>

#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define WRITE_PERMS (S_IRUSR | S_IWUSR | S_IWGRP)

#define SERVER_FIFO "/tmp/seqnum_sv"
#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE)+20)

struct request{
	pid_t pid;
	int size; /* size is 3 if matrix is 3x3 */
	int matrix[1000];
	int seqLen;
};

struct response{
	bool invertible;
	int seqNum;
};
