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

void* WaitChatResponse(void* param) {
    int res;
    Response response;
    ClientEnv* env = (ClientEnv*) param;
    while(1) {
        res = read(env->clientFd, &response, sizeof(Response));
        if (res > 0 && response.type == RESPONSE_TYPE_CHT) {
            if (response.state == CHT_TALK) {
                printf("%s", response.msg);
            }
        }
    }
}

int WaitRegResponse(ClientEnv* env) {
    int i;
    int res;
    Response response;
    while(1) {
        res = read(env->clientFd, &response, sizeof(Response));
        if (res > 0 && response.type == RESPONSE_TYPE_REG) {
            printf("%d %s", response.state, response.msg);
            return 1;
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

int WaitLoginResponse(ClientEnv* env) {
    int i;
    int res;
    Response response;
    while(1) {
        res = read(env->clientFd, &response, sizeof(Response));
        if (res > 0 && response.type == RESPONSE_TYPE_LOG) {
            if (response.state == LOG_SUCCESS) {
                i = 0;
                while(response.msg[i] != ':' && i < 31) {
                    env->username[i] = response.msg[i];
                    i++;
                }
                env->username[i] = '\0';
                printf("Login Successfully\n");
                LoopChat(env);
            }
            else {
                printf("%d %s", response.state, response.msg);
            }
            return 1;
        }
    }
}

int LoopChat(ClientEnv* env) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, &WaitChatResponse, env);
    Protocol protocol;
    protocol.pid = getpid();
    char buffer[512];
    while(1) {
        setbuf(stdin, NULL);
        printf(">>> Say: ");
        fgets(buffer, 512, stdin);
        sprintf(protocol.msg, "CHT %s", buffer);
        write(env->serverFd, &protocol, sizeof(Protocol));
    }
}

int showTips() {
    int cmdInput;
    printf("*****************************\n");
    printf("*  Please Input the number  *\n");
    printf("* 1. Register               *\n");
    printf("* 2. Login                  *\n");
    printf("* 3. Exit                   *\n");
    printf("*****************************\n");
    scanf("%d", &cmdInput);
    return cmdInput;
}

void doChoose(ClientEnv* env);
void parseInput(ClientEnv* env, int input) {
    switch(input) {
        case 1:
        RegController(env);
        WaitRegResponse(env);
        doChoose(env);
        break;
        case 2:
        LoginController(env);
        WaitLoginResponse(env);
        break;
        case 3:
        exit(0);
        break;
        default:
        doChoose(env);
        break;
    }
}

void doChoose(ClientEnv* env) {
    int chose;
    chose = showTips();
    parseInput(env, chose);
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

    doChoose(&clientEnv);

    close(clientEnv.serverFd);
    close(clientEnv.clientFd);

    unlink(clientEnv.clientFIFO);

    return 0;

}
