#include <iomanip>
#include <iostream>
#include <math.h>
using namespace std;

double HelperFunctions::cutoff(double *en, double *eps, double *f, int nmax) {
    // here we computer cutoff below which density effect is 0. However this cutoff is not used in estar.
    // Please refer to section 4 of the report for further details.
    double ycut;
    if (en[nmax-1] <= 0) {
        ycut = 0.0; // when en[nmax-1] <= 0, the substance is a metallic conductor
        // From sternhemier 1984 (paragraph above equation 14), ycut = 0.0
        // However ycut = 0.0 is not used. Please refer to 4 of the report for more details
    }
    else {
        double sum = 0.0;
        for (int n = 0; n < nmax; n++) {
            sum = sum + f[n]/eps[n];
        };
        ycut = 1/sum;
    }
    return ycut;
}