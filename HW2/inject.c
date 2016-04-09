#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

static int (*orig_open)(const char *pathname, int flags) = NULL;
static FILE *(*orig_fopen)(const char *path, const char *mode) = NULL;
static ssize_t (*orig_read)(int fildes, void *buf, size_t nbyte) = NULL;
static size_t (*orig_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static size_t (*orig_fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static int (*orig_close)(int fildes) = NULL;
static int (*orig_fclose)(FILE *fp) = NULL;
static int (*orig_fileno)(FILE *stream) = NULL;
static int (*orig_rename)(const char *old, const char *new) = NULL;
static int (*orig_bind)(int socket, const struct sockaddr *address, socklen_t address_len) = NULL;
static int (*orig_connect)(int socket, const struct sockaddr *address, socklen_t address_len) = NULL;
static int (*orig_getaddrinfo)(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) = NULL;
FILE *flogopen(const char *path, const char *mode);
int flogclose(FILE *fp);

int open(const char *pathname, int flags)
{
    if(orig_open == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL) 
        {
            orig_open = dlsym(handle, "open");
        }
    }

    if (orig_open != NULL) 
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***open*** path:%s, flags:%d\n", pathname, flags);
        flogclose(fp);
        return orig_open(pathname, flags);
    }
}


FILE *fopen(const char *path, const char *mode)
{
    if(orig_fopen == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL) 
        {
            orig_fopen = dlsym(handle, "fopen");
        }
    }

    if (orig_fopen != NULL) 
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***fopen*** path:%s, mode:%s\n", path, mode);
        flogclose(fp);
        return orig_fopen(path, mode);
    }
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
    if(orig_read == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL) 
        {
            orig_read = dlsym(handle, "read");
        }
    }

    if (orig_read != NULL) 
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***read*** file descriptor:%d, size:%zu\n", fildes, nbyte);
        flogclose(fp);
        return orig_read(fildes, buf, nbyte);
    }
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (orig_fread == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL) 
        {
            orig_fread = dlsym(handle, "fread");
        }
    }            

    if (orig_fread != NULL)
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***fread*** ptr:%s, size:%zu, nmemb:%zu\n", ptr, size, nmemb);
        flogclose(fp);
        return orig_fread(ptr, size, nmemb, stream);
    }
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (orig_fwrite == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_fwrite = dlsym(handle, "fwrite");
        }
    }

    if (orig_fwrite != NULL)
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***fwrite*** size:%zu, nmemb:%zu\n", size, nmemb);
        flogclose(fp);
        return orig_fwrite(ptr, size, nmemb, stream);
    }
}


int close(int fildes)
{
    if (orig_close == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_close = dlsym(handle, "close");
        }
    }

    if (orig_close != NULL)
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***close*** file descriptor:%d\n", fildes);
        flogclose(fp);
        return orig_close(fildes);
    }
}


int fclose(FILE *fp)
{
    if (orig_fclose == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_fclose = dlsym(handle, "fclose");
        }
    }

    if (orig_fclose != NULL)
    {
        FILE *myfp = flogopen("0456085.log","a+");
        fprintf(myfp, "***fclose***\n");
        orig_fclose(myfp);
        return orig_fclose(fp);
    }
}

int fileno(FILE *stream)
{
    if (orig_fileno == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_fileno = dlsym(handle, "fileno");
        }
    }

    if (orig_fileno != NULL)
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***fileno***");
        flogclose(fp);
        return orig_fileno(stream);
    }
}

int rename(const char *old, const char *new)
{
    if (orig_rename == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_rename = dlsym(handle, "rename");
        }
    }

    if (orig_rename != NULL)
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***rename*** from %s to %s\n", old, new);
        flogclose(fp);
        return orig_rename(old, new);
    }
}


/*int bind(int socket, const struct sockaddr *address, socklen_t address_len)*/
/*{*/
    /*if (orig_bind == NULL)*/
    /*{*/
        /*void *handle = dlopen("libc.so.6", RTLD_LAZY);*/
        
        /*if (handle != NULL)*/
        /*{*/
            /*orig_bind = dlsym(handle, "bind");*/
        /*}*/
    /*}*/

    /*if (orig_bind != NULL)*/
    /*{*/
        /*struct sockaddr_in *saddr_in = (struct sockaddr_in *)address;*/

        /*printf("***bind*** %s------------------\n",inet_ntoa(saddr_in->sin_addr));*/
        /*return orig_bind(socket, address, address_len);*/
    /*}*/
/*}*/

int connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
    if (orig_connect == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_connect = dlsym(handle, "connect");
        }
    }

    if (orig_connect != NULL)
    {
        struct sockaddr_in *saddr_in = (struct sockaddr_in *)address;
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***connect*** %s\n",inet_ntoa(saddr_in->sin_addr));
        flogclose(fp);
        return orig_connect(socket, address, address_len);
    }
    
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
    if (orig_getaddrinfo == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_getaddrinfo = dlsym(handle, "getaddrinfo");
        }
    }

    if (orig_getaddrinfo != NULL)
    {
        FILE *fp = flogopen("0456085.log","a+");
        fprintf(fp, "***getaddrinfo*** host:%s, service:%s\n",node, service);
        flogclose(fp);
        return orig_getaddrinfo(node, service, hints, res);
    }
}

FILE *flogopen(const char *path, const char *mode)
{
    if (orig_fopen == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_fopen = dlsym(handle, "fopen");
        }
    }

    if (orig_fopen != NULL)
    {
        return orig_fopen(path, mode);
    }
}

int flogclose(FILE *fp)
{
    if (orig_fclose == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        
        if (handle != NULL)
        {
            orig_fclose = dlsym(handle, "fclose");
        }
    }

    if (orig_fclose != NULL)
    {
        return orig_fclose(fp);
    }
}
