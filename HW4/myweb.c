/*
 * Author: kshuang
 * Date: 2016.05.27
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<netinet/in.h>      /* For Freebsd*/

#define MAXCONN 1000


void startServer(char *);
void respond(int)
/* clients[] is used to save socket file descriptors from client connect */
int clients[MAXCONN];

/* socket file descriptors connect to client */
int sockfd;

struct contype
{
    char *ext;
    char *filetype;
};

struct contype extensions[]={
    {"txt" , "text/html" },
    {"htm" , "text/html" },
    {"html", "text/html" },
    {"png" , "image/png" },
    {"jpg" , "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"ogg" , "audio/ogg" },
    {"mp4" ,"vedio/mpeg4"},
    {0, 0},
}

int main(int argc, char const* argv[])
{
    /* argument check */
    if(argc != 3)
    {
        fprintf(stderr,"arguments error!!\n");
        fprintf(stderr,"Please use: .prog port \"/web/root/path\"\n");
        exit(EXIT_FAILURE);
    }

    /* get port from argv[1] */
    char port[6];
    strcpy(port, argv[1]);

    /* get document root  from argv[2] */
    char *docroot = malloc(strlen(argv[2]));
    strcpy(docroot, argv[2]);

    int clicount;
    for ( clicount = 0; clicount < MAXCONN; clicount++)
    {
        clients[clicount] = -1;
    }

    startServer(port);
    
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    int slot = 0;

    /* Accept connections */
    while(1)
    {
        addrlen=sizeof(struct sockaddr_in);
        clients[slot] = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen);

        if(clients[slot] < 0)
        {
            fprintf(stderr,"accept error\n");
        }
        else
        {
            /* fork a child to respond request */
            if(fork() == 0)
            {
                respond(slot);
                exit(0);
            }
        }

        /* reuse clients */
        while(clients[slot] != -1)
        {
            slot = (slot + 1)  % MAXCONN;
        }
    }

    return 0;
}

void startServer(char *port)
{
    struct addrinfo hints, *result, *respt;

    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_flags = AI_PASSIVE;        /* For wildcard IP address */
    hints.ai_family = AF_INET;          /* Allow IPv4 */ 
    hints.ai_socktype = SOCK_STREAM;     /* stream socket */
	hints.ai_protocol = 0;          	/* Any protocol */    

	int s = getaddrinfo(NULL, port, &hints, &result); 
    if(s != 0 )
    {
    	fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(s));
    	exit(EXIT_FAILURE);
    }
	
	/* getaddrinfo() returns a list of address structures.Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket and) try the next address. */

	for(respt = result; respt != NULL; respt=respt->ai_next)
	{
	    sockfd = socket(respt->ai_family, respt->ai_socktype, respt->ai_protocol);
	    
        if (sockfd == -1) 
	    {
	        continue;
        }

        /* Success */
	    if (bind(sockfd, respt->ai_addr, respt->ai_addrlen) == 0) 
	    {
	        break;
	    }
        
        /* if binf success sockfd will not be closed,only close if fail  to bind */
        close(sockfd);
	}

    /* No address succeeded */
    if(respt == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
    
    /* No longer needed */
    freeaddrinfo(result);
    
    if(listen(sockfd, MAXCONN) < 0)
    {
        fprintf(stderr, "listen error\n");
        exit(EXIT_FAILURE);
    }
}

void respond(int clientfd)
{
    char message[99999], *reqline[3];
    int recvstatus, resourcefd;

    memset( (void*)message, (int)'\0', 99999 );
    recvstatus = recv(clients[clientfd], message, 99999, 0);

    if(recvstatus < 0)
    {
        fprintf(stderr, "recv error\n");
    }
    else if(recvstatus == 0)
    {
        fprintf(stderr, "receive socket closed\n");
    }
    else
    {
        reqline[0] = strtok(message, " \t\n");

        if(strncmp(reqline[0], "GET\0", 4) == 0)
        {
            reqline[1] = strtok(NULL, " \t");
            reqline[2] = strtok(NULL, " \t\n");


        }
    }

}
