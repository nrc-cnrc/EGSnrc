#include <iostream>
using namespace std;
#include "string.h"
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
                    int *mediaID) {
    string mainFormula;
    string mainFormula_temp_1;
    string mainFormula_temp_2;

    // The 2 lines below process formula_str to make the array readable by estar c++
    GetElements elemObject;
    GetElements::GetElementsStruct GeElems = elemObject.getElemArray(formulaStr, *NEP);
    int k = 0;
    int nepInt = *NEP;
    int isCompInt = *ISCOMP;
    int mediaNum = *mediaID; // this is the media id

    string estarFormulaArrayInput[nepInt];
    double estarWeightArrayInput[nepInt];
    int i = 0;
    while (i < nepInt) {
        estarFormulaArrayInput[i] = GeElems.elemArrayStrut[i];
        estarWeightArrayInput[i] = massFraction[i];
        i = i + 1;
    }
   
    double mediumDensity = *mediaDensity;
    // call the main estar calculation files.
    estarCalculation(isCompInt, nepInt, mediumDensity, estarFormulaArrayInput, estarWeightArrayInput, 
    numOfAtoms, densityCorr, enGrid, meanIval, ipotval, mediaNum);

    return 0;
}


