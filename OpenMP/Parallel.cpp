#include <cstdio>
#include <iostream>
#include <string>
#include <omp.h>
#define LOOP 1000000
#define INTERVAL 10000

using namespace std;

void printThreads(string prefix) {
	printf("[%s] use thread: %d, thread num: %d\n", prefix.c_str(), omp_get_num_threads(), omp_get_thread_num());
}

int main(void) {
	string p1 = "task one";
	string p2 = "task two";

	omp_set_nested(1);
	omp_set_dynamic(0);
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			printThreads("rank 1");
			for(int i = 0; i < LOOP; ++i) {
				if ( i % INTERVAL == 0 ) printf("%s %d\n", p1.c_str(), i / INTERVAL);
			}
		}

    #pragma omp section
		{
			printThreads("rank 1");
			#pragma omp parallel
			{
				printThreads("rank 2");
				#pragma omp for
				for(int i = 0; i < LOOP; ++i) {
					if ( i % INTERVAL == 0 ) printf("%s %d\n", p2.c_str(), i / INTERVAL);
				}
			}
		}
	}
	return 0;
}
