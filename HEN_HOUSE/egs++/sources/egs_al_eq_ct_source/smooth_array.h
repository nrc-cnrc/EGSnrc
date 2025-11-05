#include <iostream>
#include <stdlib.h>
#include "egs_functions.h"


EGS_Float *conv(EGS_Float *A, EGS_Float *B, int lenA, int lenB, int *lenC) {
    int nconv;
    int i, j, i1;
    EGS_Float tmp;
    EGS_Float *C;
    EGS_Float sum = 0.0;
    //allocated convolution array
    nconv = lenA;
    C = (EGS_Float *) calloc(nconv, sizeof(EGS_Float));
    C[0] = A[0];
    int n = lenB/2;
    //convolution process
    EGS_Float sum_front = 0.0;
    EGS_Float sum_last = 0.0;
    for (int k = 0; k < n; k++) {
        sum_front += A[k];
        C[k] = sum_front/(k+1);
        sum = sum + C[k];
        sum_last += A[lenA-(k+1)];
        C[lenA-(k+1)] = sum_last/(k+1);
        sum = sum + C[lenA-(k+1)];
    }
    for (i=n; i<nconv-n; i++) {
        i1 = i-n;
        tmp = 0.0;
        for (j=0; j<lenB; j++) {
            if (i1>=0 && i1<lenA) {
                tmp = tmp + (A[i1]*B[j]);
            }

            i1 = i1+1;

        }
        C[i] = tmp;
        sum = sum + tmp;
    }

    for (int j = 0; j < nconv; j++) {
        C[j] = C[j]/sum;
        //std::cout << C[j] << std::endl;
    }
    //get length of convolution array
    (*lenC) = nconv;

    //return convolution array
    return (C);
}


EGS_Float *smooth_array(EGS_Float *array, int len_array, int con_len) {
    EGS_Float val = 1/((EGS_Float)con_len);
    EGS_Float kernel[con_len];
    int lenY;
    std::fill_n(kernel, con_len, val);
    EGS_Float *smoothed_array = conv(array,kernel,len_array,con_len,&lenY);
    return smoothed_array;
}