#include <iostream>
#include <math.h>
#include "../parse.h"
#include <assert.h>
using namespace std;

/*
    The purpose of this module is to get a formula_calc object
    containing relevant data for a single chemical formula like:
    Na, Cl, NaCl2, H2O etc.
    For mixtures please refer to mixformula.cpp
*/

// The function takes in the element name as a string and simply returns
// the atomic number by using the per_table dictionary
int atom_num(string elem_name) {
    int atomic_num;
    atomic_num = per_table[elem_name];
    return atomic_num;
}

// formula_calc is a structure we defined in the module formulaStruct.cpp.
// The input of fcalc are :
//   * knmat := type of material (0-> element; 1->compound; 2:->mixture)
//   * rho := density of material
//   * elemName := is the name of the element/compound and is a string
// The return output is a structure formula_calc which contains some
// parameters (including i-value) which will be used to find the density corrections
// The parameters inside the function below.
formula_calc fcalc(int knmat, double rho, string elemName) {
    int numElemsPerTable = 100; // as 100 elements are present in our periodic table
    formula_calc fc; // we define fc to be an object of struct formula_calc
    parseformula pf = parse(elemName);

    // mmax is the number of different types of elements present in the substance.
    // for example if elemName == H2O, mmax will be 2.
    // if elemName == H2OMgClH, mmax will be 4.
    int mmax = pf.elem_types;
    int atomic_number_element;
    fc.mmax = pf.elem_types;
    double nz[numElemsPerTable]; // initialize array with numElemsPerTable elements
    int i = 0;
    while (i < mmax) {
        fc.jz[i] = atom_num(pf.str_arr[i]); // for each element we get the atomic number
        atomic_number_element = fc.jz[i];
        // if you mistype a formula, the atomic_number_element will be 0
        if (atomic_number_element<=0) {
            cout << "Incorrect formula" << "\n";
            cout << "\n***************\n";
            cout << "You have mistyped the element name or have an element whose atomic number is more than 100 \n***************\n";
        }
        assert(atomic_number_element > 0);
        /* below we have nz[i] = pf.num_arr[i]. Now pf.num_arr[i] produces
           the number of each atom present in the element/compound.
           *   For example: for H20, nz[0] will be 2 while nz[1] will be 1.
           *   For example: for Cl2, nz[0] will be 2.
        */
        nz[i] = pf.num_arr[i];
        i = i + 1;
    }

    double asum = 0.0;
    int jm;
    /* After the while loop below runs, we get asum.
       The final asum we get (at end of while loop) is the:
       sum of ATOMIC_MASS_OF_ELEMENT_i * NUMBER_OF_ATOMS_WITH_ATOMIC_NUMBER_i
       *  for example, for H2O,
       asum = 1.007940 * 2 + 32.0660 * 1
    */
    int m = 0;
    while (m < mmax) {
        jm = fc.jz[m];
        asum = asum+atb[jm-1]*nz[m];
        m = m + 1;
    }

    /* After the while loop below runs, we get fc.wt.
       The final fc.wt we get (at end of while loop) is the:
       normalized sum of ATOMIC_MASS_OF_ELEMENT_i * NUMBER_OF_ATOMS_WITH_ATOMIC_NUMBER_i
       *  for example, for H2O,
       fc.wt[0] = (1.007940 * 2)/(1.007940 * 2 + 32.0660 * 1)
       fc.wt[1] = (32.0660 * 1)/(1.007940 * 2 + 32.0660 * 1)
       Thus fc.wt gives the weight by mass of each element present in the compound/element
    */
    m = 0;
    while (m < mmax) {
        jm = fc.jz[m];
        fc.wt[m] = atb[jm-1]*(nz[m]/asum);
        m = m + 1;
    }

    fc.zav = 0.0;
    double potl = 0.0;
    double potm;
    double za;
    double rhocut = 0.1;
    m = 0;

    while (m < mmax) {
        jm = fc.jz[m];
        double jm_temp = fc.jz[m]; // we just define jm_temp as a double
        // for better accuracy (but this might be unnecessary and we could have
        // simply used jm)

        za = jm_temp/atb[jm-1]; // ratio of atomic number to atomic mass
        fc.zav = fc.zav + fc.wt[m]*za; // This Z/A is the same as the Z/A in equation 4 (Sternheimer 1948)
        // You can simple replace fc.wt[m] and za with their definitions
        // to arrive at the formula:
        // fc.zav = (total number of electrons)/(sum of atomic weights of constituent atoms)
        // as given by Sternheimer 1948 just below equation 4.

        if (knmat >= 1) { // This ensures only elements do not get boosted
            if (jm>=10) {
                // ---
                // This code snippet contains modifications made by Ernesto
                if (mmax>1) {
                    potm = 1.13*poth[jm-1]; // The 1.13 factor arises from the 'Others' condition in ICRU 37 - table 5.1
                }
                else {
                    potm = poth[jm-1];
                }
                // ---
            }
            else {
                if (rho <= rhocut) { // using <= gives correct output in ESTAR. However < was used in ESTAR
                    // Please see Rhocut Error section in my report
                    // Now when rho <= rhocut, the code assumes the material is in gaseous form
                    // and thus uses potgas.
                    potm = potgas[jm-1];
                }
                else {
                    potm = potcon[jm-1];// Now when rho > rhocut, the code assumes the material is in solid/liquid form
                    // and thus uses potcon.
                }
            }
        }
        else {
            potm = poth[jm-1];
        }
        potl = potl + fc.wt[m]*za*log(potm); // This equation represents equation 5.3 of ICRU 37.
        // fc.wt[m] is w[m] and za is Z[i]/A[i] of ICRU 37
        m = m + 1;
    }
    // fc.pot is the I-Value
    fc.pot = exp(potl/fc.zav); // we remove the log in equation 5.3 (ICRU 37) and divide by <Z/a> TO GET THE I-value
    // Note that fc.zav in the code is is <Z/a> of ICRU 37 equation 5.3
    return fc;
};
