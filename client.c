#include "seqnum.h"

static char clientFifo[CLIENT_FIFO_NAME_LEN];

static void removeFifo(void){

	unlink(clientFifo);
}

static void createFifo(char* server){

	int clientFd, i=0, bytesread=0;
	char* tempc; /* temp str/char */
	int tempi=0;
	
	tempc =  (char*)malloc(20);
	
	struct request req;
	struct response resp;
	
	req.pid = getpid();
	req.size = 0;
	
	if((clientFd = open(clientFifo, O_RDONLY)) == -1){
		perror("\nFailed to open client file\n");
		return;
	}

	for( ; ; ){
		while(((bytesread = read(clientFd, tempc, 1)) == -1) && (errno == EINTR)) ;

		if(bytesread <= 0)
			break;
		if(tempc[0] == ',' || tempc[0] == '\n' || tempc[0] == EOF){
			req.matrix[i] = tempi;
			req.size += 1;
			i++;
			tempi = 0;
		}else{
			tempi *= 10;
			tempi += tempc[0] - 48;
		}
		bytesread--;
	}
	
	req.size = sqrt(req.size);
	
	if(close(clientFd) == -1){
		perror("\nFailed to close client file\n");
		return;
	}
	
	umask(0);
	
	if(mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
		printf("mkfifo %s", clientFifo);
		
		int serverFd = open(server, WRITE_FLAGS, WRITE_PERMS);
	
	if(serverFd == -1)	printf("open %s\n", server);
	
	char temp;
	free(tempc);
	tempc = (char*)malloc(20);
	
	int length=0;
	
	int len = log10(req.pid) + 1;
	
	int rem=0, num=req.pid;
	
	for(i=0; i<len; i++){
	
		rem = num % 10;
		num = num / 10;
		tempc[len-(i+1)] = rem + '0';
	}
	
	if(write(serverFd, tempc, len) != len)
		perror("Can't write to server");
		
	temp = '\n';
	
	if(write(serverFd, &temp, 1) != 1)
		perror("Can't write to server");
	
	len = log10(req.size) + 1;
	
	rem=0, num=req.size;
	
	for(i=0; i<len; i++){
	
		rem = num % 10;
		num = num / 10;
		tempc[len-(i+1)] = rem + '0';
	}
	
	if(write(serverFd, tempc, len) != len)
		perror("Can't write to server");
		
	temp = '\n';
	
	if(write(serverFd, &temp, 1) != 1)
		perror("Can't write to server");
		
	int j;
		
	for(i=0; i<(req.size*req.size); i++){
	
		len = log10(req.matrix[i]) + 1;
	
		rem=0, num=req.matrix[i];
		
		for(j=0; j<len; j++){
		
			rem = num % 10;
			num = num / 10;
			tempc[len-(j+1)] = rem + '0';
		}
		
		if(write(serverFd, tempc, len) != len)
			perror("Can't write to server");
			
		if(i == (req.size*req.size)-1){
		
			temp = '\n';
		
			if(write(serverFd, &temp, 1) != 1)
				perror("Can't write to server");
		}else{
			temp = ',';
		
			if(write(serverFd, &temp, 1) != 1)
				perror("Can't write to server");
		}
	}
	
	/*clientFd = open(clientFifo, O_RDONLY);
	
	if(clientFd == -1)	printf("open %s", clientFifo);
	
	if(read(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
		perror("Can't read response from server");
		
	if(close(clientFd) == -1){
		perror("\nFailed to close client file\n");
		return;
	}*/

	free(tempc);
	
	if(close(serverFd) == -1){
		perror("\nFailed to close server file\n");
		return;
	}
	
	if(atexit(removeFifo) != 0)	perror("atexit");
	
	return;
}

int main(int argc, char *argv[]){

	char* server;
	
	if(argc != 5){
		perror("\nUsage: Client connects to serverY to see if the input matrix is invertible or not\nformat: ./client -s pathToServerFifo -o pathToDataFile");
		return 1;
	}
	
	if(strcmp(argv[1], "-s") == 0){
		server = (char*)malloc(strlen(argv[2]));
		strcpy(server, argv[2]);
		if(strcmp(argv[3], "-o") == 0){
			strcpy(clientFifo, argv[4]);
		}else{
			perror("\nInvalid command (path to server fifo must be after '-s' and path to data file must be after '-o')\n");
			return 1;
		}
	}
	else if(strcmp(argv[1], "-o") == 0){
		strcpy(clientFifo, argv[2]);
		if(strcmp(argv[3], "-s") == 0){
			server = (char*)malloc(strlen(argv[4]));
			strcpy(server, argv[4]);
		}else{
			perror("\nInvalid command (path to server fifo must be after '-s' and path to data file must be after '-o')\n");
			return 1;
		}
	}else{
		perror("\nInvalid command (path to server fifo must be after '-s' and path to data file must be after '-o')\n");
		return 1;
	}
	
	createFifo(server);
	
	free(server);
	
	return 0;
}
