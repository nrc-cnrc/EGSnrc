#include "newtonbisec.cpp"

double objective_function (double tau, double f[1000], double eps[1000], int nmax, double x);

double newton_bisec(double lowerbound, double upperbound, double tolerance, double tau, double f[1000], double eps[1000], int nmax);
