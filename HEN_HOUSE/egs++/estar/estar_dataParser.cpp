#include <iostream>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include "estar_dataParser.h"
#include "estar_dataTables.h"
#include "egs_functions.h"

/*
    The purpose of this module is to read data from the elementData.h
    file and then to properly structure the data for usage in other modules.
    There are 100 sets of data in elementData.h (indexed by atomic number)
    where each set of data corresponds to each element.
    * For example: the third set of data in elementData.h is the data for Lithium
*/

ElementOscillatorData parseData() {
    ElementOscillatorData ds;
    ds.numLevelsStandard = 113; // number of elements in the standard energy grid

    const int arr_len = 14532;
    int j = 0;

    for (int i = 0; i < 100; i++) {

        ds.nmax[i] = static_cast<int>(elementData[j]);

        if (ds.nmax[i] < 0 || j + 2 + 2*ds.nmax[i] + ds.numLevelsStandard > arr_len) {
            egsFatal("estar::parseData: Data layout for element %d would read past end "
                     "of elementData array (j=%d, nmax=%d, arr_len=%d).\n",
                     i, j, ds.nmax[i], arr_len);
        }

        // The second element of each set is 113 (numLevelsStandard) and is skipped
        int k = j + 2;

        /*
            Read nc values directly into ds.nc.
            ds.nmax[i] is the number of oscillators for this element.
        */
        for (int a = 0; a < ds.nmax[i]; a++) {
            if (a >= 26) {
                egsFatal("estar::parseData: Oscillator index %d exceeds nc "
                         "maximum of 25 for element index %d.\n", a, i);
            }
            ds.nc[i][a] = static_cast<int>(elementData[k]);
            k++;
        }

        /*
            Read bd values directly into ds.bd.
        */
        for (int a = 0; a < ds.nmax[i]; a++) {
            if (a >= 26) {
                egsFatal("estar::parseData: Oscillator index %d exceeds bd "
                         "maximum of 25 for element index %d.\n", a, i);
            }
            ds.bd[i][a] = elementData[k];
            k++;
        }

        // rlos (loss function) values are present in elementData but are not used
        // by the ESTAR density correction calculation. We advance k past them
        // to keep j correctly positioned for the next element's data block.
        k += ds.numLevelsStandard;

        j = k;
    }

    return ds;
}

/*
    The output of parse() below is a parseformula object. It takes input a formula string
    and produces the object or an error message
    for example: for MgCl2
    we will get: str_arr = ["Mg", "Cl"]
                 num_arr = [1,2]
                 mmax = 2
*/
parseformula parse(string str) {
    parseformula pf;
    pf.elem_types = 0;
    int str_len = str.length(); // length of formula
    int i = 0;
    int j = 0;
    while (i < str_len) {
        if (j >= 100) {
            egsFatal("estar::parse: Element type count exceeded maximum of 100"
                     " while parsing formula '%s'.\n", str.c_str());
        }

        if (isupper(static_cast<unsigned char>(str[i])) != 0) { // means str[i] is uppercase
            if (i + 1 < str_len && islower(static_cast<unsigned char>(str[i+1])) != 0) {
                pf.str_arr[j] = str.substr(i,2);
                i = i+2;
            }
            else {
                pf.str_arr[j] = str.substr(i,1);
                i = i+1;
            }
        }
        else {
            egsFatal("estar::parse: Formula '%s' is malformed at character '%c' (index %d).\n"
                     "Element symbols must begin with an uppercase letter.\n",
                     str.c_str(), str[i], i);
        }

        if (i < str_len && isdigit(static_cast<unsigned char>(str[i])) != 0) {
            int digit_start = i;
            while (i < str_len && isdigit(static_cast<unsigned char>(str[i])) != 0) {
                i = i + 1;
            }
            int digit_len = i - digit_start;

            // Guard against absurdly large numbers that would overflow strtol
            if (digit_len > 9) {
                egsFatal("estar::parse: Atom count in formula '%s' has %d digits which "
                         "exceeds the maximum of 9.\n", str.c_str(), digit_len);
            }

            std::string digit_str = str.substr(digit_start, digit_len);
            char *end;
            errno = 0;
            long val = std::strtol(digit_str.c_str(), &end, 10);

            if (errno != 0 || end == digit_str.c_str() || val <= 0 || val > 99) {
                egsFatal("estar::parse: Invalid atom count '%s' in formula '%s'.\n"
                         "Atom count must be a positive integer no greater than 99.\n",
                         digit_str.c_str(), str.c_str());
            }

            pf.num_arr[j] = static_cast<int>(val);
        }
        else {
            pf.num_arr[j] = 1;
        }
        j = j + 1;
    }
    pf.elem_types = j;
    return pf;
};

