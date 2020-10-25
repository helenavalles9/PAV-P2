#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
  float suma = 0;
  int j;
  for(j=0;j<N; j++){
    suma = suma + (x[j]*x[j]);
  }
  float res = 10*log10((1.0/N)*suma);
  return res;
}
float compute_am(const float *x, unsigned int N) {
    float suma=0;
    for(int j=0;j<N; j++){
    suma+=fabs(x[j]);
  }
  return suma/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float prev = x[0];
    float sum = 0;
    for (int n = 1; n < N; n++){
        if(x[n] != prev){
            sum++;
        }
        prev = x[n];
    }
    return (sum * fm) / (2 * (N - 1));
}