#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SERVER_PORT     9000
#define BUF_SIZE        1024

void connect_client(int* fd, struct sockaddr_in *addr);
void process_client(int fd);
void print_from_client(char *msg);
void find_ip(char *server_msg, size_t server_msg_size, char *domain);
void standardize_str(char* str_ptr, char str_array[]);
void erro(char *msg);

int main() {
    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;

    connect_client(&fd, &addr); 

    client_addr_size = sizeof(client_addr);
    while (1) {
        while(waitpid(-1,NULL,WNOHANG)>0);
        client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
        printf("Client connected.\n");
        if (client > 0) {
            if (fork() == 0) {
                close(fd);
                process_client(client);
                exit(0);
            }
        close(client);
        }
    }
    return 0;
}

////////////////////// FUNCTIONS ///////////////////////////

void connect_client(int* fd, struct sockaddr_in *addr){
    bzero(addr, sizeof(*addr));

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(SERVER_PORT);
    
    if ( (*fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        erro("in the socket function");

    if (bind(*fd,(struct sockaddr*)addr, sizeof(*addr)) < 0)
        erro("in the bind function");

    if( listen(*fd, 5) < 0)
        erro("in the listen function");
}

void process_client(int client_fd){
    int nread = 0;
    char buffer[BUF_SIZE];
    char client_msg[BUF_SIZE];
    char server_msg[BUF_SIZE];

    write(client_fd, "Welcome to DEI's name server, please enter the domain's name: ", strlen("Welcome to DEI's name server, please enter the domain's name:"));

    while (1) {
        bzero(buffer, BUF_SIZE);
        
        int n = read(client_fd, buffer, BUF_SIZE - 1);
        if (n <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        strcpy(client_msg, buffer);
        char* client_msg_ptr = client_msg;
        standardize_str(client_msg_ptr, client_msg);
		if (strcmp(client_msg, "exit") == 0){
            write(client_fd, "Goodbye!", strlen("Goodbye!"));
			break;
		}

        print_from_client(client_msg);
        find_ip(server_msg, sizeof(server_msg), client_msg);
        write(client_fd, server_msg, strlen(server_msg));
        bzero(client_msg, BUF_SIZE);
        bzero(server_msg, BUF_SIZE);
    }
    close(client_fd);
}

void print_from_client(char *msg){
    char formatted_msg[BUF_SIZE];  
    snprintf(formatted_msg, sizeof(formatted_msg), "Client > %s\n", msg);
    printf("%s", formatted_msg);
}

void find_ip(char *server_msg, size_t server_msg_size, char *domain){
    char line[BUF_SIZE];
    static char ip[BUF_SIZE];
    char dom[BUF_SIZE];

    FILE *file = fopen("IpAddress.txt", "r");
    if (file == NULL) {
        erro("didn't find the file.");
        strncpy(server_msg, "Error reading file.", server_msg_size);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s\t%s", dom, ip);  
        char* dom_ptr = dom;
        standardize_str(dom_ptr, dom);
        char* domain_ptr = domain; 
        standardize_str(domain_ptr, domain);
        if (strcmp(dom, domain) == 0) {  
            char* ip_ptr = ip;
            standardize_str(ip_ptr, ip);
            snprintf(server_msg, server_msg_size, "The domain %s has the associated IP address: %s", domain, ip);
            fclose(file);
            return; 
        }
    }
    fclose(file);
    snprintf(server_msg, server_msg_size, "The domain %s does not have an IP address associated.", domain);
}

void standardize_str(char* str_ptr, char str_array[]){
    if (str_ptr[strlen(str_array) - 1] == '\n') {
        str_ptr[strlen(str_array) - 1] = '\0';
    }
}

void erro(char *msg){
    printf("Erro: %s\n", msg);
    exit(-1);
}
