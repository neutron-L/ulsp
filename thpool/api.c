#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "thpool.h"


void* sleep_2_secs(){
	sleep(2);
	puts("SLEPT");
    return NULL;
}


int main(int argc, char *argv[]){

	int num = 0;
	thpool_t * thpool;

	/* Test if we can get the current number of working threads */
	thpool = thpool_init(10);
	thpool_add_job(thpool, (void*)sleep_2_secs, NULL);
	thpool_add_job(thpool, (void*)sleep_2_secs, NULL);
	thpool_add_job(thpool, (void*)sleep_2_secs, NULL);
	thpool_add_job(thpool, (void*)sleep_2_secs, NULL);
	sleep(1);
	num = thpool_num_threads_working(thpool);
	if (thpool_num_threads_working(thpool) != 4) {
		printf("Expected 4 threads working, got %d", num);
		return -1;
	};
	thpool_destroy(thpool);

	/* Test (same as above) */
	thpool = thpool_init(5);
	thpool_add_job(thpool, (void*)sleep_2_secs, NULL);
	thpool_add_job(thpool, (void*)sleep_2_secs, NULL);
	sleep(1);
	num = thpool_num_threads_working(thpool);
	if (num != 2) {
		printf("Expected 2 threads working, got %d", num);
		return -1;
	}

	thpool_destroy(thpool);

	// sleep(1); // Sometimes main exits before thpool_destroy finished 100%

	return 0;
}
