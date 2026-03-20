#include <iostream>
#include <stdlib.h>
#include "egs_functions.h"

int locate(EGS_Float *xx, int n, const EGS_Float x0) {

    int ju, jm, jl, jsav;
    bool ascnd=(xx[n-1] >= xx[0]);
    jl=0;
    ju=n-1;
    while (ju-jl > 1) {
        jm = (ju+jl) >> 1;
        if (x0 >= xx[jm] == ascnd) {
            jl=jm;
        }
        else {
            ju=jm;
        }
    }
    jsav = jl;
    return jsav;
}

