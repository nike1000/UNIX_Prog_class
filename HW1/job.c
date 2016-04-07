#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

void random_length_task() {
	int i;
	int s = 20 + rand() % 31;   //s range in 20~50
	int n = 1 + rand() % 3;     //n range in 1~3
	struct timeval tv;
	gettimeofday(&tv, NULL);
	s = s - (s % n);            //s range in 18~50
	printf("%ld.%06ld working for %d ms\n", tv.tv_sec, tv.tv_usec, s);
	for(i = 0; i < n; i++) {
		usleep(s / n * 1000);   //1 second = 1000000 microsecond,sleep range in 18~50ms(millisecond)
	}
	return;
}

