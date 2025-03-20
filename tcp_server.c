#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVER_PORT 9000
#define BUF_SIZE 1024

void connect_client(int* fd, struct sockaddr_in *addr);
void process_client(int fd, int client_id);
void print_from_client(int client_id, char *msg);
void find_ip(char *server_msg, size_t server_msg_size, char *domain);
void standardize_str(char* str_ptr, char str_array[]);
void erro(char *msg);

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr, client_addr;
    socklen_t client_addr_size;
    static int client_counter = 0;  // Contador de clientes únicos

    // Evitar procesos zombie
    signal(SIGCHLD, SIG_IGN);

    connect_client(&server_fd, &addr); 
    printf("Server listening on port %d...\n", SERVER_PORT);

    while (1) {
        client_addr_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
        
        if (client_fd < 0) {
            perror("Error in accept");
            continue;
        }

        client_counter++;  // Asignamos un ID único al cliente actual
        printf("New client connected. Assigned ID: %d\n", client_counter);

        pid_t pid = fork();
        if (pid == 0) {  
            close(server_fd);  
            process_client(client_fd, client_counter);  // Pasamos el ID del cliente
            close(client_fd);
            exit(0);
        } 
        else if (pid > 0) {
            close(client_fd);  
        } 
        else {
            perror("Fork failed");
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}

////////////////////// FUNCTIONS ///////////////////////////

void connect_client(int* fd, struct sockaddr_in *addr) {
    bzero(addr, sizeof(*addr));

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(SERVER_PORT);

    if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        erro("Error in socket function");

    if (bind(*fd, (struct sockaddr*)addr, sizeof(*addr)) < 0)
        erro("Error in bind function");

    if (listen(*fd, 5) < 0)
        erro("Error in listen function");
}

void process_client(int client_fd, int client_id) {
    char buffer[BUF_SIZE];
    char client_msg[BUF_SIZE];
    char server_msg[BUF_SIZE];

    write(client_fd, "Welcome to DEI's name server, please enter the domain's name: ", 
          strlen("Welcome to DEI's name server, please enter the domain's name: "));

    while (1) {
        bzero(buffer, BUF_SIZE);

        int n = read(client_fd, buffer, BUF_SIZE - 1);
        if (n <= 0) {
            printf("Client [%d] disconnected.\n", client_id);
            break;
        }

        buffer[n] = '\0';
        strcpy(client_msg, buffer);
        standardize_str(client_msg, client_msg);

        if (strcmp(client_msg, "exit") == 0) {
            printf("Client [%d] disconnected.\n", client_id);
            write(client_fd, "Goodbye!\n", strlen("Goodbye!\n"));
            break;
        }

        print_from_client(client_id, client_msg);
        find_ip(server_msg, sizeof(server_msg), client_msg);
        write(client_fd, server_msg, strlen(server_msg));
    }
}

void print_from_client(int client_id, char *msg) {
    printf("Client [%d] > %s\n", client_id, msg);
}

void find_ip(char *server_msg, size_t server_msg_size, char *domain) {
    char line[BUF_SIZE];
    char ip[BUF_SIZE];
    char dom[BUF_SIZE];

    FILE *file = fopen("IpAddress.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        snprintf(server_msg, server_msg_size, "Error reading file.");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%s %s", dom, ip) != 2)
            continue;

        standardize_str(dom, dom);
        standardize_str(domain, domain);

        if (strcmp(dom, domain) == 0) {
            standardize_str(ip, ip);
            snprintf(server_msg, server_msg_size, "The domain %s has IP address: %s", domain, ip);
            fclose(file);
            return;
        }
    }
    fclose(file);
    snprintf(server_msg, server_msg_size, "The domain %s does not have an associated IP address.", domain);
}

void standardize_str(char* str_ptr, char str_array[]) {
    size_t len = strlen(str_array);
    if (len > 0 && str_ptr[len - 1] == '\n') {
        str_ptr[len - 1] = '\0';
    }
}

void erro(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
