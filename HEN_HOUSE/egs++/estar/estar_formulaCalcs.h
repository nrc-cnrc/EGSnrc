/*
###############################################################################
#
#  EGSnrc estar
#  Copyright (C) 2026 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Sehmimul Hoque, 2022
#
#  Contributors:    Martin J. Berger
#                   Johnathan S. Coursey
#                   Reid Townson
#                   Ernesto Mainegra-Hing
#
#  Based on the original ESTAR code by Martin J. Berger,
#  National Institute of Standards and Technology (NIST). Including
#  modifications by Johnathan S. Coursey.
#
###############################################################################
*/

#pragma once

#include <string>

using namespace std;

/*
    Here we define a structure that will be useful to store the information of a
    substance (element/compound or molecule)
    The information stored are:

    *  1. jz[]: this is an array containing the atomic numbers of each of the elements
                present in the mixture. The elements can be present in the mixture as part of
                a compound or just as an element.
                There are 100 elements in our table of elements.
    *  2. wt[]: this is an array containing the weight of each of the elements
                present in the mixture. The elements can be present in the mixture as part of
                a compound or just as an element. This means for the element with atomic
                number jz[i], the weight in the mixture/compound/element is wt[i]
    *  3. zav:  This is the Z/A as used in equation 4 (Sternheimer 1984)
    *  4. pot:  This is the I-value (mean ionization energy in eV) of the substance
    *  5. mmax: The number of different elements present in the substance
    *           For example, if a mixture is made with NaCl and H2O, mmax will be 4
    *
*/
struct formula_calc {
    double wt[100];
    int jz[100];
    double zav;
    double pot;
    int mmax;
};

formula_calc getDataFromFormulae(int knmat, double rho, string *elementArray, double *massFraction, float *numOfAtoms, int NEP, int mediaNum);

/*
    This module provides functions for computing formula_calc objects
    for a single chemical formula (e.g. Na, Cl, NaCl2, H2O).
    For mixtures, refer to mixformula.cpp.
*/

/*! \brief Returns the atomic number for a given element name.
 *
 *  Looks up the element name in the atomic_number periodic table dictionary
 *  and returns the corresponding atomic number.
 */
int atom_num(std::string elem_name);

/*! \brief Computes a formula_calc object for a single element or compound.
 *
 *  \param knmat  Type of material: 0 = element, 1 = compound, 2 = mixture.
 *  \param rho    Density of the material (g/cm^3).
 *  \param elemName  Name of the element or compound (e.g. "H2O", "NaCl").
 *
 *  Returns a formula_calc struct containing the I-value, mean Z/A,
 *  weight fractions, and atomic numbers of constituent elements.
 *  These are used downstream to compute density corrections.
 */
formula_calc fcalc(int knmat, double rho, std::string elemName);

/*
    This module provides functions for computing formula_calc objects
    for a single chemical formula (e.g. Na, Cl, NaCl2, H2O).
    For mixtures, refer to mixformula.cpp.
*/

int const max_comp = 100;

// this struct stores the data present in a mixture
struct mixtureData {
    int ncomp; // number of COMPONENTS in mixture
    // * example: if a mixture is made with NaCl and H2O, ncomp will be 2
    string frm[max_comp];// array containing each formula
    // * example: if a mixture is made with NaCl and H2O, frm[] will be ["NaCl", "H2O"]
    double frac[max_comp];// array containing fraction by weight of each subsatance used in mixture
    // * example: if a mixture is made with 0.8 NaCl and 0.2 H2O, frac[] will be [0.8, 0.2]
    // NOTE: weights do not need to be normalized as program automatically normalizes weights
};

mixtureData getData();

mixtureData getEgsMediaData(string *elementArray, double *massFraction, int NEP);

formula_calc mixtureCalculation(double rho, string *elementArray, double *massFraction, int NEP);
