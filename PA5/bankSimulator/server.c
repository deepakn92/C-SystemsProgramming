// PA 5 Bank Server
// Deepak Nalla
// B. Russell CS 214

#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<pthread.h>
#include     <signal.h>
#include     <sys/shm.h>
#include     <sys/wait.h>
#include     <semaphore.h>
#include      "bank.h"
#include      <sys/time.h>

#define OPEN 1
#define START 2
#define BALANCE 3
#define CREDIT 4
#define DEBIT 5
#define FINISH 6
#define EXIT 7
#define DEBT_PREVENT 8

// global variables, shmid blocks sigint, so it is guaranteed to point a block of shared memory//


static int shmid;           //shared memory id used in (shmat) shared memory
static unit * shmaddr; // shared memory address, unit= struct account object
static int _sd;
static char curr[100];
static int indexo = 0;
static int in_session = 0;



void print_periodic_info(){ //attempts to lock the bank. If successful, prints out information. Happens everytime our alarm goes off.
    int i, z;
    
    sem_wait(&shmaddr->lock);
    if(shmaddr->total_acc_size == 0){
        write(1, "Bank has 0 accounts currently\n", strlen("Bank has 0 accounts currently\n"));
        sem_post(&shmaddr   ->lock);
        return;
    }
    char message[200];
    memset(message, 0, 200);
    
    char  my_message[20][2000]; //20 accounts
    char * value;
    
    strcat(message, "Time is up!, Here is a summary of our session:\n");
    for(i = 0; i < shmaddr->total_acc_size; i++){
        if(shmaddr->accounts[i].session == 0){
            value = "FALSE";
        }
        else{
            value = "TRUE";
        }
        sprintf(my_message[i], "Account %d: Name = %s, Balance = %f, In Session = %s\n",i, shmaddr->accounts[i].name,shmaddr-> accounts[i].balance, value);
    }
    
    for(z = 0; z < shmaddr->total_acc_size; z ++){
        strcat(message, my_message[z]);
    }
    
    write(1, message, strlen(message) + 1);
    
    sem_post(&shmaddr->lock);
    return;
    
}

void timeout_handler( int sig) /*blocks signal to ensure that no more than one timer interrupt comes, then resets the timers*/
{
    sigset_t blockalrm;
    sigemptyset (&blockalrm);
    sigaddset (&blockalrm, SIGALRM);
    sigprocmask (SIG_BLOCK, &blockalrm, NULL); //block more alarm signals from coming in
    print_periodic_info();
    sigprocmask (SIG_UNBLOCK, &blockalrm, NULL);//unblock alarm signals
    alarm(20);
    
}

void sharetocare() //creates our shared memory
{
    key_t key;
    int shmid;
    
    key = ftok("/server", '2393');
    
    if ((shmid = shmget(key, sizeof(unit), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    
    shmaddr = shmid;
    
    if ((shmaddr = shmat(shmid, NULL, 0)) == (void*) -1)
    {
        perror("shmat");
        exit(1);
    }
    
    sem_init(&shmaddr->lock, 1, 1);
    shmaddr->total_acc_size = 0;
}


/*Creates our account. First has to lock down the bank semaphore. Then it there is max capacity or the account exists it returns without creation. If less than capacity and doesn't exist yet, intializate the account with a semaphore*/

int open_acc(char * account_name,int  fd){
    char message[256];
    memset(message,0,256);
    sem_wait(&shmaddr->lock);
    
    
    write(fd, message, sprintf(message, "Banking semaphore is now open!\n"));
    
    int num = shmaddr->total_acc_size;
    if(num == 20){ //creation fails
        sem_post(&shmaddr->lock); //give up your lock
        write(fd,message, sprintf(message,"\n Max capacity of bank is reached\n"));
        return -1;
    }
  
    for(int i = 0; i < num; i ++){
        if(strcmp(account_name, shmaddr->accounts[i].name) == 0){
            sem_post(& shmaddr->lock);
            write(fd, message, sprintf(message, "\n Bank account with this name already exists\n"));
            return -1;
        }
    }
    
    sem_init(&shmaddr->accounts[num].lock, 1, 1);
    strcpy(shmaddr->accounts[num].name, account_name);
    shmaddr->accounts[num].name[strlen(account_name)] = '\0';
    shmaddr->accounts[num].balance = 0;
    shmaddr->accounts[num].session = 0;
    shmaddr->total_acc_size++;
    
    sem_post(&shmaddr->lock);
    
    
    return OPEN;
}

int balance(char * amnt)
{
    char message[256];
    memset(message, 0, 256);
    if (in_session == 0)
        return -1;
    
    sprintf(message, "Current balance: %.2f\n", shmaddr->accounts[indexo].balance);
    strcpy(amnt, message);
    message[strlen(message)] = '\0';
    return BALANCE;
}

int credit(char * amnt){
    int money;
    if(!in_session){
        
        write(1, "\nYou are not currently being serviced, you cannot check your balance from your account, if you would like to leave, type 'exit'\n",strlen("\nYou are not currently being serviced, you cannot check your balance from your account, if you would like to leave, type 'exit'\n")+1);

        return -1;
    }
    
 if ((money = atof(amnt)) == 0)
{
    write(1, "\nYou credited no money into your account\n",strlen("\nYou credited no money into your account\n")+1);

    return -1;
}
else{
    shmaddr->accounts[indexo].balance = shmaddr->accounts[indexo].balance + money;
    return CREDIT; } //success
}
/*Withdraws and ammount, can only execute if in serving session*/
int debit(char * amnt){
    int money = 0;
    if(!in_session){
        
      write(1, "\nYou are not currently being serviced, you cannot withdraw from your account, if you would like to leave, type 'exit'\n",strlen("\nYou are not currently being serviced, you cannot withdraw from your account, if you would like to leave, type 'exit'\n")+1);
        return -1;
    }
    if((money = atof(amnt)) == 0)
    {
        write(1, "\nYou debited no money from your account\n",strlen("\nYou debited no money from your account\n")+1);

        return -1;
    }
    if(money > shmaddr->accounts[indexo].balance){ //if you are trying to w ithdraw more than possible
        return DEBT_PREVENT;
    }
    
    shmaddr->accounts[indexo].balance = shmaddr->accounts[indexo].balance - money;                                                                                                                                         return DEBIT;//success
                                                                                                                                            
}
/*ends our serving session. flags are rest and releases semaphore
 */
int finish(){
    if(!in_session){
        write(1, "You are not currently being serviced, you cannot finish your session, if you would like to leave, type 'exit'\n",strlen("You are not currently being serviced, you cannot finish your session, if you would like to leave, type 'exit'\n")+1);
        return -1;
    }
     shmaddr->accounts[indexo].session = 0;

    in_session = 0;
    memset(curr, 0, sizeof(curr));
    sem_post(&shmaddr->accounts[indexo].lock); //release mutex
    indexo = 0;
    return FINISH;
}
/*tries to enter serving session with account. If it cannot enter the serving session it will wait and tell the client that it is waitng. Cannot enter session if you are currently in a session, or the account doesn't exist*/
int start_acc(char * account, int fd){
    if (strcmp(account, curr) == 0){
        return -1;
    }
    if(in_session == 1){
        return -1;
    }
    
    sem_wait(&shmaddr->lock); //wait on the bank lock
    //printf("Bank semaphore locked for account %s\n", account);
    int i;
    if(shmaddr->total_acc_size == 0){
        write(fd, "No accounts currently exist, so you cannot start session!!\n", strlen("No accounts currently exist, so you cannot start session!!\n")+ 1);
        sem_post(&shmaddr->lock); //increments counter
        return -1;
    }
    for(i = 0; i < shmaddr->total_acc_size; i++){
        if(strcmp(account, shmaddr->accounts[i].name) == 0){
            //sem_getvalue(&baddr->accounts[i].lock, &sem_val);
            char message[100];
            memset(message,0,100);
            sprintf(message, "Customer %s is now in an active session\n", shmaddr->accounts[i].name);
            shmaddr->accounts[i].session = 1;;
            strcpy(curr, account);
            curr[strlen(account)] = '\0'; //add null terminated char
            in_session = 1;
            indexo = i;
            break;
        }
    }
    if(i == shmaddr->total_acc_size){
        write(fd, "No account of this name exists!!\n", strlen("No account of this name exists!!\n"));
        sem_post(&shmaddr->lock);; //increment sem count, does this work
        return -1;
    
    }
    sem_post(&shmaddr->lock); //increment sem count
    
    return START;
}

/*parses our input, updates the client and calls method */
//
int token_parser(char * string, char * response, int fd)
{
   
    char parser[150];
    char split[2] = " ";
    char parameter[150];
    char *token = "";
    int i = 0;
    int assign;
    
    if(strlen(response)>100){
        write(fd,parser, sprintf( parser,"The account name must be shorter than 100 characters.\n"));
        return -1;
    }
    
    if (strcmp(string, "finish") == 0)
        assign = FINISH;
    else if (strcmp(string, "exit") == 0)
        assign = EXIT;
    else if (strcmp(string, "balance") == 0)
        assign = BALANCE;
    else {
        strcpy(parser, string);
        parser[strlen(string)] = '\0';
        
        token = strtok(parser, split);
        
        
        
        if (strcmp(parser, "open") == 0) {
            assign = OPEN;
        } else if (strcmp(parser, "start") == 0) {
            assign = START;
        } else if (strcmp(parser, "credit") == 0) {
            assign = CREDIT;
        } else if (strcmp(parser, "debit") == 0) {
            assign = DEBIT;
        } else {
            return -5;
        }
        
        while (token != NULL) {
            i++;
            if (i == 2) {
                strcpy(parameter, token);
                parameter[strlen(token)] = '\0';
            }
            token = strtok(NULL, split);
        }
        
        if (i > 2 || i <= 1)
            return -1;
    }
    
    char message[250];
    memset(message, 0, 250);
    switch(assign)
    {
        case OPEN:
            
            write(1, message, sprintf(message,"\x1b[2;33mAttempting to open new account \"%s\"\x1b[0m\n", parameter));
            assign = open_acc(parameter, fd);
                break;
        case START:
            write(1, message,sprintf(message, "\x1b[2;33Attempting to start session with account \"%s\"\x1b[0m\n", parameter));
            assign = start_acc(parameter, fd);
            break;
        case BALANCE:
            write(1,message, sprintf(message,"\x1b[2;33mAttempting to find balance in account \"%s\"\x1b[0m\n", curr));
            assign = balance(string);
            break;
        case CREDIT:
            assign = credit(parameter);
             write(1,message, sprintf(message,"\x1b[2;33mAttempting to credit the account \"%s\"\x1b[0m\n", curr));
            break;
        case DEBIT:
            assign = debit(parameter);
             write(1,message, sprintf(message,"\x1b[2;33mAttempting to debit the account \"%s\"\x1b[0m\n", curr));
            break;
        case FINISH:
             write(1,message, sprintf(message,"\x1b[2;33mAttempting to finish the active session \"%s\"\x1b[0m\n", curr));
            assign = finish();
            break;
    }
    
    return assign;
}


int client_session(int sd)
{
    char string[512];
    char buffer[512];
    char message[256];
    int assign;
    memset(string, 0, 512);
    _sd = sd;
    
    while ( read( sd, string, sizeof(string) ) > 0 )
    {
        printf( "incoming input to server:  %s\n", string );
        
        if ((assign = token_parser(string, buffer, sd)) == -5) {
            write(sd, buffer, sprintf(buffer, "\x1b[1;31m Incorrect form of input: please try again!! \x1b[0m\n"));
        } else {
            switch(assign)
            {
                case BALANCE:
                    write( sd, buffer, strlen(buffer) + 1 );
                    break;
                case DEBIT:
                    write(sd, buffer, sprintf(buffer, "\nWITHDRAW SUCCESS!\n"));
                    break;
                case DEBT_PREVENT:
                    write(sd, buffer, sprintf(buffer, "\nWITHDRAW FAILURE\n - Funds Insufficient\n"));
                    break;
                case -1:
                    write(sd, buffer, sprintf(buffer, "\nCOMMAND FAILED \"%s\" \n", string));
                    break;
                default:
                    write(sd, buffer, sprintf(buffer, "\nCOMMAND SUCCESS \"%s\"\n", string));
            }
        }
        
        if (assign == EXIT) {
            if(in_session){ //  to release a semaphore
                sem_post(&shmaddr->accounts[indexo].lock);
                shmaddr->accounts[indexo].session = 0;
            }
            write(1, message, sprintf(message, "child process ending now! %d\n", getpid()));
            exit(0);
        }
    }
    close( sd );
    return 0;
}


/*
 * reap up zombies from killing child process, WNOHANG makes sure its a child process that is waiting >?
 */
void sigChldHandler(int s)
{
    int pid;
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Killing a child process %d\n", pid);
    }
}

/*
 *  -- Organized cleaning
 */
void cleaning_func(int sig)
{
    if (shmdt(shmaddr) != 0) { //detach shared memory if eexists
        perror("shmdt");
        exit(1);
    }
    
    shmctl(shmaddr, IPC_RMID, NULL);
    write(shmaddr, "disconnect", 11); //kill client process
    exit(0);
}

int main( int argc, char ** argv )
{
    int sd;
    char message[256];
    pthread_attr_t _kernel;
    socklen_t ic;
    int	fd;
    struct sockaddr_in senderAddr;
    pid_t child_pid;
    
    struct sigaction sigint_action;
    struct sigaction sigchld_action;
    struct sigaction sigalrm_action;
    
    sigalrm_action.sa_handler = timeout_handler;
    sigalrm_action.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sigalrm_action, NULL);;
    alarm(3); //every 3 seconds, alarm raised
    
    sigint_action.sa_handler = cleaning_func;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;
    
    if (sigaction(SIGINT, &sigint_action, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    sigchld_action.sa_handler = sigChldHandler; // reap all dead processes
    sigemptyset(&sigchld_action.sa_mask);
    sigchld_action.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sigchld_action, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    ;
    
    
    if ( pthread_attr_init( &_kernel ) != 0 )
    {
        printf( "p thread_attr_init() failed in file %s line %d\n", __FILE__, __LINE__ );
        return 0;
    }
    else if ( pthread_attr_setscope( &_kernel, PTHREAD_SCOPE_SYSTEM ) != 0 )
    {
        printf( "pthread_attr_setscope() failed in file %s line %d\n", __FILE__, __LINE__ );
        return 0;
    }
    else if ( (sd = sockets( "34982" )) == -1 )
    {
        write( 1, message, sprintf( message,  "\x1b[1;31mUnable to bind to port %s errno %s\x1b[0m\n", "34982", strerror( errno ) ) );
        return 1;
    }
    else if ( listen( sd, 100 ) == -1 )
    {
        printf( "listen() failed in file %s line %d\n", __FILE__, __LINE__ );
        close( sd );
        return 0;
    }
    else if (sharetocare(), shmaddr == NULL) //creating shared memory
    {
        printf("NULL shared memory has been returned when creating\n");
        return 0;
    }
    else
    {
        write(1, message, sprintf(message, "\x1b[1;32m Shared memory implemented with success! \x1b[0m\n"));
        ic = sizeof(senderAddr);
        while ( (fd = accept( sd, (struct sockaddr *)&senderAddr, &ic )) != -1 )
        {
            write(1, message, sprintf(message, "\x1b[1;32mClient accepted at file descriptor %d\x1b[0m\n", fd));
            child_pid = fork();
            if (child_pid >= 0) {
                if (child_pid == 0) //know this is child processes
                {
                    sigalrm_action.sa_handler = SIG_IGN; //ignore timer calls in child process
                    close(sd);
                    client_session(fd);
                    close(fd);
                    
                }
                else //this is the parent process
                {
                    write(1, message, sprintf(message, "\x1b[1;32mCreated child process %d\x1b[0m\n", child_pid));
                    close(fd);
                }
            }
        }
        close( sd );
        return 0;
    }
}

int sockets( const char * port )
{
    struct addrinfo addr_info;
    struct addrinfo *result;
    int	sd;
    char message[256];
    int on = 1;
    
    addr_info.ai_flags = AI_PASSIVE;		// for bind()
    addr_info.ai_family = AF_INET;		// IPv4 only
    addr_info.ai_socktype = SOCK_STREAM;	// TCP/IP
    addr_info.ai_protocol = 0;
    addr_info.ai_addrlen = 0;
    addr_info.ai_addr = NULL;
    addr_info.ai_canonname = NULL;
    addr_info.ai_next = NULL;
    if ( getaddrinfo( 0, port, &addr_info, &result ) != 0 )
    {
        fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed errno is %s.  File %s line %d.\x1b[0m\n", port, strerror( errno ), __FILE__, __LINE__ );
        return -1;
    }
    else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
    {
        write( 1, message, sprintf( message, "socket() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
        freeaddrinfo( result );
        return -1;
    }
    else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
    {
        write( 1, message, sprintf( message, "setsockopt() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
        freeaddrinfo( result );
        close( sd );
        return -1;
    }
    else if ( bind( sd, result->ai_addr, result->ai_addrlen ) == -1 )
    {
        freeaddrinfo( result );
        close( sd );
        write( 1, message, sprintf( message, "\x1b[2;33mBinding to port %s ...\x1b[0m\n", port ) );
        return -1;
    }
    else
    {
        write( 1, message, sprintf( message,  "\x1b[1;32mBINDING SUCCESS : Bind to port %s\x1b[0m\n", port ) );
        freeaddrinfo( result );		
        return sd;			// bind() succeeded;
    }
}