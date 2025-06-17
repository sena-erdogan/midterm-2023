#include "seqnum.h"
#include "become_daemon.h"

void getCofactor(int mat[100][100], int temp[100][100], int p, int q, int n)
{
    int i = 0, j = 0, row, col;
 
    for (row = 0; row < n; row++) {
        for (col = 0; col < n; col++) {
            if (row != p && col != q) {
                temp[i][j++] = mat[row][col];
 
                if (j == n - 1) {
                    j = 0;
                    i++;
                }
            }
        }
    }
}
 
int determinantOfMatrix(int mat[100][100], int n)
{
    int D = 0, f;
    
    if (n == 1)	return mat[0][0];
 
    int temp[100][100];
 
    int sign = 1;
 
    for(f = 0; f < n; f++){
        getCofactor(mat, temp, 0, f, n);
        D += sign * mat[0][f] * determinantOfMatrix(temp, n - 1);
 
        sign = -sign;
    }
 
    return D;
}
 
int isInvertible(struct request req)
{
	if (req.size == 1){
		perror("input matrix size must be greater than or equal to 2");
		exit(1);
	}
	
	int temp[100][100];
	int i, j;
	
	for(i=0; i<req.size; i++){
		for(j=0; j<req.size; j++)	temp[i][j] = req.matrix[(i*req.size)+j];
	}
	
	if (determinantOfMatrix(temp, req.size) != 0)
		return 1;
	else
		return 0;
}

static void handler(){
	exit(1);
}

int serverY(char* server, char* log, int pooly, int poolz, int t){

	becomeDaemon(1);
	
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));

	int clientFd, dummyFd, i=0, j=0, bytesread=1;
	char* clientFifo;
	struct request req;
	struct response resp;
	char* tempc; /* temp char */
	int tempi=0;
	int seqNum = 0;
	int childy[pooly];
	int childz[poolz];
	pid_t ypid;
	pid_t zpid;
	int pfd[2];
	int busyy[pooly]; /* 1 if child is busy */
	int busyz[poolz];
	static int inv=0;
	static int noninv=0;
	
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	
	if (sigaction(SIGINT, &sa, NULL) == -1)	perror("sigaction");
	
	for(i=0; i<pooly; i++)	busyy[i] = 0;
	for(i=0; i<poolz; i++)	busyz[i] = 0;
	
	/*clientFifo = "clientFifo.txt";*/
	
	umask(0);
	
	/*if(mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){
		fprintf(stderr, "mkfifo %s", clientFifo);
	}
	
	clientFd = open(clientFifo, O_WRONLY);
	
	if(clientFd == -1)	perror("clientFifo open error");*/
		
	int serverFd = open(server, O_RDONLY);
	
	if(serverFd == -1)	fprintf(stderr, "open %s", server);
	
	int logFd = open(log, WRITE_FLAGS , WRITE_PERMS);
	
	if(logFd == -1)	fprintf(stderr, "open %s", log);
	
	dummyFd = open(server, O_WRONLY);
	
	if(dummyFd == -1)	fprintf(stderr, "open %s", server);
	
	ypid = getpid();
	
	/*if(write(logFd, "Server Y (", strlen("Server Y (")) != 1)	perror("Can't write to log");
	if(write(logFd, log, strlen(log)) != 1)	perror("Can't write to log");
	if(write(logFd, ", p=", strlen(", p=")) != 1)	perror("Can't write to log");
	if(write(logFd, &pooly, sizeof(pooly)) != 1)	perror("Can't write to log");
	if(write(logFd, ", t=", strlen(", t=")) != 1)	perror("Can't write to log");
	if(write(logFd, &t, sizeof(t)) != 1)	perror("Can't write to log");
	if(write(logFd, ") started\n", strlen(") started\n")) != 1)	perror("Can't write to log");
	if(write(logFd, "Instantiated server Z\n", strlen("Instantiated server Z\n")) != 1)	perror("Can't write to log");*/
	
	time_t ct = time(NULL);
    	struct tm *tm;
	tm = localtime(&ct);
	printf("%s\n", asctime(tm));
	printf("\nServer Y (%s, p=%d, t=%d) started\nInstantiated server Z\n", log, pooly, t);
	
	int k, m;
	setbuf(stdout, NULL);
	while(bytesread >= 0){
		for(k=0; k<pooly+1; k++){
			if(pipe(pfd) == -1)	perror("pipe error");
		}
		for(k=0; k<pooly+1; k++){
			if(busyy[k] == 0){ /* child k is not busy */
				busyy[k] = 1;
				pid_t child_pid;
				child_pid = fork();
				/*if(write(logFd, "Worker PID#", strlen("Worker PID#")) != 1)	perror("Can't write to log");
				if(write(logFd, &child_pid, sizeof(child_pid)) != 1)	perror("Can't write to log");
				if(write(logFd, " is handling client PID#", strlen(" is handling client PID#")) != 1)	perror("Can't write to log");*/
				
				tm = localtime(&ct);
				printf("%s\n", asctime(tm));
				printf("Worker PID#%d is handling client PID#", child_pid);
				
				switch(child_pid){
					case -1:
						perror("child not created\n");
						exit(1);
					case 0: /* child */
						if(close(pfd[1]) == -1)	perror("close");
						tempc =  (char*)malloc(20);
						i=0;
						bytesread=0;
						tempi = 0;
						
						for( ; ; ){
							while(((bytesread = read(pfd[0], tempc, 1)) == -1) && (errno == EINTR)) ;
							if(bytesread <= 0)
								break;
							if(tempc[0] == '\n'){
								req.pid = tempi;
								break;
							}else{
								tempi *= 10;
								tempi += tempc[0] - 48;
							}
							bytesread--;
						}
						
						/*if(write(logFd, &req.pid, sizeof(req.pid)) != 1)	perror("Can't write to log");
						if(write(logFd, ", matrix size ", strlen(", matrix size ")) != 1)	perror("Can't write to log");*/
						
						printf("%d, matrix size ", req.pid);

						tempi = 0;
						
						for( ; ; ){
							while(((bytesread = read(pfd[0], tempc, 1)) == -1) && (errno == EINTR)) ;

							if(bytesread <= 0)
								break;
							if(tempc[0] == '\n'){
								req.size = tempi;
								break;
							}else{
								tempi *= 10;
								tempi += tempc[0] - 48;
							}
							bytesread--;
						}
						
						/*if(write(logFd, &req.size, sizeof(req.size)) != 1)	perror("Can't write to log");
						if(write(logFd, "x", strlen("x")) != 1)	perror("Can't write to log");
						if(write(logFd, &req.size, sizeof(req.size)) != 1)	perror("Can't write to log");
						if(write(logFd, ", pool busy ", strlen(", pool busy ")) != 1)	perror("Can't write to log");
						if(write(logFd, &k, sizeof(k)) != 1)	perror("Can't write to log");
						if(write(logFd, "/", strlen("/")) != 1)	perror("Can't write to log");
						if(write(logFd, &pooly, sizeof(pooly)) != 1)	perror("Can't write to log");
						if(write(logFd, "\n", strlen("\n")) != 1)	perror("Can't write to log");*/
						printf("%dx%d, pool busy %d/%d\n", req.size, req.size, k+1, pooly);
						
						tempi = 0;
						i=0;
						
						for( ; ; ){
							if(i == (req.size*req.size))	break;
							while(((bytesread = read(pfd[0], tempc, 1)) == -1) && (errno == EINTR)) ;

							if(bytesread <= 0){
								break;
							}if(tempc[0] == ',' || tempc[0] == '\n'){
								req.matrix[i] = tempi;
								i++;
								tempi = 0;
							}else{
								tempi *= 10;
								tempi += tempc[0] - 48;
							}
							bytesread--;
						}
						free(tempc);
						sleep(t, 1, "sleep-time");
						if(close(pfd[0]) == -1)	perror("close");
						for(i=0; i<req.size*req.size; i++)	printf("%d ", req.matrix[i]);
						printf("\n");
						
						if(isInvertible(req)){
							inv++;
							tm = localtime(&ct);
							printf("%s\n", asctime(tm));
							printf("Worker PID#%d responding to client PID#", getpid());
							printf("%d: the matrix IS invertible\n", req.pid);
							printf("Total requests handled: %d, %d invertible, %d not.\n", inv+noninv, inv, noninv);
						}else{
							noninv++;
							tm = localtime(&ct);
							printf("%s\n", asctime(tm));
							printf("Worker PID#%d responding to client PID#", getpid());
							printf("%d: the matrix IS NOT invertible\n", req.pid);
							printf("Total requests handled: %d, %d invertible, %d not.\n", inv+noninv, inv, noninv);
						}
						
						busyy[k] = 0;
						_exit(EXIT_SUCCESS);
	
					default: /* parent */
						childy[k] = (int) child_pid;
						tempc =  (char*)malloc(20);
						i=0;
						bytesread=0;
						tempi = 0;
						for( ; ; ){
							while(((bytesread = read(serverFd, tempc, 1)) == -1) && (errno == EINTR)) ;
							if(bytesread <= 0)
								break;
							if(tempc[0] == '\n'){
								req.pid = tempi;
								break;
							}else{
								tempi *= 10;
								tempi += tempc[0] - 48;
							}
							bytesread--;
						}
						
						tempi = 0;
						bytesread = 0;
						for( ; ; ){
							while(((bytesread = read(serverFd, tempc, 1)) == -1) && (errno == EINTR)) ;

							if(bytesread <= 0)
								break;
							if(tempc[0] == '\n'){
								req.size = tempi;
								break;
							}else{
								tempi *= 10;
								tempi += tempc[0] - 48;
							}
							bytesread--;
						}
						tempi = 0;
						i=0;
						bytesread = 0;
						req.matrix[0] = 0;
						for( ; ; ){
							if(i == (req.size*req.size))	break;
							while(((bytesread = read(serverFd, tempc, 1)) == -1) && (errno == EINTR)) ;

							if(bytesread <= 0){
								break;
							}if(tempc[0] == ',' || tempc[0] == '\n'){
								req.matrix[i] = tempi;
								i++;
								tempi = 0;
							}else{
								tempi *= 10;
								tempi += tempc[0] - 48;
							}
							bytesread--;
						}
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
						
						if(write(pfd[1], tempc, len) != len)
							perror("Can't write to pipe");
							
						temp = '\n';
						
						if(write(pfd[1], &temp, 1) != 1)
							perror("Can't write to pipe");
						
						len = log10(req.size) + 1;
						
						rem=0, num=req.size;
						
						for(i=0; i<len; i++){
						
							rem = num % 10;
							num = num / 10;
							tempc[len-(i+1)] = rem + '0';
						}
						
						if(write(pfd[1], tempc, len) != len)
							perror("Can't write to pipe");
							
						temp = '\n';
						
						if(write(pfd[1], &temp, 1) != 1)
							perror("Can't write to pipe");
							
						int j;
							
						for(i=0; i<(req.size*req.size); i++){
						
							len = log10(req.matrix[i]) + 1;
						
							rem=0, num=req.matrix[i];
							
							for(j=0; j<len; j++){
							
								rem = num % 10;
								num = num / 10;
								tempc[len-(j+1)] = rem + '0';
							}
							
							if(write(pfd[1], tempc, len) != len)
								perror("Can't write to pipe");
								
							if(i == (req.size*req.size)-1){
							
								temp = '\n';
							
								if(write(pfd[1], &temp, 1) != 1)
									perror("Can't write to pipe");
							}else{
								temp = ',';
							
								if(write(pfd[1], &temp, 1) != 1)
									perror("Can't write to pipe");
							}
						}
						free(tempc);
						break;
				}
				child_pid = wait(NULL);
				if(child_pid == -1){
					if(errno != ECHILD){
						perror("unexpected wait error");
					}
				}
			}else{ /* all children are busy, serverZ performs the operations */
				becomeDaemon(1);
				
				/*if(write(logFd, "Z:Server Z (", strlen("Z:Server Z (")) != 1)	perror("Can't write to log");
	if(write(logFd, log, strlen(log)) != 1)	perror("Can't write to log");
	if(write(logFd, ", t=", strlen(", t=")) != 1)	perror("Can't write to log");
	if(write(logFd, &t, sizeof(t)) != 1)	perror("Can't write to log");
	if(write(logFd, ", r=", strlen(", r=")) != 1)	perror("Can't write to log");
	if(write(logFd, &poolz, sizeof(poolz)) != 1)	perror("Can't write to log");
	if(write(logFd, ") started\n", strlen(") started\n")) != 1)	perror("Can't write to log");*/
	
				printf("Z:Server Z (%s, t=%d, r=%d) started\n", log, t, poolz);
				
				int flags, opt, fd;
				mode_t perms;
				size_t size = 500;
				char *addr;
				flags = O_RDWR | O_CREAT;
				
				size = 500;
				int len;
				perms = (S_IRUSR | S_IWUSR);
				/* Create shared memory object and set its size */
				fd = shm_open("shared.txt", flags, perms);
				
				if (fd == -1)	perror("shm_open");
				if (ftruncate(fd, size) == -1)	perror("ftruncate");
				
				/* Map shared memory object */
				addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
				if (addr == MAP_FAILED)	perror("mmap");
			
				for(m=0; m<poolz; m++){
					if(busyz[m] == 0){ /* child i is not busy */
						pid_t child_pid;
						child_pid = fork();
						/*if(write(logFd, "Worker PID#", strlen("Worker PID#")) != 1)	perror("Can't write to log");
				if(write(logFd, &child_pid, sizeof(child_pid)) != 1)	perror("Can't write to log");
				if(write(logFd, " is handling client PID#", strlen(" is handling client PID#")) != 1)	perror("Can't write to log");*/
						printf("Worker PID#%d is handling client PID#", getpid());
						
						if(child_pid == -1){
							perror("child not created\n");
							exit(1);
						}else if(child_pid != -1 && child_pid != 0){/*parent*/
							childy[m] = (int) child_pid;
							tempc =  (char*)malloc(20);
							i=0;
							bytesread=0;
							tempi = 0;
							for( ; ; ){
								while(((bytesread = read(serverFd, tempc, 1)) == -1) && (errno == EINTR)) ;
								if(bytesread <= 0)
									break;
								if(tempc[0] == '\n'){
									req.pid = tempi;
									break;
								}else{
									tempi *= 10;
									tempi += tempc[0] - 48;
								}
								bytesread--;
							}
							
							tempi = 0;
							bytesread = 0;
							for( ; ; ){
								while(((bytesread = read(serverFd, tempc, 1)) == -1) && (errno == EINTR)) ;

								if(bytesread <= 0)
									break;
								if(tempc[0] == '\n'){
									req.size = tempi;
									break;
								}else{
									tempi *= 10;
									tempi += tempc[0] - 48;
								}
								bytesread--;
							}
						
							tempi = 0;
							i=0;
							bytesread = 0;
							req.matrix[0] = 0;
							for( ; ; ){
								if(i == (req.size*req.size))	break;
								while(((bytesread = read(serverFd, tempc, 1)) == -1) && (errno == EINTR)) ;

								if(bytesread <= 0){
									break;
								}if(tempc[0] == ',' || tempc[0] == '\n'){
									req.matrix[i] = tempi;
									i++;
									tempi = 0;
								}else{
									tempi *= 10;
									tempi += tempc[0] - 48;
								}
								bytesread--;
							}
							char temp;
							free(tempc);
							tempc = (char*)malloc(20);
							char* str;
							str = (char*)malloc(500);
							
							int length=0;
							
							int len = log10(req.pid) + 1;
							
							int rem=0, num=req.pid;
							
							for(i=0; i<len; i++){
							
								rem = num % 10;
								num = num / 10;
								tempc[len-(i+1)] = rem + '0';
							}
							
							strncat(str, tempc, len);
								
							temp = '\n';
							
							strncat(str, &temp, 1);
							
							len = log10(req.size) + 1;
							
							rem=0, num=req.size;
							
							for(i=0; i<len; i++){
							
								rem = num % 10;
								num = num / 10;
								tempc[len-(i+1)] = rem + '0';
							}
							
							strncat(str, tempc, len);
								
							temp = '\n';
							
							strncat(str, &temp, 1);
								
							int j;
								
							for(i=0; i<(req.size*req.size); i++){
							
								len = log10(req.matrix[i]) + 1;
							
								rem=0, num=req.matrix[i];
								
								for(j=0; j<len; j++){
								
									rem = num % 10;
									num = num / 10;
									tempc[len-(j+1)] = rem + '0';
								}
								
								strncat(str, tempc, len);
									
								if(i == (req.size*req.size)-1){
								
									temp = '\n';
								
									strncat(str, &temp, 1);
								}else{
									temp = ',';
								
									strncat(str, &temp, 1);
								}
							}
							memcpy(addr, str, strlen(str));
							free(tempc);
						}else if(child_pid == 0){ /*child*/
							busyz[m] = 1;
							tempc =  (char*)malloc(20);
							i=0;
							bytesread=0;
							tempi = 0;
							free(tempc);
							sleep(t, 1, "sleep-time");
							char* string;
							
							string = addr;
							
							while(string[i] != '\n' && i<strlen(string)){
								tempi *= 10;
								tempi += string[i] - 48;
								i++;
							}
							
							req.pid = tempi;
							
							/*if(write(logFd, "Forwarding request of client PID#", strlen("Forwarding request of client PID#")) != 1)	perror("Forwarding request of client PID#");
							
							if(write(logFd, &req.pid, sizeof(req.pid)) != 1)	perror("Can't write to log");
							if(write(logFd, ", matrix size ", strlen(", matrix size ")) != 1)	perror("Can't write to log");
							if(write(logFd, &req.size, sizeof(req.size)) != 1)	perror("Can't write to log");
							if(write(logFd, "x", strlen("x")) != 1)	perror("Can't write to log");
							if(write(logFd, &req.size, sizeof(req.size)) != 1)	perror("Can't write to log");
							if(write(logFd, ", pool busy ", strlen(", pool busy ")) != 1)	perror("Can't write to log");
							if(write(logFd, &m, sizeof(m)) != 1)	perror("Can't write to log");
							if(write(logFd, "/", strlen("/")) != 1)	perror("Can't write to log");
							if(write(logFd, &poolz, sizeof(poolz)) != 1)	perror("Can't write to log");
							if(write(logFd, "\n", strlen("\n")) != 1)	perror("Can't write to log");*/
							tm = localtime(&ct);
							printf("%s\n", asctime(tm));
							printf("Z:Forwarding request of client PID#%d, matrix size %dx%d, pool busy %d/%d\n", req.pid, req.size, req.size, m+1, poolz);
							
							tempi = 0;
							
							i++;
							
							while(string[i] != '\n' && i<strlen(string)){
								tempi *= 10;
								tempi += string[i] - 48;
								i++;
							}
							
							req.size = tempi;
							
							tempi = 0;
							j=0;
							i++;
							
							while(string[i] != EOF && i<=strlen(string) && j < (req.size*req.size)){
								if(string[i] == ',' || string[i] == '\n'){
									req.matrix[j] = tempi;
									j++;
									tempi = 0;
								}else{
									tempi *= 10;
									tempi += string[i] - 48;
								}
								i++;
							}
							
							for(i=0; i<req.size*req.size; i++)	printf("%d ", req.matrix[i]);
							printf("\n");
							
							if(isInvertible(req)){
								inv++;
								tm = localtime(&ct);
								printf("%s\n", asctime(tm));
								printf("Z:Worker PID#%d responding to client PID#", getpid());
								printf("%d: the matrix IS invertible\n", req.pid);
								printf("Total requests handled: %d, %d invertible, %d not.\n", inv+noninv, inv, noninv);
							}else{
								noninv++;
								tm = localtime(&ct);
								printf("%s\n", asctime(tm));
								printf("Z:Worker PID#%d responding to client PID#", getpid());
								printf("%d: the matrix IS NOT invertible\n", req.pid);
								printf("Total requests handled: %d, %d invertible, %d not.\n", inv+noninv, inv, noninv);
							}
							
							busyz[m] = 0;
							_exit(EXIT_SUCCESS);
						}
						if (shm_unlink("shared.txt") == -1)	perror("shm_unlink");
						child_pid = wait(NULL);
						if(child_pid == -1){
							if(errno != ECHILD){
								perror("unexpected wait error");
							}
						}
						_exit(EXIT_SUCCESS);
					}else{ /* all children are busy, client needs to wait */
						
						k=0;
					}
				}
				if (close(fd) == -1)
				perror("close");
				exit(EXIT_SUCCESS);
			}
		}
	}

	if(close(dummyFd) == -1){
		perror("\nFailed to close the dummy file\n");
		return 1;
	}
	
	if(close(serverFd) == -1){
		perror("\nFailed to close the server file\n");
		return 1;
	}
	
	if(close(logFd) == -1){
		perror("\nFailed to close the log file\n");
		return 1;
	}
	
	/*if(close(clientFd) == -1){
		perror("\nFailed to close clientFifo\n");
		return 1;
	}*/
	
	return 0;
}

int main(int argc, char *argv[]){

	char* server;
	char* log;
	int pooly, poolz, sleep, i;
	
	if(argc != 11){
		perror("\nUsage: serverY and serverZ are created\nformat: ./serverY -s pathToServerFifo -o pathToLogFile –p poolSize -r poolSize2 -t 2");
		return 1;
	}
	
	for(i=1; i<=10; i+=2){
		if(strcmp(argv[i], "-s") == 0){
			server = (char*)malloc(strlen(argv[i+1]));
			strcpy(server, argv[i+1]);
		}else if(strcmp(argv[i], "-o") == 0){
			log = (char*)malloc(strlen(argv[i+1]));
			strcpy(log, argv[i+1]);
		}else if(strcmp(argv[i], "-p") == 0){
			pooly = atoi(argv[i+1]);
			if(pooly < 2){
				perror("pool size of serverY must be greater than or equal to 2");
				return 1;
			}
		}else if(strcmp(argv[i], "-r") == 0){
			poolz = atoi(argv[i+1]);
			if(poolz < 2){
				perror("pool size of serverZ must be greater than or equal to 2");
				return 1;
			}
		}else if(strcmp(argv[i], "-t") == 0){
			sleep = atoi(argv[i+1]);
		}else{
			perror("\nInvalid command\nformat: ./serverY -s pathToServerFifo -o pathToLogFile –p poolSize -r poolSize2 -t 2\n");
			return 1;
		}
	}
	
	serverY(server, log, pooly, poolz, sleep);
	
	free(server);
	
	return 0;
}
