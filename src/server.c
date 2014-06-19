#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "structs.h"
#include "parser.h"
 
int main(int argc, char * argv[]) {
    int res, client_fd;
    Response* response;
    char pipe[200];
    Protocol protocol;
    ServerEnv serverEnv;
    serverEnv.userCount = 0;
 
    // create FIFO, if necessary 
    if (access(SERVER_FIFO, F_OK) == -1) {
        serverEnv.serverFIFO = mkfifo(SERVER_FIFO, 0777);
        if (serverEnv.serverFIFO != 0) {
            printf("FIFO %s was not created.\n", SERVER_FIFO);
            exit(EXIT_FAILURE);
        }
    }

    // open FIFO for reading
    serverEnv.serverFd = open(SERVER_FIFO, O_RDONLY);
    if (serverEnv.serverFd == -1) {
        printf("Could not open %s for read only access.\n", SERVER_FIFO);
        exit(EXIT_FAILURE);
    }
    printf("\nServer is runing!\n");

    while(1) {
        res = read(serverEnv.serverFd, &protocol, sizeof(Protocol));
        if (res != 0) {
            printf("Protocol: %d - %s", protocol.pid, protocol.msg);
            response = parse(&serverEnv, &protocol);
            sprintf(pipe, CLIENT_FIFO_PATTERN, protocol.pid);
            client_fd = open(pipe, O_WRONLY | O_NONBLOCK);
            write(client_fd, response, sizeof(Response));
            free(response);
            close(client_fd);
        }
    }
    return 0;
}