#include <iostream>
#include <math.h>
#include <iomanip>
using namespace std;


// first we define the function which we want to do bisection over
// we want to solve for x in the function accurately

double yl;
double yql;
double obj_func;
double objective_function(double tau, double f[1000], double eps[1000], int nmax, double x) {
    yl = log(tau*(tau+2.0));
    yql = 0;
    for (int i = 0; i < nmax; i++) {
        yql = yql + f[i]/(eps[i]+x);
    };
    yql = log(1/yql);
    obj_func = yl - yql;
    return obj_func;
}

// now we have to write the bisection algorithm

double bisec(double lowerbound, double upperbound, double tolerance, double tau, double f[1000], double eps[1000], int nmax) {
    // we know objective_function(lowebound) will be positive
    double del = 1;
    double x_mid;
    if ((objective_function(tau, f, eps, nmax, lowerbound) > 0) && (objective_function(tau, f, eps, nmax, upperbound) < 0)) {
        while (del > tolerance) {
            x_mid = (lowerbound + upperbound)/2;
            if (objective_function(tau, f, eps, nmax, x_mid) > 0) {
                lowerbound = x_mid;
            }
            else {
                upperbound = x_mid;
            }
            del = objective_function(tau, f, eps, nmax, x_mid) ;
        }
    }
    else if ((objective_function(tau, f, eps, nmax, lowerbound) < 0) && (objective_function(tau, f, eps, nmax, upperbound) > 0)) {
        while (del > tolerance) {
            x_mid = (lowerbound + upperbound)/2;
            if (objective_function(tau, f, eps, nmax, x_mid) > 0) {
                upperbound = x_mid;
            }
            else {
                lowerbound = x_mid;
            }
            del = objective_function(tau, f, eps, nmax, x_mid);
        }
    }
    else {
        cout << "The lower and upper bounds are wrong\n";
        exit(1);
    }
    return x_mid;
}
