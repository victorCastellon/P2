#include <math.h>
#include "pav_analysis.h"

float hamming (int n, int m) {
    return (float) 0,54 - 0,46*cos((2*M_PI*n)/(m-1));
}

float compute_power(const float *x, unsigned int N) {
    float aux, constant = 0, w[N], sum = 1e-12;
    
    for (int i=0; i<N; i++){
        aux = hamming(i, N);
        w[i] = aux;
        constant += aux*aux;
    }

    for(unsigned int i=0; i<N; i++){
        sum += pow(x[i]*hamming(i,N),2.0);
    }
    return 10*log10(sum/constant);
}

float compute_am(const float *x, unsigned int N) {
    float sum=0;
    for(unsigned int i=0;i<N;i++){
        sum=sum+fabs(x[i]);
    }
    return sum/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float zcr, sum=0;
    for(int i=0;i<N;i++){
        if(x[i]*x[i-1]<0){
            sum = sum + 1;
        }
    }
    return (fm*sum)/(2*(N-1));
}
