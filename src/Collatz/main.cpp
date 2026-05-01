#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <bitset>
#include <string>
#include <ctime>

#define TYPE unsigned long long
#define SIZE 64

void imprimir_timestamp(void) {
  time_t now;
  struct tm *local_time;
  char buffer[80];
  now = time(NULL);
  local_time = localtime(&now);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
  printf("timestamp: %s\n\n", buffer);
}

int main(int argc, char** argv) {
  int i, n, iter, index,converge;
  TYPE c, num, *seq, *acc;

  imprimir_timestamp();

  n = atoi(argv[1]);

  seq = (TYPE*) malloc(sizeof(TYPE) * n+1);
  acc = (TYPE*) calloc(sizeof(TYPE), n+1);

  seq[0] = 1ULL;

  for (i=1; i<=n; i++) {
    seq[i] = (seq[i-1] << 2) + 1;
  }
  for (i=0; i<=n; i++) 
    fprintf(stderr, "a(%d)=%llu\n", i, seq[i]);

  //for (i=1; i<=n; i++) {
  i=n;
	
    fprintf(stderr, "Verificando convergência para a(%d)=%llu\n", i, seq[i]);

    /**
     * COLLATZ
     */
    #pragma omp parallel for private(iter,c,num,index,converge) schedule(dynamic)
    for (num=seq[i-1]+1; num<seq[i]; num++) {
      //printf("\t%llu -> ", num);
      c = num;
      iter = 0;
      converge = 0;
      if (std::bitset<SIZE>(num).count() != 1) {

        while (!converge) {

          if ((c | 0x1) == c) {//impar
            c = ((c<<1)+c)+1;
          } else {//par
            c >>=1;
          }
          iter++;
          #pragma omp simd
          for (index=1; index<=i; index++) {
            //fprintf(stderr, "a(%d)=%llu ... num=%llu c=%llu\n", index, seq[index], num, c);
            if (seq[index] == c) {
              converge=1;

              //pragma omp critical
              #pragma omp atomic 
		acc[index]++;

              index=i+1;
            }
          }
        }

      }
    }

    std::cerr << "   OK -- todos entre " << seq[i-1] << " e " << seq[i] << " convergiram" << std::endl;

    printf("Acumulador (%d):\n", i);
    for (index=0; index<=n; index++) 
      fprintf(stdout, "\ta(%d)=%llu, convergiram %llu\n", index, seq[index], acc[index]);

    imprimir_timestamp();
  //}

  return 0;

}
