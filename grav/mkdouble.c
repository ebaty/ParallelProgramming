#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define N 1024

void file_read(char *path,double *buff,int size);
void file_write(double *buff,char *path,int size);

void init_r(double *r, int num){
  int i;
  for( i=0; i < num; i++)
    r[i] = rand()/(RAND_MAX+1.0)*100.0-50.0;
}
void init_m(double *m, int num){
  int i;
  for(i=0; i < num; i++)
    m[i] = 1.0+rand()/(RAND_MAX+1.0);
}
void init_v(double *v, int num){
  init_r(v,num);
}

int main(int argc, char* argv[]){
  int size;
  double *m,*x,*y,*z,*vx, *vy, *vz;

  if(argc == 1){
    size = N;
  }
  else {
    size = atoi(argv[1]);
    if(size == 0){
      size = atoi(argv[1]);
    }
  }
  
  m  = (double*)malloc(sizeof(double)*size);
  x  = (double*)malloc(sizeof(double)*size);
  y  = (double*)malloc(sizeof(double)*size);
  z  = (double*)malloc(sizeof(double)*size);
  vx = (double*)malloc(sizeof(double)*size);
  vy = (double*)malloc(sizeof(double)*size);
  vz = (double*)malloc(sizeof(double)*size);

  init_m(m,size);
  init_r(x,size);
  init_r(y,size);
  init_r(z,size);
  init_v(x,size);
  init_v(y,size);
  init_v(z,size);


  file_write(m,"m.double",size);
  file_write(x,"x.double",size);
  file_write(y,"y.double",size);
  file_write(z,"z.double",size);
  file_write(vx,"vx.double",size);
  file_write(vy,"vy.double",size);
  file_write(vx,"vz.double",size);

  free(m);
  free(x);
  free(y);
  free(z);
  free(vx);
  free(vy);
  free(vz);

  printf("success\n");
  return 0;
}



void file_write(double *buff,char *path,int size){
  FILE *fpw;
  //double tmp;
  //struct stat filestat;
  int ended;
  printf("create \"%s\"\n", path);
  if((fpw = fopen(path,"w"))==NULL){
    printf("file open error\n");
    exit(1);
  }

  ended = 0;
  while(1){
    size_t len;
    len = fwrite(buff+ended,1,sizeof(double)*size-ended,fpw);
    if(len <= 0){
      perror("fwrite");
    }
    ended += len;
    if(ended>=sizeof(double)*size)
      break;
  }
  fclose(fpw);
 
}
void file_read(char *path,double *buff,int size){
  FILE *fpr;
  //double tmp;
  struct stat filestat;
  int ended;

  if((fpr = fopen(path,"r"))==NULL){
    printf("file not found\n");
    exit(1);
  }
  stat(path,&filestat);
  if(filestat.st_size!=size*sizeof(double)){
    printf("wrong file size\n");
    //MPI_Finalize();
    //exit(1);
  }
  ended = 0;
  while(1){
    ended += fread(buff+ended,1,sizeof(double)*size-ended,fpr);
    if(ended>=sizeof(double)*size)
      break;
  }
  fclose(fpr);
 
}
