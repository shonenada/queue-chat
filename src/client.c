#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "structs.h"
 
int RegController(ClientEnv* env) {
    char username[32];
    char password[32];
    Protocol protocol;
    protocol.pid = getpid();
    printf("Please input your username: ");
    scanf("%s", username);
    printf("Please input your password: ");
    scanf("%s", password);
    sprintf(protocol.msg, "REG %s %s\n", username, password);
    write(env->serverFd, &protocol, sizeof(Protocol));
    return 1;
}

int WaitResponse(ClientEnv* env) {
    int res;
    Response response;
    while(1) {
        res = read(env->clientFd, &response, sizeof(Response));
        if (res > 0) {
            printf("%d %s", response.state, response.msg);
            return 0;
        }
    }
}

int LoginController(ClientEnv* env) {
    char username[32];
    char password[32];
    Protocol protocol;
    protocol.pid = getpid();
    printf("Please input your username: ");
    scanf("%s", username);
    printf("Please input your password: ");
    scanf("%s", password);
    sprintf(protocol.msg, "LOG %s %s\n", username, password);
    write(env->serverFd, &protocol, sizeof(Protocol));
    return 1;
}

void showTips(ClientEnv* env) {
    int cmdInput;
    printf("*****************************\n");
    printf("*  Please Input the number  *\n");
    printf("* 1. Register               *\n");
    printf("* 2. Login                  *\n");
    printf("* 3. Exit                   *\n");
    printf("*****************************\n");
    scanf("%d", &cmdInput);
    switch(cmdInput) {
        case 1:
        RegController(env);
        WaitResponse(env);
        break;
        case 2:
        LoginController(env);
        WaitResponse(env);
        break;
        case 3:
        exit(0);
        break;
        default:
        showTips(env);
        break;
    }
}

int main (int argc, char* argv[]) {

    // Initialize
    ClientEnv clientEnv;
    clientEnv.pid = getpid();

    if (access(SERVER_FIFO, F_OK) == -1) {
        printf("Could not open FIFO %s.\n", SERVER_FIFO);
        exit (EXIT_FAILURE);
    }

    clientEnv.serverFd = open(SERVER_FIFO, O_WRONLY);
    if (clientEnv.serverFd == -1) {
        printf("Could not open %s for write access.\n", SERVER_FIFO);
        exit (EXIT_FAILURE);
    }

    sprintf(clientEnv.pipe, CLIENT_FIFO_PATTERN, getpid());
    clientEnv.clientFIFO = mkfifo(clientEnv.pipe, 0777);
    if (clientEnv.clientFIFO != 0) {
        printf("FIFO %s was not created! \n", clientEnv.pipe);
        exit (EXIT_FAILURE);
    }
 
    clientEnv.clientFd = open(clientEnv.pipe, O_RDONLY | O_NONBLOCK);
    if (clientEnv.clientFd == -1) {
        printf("Could not open %s for read only access.\n", SERVER_FIFO);
        exit(EXIT_FAILURE);
    }

    showTips(&clientEnv);

    /** 
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

    **/

    close(clientEnv.serverFd);
    close(clientEnv.clientFd);

    return 0;

}
