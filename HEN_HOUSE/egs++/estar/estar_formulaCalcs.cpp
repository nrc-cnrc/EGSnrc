#include <iostream>
#include <iomanip>
#include <vector>
#include <math.h>
#include "estar_formulaCalcs.h"
#include "estar_dataParser.h"
#include "estar_dataTables.h"
#include "egs_functions.h"

// The objective of this module is to determine whether to call
// fcalc() or mixtureCalculation()

namespace {
/*
    The class contains information on pre-processing compound formula
*/
class compFormulaPreprocess {
public:
    struct RestructureCompound {
        int finalNumOfElems; // the number of different elements present in the compound
        string finalElemArray[100]; // array containing the elements present
        float finalNumAtoms[100]; // array containing the number of atoms of each element present
    };

    // Here I am constructing a function which takes in the elemArray and massFraction arrays
    // and returns a string with the whole formula of the compound.
    // for example if inputElemArray = {H,H,O} and inputNumAtomArray = {2,2,1},
    // then the function returns an object of RestructureCompound where:
    // * finalElemArray = {H,O}
    // * finalNumAtoms = {2,1}
    // * finalNumOfElems = 2
    RestructureCompound compRes(string *inputElemArray, float *inputNumAtomArray, int NEP) {
        // NEP is the length of inputElemArray and inputNumAtomArray
        int j = 0;
        int perTableLength = 100; // we work with elements from atomic number 1-100
        int elemPresent[perTableLength]; // elemPresent[i-1] is set to 1 if element with Z=i is
        // present at least once in the compound
        float numAtomsArray[perTableLength]; // numAtomsArray[i-1] stores the number of atoms of Z=i
        // present in the compound
        string tempElemArray[perTableLength];
        while (j < perTableLength) {
            // initialize elemPresent to contain zeros
            elemPresent[j] = 0;
            j = j + 1;
        }
        // Now get the corresponding atomic number array
        vector<int> atomicNumArray(NEP);
        int i = 0;
        while (i < NEP) {
            auto it = atomic_number.find(inputElemArray[i]);
            if (it == atomic_number.end()) {
                egsFatal("estar::compRes: Unrecognised element symbol '%s' at index %d.\n"
                         "Check the formula input.\n", inputElemArray[i].c_str(), i);
            }
            atomicNumArray[i] = it->second;

            i = i + 1;
        }
        i = 0;
        int zIndex; // zIndex = atomic number -1
        int numDiffAtoms = 0;
        while (i < NEP) {
            zIndex = atomicNumArray[i] - 1; // define this for simplicity

            if (zIndex < 0 || zIndex >= perTableLength) {
                egsFatal("estar::compRes: Atomic number index %d is out of bounds [0, %d).\n"
                         "Element '%s' may not be in the periodic table.\n",
                         zIndex, perTableLength, inputElemArray[i].c_str());
            }

            if (elemPresent[zIndex] == 0) {
                elemPresent[zIndex] = 1;
                numAtomsArray[zIndex] = inputNumAtomArray[i];
                tempElemArray[zIndex] = inputElemArray[i]; // we use tempElemArray for simplicity
                numDiffAtoms = numDiffAtoms + 1;
            }
            else {
                // elemPresent[zIndex] is 1
                numAtomsArray[zIndex] = numAtomsArray[zIndex] + inputNumAtomArray[i];
            }
            i = i + 1;
        }
        int k = 0;
        RestructureCompound compForm;
        compForm.finalNumOfElems = numDiffAtoms;
        for (int m=0; m < perTableLength; ++m) {
            if (elemPresent[m] == 1) {
                compForm.finalElemArray[k] = tempElemArray[m];
                compForm.finalNumAtoms[k] = numAtomsArray[m];
                k = k + 1;
            }
        }
        return compForm;
    }

    // This function produces the chemical formula of a compound from the
    // elementrray and numofAtoms array
    string getCompFormula(string *elementArray, float *numOfAtoms, int NEP, int mediaNum) {
        vector<int> numberOfAtoms(NEP);
        vector<string> numberOfAtomsStr(NEP);
        string compoundFormula = "";
        for (int i=0; i < NEP; ++i) {
            numberOfAtoms[i] = static_cast<int>(numOfAtoms[i]); // convert float to int
            numberOfAtomsStr[i] = to_string(numberOfAtoms[i]); // convert int to string
        }

        // now we produce the final string
        for (int i=0; i < NEP; ++i) {
            compoundFormula = compoundFormula + elementArray[i] + numberOfAtomsStr[i];
        }
        cout << "\n";
        cout << "The medium " << mediaNum << " is a compound of " << NEP << " elements with formula: " << compoundFormula << "\n";
        return compoundFormula;
    }
};
}


/*
    This is a simple function which runs either fcalc or mixtureCalculation
    depending on whether the substance is a compound/element or whether it is a mixture respectively
*/
formula_calc getDataFromFormulae(int knmat, double rho, string *elementArray, double *massFraction, float *numOfAtoms, int NEP, int mediaNum) {
    formula_calc fc;
    string formula;
    string formulaCompound;
    if (knmat == 0) { // Element
        formula = elementArray[0];
        fc = fcalc(knmat, rho, formula);

        egsInformation("\nestar::getDataFromFormulae: Medium %d treated as element.\n", mediaNum);

        return fc;
    }
    else if (knmat == 1) { // Compound
        compFormulaPreprocess compObject; // Pre-processing needed only if material is a compound
        compFormulaPreprocess::RestructureCompound rc = compObject.compRes(elementArray, numOfAtoms, NEP);
        string compFormula = compObject.getCompFormula(rc.finalElemArray, rc.finalNumAtoms, rc.finalNumOfElems, mediaNum);
        fc = fcalc(knmat, rho, compFormula);

        egsInformation("\nestar::getDataFromFormulae: Medium %d treated as compound.\n", mediaNum);

        return fc;
    }
    else { // Mixture
        fc = mixtureCalculation(rho, elementArray, massFraction, NEP);

        egsInformation("\nestar::getDataFromFormulae: Medium %d treated as mixture.\n", mediaNum);

        return fc;
    }
}

/*
    The purpose of this module is to get a formula_calc object
    containing relevant data for a single chemical formula like:
    Na, Cl, NaCl2, H2O etc.
    For mixtures please refer to mixformula.cpp
*/

// The function takes in the element name as a string and simply returns
// the atomic number by using the atomic_number dictionary
int atom_num(string elem_name) {
    auto it = atomic_number.find(elem_name);
    if (it == atomic_number.end()) {
        egsFatal("estar::atom_num: Unrecognised element symbol '%s'.\n"
                 "Check the formula input, they must be characters not integers.\n", elem_name.c_str());
    }
    return it->second;
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

    if (mmax > numElemsPerTable) {
        egsFatal("estar::fcalc: Parsed element type count %d exceeds maximum of %d.\n"
                 "Formula '%s' may be malformed.\n",
                 mmax, numElemsPerTable, elemName.c_str());
    }

    int atomic_number_element;
    fc.mmax = pf.elem_types;
    double nz[numElemsPerTable]; // initialize array with numElemsPerTable elements
    int i = 0;
    while (i < mmax) {
        fc.jz[i] = atom_num(pf.str_arr[i]); // for each element we get the atomic number
        atomic_number_element = fc.jz[i];

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

    if (asum == 0.0) {
        egsFatal("estar::fcalc: Total atomic mass sum is zero for formula '%s'.\n"
                 "Check that atom counts are non-zero.\n", elemName.c_str());
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
    double potm = 0.0;
    double za;

    // g/cm^3 threshold below which a material is treated as a gas for I-value selection
    const double rhocut = 0.1;

    m = 0;

    while (m < mmax) {
        jm = fc.jz[m];

        za = fc.jz[m]/atb[jm-1]; // ratio of atomic number to atomic mass
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

        if (potm <= 0.0) {
            egsFatal("estar::fcalc: I-value potm=%g is non-positive for Z=%d.\n"
                     "Cannot take log. Check poth/potgas/potcon tables.\n", potm, jm);
        }

        potl = potl + fc.wt[m]*za*log(potm); // This equation represents equation 5.3 of ICRU 37.
        // fc.wt[m] is w[m] and za is Z[i]/A[i] of ICRU 37
        m = m + 1;
    }
    // fc.pot is the I-Value
    if (fc.zav == 0.0) {
        egsFatal("estar::fcalc: Mean Z/A (zav) is zero for formula '%s'.\n"
                 "Cannot compute I-value.\n", elemName.c_str());
    }
    fc.pot = exp(potl/fc.zav); // we remove the log in equation 5.3 (ICRU 37) and divide by <Z/a> TO GET THE I-value
    // Note that fc.zav in the code is is <Z/a> of ICRU 37 equation 5.3
    return fc;
};

// This processes the input data and puts them in a mixtureData structure object
mixtureData getEgsMediaData(string *elementArray, double *massFraction, int NEP) {
    mixtureData md;
    md.ncomp = NEP;
    int ncomp = NEP;
    if (ncomp <= 0) {
        egsFatal("estar::getEgsMediaData: Number of components must be > 0, got %d.\n", ncomp);
    }
    for (int i=0; i < ncomp; ++i) {
        md.frm[i] = elementArray[i];
    }

    double sumf = 0;
    for (int i=0; i < ncomp; ++i) {
        if (massFraction[i] <= 0) {
            egsFatal("estar::getEgsMediaData: Mass fraction for component %d is %g.\n"
                     "Mass fractions must be > 0.\n", i, massFraction[i]);
        }
        md.frac[i] = massFraction[i];
        sumf = sumf + md.frac[i];
    }
    // normalize
    for (int i=0; i < ncomp; ++i) {
        md.frac[i] = md.frac[i]/sumf;
    }
    return md;
}

// Here we find the I-value of the whole mixture
// we return a formula_calc structure object where
// the object has the relevant properties of the mixture which are:
// 1. I-value of the mixture
// 2. fractional weight of each element used
// 3. atomic number of the different elements present
formula_calc mixtureCalculation(double rho, string *elementArray, double *massFraction, int NEP) {

    mixtureData md = getEgsMediaData(elementArray, massFraction, NEP);
    int numComp = md.ncomp;
    vector<string> formulaArray(numComp); // array containing all the formula
    vector<double> fractionArray(numComp); // array contaning all the weights
    for (int i = 0; i < numComp; i++) {
        formulaArray[i] = md.frm[i];
        fractionArray[i] = md.frac[i];
    };
    int num_elems = 100; // we work with elements from atomic number 1-100
    bool lh[num_elems];
    double wate[num_elems];
    for (int j=0; j < num_elems; ++j) {
        // lh and wate to contain zeros
        lh[j] = 0;
        wate[j] = 0.0;
    }
    int atmoicNumIndex;
    vector<double> zavArray(numComp); // array containing Z/A of each component
    vector<double> potArray(numComp); // array containing I-Value of each component

    for (int i = 0; i < numComp; i++) {
        formula_calc fc; // this object is redefined for every different formula used in the mixture
        fc = fcalc(2, rho, formulaArray[i]);
        for (int j = 0; j < fc.mmax; j++) {
            atmoicNumIndex = fc.jz[j] - 1; // define this for simplicity
            /*
            lh is used to denote whether a particular element was not encountered before
            or whether it was part of some other compounds used in the mixture
            */
            if (lh[atmoicNumIndex] == 0) {
                lh[atmoicNumIndex] = 1;
                wate[atmoicNumIndex] = md.frac[i]*fc.wt[j]; // if we encounter a new element, we do this
            }
            else {
                wate[atmoicNumIndex] = wate[atmoicNumIndex] + md.frac[i]*fc.wt[j]; // if we encounter
                // an element which was part of a previous compound we add
                // the previous weight to  md.frac[i]*fc.wt[j]
            }
        }
        zavArray[i] = fc.zav;
        potArray[i] = fc.pot;

    }
    formula_calc ffc; // this object contains the final data we want to return
    int index = 0;
    for (int k = 0; k < num_elems; k++) {
        if (lh[k]) {
            /*
                if lh[k] == 1, this means the element with atomic number k+1 is
                part of some compound of the mixture or is in elemental form in the mixture
            */
            // here we just put the atomic numbers and weights of the elements present in the mixture in
            // arrays.
            ffc.jz[index] = k+1;
            ffc.wt[index] = wate[k];

            index = index + 1;
        }
    }

    int numDiffElemsUsed = index; // this is the number of differents used.
    // * example: if a mixture is made with NaCl and H2O, numDiffElemsUsed will be 4
    ffc.mmax = numDiffElemsUsed;
    ffc.zav = 0.0;
    double potl = 0.0;
    for (int i = 0; i < numComp; i++) {
        ffc.zav = ffc.zav + fractionArray[i]*zavArray[i];  // --------------------------(i)
        potl = potl+fractionArray[i]*zavArray[i]*log(potArray[i]); // -----------------(ii)
    }
    if (ffc.zav == 0.0) {
        egsFatal("estar::mixtureCalculation: Mixture mean Z/A is zero.\n"
                 "Check that mass fractions and element Z/A values are non-zero.\n");
    }
    ffc.pot = exp(potl/ffc.zav); // --------------------------------------------------(iii)
    // equations i,ii and iii are used to find the I-value of the mixture from equation 5.3 of ICRU 37
    return ffc;
}

