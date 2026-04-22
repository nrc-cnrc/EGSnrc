#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <assert.h>
#include <cmath>

#include "estar.h"
#include "estar_dataTables.h"
#include "egs_functions.h"

/*
    The purpose of this module is to process the arrays and data received from
    pegs4_routine.mortran and then to call the main function (estarCalculation)
    which is given in estarMainCalc.cpp.
    estarCalculation -> calculates the density correction factors

    Note that more pre-processing is done for compounds in formula_calculation.cpp
*/

/*
    This class is used to process the formula array from fortran
    to make the formula array readable by c++
*/
namespace {
class GetElements {
public:
    struct GetElementsStruct {
        // this array contains all elements present in a medium
        string elemArrayStrut[100];
    };
    void tokenize(std::string const &str, const char delim,
            std::vector<std::string> &out) {
        // Construct a stream from the string
        std::stringstream ss(str);

        std::string s;
        while (std::getline(ss, s, delim)) {
            if(s.empty()) continue;
            out.push_back(s);
        }
    }
    // this function below parses fortran array to produce an array
    // which can be used in our C++ estar.
    // This is needed as arrays returned by fortran cannot be read by C++
    // without this pre-processing.
    //
    // Returns true on success, false if the component list is inconsistent
    // with NEP.
    bool getElemArray(char *formulaStr, int NEP, GetElementsStruct &GElem) {
        const char delim = ' ';
        std::vector<std::string> components;
        tokenize(string(formulaStr), delim, components);

        if (components.size() < (size_t)NEP) {
            cout << "\n***************\n";
            cout << "Error: List of elements is inconsistent with the number expected.\n";
            cout << "Expected " << NEP << " elements but only found " << components.size() << ".\n";
            cout << "\n***************\n";
            return false;
        }

        int k = 0;
        while (k < NEP) {
            if(components[k].size() > 1) {
                components[k][0] = static_cast<char>(std::toupper(static_cast<unsigned char>(components[k][0])));
                components[k][1] = static_cast<char>(std::tolower(static_cast<unsigned char>(components[k][1])));
            }
            GElem.elemArrayStrut[k] = components[k];
            k = k + 1;
        }
        return true;
    }
};
}

/*
    The function below takes inputs from pegs4_routines.mortran and then
    calculates the density correction factors. The density correction factors are stored in the
    variable densityCorr. This array is then passed to pegs4_routine where it is read.

    Now we explain the meaning of the variables:

        1. formulaStr   : is a FORTRAN array containing chemical symbols of the
                        elements present.

        2. massFraction : is a FORTRAN array containing mass fractions (weights) of the
                        elements present.

        3. numOfAtoms   : is a FORTRAN array containing number atoms of each of the
                        elements present in the material.

        4. mediaDensity : is the user defined medium density for a particular medium

        5. densityCorr  : is the array containing the density correction factors which are computed
                        by the ESTAR integrated in EGSnrc

        6. enGrid       : is the standard energy grid

        7. NEP          : is the number of elements mentioned in the MEDIA DEFINITION block of
                        the egsinp file.
                        * example: if we have -> elements: H,O,C
                                   then NEP will be 3

        8. iscomp       : is a binary variable to denote if the substance is a compound or not.
                        iscomp = 1 means the material is a compound
                        iscomp = 0 means the material is not a compound

        9. meanIval     : is the ivalue of the material. If the user defines ivalue = a in egsinp file,
                        then, meanIval = a. However, if the user does not define ivalue, then meanIval is the
                        ivalue that is computed by ESTAR.

        10. ipotval     : is used to determine if ivalue has been defined by the user. If the user
                        does not define a custom ivalue, then ipotval = -1 and so ESTAR computes
                        the ivalue. However if the user
                        defines ivalue = a in egsinp file, then ipotval = meanIval = a.

        11. mediaID     : This is the id of the medium. This depends on the order in which
                        the media have been listed in the media input block of an egsinp file.
                        * For example: Say we have -
                            :start media input:
                                media = air, water
                                set medium = 1
                            :stop media input:
                            Then, air will have mediaID = 1 and water will have mediaID = 2.

*/


extern "C" int estar_(char *formulaStr,
                      float *massFraction,
                      float *numOfAtoms,
                      float *mediaDensity,
                      double *densityCorr,
                      double *enGrid,
                      int *NEP,
                      int *ISCOMP,
                      float *meanIval,
                      float *ipotval,
                      int *mediaID,
                      char *outputFilename
                     ) {

    // Validate all pointers before dereferencing
    if (!formulaStr) {
        egsFatal("estar::estar_: formulaStr pointer is null for medium.\n");
    }
    if (!massFraction) {
        egsFatal("estar::estar_: massFraction pointer is null.\n");
    }
    if (!numOfAtoms) {
        egsFatal("estar::estar_: numOfAtoms pointer is null.\n");
    }
    if (!mediaDensity) {
        egsFatal("estar::estar_: mediaDensity pointer is null.\n");
    }
    if (!densityCorr) {
        egsFatal("estar::estar_: densityCorr pointer is null.\n");
    }
    if (!enGrid) {
        egsFatal("estar::estar_: enGrid pointer is null.\n");
    }
    if (!NEP) {
        egsFatal("estar::estar_: NEP pointer is null.\n");
    }
    if (!ISCOMP) {
        egsFatal("estar::estar_: ISCOMP pointer is null.\n");
    }
    if (!meanIval) {
        egsFatal("estar::estar_: meanIval pointer is null.\n");
    }
    if (!ipotval) {
        egsFatal("estar::estar_: ipotval pointer is null.\n");
    }
    if (!mediaID) {
        egsFatal("estar::estar_: mediaID pointer is null.\n");
    }
    if (!outputFilename) {
        egsFatal("estar::estar_: outputFilename pointer is null.\n");
    }

    // Validate the values pointed to
    if (*NEP <= 0 || *NEP > 100) {
        egsFatal("estar::estar_: NEP=%d is out of valid range [1, 100].\n", *NEP);
    }
    if (*mediaID <= 0) {
        egsFatal("estar::estar_: mediaID=%d is invalid, must be > 0.\n", *mediaID);
    }
    if (*ISCOMP != 0 && *ISCOMP != 1) {
        egsFatal("estar::estar_: ISCOMP=%d is invalid, must be 0 or 1.\n", *ISCOMP);
    }
    if (*mediaDensity <= 0.0f) {
        egsFatal("estar::estar_: mediaDensity=%g is invalid, must be > 0.\n",
                *mediaDensity);
    }

    egsInformation("\n-------------------------\n"
                "== MEDIUM %d BLOCK FOR ESTAR ==\n", *mediaID);

    // The lines below process formula_str to make the array readable by estar c++
    GetElements elemObject;
    GetElements::GetElementsStruct GeElems;
    int nepInt = *NEP;
    int isCompInt = *ISCOMP;
    int mediaNum = *mediaID; // this is the media id

    // getElemArray now returns bool; propagate any parse failure back to the
    // Fortran caller rather than crashing via assert.
    if (!elemObject.getElemArray(formulaStr, nepInt, GeElems)) {
        return 9;
    }

    // Use std::vector instead of a VLA (variable-length arrays are a GCC
    // extension, not valid standard C++14).
    vector<string> estarFormulaArrayInput(nepInt);
    vector<double> estarWeightArrayInput(nepInt);

    int i = 0;
    while (i < nepInt) {
        estarFormulaArrayInput[i] = GeElems.elemArrayStrut[i];
        estarWeightArrayInput[i] = massFraction[i];

        egsInformation("estar::estar_: Formula is %s with fraction %g\n",
                    estarFormulaArrayInput[i].c_str(), estarWeightArrayInput[i]);

        i = i + 1;
    }

    double mediumDensity = *mediaDensity;

    // Call the main estar calculation.
    // Check the return value and propagate any error back to the Fortran caller
    // so failures are not silently swallowed.
    int result = estarCalculation(isCompInt, nepInt, mediumDensity,
                                  estarFormulaArrayInput.data(),
                                  estarWeightArrayInput.data(),
                                  numOfAtoms, densityCorr, enGrid,
                                  meanIval, ipotval, mediaNum,
                                  string(outputFilename));
    if (result != 0) {
        egsFatal("estar::estar_: estarCalculation failed for medium %d with error code %d.\n",
         mediaNum, result);
        return result;
    }

    egsInformation("-------------------------\n");

    return 0;
}


extern "C" int compoundstoelements_(char *formulaStr,
                                   double *massFraction,
                                   char *elementStr,
                                   double *rhoz,
                                   double *zelem,
                                   int *ncomp,
                                   int *NEP
                                  ) {

    // Validate all pointers before dereferencing
    if (!formulaStr) {
        egsFatal("estar::compoundstoelements_: formulaStr pointer is null.\n");
    }
    if (!massFraction) {
        egsFatal("estar::compoundstoelements_: massFraction pointer is null.\n");
    }
    if (!elementStr) {
        egsFatal("estar::compoundstoelements_: elementStr pointer is null.\n");
    }
    if (!rhoz) {
        egsFatal("estar::compoundstoelements_: rhoz pointer is null.\n");
    }
    if (!zelem) {
        egsFatal("estar::compoundstoelements_: zelem pointer is null.\n");
    }
    if (!ncomp) {
        egsFatal("estar::compoundstoelements_: ncomp pointer is null.\n");
    }
    if (!NEP) {
        egsFatal("estar::compoundstoelements_: NEP pointer is null.\n");
    }

    // Validate the values pointed to
    if (*ncomp <= 0 || *ncomp > 100) {
        egsFatal("estar::compoundstoelements_: ncomp=%d is out of valid range "
                "[1, 100].\n", *ncomp);
    }

    // In egsnrc.macros, the max number of elements per medium is $MXELE=50
    // And these arrays are 50 characters long, so we can restrict to that size
    const size_t MAX_ELEMENT_STR_SIZE = 50 * 50; // 50 elements * 50 chars each

    GetElements elemObject;
    GetElements::GetElementsStruct GeElems;
    int numCompounds = *ncomp;

    // getElemArray now returns bool; propagate any parse failure back to the
    // Fortran
    if (!elemObject.getElemArray(formulaStr, numCompounds, GeElems)) {
        return 9;
    }

    // Use std::vector instead of VLAs (variable-length arrays are a GCC
    // extension, not valid standard C++14).
    vector<string> estarFormulaArrayInput(numCompounds);
    vector<double> estarWeightArrayInput(numCompounds);

    for(size_t i=0; i < (size_t)numCompounds; ++i) {
        estarFormulaArrayInput[i] = GeElems.elemArrayStrut[i];
        estarWeightArrayInput[i] = massFraction[i];
    }

    double rho = 1; // Material density not actually needed, so use a dummy
    formula_calc fc = mixtureCalculation(rho,
                                         estarFormulaArrayInput.data(),
                                         estarWeightArrayInput.data(),
                                         numCompounds);

    // Set NEP to actually be the number of elements now, instead of the number of compounds
    NEP[0] = fc.mmax;

    size_t charPos = 0;
    for(size_t i=0; i != fc.mmax; ++i) {

        // For each Z value we have, look up the element string
        // They are already sorted by increasing Z
        for (auto it = per_table.begin(); it != per_table.end(); ++it) {
            if (it->second == fc.jz[i]) {
                if (charPos >= MAX_ELEMENT_STR_SIZE) {
                    egsFatal("estar::compoundstoelements_: elementStr buffer overflow at "
                            "charPos=%zu. Buffer may be too small.\n", charPos);
                }

                // For each character in the element string
                for (auto &ch : it->first) {
                    elementStr[charPos++] = ch;
                }

                // Pad out to 50 characters with spaces
                for (auto j=it->first.length(); j < 50; j++) {
                    elementStr[charPos++] = ' ';
                }

                break;
            }
        }

        // Set the mass fraction for the element, for the whole mixture
        rhoz[i] = fc.wt[i];
        zelem[i] = fc.jz[i];
    }

    return 0;
}

string getFileNameWithoutExtension(const string &s) {
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif

    size_t i = s.rfind(sep, s.length());
    if (i != string::npos) {
        string filename = s.substr(i+1, s.length() - i);
        size_t lastindex = filename.find_last_of(".");
        string rawname = filename.substr(0, lastindex);
        return (rawname);
    }
    return ("");
}

// Output a density correction file
void outputDensityFile(float mediaDensity, double *densityCorr, double *enGrid, float *meanIval, formula_calc fc, string outputFilename) {
    size_t firstSpace = outputFilename.find_first_of(" \n\r\t");
    if(firstSpace != string::npos) {
        outputFilename = outputFilename.erase(firstSpace);
    }
    if (outputFilename.empty()) {
        return;
    }

    egsInformation("estar::outputDensityFile: Writing density correction file '%s'.\n",
               outputFilename.c_str());

    std::ofstream f(outputFilename.c_str());

    if (!f.is_open()) {
        egsFatal("estar::outputDensityFile: Could not open output file '%s'.\n",
                outputFilename.c_str());
        return;
    }

    f << getFileNameWithoutExtension(outputFilename).c_str() << endl;
    f << setprecision(8);
    f << "113 " << *meanIval << " " << mediaDensity << " " << fc.mmax << endl;

    // Output the atomic numbers and mass fractions
    unsigned int k = 0;
    while (k < fc.mmax) {
        f << fc.jz[k] << " " << fc.wt[k];

        // Put a new line every 6 elements, or after the last element
        if ((k+1) % 6 == 0 || k+1 == fc.mmax) {
            f << endl;
        }
        else {
            f << " ";
        }

        k += 1;
    }

    // Set formatting for the density data
    f << scientific << showpoint;

    // Output the energy grid and density effect corrections
    for (int i = 0; i < 113; i++) {
        f << setprecision(2);
        f << enGrid[i];
        f << setprecision(3);
        f << "," << densityCorr[i];

        // Put a new line every 4 values, or after the last value
        if ((i+1) % 4 == 0 || i+1 == 113) {
            f << endl;
        }
        else {
            f << "  ";
        }
    }

    f.close();
}

/*
    This function computes the density correction factors by using the processed input arrays and variables
    from estarCalc.cpp. The density correction factors are stored in the variable densityCorr.
*/

int estarCalculation(int isCompound, int NEP, float mediaDensity, string *elementArray, double *massFraction,
                     float *numOfAtoms, double *densityCorr, double *enGrid, float *meanIval, float *ipotval, int mediaNum, string outputFilename) {
    //------------------------------------------------//
    int knmat;

    if (isCompound == 1) { // 1 means compound and 0 means not compound
        knmat = 1;
    }
    else if (NEP == 1) {
        knmat = 0; // substance is an element
    }
    else {
        knmat = 2; // substance is a mixture
    }

    // HelperFunctions hf; // Removed: hf was declared but never used (ycut call is commented out)

    formula_calc fc;
    double rho;
    rho = mediaDensity;
    if (rho <= 0) {
        egsFatal("estar::estarCalculation: Density must be greater than 0, got %g "
                "for medium %d.\n", rho, mediaNum);
        return 9;
    };

    // Here we pass the processed data from the .egsinp file
    // in getDataFromFormulae which computes the ivalue and other relevant quantities
    fc = getDataFromFormulae(knmat, rho, elementArray, massFraction, numOfAtoms, NEP, mediaNum);
    cout << "\n";

    if (*ipotval >= 0) {
        fc.pot = *ipotval;

        egsInformation("estar::estarCalculation: For medium %d I-value (eV) given "
               "in egsinp file is %g.\n", mediaNum, fc.pot);
    }
    else {
        egsInformation("estar::estarCalculation: For medium %d I-value (eV) not "
               "provided in egsinp file. I-value calculated by ESTAR is %g.\n",
               mediaNum, fc.pot);
    }
    *meanIval = fc.pot;

    // Removed: int p = 0; was declared here but never used at this scope;
    // it was shadowed by loop variable p in later for-loops.

    //------------------------------------------------//
    int lkmax = 113; // number of elements in the energy grid ->sizeof(er)/sizeof(*er)

    //================================================//
    // define q here. Further information about q is given in 2.1 of report.
    long double qfac = exp(log(10)/50);

    double qbeg = 1e-04;
    long double q[1200];
    int lmax = 1101;
    q[0] = qbeg;

    /*
        The q[] array we find below will help us to find an approximate solution of l^2
        in eqation 2 of Sternheimer 1984.
        Then using the approximate solution, we can find the exact value of the density factor using
        equation 1.
    */
    for (int i = 1; i < lmax; i++) {
        q[i] = q[i-1]*qfac;
    };
    //================================================//


    //-----------------------//------------------------------//
    /*
        In this snippet we find f (the oscillator strength for each oscillator)
        and some other parameters
    */
    int mmax = fc.mmax; // this is the number of different types of elements present in the compound/mixture

    // Guard against mmax exceeding the fixed array sizes below.
    // at[] and g[] are sized 50; if mmax exceeds this we would overflow.
    if (mmax <= 0 || mmax > 50) {
        egsFatal("estar::estarCalculation: mmax=%d is out of valid range [1, 50] "
                "for medium %d.\n", mmax, mediaNum);
        return 9;
    }

    double at[50]; // mass number
    double g[50];  // This is the weight*atmic_number/atomic_mass
    double a;      // mass number (defined for convenience)
    int jz;        // atomic number

    // atb is indexed as atb[jz-1]. Determine its size for bounds checking.
    const int ATB_SIZE = sizeof(atb) / sizeof(atb[0]);

    for (int i = 0; i < mmax; i++) {
        jz = fc.jz[i];

        // Guard against an out-of-bounds index into atb[] before we use it.
        if (jz <= 0 || jz > ATB_SIZE) {
            egsFatal("estar::estarCalculation: Atomic number Z=%d at index %d is out of "
                    "valid range [1, %d] for medium %d.\n", jz, i, ATB_SIZE, mediaNum);
            return 9;
        }

        at[i] = atb[jz-1];
        double z = fc.jz[i];
        a = at[i];

        // Guard against divide-by-zero: atomic mass should never be zero for a
        // real element. A zero here means the atb table entry is missing or corrupt.
        if (a == 0.0) {
            egsFatal("estar::estarCalculation: Atomic mass is zero for Z=%d at index %d "
                    "for medium %d. Check atb table entry.\n", jz, i, mediaNum);
            return 9;
        }

        g[i] = fc.wt[i]*(z/a); // This is the weight*atmic_number/atomic_mass
    }

    double zav = fc.zav;

    // Guard against sqrt of non-positive value in hom, and against zav==0
    // which would also make phil and cbar undefined.
    if (zav <= 0.0) {
        egsFatal("estar::estarCalculation: zav=%g is non-positive for medium %d. "
                "Cannot compute hom.\n", zav, mediaNum);
        return 9;
    }

    double hom = 28.81593*sqrt(rho*zav); // this is equation 4 of Sternheimer 1984
    double phil = 2.0*log(fc.pot/hom);   // this is equation 7 of Sternheimer 1984 with a slight modification.
    // Please refer to the report (2.2) to understand the modification.

    for (int i = 0; i < mmax; i++) {
        // Guard against divide-by-zero: zav was already checked above so this
        // is just a safeguard in case something changed it unexpectedly.
        if (zav == 0.0) {
            egsFatal("estar::estarCalculation: zav is zero when normalising g[%d] "
                    "for medium %d.\n", i, mediaNum);
            return 9;
        }
        g[i] = g[i]/zav;
    };

    int nbas = 0; // index used to compute the oscillator strength for each oscillator
    int nmax;
    int record;
    double sum;

    // f[] and en[] are sized 1000. nbas accumulates across elements; guard below.
    double f[1000];
    double en[1000];

    // parseData() reads/parses data from disk. Moved outside the loop so it is
    // only called once rather than once per element.
    ElementOscillatorData ds = parseData();

    for (int m = 0; m < mmax; m++) {
        int iz = fc.jz[m]; // atomic number
        record = iz-1; // this is the index used for simplicity (atomic number - 1)
        nmax = ds.nmax[record];

        if (iz == 6 && mmax==1) { // elemental carbon is dealt differently
            ds.nc[record][nmax -1] = 1;
            ds.nc[record][nmax] = -1;
            ds.bd[record][nmax] = ds.bd[record][nmax-1];
            nmax = nmax + 1;
        };

        if (ds.nc[record][nmax-1] <0) {
            // This condition is true ONLY when the element is a metallic conductor.
            // * example: Please look at elementData.h. More information is given in the report (Integration of ESTAR in EGSnrc) on
            // where to find the nc section. You will see that for metalllic conductors the last number of nc[] is
            // negative of number of electrons in last subshell. This is how we know the element is a conductor.
            // Otherwise it is treated as a non-conductor. Once we know the element is a conductor,
            // we make the negative number positive with the code below
            ds.nc[record][nmax-1] = - ds.nc[record][nmax-1];
            if (mmax<=1) {
                /*
                    It was discussed in Sternheimer 1984 (just below equation 8) that when the substance is a metallic conductor,
                    there is a term ouside the summation in the right hand side of equation 8. Now the code below ensures that we
                    have the correct term outside the summation for metallic conductors.
                */
                ds.bd[record][nmax-1] = 0.0;
            }
        }

        int nsum = 0;
        for (int p = 0; p < nmax; p++) {
            // This is just the sum of the number of electrons present
            // and is the same as the atomic number
            nsum = nsum + ds.nc[record][p];
        };
        sum = nsum;

        // Guard against nbas + nmax overflowing f[] and en[] (both sized 1000).
        if (nbas + nmax > 1000) {
            egsFatal("estar::estarCalculation: Total oscillator count nbas+nmax=%d "
                    "exceeds maximum of 1000 at element index %d for medium %d.\n",
                    nbas+nmax, m, mediaNum);
            return 9;
        }

        // Guard against divide-by-zero in f[nn] calculation.
        if (sum == 0.0) {
            egsFatal("estar::estarCalculation: Electron sum is zero for element index %d "
                    "(Z=%d) in medium %d. Cannot compute oscillator strengths.\n",
                    m, iz, mediaNum);
            return 9;
        }

        int nn;
        /*
            In the loop below, we compute the oscillator strength for each oscillator.
            Now this calculation is a bit different depending on whether the substance is an element/anything else.
            When the substance is an element, f is computed using just the formula given in Sternheimer 1984.
            However when the substance is a compound/mixture, the calculation is a bit different and it is not given in Sternheimer 1984.
            I could not find this calculation in any other source
        */
        for (int n = 0; n < nmax; n++) {
            nn = n + nbas;
            f[nn] = ds.nc[record][n]*g[m]/sum;
            en[nn] = ds.bd[record][n]; // we redefine for convenience

        };
        nbas = nbas + nmax;
    };
    //-----------------------//------------------------------//

    nmax =  nbas; // nmax is the total number of dispersion oscillators present

    //--------------------------------------------------------------------//
    /*
        The code in this snippet is used to construct some parameters of Sternheimer equation 8.
        They will also be used to construct some other equations.
    */
    double alf[1000];
    double eps[1000];
    for (int n = 0; n < nmax; n++) {
        alf[n] = 2.0/3.0;  // This is the 2/3 factor in equation 5 Sternheimer 1984
    };

    if (en[nmax-1]<=0) {   // when en[nmax-1]==0 is true, it means the substance is a conductor.
        alf[nmax-1] = 1.0; // when it is a conductor, this code ensures there is this
        // term outside the loop according to equation 8
    };

    for (int n = 0; n < nmax; n++) {
        eps[n] = (en[n]/hom) * (en[n]/hom); // This is square of equation 3 (Sternheimer 1984) without
        // the (adjustment factor)^2 which we will find
    };
    //--------------------------------------------------------------------//
    // The variables defined above are used to solve equation 8 of Sternheimer 1984

    //=============================//====================================//
    /*
        The code snippet here is used to solve equation 8 of Sternheimer 1984 using
        Newton's Method to solve for root (adjustment_factor^2 of the paper).
        Note that the equation constructed here is a bit different from equation 8.
        The differences are explained in the report (2.1).
    */
    double root; //root is the adjustment_factor^2 we want to find
    double fun;  // this stores the function value
    double der;  // der is derivative of fun with respect to root
    double trm;  // this is one term in the function
    root = 1.0;
    double droot = 1; // initialization to ensure loop runs at least once

    const int MAX_NEWTON_ITER = 1000;
    int newtonIter = 0;

    while (std::abs(droot)-0.00001 > 0) {
        fun = -phil;
        der = 0.0;
        for (int n = 0; n < nmax; n++) {
            trm = root*eps[n] + alf[n]*f[n];

            // Guard against log of non-positive and divide-by-zero in der.
            if (trm <= 0.0) {
                egsFatal("estar::estarCalculation: Non-positive trm=%g at oscillator n=%d "
                        "during Newton's method for medium %d. log and division are undefined.\n",
                        trm, n, mediaNum);
                return 9;
            }

            fun = fun +f[n]*log(trm);
            der = der + f[n]*eps[n]/trm; //  der is derivative of fun with respect to root
        };

        // Guard against zero derivative (would cause divide-by-zero in droot).
        if (der == 0.0) {
            egsFatal("estar::estarCalculation: Zero derivative in Newton's method at "
                    "iteration %d (root=%g) for medium %d. Cannot continue.\n",
                    newtonIter, root, mediaNum);
            return 9;
        }

        droot = fun/der;

        root = root - droot;

        if (++newtonIter >= MAX_NEWTON_ITER) {
            egsFatal("estar::estarCalculation: Newton's method failed to converge after "
                    "%d iterations for medium %d.\n", MAX_NEWTON_ITER, mediaNum);
            return 9;
        }
    };
    //=============================//====================================//

    // Guard against sqrt of a negative root (Newton's method could converge
    // to a negative value if the initial guess is poor or input is bad).
    if (root < 0.0) {
        egsFatal("estar::estarCalculation: Newton's method converged to negative "
                "root=%g for medium %d. Cannot compute adjustment factor.\n",
                root, mediaNum);
        return 9;
    }

    //double factor = sqrt(root); //  this is the adjustment factor


    for (int n = 0; n < nmax; n++) {
        eps[n] = root*eps[n]; // after we find the adjustment factor we can write down the square of
        // equation 3 of Sternheimer 1984 explicitly
    };

    // ========================================================== //
    /*
        In this snippet we mainly find d, which is equation 1 of Sternheimer 1984
        with l^2 being replaced by q[n]. However the formulation of d[n] is slightly different
        from the formulation in the paper. This has been described in detail in section 2.2 of the
        report. Furthermore, I have discussed about yql and yq in 2.1 of the report.
        The idea behind using d[n] is discussed at the end of 2.2.
    */
    double yq[1200];
    double yql[1200];
    double d[1200];
    double arg;

    for (int n = 0; n < lmax; n++) {
        sum = 0.0;
        for (int m = 0; m < nmax; m++) {
            sum = sum + f[m]/(eps[m] + q[n]);
        };

        // Guard against divide-by-zero when computing yq[n].
        if (sum == 0.0) {
            egsFatal("estar::estarCalculation: Zero sum when computing yq[%d] for "
                    "medium %d. Cannot compute 1/sum.\n", n, mediaNum);
            return 9;
        }

        yq[n] = 1/sum;

        yql[n] = log(yq[n]);
        sum = 0.0;
        for (int m = 0; m < nmax; m++) {
            double denom = eps[m] + alf[m] * f[m];

            // Guard against divide-by-zero inside the log argument.
            if (denom == 0.0) {
                egsFatal("estar::estarCalculation: Zero denominator in arg calculation at "
                        "n=%d, m=%d for medium %d.\n", n, m, mediaNum);
                return 9;
            }

            arg = 1 + q[n]/denom;
            sum =  sum + f[m]*log(arg);
        };
        d[n] = sum - q[n]/(yq[n] + 1.0);
    };

    // ========================================================== //


    double rmass = 0.510999906; // this is the rest mass of an electron

    // we call scof to get a,b,c,d which will be used to find density effect parameters with bspol
    // please see 2.1 of the report for more details.
    scof sf2 = fscof(lmax, yql, d);

    double adel[1200];
    double bdel[1200];
    double cdel[1200];
    double ddel[1200];
    for (int i = 0; i < lmax; i++) {
        adel[i] = sf2.a[i];
        bdel[i] = sf2.b[i];
        cdel[i] = sf2.c[i];
        ddel[i] = sf2.d[i];
    };

    //---------------------------------------------//
    // The following code is used to obtain
    // density corrections. Some details on what is happening here is givn in
    // section 2.1 of the notes.
    double tau;
    double y;
    double delta;
    double yl;
    double dlt[lkmax];
    double tol = 0.000000001; // for bisection method
    double xroot;
    double nb_density; // density factor from bisection method
    /*
        solver = 1 := use approximation in estar
        solver = 2 := use bisection method (section 2.2 of report)
    */
    int solver = 2;
    bspol bp;

    for (int i = 0; i < lkmax; i++) {
        tau = er[i]/rmass;
        y = tau*(tau+2.0);
        delta = 0.0;
        nb_density = 0.0;
        // y must be less than yq[lmax-1]
        // section 2.3 of the report gives more detail about the range error.
        if (y>=yq[0]) {
            if (y-yq[lmax-1] <= 0) {
                yl = log(y);
                bp = fbspol(yl, yql, adel, bdel, cdel, ddel, lmax);
                if (solver == 1) {
                    delta =  bp.density_corr;
                }
                else if (solver == 2) {
                    xroot = bisec(q[bp.lb_index], q[bp.ub_index], tol, tau, f, eps, nmax);
                    double yqn = 0;
                    sum = 0.0;
                    for (int m = 0; m < nmax; m++) {
                        double denom = eps[m] + alf[m] * f[m];

                        // Guard against divide-by-zero inside bisection solver arg.
                        if (denom == 0.0) {
                            egsFatal("estar::estarCalculation: Zero denominator in bisection arg at "
                                    "energy index %d, m=%d for medium %d.\n", i, m, mediaNum);
                            return 9;
                        }

                        arg = 1 + xroot/denom;
                        sum =  sum + f[m]*log(arg);
                        yqn = yqn + f[m]/(eps[m] + xroot);
                    };

                    // Guard against divide-by-zero when inverting yqn.
                    if (yqn == 0.0) {
                        egsFatal("estar::estarCalculation: yqn is zero at energy index %d for "
                                "medium %d. Cannot compute 1/yqn.\n", i, mediaNum);
                        return 9;
                    }

                    yqn = 1/yqn;
                    nb_density = sum - xroot/(yqn + 1.0);
                    delta =  nb_density;
                }
                else {
                    egsFatal("estar::estarCalculation: Invalid solver option %d for medium %d. "
                            "Must be 1 or 2.\n", solver, mediaNum);
                    return 9;
                }

            }
            else {
                egsFatal("estar::estarCalculation: Energy er[%d]=%g is too high and out of "
                        "range for medium %d.\n", i, er[i], mediaNum);
                return 9;
            }
        }
        dlt[i] = delta;
    };
    //---------------------------------------------//


    for (int i = 0; i < lkmax; i++) {

        densityCorr[i] = dlt[i];
        enGrid[i] = er[i];
    }

    egsInformation("\nestar::estarCalculation: Density correction factors have been "
               "calculated by ESTAR for medium %d.\n", mediaNum);

    // output a density correction file
    outputDensityFile(mediaDensity, densityCorr, enGrid, meanIval, fc, outputFilename);

    return 0;

};

bspol fbspol(double s, double x[1000], double a[1000], double b[1000], double c[1000],double d[1000], int n) {
    bspol bp;
    int idir;
    int mlb;
    int mub;
    int mu;
    int ml;
    int mav;
    double k;
    double g;

    if (n < 2) {
        egsFatal("estar::fbspol: n=%d is too small, need at least 2 points.\n", n);
    }

    if (x[0] <= x[n-1]) {
        idir =0;
        mlb =0;
        mub = n;
    }
    else {
        idir =1;
        mlb =n;
        mub = 0;
    }
    if (s>=x[mub+idir-1]) {
        mu = mub + 2*idir - 1;
    }
    else if (s<=x[mlb+1-idir-1]) {
        mu = mlb - 2*idir + 1;
    }
    else {
        ml = mlb;
        mu = mub;
        int binary_temp = 0; // we define binary_temp = 0 as the loop must run at least once
        while (std::abs(mu-ml)>1 || binary_temp==0) {
            mav = (ml+mu)/2;
            if (s<x[mav]) {
                mu = mav;
            }
            else {
                ml = mav;
            }
            binary_temp=1;
        }
        mu = mu + idir - 1;
    }
    mu = mu + 1;

    // s lies between x[mu-1] and x[mu]
    k = s - x[mu-1];
    /*
        g gives you the density correction factor
        by using the variables from scof - a,b,c,d
    */
    g = ((d[mu-1]*k+c[mu-1])*k+b[mu-1])*k+a[mu-1];

    // putting in the structure
    bp.lb_index = mu-1;
    bp.ub_index = mu;
    bp.density_corr = g;
    return bp;

}

scof fscof(int nmax, double x[1200], double f[1200]) {
    scof sf;

    if (nmax < 2) {
        egsFatal("estar::fscof: fscof requires at least 2 points, got nmax=%d\n", nmax);
        return sf;
    }

    int m1 = 2;
    int m2 = nmax-1;
    double s = 0.0;
    double r;
    for (int m = 0; m < m2; m++) {
        sf.d[m] = x[m+1] - x[m];
        r = (f[m+1] - f[m])/sf.d[m];
        sf.c[m] = r - s;
        s = r;
    };

    s = 0.0;
    r = 0.0;
    sf.c[0] = 0.0;
    sf.c[nmax-1] = 0.0;

    for (int m = 1; m < m2; m++) {
        sf.c[m] = sf.c[m] + r*sf.c[m-1];
        sf.b[m] = (x[m-1] - x[m+1])*2 - r*s;
        if (sf.b[m] == 0.0) {
            egsFatal("estar::fscof: Zero pivot sf.b[%d] in spline solve. "
                    "Knot data may be degenerate.\n", m);
        }

        s = sf.d[m];
        r = s/sf.b[m];
    };
    int mr = m2 - 1;
    for (int m = 1; m < m2; m++) {
        sf.c[mr] = (sf.d[mr] * sf.c[mr+1] - sf.c[mr])/sf.b[mr];
        mr = mr - 1;
    };
    for (int m = 0; m < m2; m++) {
        if (sf.d[m] == 0.0) {
            egsFatal("estar::fscof: Zero interval at m=%d. "
                    "Knot positions x[%d] and x[%d] may be identical.\n", m, m, m+1);
        }
        s = sf.d[m];
        r = sf.c[m+1] - sf.c[m];
        sf.d[m] = r/s;
        sf.c[m] = sf.c[m]*3.0;
        sf.b[m] = (f[m+1]-f[m])/s - (sf.c[m]+r)*s;
        sf.a[m] = f[m];
    };
    return sf;
}

double objective_function(double tau, double f[1000], double eps[1000], int nmax, double x) {
    double yl = log(tau*(tau+2.0));
    double yql = 0;
    for (int i = 0; i < nmax; i++) {
        yql = yql + f[i]/(eps[i]+x);
    }
    // Guard against divide-by-zero before log
    if (yql == 0.0) {
        egsFatal("estar::objective_function: yql is zero at x=%g. "
                "Cannot compute log.\n", x);
    }
    return yl - log(1.0/yql);
}

// now we have to write the bisection algorithm

double bisec(double lowerbound, double upperbound, double tolerance, double tau, double f[1000], double eps[1000], int nmax) {
    if (tolerance <= 0) {
        egsFatal("estar::bisec: bisection tolerance must be positive, got %g\n", tolerance);
    }

    double fLower = objective_function(tau, f, eps, nmax, lowerbound);
    double fUpper = objective_function(tau, f, eps, nmax, upperbound);

    if ((fLower > 0) == (fUpper > 0)) {
        egsFatal("estar::bisec: Bounds [%g, %g] do not bracket a root for tau=%g.\n",
                lowerbound, upperbound, tau);
        return lowerbound; // signal failure; caller should check
    }

    double x_mid = lowerbound;
    double del = 1.0;
    while (del > tolerance) {
        x_mid = (lowerbound + upperbound) / 2.0;
        double fMid = objective_function(tau, f, eps, nmax, x_mid);
        if ((fMid > 0) == (fLower > 0)) {
            lowerbound = x_mid;
            fLower = fMid;
        } else {
            upperbound = x_mid;
        }
        del = std::abs(fMid);
    }
    return x_mid;
}
