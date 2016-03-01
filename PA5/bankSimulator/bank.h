//
//  server.h
//  Bank
//  Deepak Nalla
//


#include <semaphore.h>

typedef struct Account
{
    sem_t lock;
    int session;
    char name[100];
    float balance;
} Account;

// struct of 20 bank accounts or a bank of 20 accounts :-)
typedef struct Bank
{
    sem_t lock;
    int total_acc_size;
    Account accounts[20];
} unit;


void sharetocare();

int sockets(const char * port);

int client_session(int sd);

int token_parser(char * request, char * response, int fd);
