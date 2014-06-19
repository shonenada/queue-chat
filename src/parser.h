#ifndef _PARSER_H
#define _PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "structs.h"

Response* RegHandler(ServerEnv* env, Protocol* protocol) {
    int i, j;
    User newUser;
    Response* rep = (Response*) malloc(sizeof(Response));
    
    // parse username
    i = 0;
    j = 4;
    while(protocol->msg[j] != ' ') {
        newUser.username[i] = protocol->msg[j];
        i++;
        j++;
    }
    newUser.username[i] = '\0';

    if (isUsernameExist(env, newUser.username)) {
        printf("%d\n", REG_USERNAME_EXIST);
        rep->state = REG_USERNAME_EXIST;
        sprintf(rep->msg, "Username exists, please login.\n");
        return rep;
    }

    // parse password
    i = 0;
    j += 1;
    while(protocol->msg[j] != ' ' && protocol->msg[j] != '\n') {
        newUser.password[i] = protocol->msg[j];
        i++;
        j++;
    }
    newUser.password[i] = '\0';

    if (regUser(env, newUser)) {
        rep->state = REG_SUCCESS;
        sprintf(rep->msg, "Reg Success\n");
        return rep;
    }
    else {
        rep->state = REG_UNSUCCESS;
        sprintf(rep->msg, "Reg Unsuccess\n");
        return rep;
    }

    rep->state = REG_UNKNOWN;
    sprintf(rep->msg, "Reg Unknown\n");
    return rep;
}

int LoginHandler(ServerEnv* env, Protocol* protocol) {
    int i, j;
    char username[32];
    char password[32];
    i = 0;
    j = 4;
    while(protocol->msg[j] != ' ') {
        username[i] = protocol->msg[j];
        i++;
        j++;
    }
    username[i] = '\0';

    i = 0;
    j += 1;
    while(protocol->msg[j] != ' ' && protocol->msg[j] != '\n') {
        password[i] = protocol->msg[j];
        i++;
        j++;
    }

    if(loginUser(env, username, password, protocol->pid)) {
        return 1;
    }
    else {
        return 0;
    }

    return -1;
}

int IndirectChatHandler(ServerEnv* env, Protocol* protocol) {
    int i, j;
    char msg[250];
    i = 0;
    j = 5;
    while(protocol->msg[j] != ' ' && protocol->msg[j] != '\n') {
        msg[i] = protocol->msg[j];
        i++;
        j++;
    }
    msg[i] = '\0';

    User* user = findUserByPid(env, protocol->pid);
    int client_pid;
    int client_fd;
    char pipe[200];
    char buffer[1024];
    sprintf(buffer, "%s say: %s", user->username, msg);
    for (i=0; i<env->userCount; ++i) {
        client_pid = env->online[i];
        if (client_pid > 0){
            sprintf(pipe, "/tmp/client_%d_fifo", env->online[i]);
            client_fd = open(pipe, O_WRONLY | O_NONBLOCK);
            write(client_fd, buffer, strlen(buffer) + 1);
            close(client_fd);
        }
    }
}

int DirectChatHandler(ServerEnv* env, Protocol* protocol) {
    int i, j;
    char directUsername[32];
    char msg[250];

    i = 0;
    j = 6;
    while(protocol->msg[j] != ' ' && i < 31) {
        directUsername[i] = protocol->msg[j];
        i++;
        j++;
    }
    directUsername[i] = '\0';

    i = 0;
    j += 1;
    while(protocol->msg[j] != ' ' && protocol->msg[j] != '\n') {
        msg[i] = protocol->msg[j];
        i++;
        j++;
    }
    msg[i] = '\0';
    
    User *fromUser = findUserByPid(env, protocol->pid);
    int toUserId = findUserIdByUsername(env, directUsername);
    int client_fd;
    int client_pid;
    char pipe[200];
    char buffer[1024];
    client_pid = env->online[toUserId];
    sprintf(buffer, "%s talk to you: %s", fromUser->username, msg);
    sprintf(pipe, "/tmp/client_%d_fifo", client_pid);
    client_fd = open(pipe, O_WRONLY | O_NONBLOCK);
    write(client_fd, buffer, strlen(buffer) + 1);
    close(client_fd);
}

Response* parse(ServerEnv* env, Protocol* protocol) {
    int ret;
    char firstChar = protocol->msg[0];
    switch (firstChar) {
        case 'R':
            return RegHandler(env, protocol);
        break;
        // // case 'L':
        //     ret = LoginHandler(env, protocol);
        //     return ret;
        // break;
        // case 'C':
        //     if (protocol->msg[5] == '@') {
        //         ret = IndirectChatHandler(env, protocol);
        //     }
        //     else {
        //         ret = DirectChatHandler(env, protocol);
        //     }
        //     return ret;
        // break;
    }
}

#endif // _PARSER_H
