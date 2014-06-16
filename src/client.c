#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "structs.h"
 
#define FIFO_NAME "/tmp/server_fifo"
#define CLIENT_FIFO_NAME "/tmp/client_fifo"
#define BUFF_SIZE 200
 
char pipe_name [BUFF_SIZE];
 
void handler (int sig) {
    unlink(pipe_name);
    exit (sig);
}
 
void* pthread_read_fifo(void* param) {
    int result;
    int* client_fifo = (int*) param;
    char buffer[BUFF_SIZE];
    while(1) {
        result = read(*client_fifo, buffer, BUFF_SIZE);
        if (result > 0) {
            printf("%s", buffer);
        }
    }
}
 
int main (int argc, char* argv[]) {
    int fd;
    int res;
    int fifo_fd, client_fifo;
    CLIENT_INFO info;
    char buffer[BUFF_SIZE];
    char username[20];
    pthread_t thread_id;
 
    signal (SIGKILL, handler);
    signal (SIGINT, handler);
    signal (SIGTERM, handler);
 
    if (access(FIFO_NAME, F_OK) == -1) {
        printf("Could not open FIFO %s.\n", FIFO_NAME);
        exit (EXIT_FAILURE);
    }
 
    fifo_fd = open(FIFO_NAME, O_WRONLY);
    if (fifo_fd == -1) {
        printf("Could not open %s for write access.\n", FIFO_NAME);
        exit (EXIT_FAILURE);
    }
 
    sprintf(pipe_name, "/tmp/client_%d_fifo", getpid());
    res = mkfifo(pipe_name, 0777);
    if (res != 0) {
        printf("FIFO %s was not created! \n", pipe_name);
        exit (EXIT_FAILURE);
    }
 
    client_fifo = open(pipe_name, O_RDONLY | O_NONBLOCK);
    if (client_fifo == -1) {
        printf("Could not open %s for read only access.\n", FIFO_NAME);
        exit(EXIT_FAILURE);
    }
 
    info.pid = getpid();
    strcpy(info.client_fifo_name, pipe_name);
    printf("Please input your username (less than 10 char): ");
    scanf("%s", &username);
    strcpy(info.username, username);
    printf("Welcome %s!\n", info.username);
 
    printf("Cammands:\n\tquit\tQuit client\n\thelp\tShow Help.\n");
 
    setbuf(stdin, NULL);
    // 清空输入缓存先
 
    pthread_create(&thread_id, NULL, &pthread_read_fifo, &client_fifo);
 
    while (1) {
        printf(">>> Say: ");
        fgets(buffer, BUFF_SIZE, stdin);
        if (strcmp(buffer, "quit\n") == 0) {
            break;
        }
        if (strcmp(buffer, "\n") == 0) {
            continue;
        }
        strcpy(info.content, buffer);
        write(fifo_fd, &info, sizeof(CLIENT_INFO));
    }
 
    close(fifo_fd);
    close(client_fifo);
 
    (void) unlink(pipe_name);
 
    return 0;
 
}