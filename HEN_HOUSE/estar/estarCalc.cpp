#include <iostream>
using namespace std;
#include "estarCalc.h"
#include "string.h"
#include "cprep.cpp"

class GetElements {
    public:
        struct GetElementsStruct {
            // this array contains all elements present in a medium
            string elemArrayStrut[100];
        };  
        // this function below parses fortran array to produce an array
        // which can be used in our C++ estar.
        // This is needed as arrays returned by fortran cannot be read by C++
        // without this pre-processing
        GetElementsStruct getElemArray(char *formulaStr, int NEP) {
            int elemArraySize = NEP*2;
            int i = 0;
            int elemIndex = 0;
            string elemArray[elemArraySize];
            string elemTemp_1;
            string elemTemp_2;
            while (i < elemArraySize) {
                elemTemp_1 = formulaStr[i];
                if (formulaStr[i+1] == ' ') {
                    elemArray[elemIndex] = elemTemp_1;
                } else {
                    elemTemp_2 = tolower(formulaStr[i+1]);
                    elemArray[elemIndex] = elemTemp_1 + elemTemp_2;
                }
                i = i + 2;
                elemIndex = elemIndex + 1;
            }
            int k = 0;
            while (k < NEP) {
                k = k + 1;
            }
            GetElementsStruct GElem;
            k = 0;
            while (k < NEP) {
                GElem.elemArrayStrut[k] = elemArray[k];
                k = k + 1;
            }
            return GElem;
        }  
};




extern "C" int mainf_(char *formulaStr, 
                    float *massFraction, 
                    float *numOfAtoms, 
                    float *mediaDensity, 
                    double *densityCorr, 
                    double *enGrid,
                    int *NEP, 
                    int *ISCOMP,
                    float *meanIval,
                    float *ipotval) {
    string mainFormula;
    string mainFormula_temp_1;
    string mainFormula_temp_2;

    // The   2 lines below process   formula_str   to make the array readable by estar c++
    GetElements elemObject;
    GetElements::GetElementsStruct GeElems = elemObject.getElemArray(formulaStr, *NEP);
    int k = 0;
    int nepInt = *NEP;
    int isCompInt = *ISCOMP;

    string estarFormulaArrayInput[nepInt];
    double estarWeightArrayInput[nepInt];
    int i = 0;
    while (i < nepInt) {
        estarFormulaArrayInput[i] = GeElems.elemArrayStrut[i];
        estarWeightArrayInput[i] = massFraction[i];
        i = i + 1;
    }
    // call the main estar calculation files.
    double mediumDensity = *mediaDensity;
    //double mIval = *meanIval;
    estarCalculation(isCompInt, nepInt, mediumDensity, estarFormulaArrayInput, estarWeightArrayInput, 
    numOfAtoms, densityCorr, enGrid, meanIval, ipotval);

    return 0;
}


