#include "mixformula.cpp"

struct formula_calc;

struct mixtureData;

mixtureData getData();

mixtureData getEgsMediaData(string *elementArray, double *massFraction, int NEP);

formula_calc mixtureCalculation(double rho, string *elementArray, double *massFraction, int NEP);
