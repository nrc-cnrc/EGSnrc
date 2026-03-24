#pragma once
#include <string>
#include "estar_formulaCalcs.h"
#include "estar_dataParser.h"

/*! \brief Output a density correction file.
 *
 *  Writes the density correction factors, energy grid, mean I-value,
 *  atomic numbers and mass fractions to a file in the standard EGSnrc
 *  density correction format.
 */
void outputDensityFile(float mediaDensity, double *densityCorr, double *enGrid,
                       float *meanIval, formula_calc fc, std::string outputFilename);

/*! \brief Compute density correction factors.
 *
 *  Takes the processed input arrays and variables and computes the density
 *  correction factors using the ESTAR method (Sternheimer 1984).
 *  Stores the results in densityCorr and enGrid.
 *
 *  Returns 0 on success, 9 on error.
 */
int estarCalculation(int isCompound, int NEP, float mediaDensity,
                     std::string *elementArray, double *massFraction,
                     float *numOfAtoms, double *densityCorr, double *enGrid,
                     float *meanIval, float *ipotval, int mediaNum,
                     std::string outputFilename);

struct bspol {
    double density_corr;
    int lb_index;
    int ub_index;

};

bspol fbspol(double s, double x[1000], double a[1000], double b[1000], double c[1000],double d[1000], int n);

struct scof {
    double a[1200];
    double b[1200];
    double c[1200];
    double d[1200];
};

scof fscof(int nmax, double x[1200], double f[1200]);

double objective_function(double tau, double f[1000], double eps[1000], int nmax, double x);

double bisec(double lowerbound, double upperbound, double tolerance, double tau, double f[1000], double eps[1000], int nmax);
