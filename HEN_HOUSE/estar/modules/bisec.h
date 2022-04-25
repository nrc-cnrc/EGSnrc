#include "bisec.cpp"

double objective_function (double tau, double f[1000], double eps[1000], int nmax, double x);

double bisec(double lowerbound, double upperbound, double tolerance, double tau, double f[1000], double eps[1000], int nmax);
