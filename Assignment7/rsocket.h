#ifndef RSOCKET_H
#define RSOCKET_H

#include<stdlib.h>
#include<sys/socket.h>
#include <signal.h>
#include <stdio.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<time.h>
#include<sys/time.h>
#include<arpa/inet.h>

// Timer defines
#define INTERVAL 1000
#define T 2

// Drop probability
#define Prob 0.1

// Type of message
#define ackFlag 0
#define appFlag 1

// Socket type
#define SOCK_MRP 40

// Max function
#define max(a,b) a<b?b:a

// Structure for storing message
struct data{
    int msg_len;
    void *msg;
};

// Structure for storing sending packet
struct sendPkt{
    struct sockaddr_in to;
    int type;
    int seqNo;
    struct data d;
};

// Structure for storing received packets
struct rcvPkt{
    struct sockaddr_in from;
    struct data d;
};

// Element of unACK Table
struct unAckPacket{
    time_t t;
    struct sendPkt p;
};

// Sending Buffer
struct sendBuffer{
    int front;
    int end;
    int size;
    struct sendPkt **p;
};

// Receiving buffer
struct recvBuffer{
    int front;
    int end;
    int size;
    struct rcvPkt **p;
};

// unACKTable
struct unAckTable{
    int size;
    struct unAckPacket **p;
};

// Received IDs structure
struct recvIDs{
    int size;
    int *IDs;
};

// Functions available to user

int r_socket(int domain, int type, int protocol);

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len);

int r_sendto(int socket, const void *message, size_t length, 
        int flags, const struct sockaddr *dest_addr, socklen_t dest_len);

ssize_t r_recvfrom(int socket,  void * restrict buffer, size_t length, 
        int flags, struct sockaddr * restrict address, socklen_t * restrict address_len);

int r_close(int socket);

int dropMessage(float p);

#endif
