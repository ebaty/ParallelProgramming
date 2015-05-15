#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <functional>

#include <omp.h>

#define S 1000
#define T 1000
#define U 1000

using namespace std;

void printThreads() {
	int count;
	#pragma omp parallel
	{
		count = omp_get_num_threads();
	}
	cout << "Use threads: " << count << endl;
}

vector< vector<double> > myMat(int width, int height) {
	vector< vector<double> > mat(width, vector<double>(height));

	for(int i = 0; i < mat.size(); ++i) {
		for(int j = 0; j < mat[i].size(); ++j) {
			mat[i][j] = sin( (i+1) / (j+1) );
		}
	}

	return mat;
}

int main(void) {
	printThreads();

	vector< vector<double> > A = myMat(S, T);
	vector< vector<double> > B = myMat(T, U);
	vector< vector<double> > C(S, vector<double>(U));

	double sum;
	#pragma omp parallel
	{
		#pragma omp for private(sum)
		for(int i = 0; i < S; ++i) {
			for(int j = 0; j < U; ++j) {
				sum = 0.0f;

				for(int k = 0; k < T; ++k) {
					sum += A[i][k] * B[k][j];
				}

				C[i][j] = sum;
			}
		}
	}

	return 0;
}
