#include <iostream>
#include <iomanip>
#include <assert.h>
using namespace std;

/*
    The purpose of this module is to get a formula_calc object
    containing relevant data for a mixture.
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
        assert (ncomp >= 0);
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
            assert (weight >= 0);
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
        assert (ncomp >= 0);
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
            assert (massFraction[i] > 0);
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
        cout << "formula is " << formulaArray[i] << " with fraction " << fractionArray[i] << "\n";
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
            } else {
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

