#include <iostream>
using namespace std;
/*
    Here we define a structure that will be useful to store the information of a 
    substance (element/compound or molecule)
    The information stored are:
    *  1. jz[]: this is an array containing the atomic numbers of each of the elements 
                present in the mixture. The elements can be present in the mixture as part of
                a compound or just as an element. 
    *  2. wt[]: this is an array containing the weight of each of the elements 
                present in the mixture. The elements can be present in the mixture as part of
                a compound or just as an element. This means for the element with atomic 
                number jz[i], the weight in the mixture/compound/element is wt[i]
    *  3. zav:  This is the Z/A as used in equation 4 (Sternheimer 1948)
    *  4. pot:  This is the I-value (mean ionization energy in eV) of the substance
    *  5. mmax: The number of different elements present in the substance
    *           For example, if a mixture is made with NaCl and H2O, mmax will be 4
*/
struct formula_calc {
    double wt[200];
    int jz[200];
    double zav;
    double pot;
    int mmax;
};