all:
	gcc -c -g client.c -ansi
	gcc -o client client.o -lm
	gcc -c -g -D_POSIX_C_SOURCE serverY.c become_daemon.c -ansi
	gcc -o serverY serverY.o become_daemon.o -lm -lrt

clean:
	rm -rf *o client
	rm -rf *o serverY
