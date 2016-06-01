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

void startServer(char *);           /* Start web server listen on port */
char* parseReq(int, char *);        /* parse client request to split host and resource path from url, return fullpath content docroot and resource path */
int regDir(char *);                 /* return 0 if given path is a regular file, 1 if directory, else -1 */
int checkResource(char *);          /* return -1 if resource is not exist, -2 if resource exist but can't access */
void respond(int, int, char *);     /* respond 403 if not exist, 404 if can't access ( only for regular file, INTENTIONALLY in this homework) */
void respondReg(int, char *);       /* respond 200 and correct content-type to client */
void respondDir(int, char *);       /* if request path not end with / respond 301, if dir have index.html respond 200, else  respond ls -lan result */

/* clients[] is used to save socket file descriptors from client connect */
int clients[MAXCONN];

/* socket file descriptors connect to client */
int sockfd;

/* host for 301 redirect */
char *host;

/* request path for 301 redirect */
char *reqpath;

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

/* Start web server listen on port */
void startServer(char *port)
{
    struct addrinfo hints, *result, *respt;

    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_flags = AI_PASSIVE;        /* For wildcard IP address */
    hints.ai_family = AF_INET;          /* Allow IPv4 */ 
    hints.ai_socktype = SOCK_STREAM;    /* stream socket */
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

/* parse client request to split host and resource path from url, return fullpath content docroot and resource path */
char* parseReq(int clientfd, char *docroot)
{
    char message[99999], *reqline[4];
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
        reqline[0] = strtok(message, " \t\n");          /* get HTTP method */
        
        /* we only support GET method in myweb */
        if(strncmp(reqline[0], "GET\0", 4) == 0)
        {
            reqline[1] = strtok(NULL, " \t");           /* get request resource path */
            reqline[2] = strtok(NULL, " \t\n");         /* get HTTP protocol */
            strtok(NULL," \t");                         /* parse "HOST:" */
            reqline[3] = strtok(NULL, " \t\n");         /* get host content */
            
            /* remove string after question mark in resource path */
            char *ques;
            ques = strchr(reqline[1], '?');
            if(ques != NULL)
            {
                *ques = '\0';
            }
            
            int hostlen = strlen(reqline[3]);
            reqline[3][hostlen-1]='\0';
            
            host = reqline[3];
            reqpath = reqline[1];
            
            char *fullpath = strcat(docroot, reqline[1]);
	        return fullpath;
        }
    }

	return NULL;
}

/* return 0 if given path is a regular file, 1 if directory, else -1 */
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
	    return 0;
    }
    else if(S_ISDIR(filestat.st_mode))      /* request file is a directory */
    {
	    return 1;
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

/* respond 403 if not exist, 404 if can't access ( only for regular file, INTENTIONALLY in this homework) */
void respond(int clientfd, int respm, char *path)
{
    char buffer[1024];
    
    if(respm == -1)	/* not exist */
	{
        sprintf(buffer,"HTTP/1.1 403 Forbidden\nContent-Type: text/html\n\n<h1>403 Forbidden</h1>\n");
        send(clientfd, buffer, strlen(buffer), 0);
        return;
    }
    else if(respm == -2)	/* exist but not readable */
    {
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

/* respond 200 and correct content-type to client */
void respondReg(int clientfd, char *path)
{
    int i, resfd, bytes_read, extlen, pathlen=strlen(path);
    char *resext = (char *)0;
	char buffer[1024];

    /* get resource content-type */
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

/* if request path not end with / respond 301, if dir have index.html respond 200, else  respond ls -lan result */
void respondDir(int clientfd, char *path)
{
    char buffer[1024];
    int resfd, bytes_read;

    /* check whether resource path end with '/' */
    int reslen = strlen(path);
    if(path[reslen-1] == '/')
    {
        char *indexpath = malloc(strlen(path)+10);
        
        strcpy(indexpath, path);
        strcat(indexpath, "index.html");
        
        /* check index.html state */
        int indexpm = checkResource(indexpath);
        
        if(indexpm == 0)    /* can read */
        {
            sprintf(buffer,"HTTP/1.1 200 OK\nContent-Type: text/html\n\n");
            send(clientfd, buffer, strlen(buffer), 0);
            
            if ((resfd=open(indexpath, O_RDONLY)) != -1)
            {
                while ((bytes_read=read(resfd, buffer, 1024)) > 0 )
                {
                    send(clientfd, buffer, bytes_read, 0);
                }
            }
            
        }
        if(indexpm == -1)	/* not exist */
        {
            /* list file in dir */
            sprintf(buffer,"HTTP/1.1 200 OK\nContent-Type: text/html\n\n");
            send(clientfd, buffer, strlen(buffer), 0);
            
            sprintf(buffer, "<html>\n<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset==utf-8\"/>\n");
            send(clientfd, buffer, strlen(buffer), 0);
            
            sprintf(buffer, "<style>\nbody {\n\tfont-family: monospace;\n\twhite-space: pre;\n}</style>\n");
            send(clientfd, buffer, strlen(buffer), 0);
            
            sprintf(buffer, "</head>\n<body>\n<hr/>\n");
            send(clientfd, buffer, strlen(buffer), 0);
            
            char cmd[1024]="/bin/ls -lan ";
            strcat(cmd, path);
            FILE *fp = popen(cmd, "r");
            
            if(fp == NULL)
            {
                fprintf(stderr, "command run error.");
                return;
            }
            
            int firstline=1;
            
            while (fgets(buffer, sizeof(buffer)-1, fp) != NULL)
            {
                if(firstline==1)
                {
                    firstline=0;
                    continue;
                }
                
                /* get last entry */
                int bufferlen = strlen(buffer);
                char *filenam = strrchr(buffer, ' ') ;
                int namlen = strlen(filenam);                       /* get filenam length */
                filenam[namlen-1]='\0';
                char *prefix = malloc(bufferlen-namlen+1);          /* ls -la result without filenam */
                
                strncpy(prefix, buffer, bufferlen-namlen+1);     /* get prefix */
                prefix[bufferlen-namlen] = '\0';                    /* end with '\0' */
                
                char* fileinfo;
                fileinfo = strcat(prefix, "<a href=\"");
                fileinfo = strcat(fileinfo, filenam);
                fileinfo = strcat(fileinfo, "\">");
                fileinfo = strcat(fileinfo, filenam);
                fileinfo = strcat(fileinfo, "</a>\n");
                send(clientfd, fileinfo, strlen(fileinfo), 0);
            }
            
            pclose(fp);
            
            sprintf(buffer, "<hr/>\n</body>\n</html>\n");
            send(clientfd, buffer, strlen(buffer), 0);
        }
        else if(indexpm == -2)	/* exist but not readable */
        {
            sprintf(buffer,"HTTP/1.1 403 Forbidden\nContent-Type: text/html\n\n<h1>403 Forbidden</h1>\n");
            send(clientfd, buffer, strlen(buffer), 0);
            return;
        }
    }
    else
    {
        sprintf(buffer,"HTTP/1.1 301 Moved Permanently\nContent-Type: text/html\nLocation: http://%s%s/\n\n<h1>301 Moved Permanently</h1>",host,reqpath);
        send(clientfd, buffer, strlen(buffer), 0);
    } 
}
