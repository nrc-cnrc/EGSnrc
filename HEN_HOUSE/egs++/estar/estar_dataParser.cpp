#include <iostream>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include "estar_dataParser.h"
#include "estar_dataTables.h"
#include "egs_functions.h"

using namespace std;

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

    double arr[14532];
    int arr_len = 14532;
    int i = 0;
    while (i < arr_len) {
        arr[i] = elementData[i];
        i = i + 1;
    }
    //

    //--//--//--//--//
    // In the snippet below, we structure the data in arrays
    int j = 0;
    int temp_nmax[100];
    int nc_temp[26];
    double bd_temp[26];
    for (i = 0; i < 100; i++) {

        temp_nmax[i] = arr[j];

        if (temp_nmax[i] < 0 || j + 2 + 2*temp_nmax[i] + ds.numLevelsStandard > arr_len) {
            egsFatal("estar::parseData: Data layout for element %d would read past end "
                    "of elementData array (j=%d, temp_nmax=%d, arr_len=%d).\n",
                    i, j, temp_nmax[i], arr_len);
        }

        int k = j+2; // the second element of each set of data is 113 and thus we ignore the second element
        int a = 0;
        /*
            temp_nmax[i] is the first element of each set and this is an integer
            length of nc_tamp array is temp_nmax[i]
        */
        while (k < j + 2 + temp_nmax[i]) {
            if (a >= 26) {
                egsFatal("estar::parseData: Oscillator index %d exceeds nc_temp "
                        "maximum of 25 for element index %d.\n", a, i);
            }
            nc_temp[a] = arr[k];
            a = a + 1;
            k = k + 1;
        };
        int m = k;
        a = 0;
        /*
            Recall temp_nmax[i] is the first element of each set and this is an integer
            length of bd_tamp array is temp_nmax[i]
        */
        while (m < k + temp_nmax[i]) {
            if (a >= 26) {
                egsFatal("estar::parseData: Oscillator index %d exceeds bd_temp "
                        "maximum of 25 for element index %d.\n", a, i);
            }
            bd_temp[a] = arr[m];
            a = a + 1;
            m = m + 1;
        };
        int n = m;
        // rlos (loss function) values are present in elementData but are not used
        // by the ESTAR density correction calculation. We advance n past them
        // to keep j correctly positioned for the next element's data block.
        while (n < m + ds.numLevelsStandard) {
            n = n + 1;
        };
        //--//--//--//--//

        //--------------------
        // In this snippet we input the data from nc_temp and bd_temp
        // into the structure
        // Note that i is used to keep track of the set of data we are dealing with
        // and the range of i is 0 <= i <= 99
        ds.nmax[i] = temp_nmax[i];
        for (int p = 0; p < temp_nmax[i]; p++) {
            ds.nc[i][p] = nc_temp[p];
        };
        for (int p = 0; p < temp_nmax[i]; p++) {
            ds.bd[i][p] = bd_temp[p];
        };
        //--------------------

        j = n;
    };
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

