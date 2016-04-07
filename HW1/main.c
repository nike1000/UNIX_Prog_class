#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

extern void random_length_task();

struct timeval tvdiff(struct timeval t1, struct timeval t0) {
    t1.tv_sec -= t0.tv_sec;
    if(t1.tv_usec >= t0.tv_usec) {
        t1.tv_usec = t1.tv_usec - t0.tv_usec;
    } else {
        t1.tv_usec = t1.tv_usec + 1000000 - t0.tv_usec;
        t1.tv_sec--;
    }
	
	struct timeval diff;
	diff.tv_sec=t1.tv_sec;
	diff.tv_usec=t1.tv_usec;
    return diff;
}


int main(int argc,char *argv[]) {
	int n = 10;
    if(argc == 2)
    {
        sscanf (argv[1],"%d",&n);
    }

	srand(time(0) ^ getpid());
	while(1) {
		struct timeval tv1;
        gettimeofday(&tv1, NULL);
		
        struct timeval diff;
        
        if(n<=0)
            break;
		n--;

        random_length_task();
        tv1.tv_usec+=99900;

        struct timeval tv2;
        gettimeofday(&tv2, NULL);

        diff=tvdiff(tv1,tv2);
        select(0, NULL, NULL, NULL, &diff);
	}
	return 0;
}

