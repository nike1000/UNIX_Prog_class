/*
 * Author: kshuang
 * Date: 2016.05.27
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>      /* For Freebsd*/
#include <sys/stat.h>
#include <fcntl.h>

#define MAXCONN 1000


void startServer(char *);
char* parseReq(int, char *);
int regDir(char *);
int checkResource(char *);
void respond(int, int, char *);
void respondReg(int, char *);
void respondDir(int, char *);
/* clients[] is used to save socket file descriptors from client connect */
int clients[MAXCONN];

/* socket file descriptors connect to client */
int sockfd;

/* HTTP Content-Type Structure*/
struct contype
{
    char *ext;
    char *filetype;
};

/* Content-Type support in myweb */
struct contype extensions[]={
    {"txt" , "text/html" },
    {"htm" , "text/html" },
    {"html", "text/html" },
    {"png" , "image/png" },
    {"jpg" , "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"ogg" , "audio/ogg" },
    {"mp4" , "video/mp4" },
    {0, 0},
};

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

    /* init clients fd with -1 */
    int clicount;
    for ( clicount = 0; clicount < MAXCONN; clicount++)
    {
        clients[clicount] = -1;
    }

    startServer(port);
    
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    int slot = 0;           /* record which clients in used now */

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
            /* fork a child to handle client request */
            if(fork() == 0)
            {
                char *respath=parseReq(slot, docroot);
    			
                int respm = checkResource(respath);

                respond(clients[slot], respm, respath);


                shutdown (clients[slot], SHUT_RDWR);
                close(slot);
                clients[slot] = -1;
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

char* parseReq(int clientfd, char *docroot)
{
    char message[99999], *reqline[3];
    int recvstatus;

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
        reqline[0] = strtok(message, " \t\n");          /* HTTP method */

        /* we only support GET method in myweb */
        if(strncmp(reqline[0], "GET\0", 4) == 0)
        {
            reqline[1] = strtok(NULL, " \t");           /* get request resource path */
            reqline[2] = strtok(NULL, " \t\n");         /* HTTP protocol */

            /* remove string after question mark in resource path */
            char *ques;
            ques = strchr(reqline[1], '?');
            if(ques != NULL)
            {
                *ques = '\0';
            }

            /* default file is index.html */
            if(strncmp(reqline[1], "/\0", 2) == 0)
            {
                reqline[1] = "/index.html";
            }

            //fprintf(stdout,"%s\n",reqline[0]);
            //fprintf(stdout,"%s\n",reqline[1]);
            //fprintf(stdout,"%s\n",reqline[2]);

            /* check whether resource path end with '/' */
            int reslen = strlen(reqline[1]);
            if(reqline[1][reslen-1] == '/')
            {
                /* remove '/' char at the end of resource path */
                reqline[1][reslen-1] = '\0';
            }
            else
            {
                //fprintf(stdout,"Last one char in resource path is: %c\n",reqline[1][reslen-1]);
            }

            /* get last entry from resource path */
            char *lastentry = strrchr(reqline[1], '/') ;
            int entrylen = strlen(lastentry);                   /* get last entry length */
            //char *reqpath = malloc(reslen-entrylen+1);          /* request path is resource path without last entry */
            //strncpy(reqpath, reqline[1], reslen-entrylen+1);    /* get request path */
            //reqpath[reslen-entrylen] = '\0';                    /* end with '\0' */

            //fprintf(stdout, "lastentry: %s\n", lastentry);
            //fprintf(stdout, "reqpath: %s\n", reqpath);

            char *fullpath = strcat(docroot, reqline[1]);
            fprintf(stdout, "fullpath: %s\n", fullpath);
	        return fullpath;
        }
    }

	return NULL;
}

int regDir(char *path)
{
    /* get request file stat */
    struct stat filestat;
    if(stat(path, &filestat) != 0)
    {
        fprintf(stderr, "stat error\n");
    }

    if(S_ISREG(filestat.st_mode))           /* request file is a regular file */
    {
        fprintf(stdout,"%s is a regural file",path);
	    return 0;
    }
    else if(S_ISDIR(filestat.st_mode))      /* request file is a directory */
    {
        fprintf(stdout,"%s is a directory",path);
	    return 1;
        /* 檢查有 slash 可以處理 301 */
        /* call getDirResource() 處理有 index.html 和沒有的情況 */
    } 

	return -1;
}

int checkResource(char *path)
{
    if(access(path, F_OK) == -1)        /* file not exist */
    {
        return -1;
    }
    else if(access(path, R_OK) == -1)   /* file exist but not allow read access */
    {
        return -2;
    }

    return 0;
}

void respond(int clientfd, int respm, char *path)
{
    char buffer[1024];
    
    if(respm == -1)	/* not exist */
	{
        fprintf(stderr,"403 FORBIDDEN\n");
        sprintf(buffer,"HTTP/1.1 403 Forbidden\nContent-Type: text/html\n\n<h1>403 Forbidden</h1>\n");
        send(clientfd, buffer, strlen(buffer), 0);
        return;
    }
    else if(respm == -2)	/* exist but not readable */
    {
        fprintf(stderr,"404 NOT FOUND\n");
        sprintf(buffer,"HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<h1>404 Not Found</h1>\n");
	    send(clientfd, buffer, strlen(buffer), 0);
        return;
    }
	
    /* exist and readable */
    int isdir = regDir(path);

    if(isdir == 0)          /* request resource is regural file */
    {
        respondReg(clientfd, path);
    }
    else if(isdir == 1)     /* request resource is directory */
    {
        respondDir(clientfd, path);
    }

}

void respondReg(int clientfd, char *path)
{
    int i, resfd, bytes_read, extlen, pathlen=strlen(path);
    char *resext = (char *)0;
	char buffer[1024];

    for(i = 0; extensions[i].ext != 0; i++)
    {
        extlen = strlen(extensions[i].ext);
        if(strncmp(&path[pathlen-extlen], extensions[i].ext, extlen) == 0)
        {
            resext = extensions[i].filetype;
            break;
        }
    }

    if(resext == 0)
    {
        sprintf(buffer,"HTTP/1.1 200 OK\n");
        send(clientfd, buffer, strlen(buffer), 0);
    }
    else
    {
        sprintf(buffer,"HTTP/1.1 200 OK\nContent-Type: %s\n\n", resext);
        send(clientfd, buffer, strlen(buffer), 0);

	    if ((resfd=open(path, O_RDONLY)) != -1)
	    {
	        while ((bytes_read=read(resfd, buffer, 1024)) > 0 )
	        {
	            send(clientfd, buffer, bytes_read, 0);
            }
        }
    }
}

void respondDir(int clientfd, char *path)
{

}
