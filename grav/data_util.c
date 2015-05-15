#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_SIZE 1024

int write_data(double *buf, char *path, int size)
{
	int i;
	FILE *fpw;

	fpw = fopen(path, "w");
	if (fpw == NULL) {
			printf("Failed open file: \"%s\"\n", path);
			exit(1);
		}
	for (i = 0; i < size; ++i) {
			fprintf(fpw, "%lf\n", buf[i]);
		}
	fclose(fpw);
	return 0;
}

int read_data(char *path, double *buf, int size)
{
	int i;
	FILE *fpr;

	fpr = fopen(path, "r");
	if (fpr == NULL) {
			printf("Failed open file: \"%s\"\n", path);
			exit(1);
		}

	for (i = 0; i < size; ++i) {
			fscanf(fpr, "%lf\n", &buf[i]);
		}
	fclose(fpr);
	return 0;
}
