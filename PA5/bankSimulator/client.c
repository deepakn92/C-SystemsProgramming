//
//  client.c
//  Bank Server PA5
// Deepak Nalla
//

#include "client.h"

#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<pthread.h>
#include      "client.h"


#define EXIT "exit" //can exit at any time in client

void * inputCommands( void * arg )
{
     printf("\nWelcome. Please choose one of the following options:\n \"open <account name>\",\n \"start <account name>\",\n \"credit <amount>\",\n \"debit <amount>\",\n \"balance\",\n \"finish\",\n or \"exit\".\n\n Thank you!");
    int	sd;
    char prompt[] = "Enter string";
    char string[100];
    int len;
    
    sd = *(int *)arg;
    while ( write( 1, prompt, sizeof(prompt)), (len = read( 0, string, sizeof(string))) > 0)
    {
        string[len-1]= '\0';
        
        if (strcmp(string, EXIT) == 0) {
            printf("Exiting bank. Goodbye!\n");
            write(sd, "exit", 5);
            return 0;
        }
        
        printf( "SENDING REQUEST TO SERVER: %s\n", string );
        write( sd, string, strlen(string) + 1 );
        sleep(2);
    }
    
    return 0;
}
void * outputResponse( void * arg )
{
    int	sd;
    char response[2048];
    char message[2048];
    pthread_detach(pthread_self());
    
    sd = *(int *)arg;
    while ( read( sd, response, sizeof(response) ) > 0 )
    {
        sleep( 3 );
        write(1, message,sprintf(message, "RECEIVING REQUEST FROM SERVER:  %s\n", response));
        if (strcmp(response, "disconnect") == 0)
            
            exit(0);
    }
    return 0;
}

int connect_to_server( const char * server, const char * port )
{
    int	sd;
    struct addrinfo addrinfo;
    struct addrinfo * result;
    char message[256];
    
    addrinfo.ai_flags = 0;
    addrinfo.ai_family = AF_INET;		// IPv4 only
    addrinfo.ai_socktype = SOCK_STREAM;	// Want TCP/IP
    addrinfo.ai_protocol = 0;		// Any protocol
    addrinfo.ai_addrlen = 0;
    addrinfo.ai_addr = NULL;
    addrinfo.ai_canonname = NULL;
    addrinfo.ai_next = NULL;
    if ( getaddrinfo( server, port, &addrinfo, &result ) != 0 )
    {
        fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed.  File %s line %d.\x1b[0m\n", server, __FILE__, __LINE__ );
        return -1;
    }
    else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
    {
        freeaddrinfo( result );
        return -1;
    }
    else
    {
        do {
            if ( errno = 0, connect( sd, result->ai_addr, result->ai_addrlen ) == -1 )
            {
                sleep( 3 );
                write( 1, message, sprintf( message, "\x1b[2;33mConnecting to server %s ...\x1b[0m\n", server ) );
            }
            else
            {
                freeaddrinfo( result );
                return sd;		
            }
        } while ( errno == ECONNREFUSED );
        freeaddrinfo( result );
        return -1;
    }
}


int main( int argc, char ** argv )
{
    int	sd;
    
    if ( argc < 2)
    {
        fprintf( stderr, "\x1b[1;31m INVALID ARGUMENT. RE-ENTER WITH HOST NAME! File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
        exit( 0 );
    }
    while ((sd = connect_to_server( argv[1], "34982" )) == -1 ) //hardcodeport
        sleep(3);
    
    
    printf( "Connected to server %s\n", argv[1] );
    
    pthread_t tid[2];
    pthread_attr_t kernel_;
    int * fs;
    
    if ( pthread_attr_init( &kernel_ ) != 0 )
    {
        printf( "pthread_attr_init() failed in file %s line %d\n", __FILE__, __LINE__ );
        return 0;
    }
    else if ( pthread_attr_setscope( &kernel_, PTHREAD_SCOPE_SYSTEM ) != 0 )
    {
        printf( "pthread_attr_setscope() failed in file %s line %d\n", __FILE__, __LINE__ );
        return 0;
    }
    else
    {
        fs = (int *)malloc(sizeof(int *));
        *fs = sd;
        
        if (pthread_create( &tid[0], &kernel_, inputCommands, fs ) != 0) {
            printf( "pthread_create() failed in file %s line %d\n", __FILE__, __LINE__ );
            return 0;
        }
        
        if (pthread_create( &tid[1], &kernel_, outputResponse, fs ) != 0) {
            printf( "pthread_create() failed in file %s line %d\n", __FILE__, __LINE__ );
            return 0;
        }
        
        pthread_join(tid[0], NULL);
        
        free(fs);
    }

    close( sd );
    return 0;}