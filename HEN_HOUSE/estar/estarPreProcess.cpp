#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

#include "estarMainCalc.cpp"


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
    // without this pre-processing
    GetElementsStruct getElemArray(char *formulaStr, int NEP) {
        const char delim = ' ';
        std::vector<std::string> components;
        tokenize(string(formulaStr), delim, components);

        if(components.size() < NEP) {
            cout << "\n***************\n";
            cout << "Error: List of elements is inconsistent with the number expected\n";
            cout << "\n***************\n";
            assert(components.size() >= NEP);
        }

        GetElementsStruct GElem;
        int k = 0;
        while (k < NEP) {
            GElem.elemArrayStrut[k] = components[k];
            k = k + 1;
        }
        return GElem;
    }
};

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

    cout << "\n-------------------------\n";
    cout << "== MEDIUM " << *mediaID << " BLOCK FOR ESTAR ==\n";

    string mainFormula;
    string mainFormula_temp_1;
    string mainFormula_temp_2;

    // The 2 lines below process formula_str to make the array readable by estar c++
    GetElements elemObject;
    GetElements::GetElementsStruct GeElems = elemObject.getElemArray(formulaStr, *NEP);
    int nepInt = *NEP;
    int isCompInt = *ISCOMP;
    int mediaNum = *mediaID; // this is the media id

    string estarFormulaArrayInput[nepInt];
    double estarWeightArrayInput[nepInt];
    int i = 0;
    while (i < nepInt) {
        estarFormulaArrayInput[i] = GeElems.elemArrayStrut[i];
        estarWeightArrayInput[i] = massFraction[i];

        cout << "Formula is " << estarFormulaArrayInput[i] << " with fraction " << estarWeightArrayInput[i] << endl;

        i = i + 1;
    }

    double mediumDensity = *mediaDensity;
    // call the main estar calculation files.
    estarCalculation(isCompInt, nepInt, mediumDensity, estarFormulaArrayInput, estarWeightArrayInput,
                     numOfAtoms, densityCorr, enGrid, meanIval, ipotval, mediaNum, string(outputFilename));

    cout << "-------------------------\n";

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
    GetElements elemObject;
    GetElements::GetElementsStruct GeElems = elemObject.getElemArray(formulaStr, *ncomp);
    int numCompounds = *ncomp;

    string estarFormulaArrayInput[numCompounds];
    double estarWeightArrayInput[numCompounds];
    vector<string> elementList;
    vector<double> elementWeightList;
    for(size_t i=0; i < numCompounds; ++i) {
        estarFormulaArrayInput[i] = GeElems.elemArrayStrut[i];
        estarWeightArrayInput[i] = massFraction[i];
    }

    double rho = 1; // Material density not actually needed, so use a dummy
    formula_calc fc = mixtureCalculation(rho, estarFormulaArrayInput, estarWeightArrayInput, numCompounds);

    // Set NEP to actually be the number of elements now, instead of the number of compounds
    NEP[0] = fc.mmax;

    size_t charPos = 0;
    for(size_t i=0; i != fc.mmax; ++i) {

        // For each Z value we have, look up the element string
        // They are already sorted by increasing Z
        for (auto it = per_table.begin(); it != per_table.end(); ++it) {
            if (it->second == fc.jz[i]) {

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
