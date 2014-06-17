#ifndef _STRUCT_H
#define _STRUCT_H

typedef struct {
    int action;
    char msg[200];
} RESPONSE_STRUCT, *RESPONSE_STRUCT;

typedef struct {
    int pid;
    char client_fifo_name[100];
    char username[20];
    char content[200];
} CLIENT_INFO, *CLIENT_INFO_PTR;

#endif // _STRUCT_H
