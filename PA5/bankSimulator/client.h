//
//  client.h
//  Bank
// Deepak Nalla
//

#ifndef __Bank__client__
#define __Bank__client__

#include <stdio.h>


void * inputCommands(void * arg);

void * outputResponse(void * arg);

int connect_to_server(const char * server, const char * port);



#endif /* defined(__Bank__client__) */
