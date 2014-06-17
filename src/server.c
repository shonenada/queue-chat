#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "structs.h"
 
#define FIFO_NAME "/tmp/server_fifo"
#define CLIENT_FIFO_NAME "/tmp/client_fifo"
 
void handler(int sig) {
    unlink(FIFO_NAME);
    exit(1);
}
 
int add_client(int client_pids[], int client, int total) {
    int i;
    for (i=0;i<total;i++) {
        if (client_pids[i] == client) {
            return total;
        }
    }
    client_pids[total] = client;
    total = total + 1;
    return total;
}

void broadcast(int clients[], CLIENT_INFO info, int total) {
    int i;
    char pipe_name[250];
    char buffer[250];
    int client_fd;
    sprintf(buffer, "%s (%d) say: %s", info.username, info.pid, info.content);
    for (i=0;i<total;i++) {
        sprintf(pipe_name, "/tmp/client_%d_fifo", clients[i]);
        client_fd = open(pipe_name, O_WRONLY | O_NONBLOCK);
        write(client_fd, buffer, strlen(buffer) + 1);
        close(client_fd);
    }
}

void sendTo(int clients[], CLIENT_INFO info, int total, char* target) {
    int i;
    char pipe_name[250];
    char buffer[250];
    int client_fd;
    sprintf(buffer, "%s (%d) say: %s", info.username, info.pid, info.content);
    for (i=0;i<total;i++) {
        if (pid == clients[i]) {
            sprintf(pipe_name, "/tmp/client_%d_fifo", clients[i]);
            client_fd = open(pipe_name, O_WRONLY | O_NONBLOCK);
            write(client_fd, buffer, strlen(buffer) + 1);
            close(client_fd);
            break;
        }
    }
}

int main(int argc, char * argv[]) {
    int i;
    int res;
    int fifo_fd, client_fd;
    CLIENT_INFO info;
    // char buffer[250];
    int client_pids[20];
    int client_total = 0;
 
    signal (SIGKILL, handler);
    signal (SIGINT, handler);
    signal (SIGTERM, handler);
 
    // create FIFO, if necessary 
    if (access(FIFO_NAME, F_OK) == -1) {
        res = mkfifo(FIFO_NAME, 0777);
        if (res != 0) {
            printf("FIFO %s was not created.\n", FIFO_NAME);
            exit(EXIT_FAILURE);
        }
    }
 
    // open FIFO for reading
    fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1) {
        printf("Could not open %s for read only access.\n", FIFO_NAME);
        exit(EXIT_FAILURE);
    }
    printf("\nServer is runing!\n");
 
    while(1) {
        res = read(fifo_fd, &info, sizeof(CLIENT_INFO));
        if (res != 0) {
            client_total = add_client(client_pids, info.pid, client_total);
            if (info.content[0] != '@') {
                broadcast(client_pids, info, client_total);
            }else {
                int j=1;
                char target[200];
                int target_idx=0;
                while(info.content[j] != ' ') {
                    target[target_idx] = info.content[j];
                    j++;
                    target_idx++;
                }
                target[target_idx] = '\0';
                sendTo(client_pids, info, client_total, target);
            }
            printf("%s (%d) say: %s", info.username, info.pid, info.content);
        }
    }

    return 0;

}