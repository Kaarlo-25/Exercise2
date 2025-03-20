/**********************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado
 * (em argv[2]), escrevendo a palavra predefinida (em argv[3]).
 * USO: >cliente <enderecoServidor>  <porto>  <Palavra>
 **********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE 1024 

void erro(char *msg);
char* send2server(int server_socket, char msg[]);

int main(int argc, char *argv[]) {
	char endServer[100];
	int fd;
	struct sockaddr_in addr;
	struct hostent *hostPtr;
	char buffer[BUF_SIZE];
	int nread;
	
	if (argc != 3) {
		printf("client <host> <port>\n");
		exit(-1);
	}

	strcpy(endServer, argv[1]);
	if ((hostPtr = gethostbyname(endServer)) == 0)
		erro("Couldn't get server's address.\n");

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
	addr.sin_port = htons((short) atoi(argv[2]));

	if ((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		erro("socket");
	if (connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
		erro("connect");

	//welcome message
	bzero(buffer, sizeof(buffer));
	read(fd, buffer, sizeof(buffer));
	printf("Connected to server.\n%s", buffer);
	
	// Receive and send messages
	while (1) {
        fgets(buffer, sizeof(buffer), stdin); 
        send2server(fd, buffer);

        if (strncmp(buffer, "exit", 5) == 0) {
            printf("Disconnecting...\n");
            break;
        }
        bzero(buffer, sizeof(buffer));
        read(fd, buffer, sizeof(buffer));
        printf("%s", buffer);
    }

	close(fd);
	exit(0);
}

char* send2server(int server_socket, char msg[]){
	char formatted_msg[BUF_SIZE];
    snprintf(formatted_msg, sizeof(formatted_msg), "%s", msg);
    write(server_socket, formatted_msg, strlen(formatted_msg));
}

void erro(char *msg) {
	printf("Erro: %s\n", msg);
	exit(-1);
}
