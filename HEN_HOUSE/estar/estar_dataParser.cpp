#include <iostream>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include "estar_dataParser.h"
#include "estar_dataTables.h"

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

/*
    The output of parse() below is a parseformula object. It takes input a formula string
    and produces the object or an error message
    for example: for MgCl2
    we will get: str_arr = ["Mg", "Cl"]
                 num_arr = [1,2]
                 mmax = 2
*/
parseformula parse(string str) {
    char temp_carr[100];
    /*
        The error handler is used to denote if the format of the input formula is wrong
        For example: If someone enters nA instaed of Na, the error handler will be set to 1
        and an error message will be displayed.
    */
    int error_handler = 0;
    parseformula pf;
    pf.elem_types = 0;
    int str_len = str.length(); // length of formula
    int i = 0;
    int j = 0;
    while (i < str_len) {
        if (isupper(str[i]) != 0) { // means str[i] is uppercase
            if (islower(str[i+1]) != 0) {
                pf.str_arr[j] = str.substr(i,2);
                i = i+2;
            }
            else {
                pf.str_arr[j] = str.substr(i,1);
                i = i+1;
            }
        }
        else {
            error_handler = 1;
            cout << "\n***************\n";
            cout << "Please enter input formula correctly\n";
            cout << "\n***************\n";
            assert(error_handler==0);
        }
        int p = 0;
        if (isdigit(str[i]) != 0) {   // means str[i] is a digit
            while (isdigit(str[i]) != 0) {
                temp_carr[p] = str[i]; // temp_carr is used to store the numeric elements
                // until a non numeric element is encountered
                // as ascii numbers
                // for example: for Na21,
                // temp_carr[] = [50,49]
                p = p + 1;
                i = i + 1;
            }
            int val = 0;
            /*
                Say I have Na21. Now while parsing, kfac is used to record that there are 21 Na atoms.
            */
            int kfac = 1;
            p = p - 1;
            /*
                Let us stick with the Na21 example
                my temp_carr[] = [2,1]. So, to make the algorithm understand there are 21 Na atoms, we do:
                val = 1*(49-48) + 10*(50-48)
                The while loop below does this
            */
            while (p>=0) {
                val =  val + kfac*(temp_carr[p]-48); // the ascii value of 0 is 48
                p = p - 1;
                kfac = kfac*10;
            }
            pf.num_arr[j] = val;
        }
        else {
            pf.num_arr[j] = 1;
        }
        j = j + 1;
    }
    pf.elem_types = j;
    return pf;
};

