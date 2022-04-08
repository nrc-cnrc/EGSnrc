#include <iostream>
#include <math.h>
using namespace std;

struct scof {
    double a[1200];
    double b[1200];
    double c[1200];
    double d[1200];
}; 

scof fscof(int nmax, double x[1200], double f[1200]) {
    scof sf;   
    int m1 = 2;
    int m2 = nmax-1;
    double s = 0.0;
    double r;
    for (int m = 0; m < m2; m++) {
        sf.d[m] = x[m+1] - x[m];
        r = (f[m+1] - f[m])/sf.d[m];
        sf.c[m] = r - s;
        s = r;
    };

    s = 0.0;
    r = 0.0;
    sf.c[0] = 0.0;
    sf.c[nmax-1] = 0.0; 

    for (int m = 1; m < m2; m++) {
        sf.c[m] = sf.c[m] + r*sf.c[m-1];
        sf.b[m] = (x[m-1] - x[m+1])*2 - r*s;
        s = sf.d[m];
        r = s/sf.b[m];
    };
    int mr = m2 - 1; 
    for (int m = 1; m < m2; m++) {
        sf.c[mr] = (sf.d[mr] * sf.c[mr+1] - sf.c[mr])/sf.b[mr];
        mr = mr - 1;
    };
    for (int m = 0; m < m2; m++) {
        s = sf.d[m];
        r = sf.c[m+1] - sf.c[m];
        sf.d[m] = r/s;
        sf.c[m] = sf.c[m]*3.0;
        sf.b[m] = (f[m+1]-f[m])/s - (sf.c[m]+r)*s;
        sf.a[m] = f[m];
    };
    return sf;
}

