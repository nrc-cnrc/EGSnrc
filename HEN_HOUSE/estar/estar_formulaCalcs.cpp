#include <iostream>
#include <iomanip>
#include <math.h>
#include <assert.h>
#include "estar_formulaCalcs.h"
#include "estar_dataParser.h"
#include "estar_dataTables.h"

using namespace std;

// The objective of this module is to determine whether to call
// fcalc() or mixtureCalculation()

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
            // initialize elemPresent and elemPresent to contain zeros
            elemPresent[j] = 0;
            elemPresent[j] = 0.0;
            j = j + 1;
        }
        // Now get the corresponding atomic number array
        int atomicNumArray[NEP];
        int i = 0;
        while (i < NEP) {
            atomicNumArray[i] = per_table[inputElemArray[i]];
            i = i + 1;
        }
        i = 0;
        int zIndex; // zIndex = atomic number -1
        int numDiffAtoms = 0;
        while (i < NEP) {
            zIndex = atomicNumArray[i] - 1; // define this for simplicity
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
        int m = 0;
        RestructureCompound compForm;
        compForm.finalNumOfElems = numDiffAtoms;
        while (m < perTableLength) {
            if (elemPresent[m] == 1) {
                compForm.finalElemArray[k] = tempElemArray[m];
                compForm.finalNumAtoms[k] = numAtomsArray[m];
                k = k + 1;
            }
            m = m + 1;
        }
        return compForm;
    }

    // This function produces the chemical formula of a compound from the
    // elementrray and numofAtoms array
    string getCompFormula(string *elementArray, float *numOfAtoms, int NEP, int mediaNum) {
        RestructureCompound compRestruct;
        int numberOfAtoms[NEP];
        string numberOfAtomsStr[NEP];
        int i = 0;
        string compoundFormula = "";
        while (i < NEP) {
            numberOfAtoms[i] = static_cast<int>(numOfAtoms[i]); // convert float to int
            numberOfAtomsStr[i] = to_string(numberOfAtoms[i]); // convert int to string
            i = i + 1;
        }
        i = 0;
        // now we produce the final string
        while (i < NEP) {
            compoundFormula = compoundFormula + elementArray[i] + numberOfAtomsStr[i];
            i = i + 1;
        }
        cout << "\n";
        cout << "The medium " << mediaNum << " is a compound of " << NEP << " elements with formula: " << compoundFormula << "\n";
        return compoundFormula;
    }
};


/*
    This is a simple function which runs either fcalc or mixtureCalculation
    depending on whether the substance is a compound/element or whether it is a mixture respectively
*/
formula_calc getDataFromFormulae(int knmat, double rho, string *elementArray, double *massFraction, float *numOfAtoms, int NEP, int mediaNum) {
    formula_calc fc;
    string formula;
    string formulaCompound;
    if (knmat == 0) {
        formula = elementArray[0];
        fc = fcalc(knmat, rho, formula);
        cout << "\n";
        cout << "Medium " << mediaNum << " treated as element in ESTAR\n";
        return fc;
    }
    else if (knmat == 1) {
        compFormulaPreprocess compObject; // Pre-processing needed only if material is a compound
        compFormulaPreprocess::RestructureCompound rc = compObject.compRes(elementArray, numOfAtoms, NEP);
        string compFormula = compObject.getCompFormula(rc.finalElemArray, rc.finalNumAtoms, rc.finalNumOfElems, mediaNum);
        fc = fcalc(knmat, rho, compFormula);
        return fc;
    }
    else {
        fc = mixtureCalculation(rho, elementArray, massFraction, NEP);
        cout << "\n";
        cout << "Medium " << mediaNum << " treated as mixture in ESTAR\n";
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

// This is a simple function where we output an object of mixtureData structure
// where the object has the relevant data of the mixture
// UPDATE: getData() is no longer called as we get the data from .egsinp files and not
//         through command prompts anymore
mixtureData getData() {
    mixtureData md;
    int ncomp;
    cout << "how many components? ";
    cin >> ncomp;
    if (ncomp <= 0) {
        cout << "\n***************\n";
        cout << "Error! Number of components must be greater than 0 and an integer";
        cout << "\n***************\n";
        assert(ncomp >= 0);
    };
    cout << "num is " << ncomp << "\n";
    md.ncomp = ncomp;
    string formula;
    double weight;
    int i = 0;
    double sumf = 0.0;
    while (i < ncomp) {
        cout << "give formula for component " << i+1 << ": ";
        cin >> formula;
        md.frm[i] = formula;

        cout << "give fraction by weight for component " << i+1 << ": ";
        cin >> weight;
        if (weight <= 0) {
            cout << "\n***************\n";
            cout << "Error! weight must be greater than 0";
            cout << "\n***************\n";
            assert(weight >= 0);
        };
        md.frac[i] = weight;

        sumf = sumf + md.frac[i];
        i = i+1;
    }
    // normalize
    i = 0;
    while (i < ncomp) {
        md.frac[i] = md.frac[i]/sumf;
        i = i + 1;
    }
    return md;
}

// This processes the input data and puts them in a mixtureData structure object
mixtureData getEgsMediaData(string *elementArray, double *massFraction, int NEP) {
    mixtureData md;
    md.ncomp = NEP;
    int ncomp = NEP;
    if (ncomp <= 0) {
        cout << "\n***************\n";
        cout << "Error! Number of components must be greater than 0 and an integer";
        cout << "\n***************\n";
        assert(ncomp >= 0);
    };
    int i = 0;
    while (i < ncomp) {
        md.frm[i] = elementArray[i];
        i = i + 1;
    }
    i = 0;
    double sumf = 0;
    while (i < ncomp) {
        if (massFraction[i] <= 0) {
            cout << "\n***************\n";
            cout << "Error! mass fraction must be greater than 0";
            cout << "\n***************\n";
            assert(massFraction[i] > 0);
        };
        md.frac[i] = massFraction[i];
        sumf = sumf + md.frac[i];
        i = i + 1;
    }
    // normalize
    i = 0;
    while (i < ncomp) {
        md.frac[i] = md.frac[i]/sumf;
        i = i + 1;
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
    string formulaArray[numComp]; // array containing all the formula
    double fractionArray[numComp]; // srray contaning all the weights
    for (int i = 0; i < numComp; i++) {
        formulaArray[i] = md.frm[i];
        fractionArray[i] = md.frac[i];
    };
    int j = 0;
    int num_elems = 100; // we work with elements from atomic number 1-100
    double lh[num_elems];
    double wate[num_elems];
    while (j < num_elems) {
        // lh and wate to contain zeros
        lh[j] = 0;
        wate[j] = 0.0;
        j = j + 1;
    }
    int atmoicNumIndex;
    double zavArray[numComp]; // array containing Z/A of each component
    double potArray[numComp]; // array containing I-Value of each component

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
        if (lh[k] == 1) {
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
    ffc.pot = exp(potl/ffc.zav); // --------------------------------------------------(iii)
    // equations i,ii and iii are used to find the I-value of the mixture from equation 5.3 of ICRU 37
    return ffc;
}

