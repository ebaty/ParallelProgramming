#include <cstdio>
#include <iostream>
#include <cmath>
#include <omp.h>

#define kSize 1000000000
using namespace std;

void printThreads() {
	int count;
	#pragma omp parallel
	{
		count = omp_get_num_threads();
	}
	cout << "use thread: " << count << endl;
}

int main(void) {
	printThreads();

	static double array[kSize];
	for(int i = 0; i < kSize; ++i) array[i] = sin( (i+1) / kSize );
	
	double sum = 0;
	#pragma omp parallel
	{
		#pragma omp for reduction(+:sum)
		for(int i = 0; i < kSize; ++i) sum += array[i];
	}

	cout << sum << endl;

	return 0;
}
