#include <iostream>
#include <math.h>
using namespace std;

struct bspol {
    double density_corr;
    int lb_index;
    int ub_index;

};
bspol fbspol(double s, double x[1000], double a[1000], double b[1000], double c[1000],double d[1000], int n) {
    bspol bp;
    int idir;
    int mlb;
    int mub;
    int mu;
    int ml;
    int mav;
    double k;
    double g;
    if (x[0] <= x[n-1]) {
        idir =0;
        mlb =0;
        mub = n;
    }
    else {
        idir =1;
        mlb =n;
        mub = 0;
    }
    if (s>=x[mub+idir-1]) {
        mu = mub + 2*idir - 1;
    }
    else if (s<=x[mlb+1-idir-1]) {
        mu = mlb - 2*idir + 1;
    }
    else {
        ml = mlb;
        mu = mub;
        int binary_temp = 0; // we define binary_temp = 0 as the loop must run at least once
        while (abs(mu-ml)>1 || binary_temp==0) {
            mav = (ml+mu)/2;
            if (s<x[mav]) {
                mu = mav;
            }
            else {
                ml = mav;
            }
            binary_temp=1;
        }
        mu = mu + idir - 1;
    }
    mu = mu + 1;

    // s lies between x[mu-1] and x[mu]
    k = s - x[mu-1];
    /*
        g gives you the density correction factor
        by using the variables from scof - a,b,c,d
    */
    g = ((d[mu-1]*k+c[mu-1])*k+b[mu-1])*k+a[mu-1];

    // putting in the structure
    bp.lb_index = mu-1;
    bp.ub_index = mu;
    bp.density_corr = g;
    return bp;

}

