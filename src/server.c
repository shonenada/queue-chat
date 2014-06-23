#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "structs.h"
#include "parser.h"
 
ServerEnv serverEnv;

int loadFromFile() {
    int i;
    ServerEnv* env = &serverEnv;
    FILE* file = NULL;

    if ((file = fopen(USER_LIST_FILENAME, "r")) == NULL) {
        perror("Error Raised.\n");
        exit(EXIT_FAILURE);
    }

    fread(&(env->userCount), sizeof(int), 1, file);
    fread(env->userList, sizeof(User), env->userCount, file);

    fclose(file);

    return 1;
}

int saveToFile() {
    FILE* file = NULL;
    ServerEnv* env = &serverEnv;

    if ((file = fopen(USER_LIST_FILENAME, "w+")) == NULL) {
        perror("Error Raised.\n");
        exit(EXIT_FAILURE);
    }

    fwrite(&(env->userCount), sizeof(int), 1, file);
    fwrite(env->userList, sizeof(User), env->userCount, file);

    fclose(file);

    return 1;
}

void beforeExit(int sig) {
    saveToFile();
    exit(sig);
}

int main(int argc, char * argv[]) {

    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("Fork\n");
        exit(EXIT_FAILURE);
    }
    if (pid != 0) {
        exit(0);
    }

    pid = setsid();
    if (pid < -1) {
        perror("Setsid\n");
        exit(EXIT_FAILURE);
    }

    int dev_fd = open("/dev/null", O_RDWR, 0);
    if (dev_fd == -1) {
        dup2(dev_fd, STDIN_FILENO);
        dup2(dev_fd, STDOUT_FILENO);
        dup2(dev_fd, STDERR_FILENO);
        if (dev_fd > 2) {
            close(dev_fd);
        }
    }
    umask(0000);

    int res, client_fd;
    Response* response;
    char pipe[200];
    Protocol protocol;
    serverEnv.userCount = 0;

    loadFromFile();

    signal(SIGKILL, beforeExit);
    signal(SIGINT, beforeExit);
    signal(SIGTERM, beforeExit);
 
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