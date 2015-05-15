#include <cstdio>
#include <iostream>
#include <omp.h>

using namespace std;

void printThreads() {
	int count;
	#pragma omp parallel
	{
		count = omp_get_num_threads();
	}
	cout << "Use threads: " << count << endl;
}

int main(void) {
	printThreads();

	return 0;
}
