#include <iostream>
#include <fstream>
#include <iomanip>
#include "elementData.h"
using namespace std;

/*
    The purpose of this module is to read data from the data.txt
    file and then to properly structure the data for usage in other modules.
    There are 100 sets of data in data.txt (indexed by atomic number)
    where each set of data corresponds to each element.
    * For example: the third set of data in data.txt is the data for Lithium
*/

// 'data' structure will help us to properly organize the data from the data.txt file
struct  data {
    int nmax[100];      // number of dispersion oscillators required to describe 
                        // the atoms of a medium made solely with a particular element
                        // * example: nmax[6-1] is the number of dispersion oscillators required to describe a medium made of Carbon
                        // note that for mixtures/compounds the nmax is later on modified

    int numLevelsStandard;          // number of energylevels in standard energy grid

    int nc[100][26];    // This is the number of electrons present in the subshell we are considering
                        // * example, L-III subshell has 4 electrons. So L[i - 1] = 4 when I >= 10.
                        // Please refer to the report to know which portion of the data.txt file represents the subshell data.

    double bd[100][26]; // planck constant * bd[i-1][j] represents the absorption edge of the j+1 th oscillator for 
                        // the dispersion model with atomic number i where 0<=j<=nmax-1.
                        // Please refer to the report to know which portion of the data.txt file represents the subshell data.
};

data parseData() {
    data ds;
    ds.numLevelsStandard = 113; // number of elements in the standard energy grid

    double arr[14532];
    int arr_len = 14532;
    int i = 0;
    double x;
    while (i < arr_len) {
        arr[i] = elementData[i];
        i = i + 1;
    }
    //   

    //--//--//--//--//
    // In the snippet below, we structure the data in arrays
    i = 0;
    int j = 0;
    int temp_nmax[100];
    int nc_temp[26];
    double bd_temp[26];
    double rlos_temp[113];
    while (j<arr_len) {
        temp_nmax[i] = arr[j];
        int k = j+2; // the second element of each set of data is 113 and thus we ignore the second element
        int a = 0;
        /*  
            temp_nmax[i] is the first element of each set and this is an integer
            length of nc_tamp array is temp_nmax[i]
        */
        while (k < j + 2 + temp_nmax[i]) {
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
            bd_temp[a] = arr[m];
            a = a + 1;
            m = m + 1;
        };
        int n = m;
        a = 0;
        while (n < m + ds.numLevelsStandard) {
            rlos_temp[a] = arr[n]; // the remaining 113 numbers in each set is stored in rlos_temp array
            a = a + 1;
            n = n + 1;
        };
        //--//--//--//--//

        //--------------------
        // In this snippet we input the data from nc_temp, bd_temp and rlos_temp
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
        i = i + 1;
        j = n;    
    };
    return ds;
}