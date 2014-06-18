#ifndef _STRUCT_H
#define _STRUCT_H

// *** settings ***
#define MAX_USER 20
#define SERVER_FIFO "/tmp/server_fifo"
#define CLIENT_FIFO_PATTERN "/tmp/client_%d_fifo"
// *** end settings ***

/**
 * 请求协议定义
 *  首 3 个字符为 REG LGN CHT 分别表示，注册、登录、发送信息
 *  第 4 个字符为 空格
 *  随后的字符根据不同的 method 有不同的定义。
 *   REG：第 5 个字符起到末尾(\n) 表示欲注册的用户名和密码。用空格分隔开。
 *   LGN：第 5 个字符起到末尾(\n) 表示欲登录的用户名和密码。用空格分隔开。
 *   CHT：第 5 个字符起到末尾，表示发送的信息。
 **/
typedef struct {
    int pid;
    char msg[1024];
} Protocol;

/**
 * 响应协议定义
 *  state 响应的状态。
 *  msg 表示响应的信息。
 **/
typedef struct {
    int state;
    char msg[1024];
} Response;

/**
 * 定义 User 模型
 **/
typedef struct {
    char username[32];
    char password[32];
    char nickname[16];
} User;

/**
 * Server Environment
 */
typedef struct {
    int userCount;
    int onlineCount;
    int serverFIFO;
    int serverFd;
    int clientFd;
    User userList[MAX_USER];
    int online[MAX_USER];
} ServerEnv;

/**
 * Client Environment
 **/
typedef struct {
    int pid;
    int clientFd;
    int serverFd;
    int clientFIFO;
    char pipe[200];
} ClientEnv;



typedef struct {
    int pid;
    char client_fifo_name[100];
    char username[20];
    char content[200];
} client_info, *client_info_ptr;



// Method for User Model

// Find an instnace of User by username
// Return the index of userlist.
// Return -1 if username not found
int findUserIdByUsername(ServerEnv* env, char* username) {
    int i;
    for(i=0; i<env->userCount; ++i) {
        if (strcmp(username, env->userList[i].username) == 0) {
            return i;
        }
    }
    return -1;
}

int findUserIdByPid(ServerEnv* env, int pid) {
    int i;
    for (i=0;i<env->userCount;++i) {
        if (pid == env->online[i]) {
            return i;
        }
    }
    return -1;
}

User* findUserByPid(ServerEnv* env, int pid) {
    int idx = findUserIdByPid(env, pid);
    if (idx == -1) {
        return NULL;
    }
    return &(env->userList[idx]);
}

// Check if the input username is exist.
int isUsernameExist(ServerEnv* env, char* username) {
    int idx = findUserIdByUsername(env, username);
    if (idx == -1)
        return 0;
    return 1;
}

// Register User.
int regUser(ServerEnv* env, User user) {
    if (env->userCount >= MAX_USER) {
        printf("Reach max user.");
        return -1;
    }
    memcpy(&(env->userList[env->userCount]), &user, sizeof(User));
    env->userCount += 1;
    return 1;
}

// Login
int loginUser(ServerEnv* env, char* username, char* password, int pid) {
    int i;
    int idx;
    User user;
    idx = findUserIdByUsername(env, username);
    if (idx == -1) {
        printf("Username not exist");
        return 0;
    }
    else if (idx > 0 && idx < MAX_USER) {
        user = env->userList[idx];
        if (strcmp(password, user.password) == 0) {
            env->online[idx] = pid;
            env->onlineCount++;
            return 1;
        }
        return 0;
    }
}

#endif // _STRUCT_H
