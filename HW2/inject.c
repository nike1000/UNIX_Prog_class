#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>

static int (*orig_open)(const char *pathname, int flags) = NULL;
static FILE *(*orig_fopen)(const char *path, const char *mode) = NULL;
static ssize_t (*orig_read)(int fildes, void *buf, size_t nbyte) = NULL;
static size_t (*orig_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static size_t (*orig_fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static int (*orig_close)(int fildes) = NULL;
static int (*orig_fclose)(FILE *fp) = NULL;
static int (*orig_fileno)(FILE *stream) = NULL;
static int (*orig_rename)(const char *old, const char *new) = NULL;

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
        /*fprintf(stderr, "***kshuang Open***\n");*/
        printf("***open*** path:%s, flags:%d\n", pathname, flags);
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
        /*fprintf(stderr, "***kshuang fopen***\n");*/
        printf("***fopen*** path:%s, mode:%s\n", path, mode);
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
        /*fprintf(stderr, "***kshuang Read***\n");*/
        printf("***read*** file descriptor:%d, size:%zu\n", fildes, nbyte);
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
        /*fprintf(stderr, "***kshuang fread***\n");*/
        printf("***fread*** ptr:%s, size:%zu, nmemb:%zu\n", ptr, size, nmemb);
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
        //fprintf(stderr, "***kshuang fwrite***\n");
        printf("***fwrite*** size:%zu, nmemb:%zu\n", size, nmemb);
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
        /*fprintf(stderr, "***kshuang close***\n");*/
        printf("***close*** file descriptor:%d\n", fildes);
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
        /*fprintf(stderr, "***kshuang fclose***\n");*/
        printf("***fclose***\n");
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
        /*fprintf(stderr, "***kshuang fileno***\n");*/
        printf("***fileno***\n");
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
        printf("***rename*** from %s to %s\n", old, new);
        return orig_rename(old, new);
    }
}













































