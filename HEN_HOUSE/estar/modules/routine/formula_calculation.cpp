#include <iostream>
#include <iomanip>
#include "formulaStruct.cpp"
#include "formula.h"
#include "mixformula.h"
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
        cout << "The medium " << mediaNum << " is a compound with formula: " << compoundFormula << "\n";
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
    cout << "\n-------------------------\n";
    cout << "== MEDIUM " << mediaNum << " BLOCK FOR ESTAR ==\n";
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
