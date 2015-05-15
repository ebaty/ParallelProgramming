#include <cstdio>
#include <iostream>
#include <omp.h>

#define kSize 1000000
using namespace std;

void printThreads() {
	int count = 0;
	#pragma omp parallel
	{
		count = omp_get_num_threads();
	}
	printf("use thread: %d\n", count);
}

int main(void) {
	printThreads();

	static int array[kSize];
	for(int i = 0; i < kSize; ++i) array[i] = i;

	long long sum = 0;
	#pragma omp parallel
	{
		#pragma omp for reduction(+:sum)
		for(int i = 0; i < kSize; ++i) sum += array[i];
	}

	cout << sum << endl;
	return 0;
}
