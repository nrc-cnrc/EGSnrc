#pragma once

#include <string>

/*
    This module reads data from the elementData.h file and structures it
    for use in other modules. There are 100 sets of data in elementData.h
    (indexed by atomic number - 1), where each set corresponds to one element.
    * For example: the third set of data in elementData.h is the data for Lithium.
*/

/*! \brief Holds the dispersion oscillator data for all 100 elements.
 *
 *  Populated by parseData() from the raw flat array in elementData.h.
 *  Arrays are indexed by (atomic_number - 1), i.e. index 0 = Hydrogen,
 *  index 5 = Carbon, etc.
 */
struct ElementOscillatorData {

    /*! \brief Number of dispersion oscillators for each element.
     *
     *  nmax[Z-1] is the number of dispersion oscillators required to
     *  describe a medium made solely of the element with atomic number Z.
     *  Note that for mixtures/compounds nmax is modified later in
     *  estarMainCalc.cpp.
     *  * example: nmax[6-1] is the number of oscillators for Carbon.
     */
    int nmax[100];

    /*! \brief Number of elements in the standard energy grid. */
    int numLevelsStandard;

    /*! \brief Number of electrons in each subshell for each element.
     *
     *  nc[Z-1][j] is the number of electrons in the j-th oscillator subshell
     *  for the element with atomic number Z, where 0 <= j < nmax[Z-1].
     *  * example: the L-III subshell has 4 electrons, so nc[i-1][j] = 4
     *    for the appropriate j when Z >= 10.
     *  Refer to the report for which portion of elementData.h holds subshell data.
     */
    int nc[100][26];

    /*! \brief Absorption edge energies (in units of hbar) for each oscillator.
     *
     *  bd[Z-1][j] is the absorption edge of the (j+1)-th oscillator for the
     *  element with atomic number Z, where 0 <= j < nmax[Z-1].
     *  planck_constant * bd[Z-1][j] gives the absorption edge energy.
     *  Refer to the report for which portion of elementData.h holds this data.
     */
    double bd[100][26];
};

/*! \brief Parses elementData.h into a structured ElementOscillatorData object.
 *
 *  Reads the raw flat array elementData[] and unpacks it into the nmax,
 *  nc and bd arrays of the returned struct. The rlos (loss function)
 *  values present in elementData.h are read but not stored, as they are
 *  not used by the ESTAR calculation.
 *
 *  Returns a fully populated ElementOscillatorData struct.
 */
ElementOscillatorData parseData();

/*
    This module parses and interprets a single chemical formula for a
    compound or element. The final product is a parseformula object.
*/

/*! \brief Holds the parsed representation of a chemical formula.
 *
 *  Produced by parse() from a formula string such as "MgCl2" or "H2O".
 *  * example: for MgCl2
 *      str_arr  = ["Mg", "Cl"]
 *      num_arr  = [1, 2]
 *      elem_types = 2
 */
struct parseformula {
    std::string str_arr[100]; // stores the chemical symbol of each element
    // present in the compound/element
    int num_arr[100];         // stores the number of atoms of the corresponding
    // element in str_arr
    // * example: for H2O, num_arr = [2, 1]
    int elem_types;           // number of different types of elements present
    // * example: for H2O, elem_types = 2
};

/*! \brief Parses a chemical formula string into a parseformula object.
 *
 *  Accepts a single chemical formula string (e.g. "Na", "MgCl2", "H2O")
 *  and splits it into its constituent element symbols and atom counts.
 *
 *  Prints an error message and asserts if the formula is incorrectly
 *  formatted (e.g. "nA" instead of "Na" — element symbols must begin
 *  with an uppercase letter).
 *
 *  \param str  The chemical formula string to parse.
 *  \returns    A fully populated parseformula struct.
 */
parseformula parse(std::string str);

