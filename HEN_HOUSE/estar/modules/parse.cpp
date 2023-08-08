#include <iomanip>
#include <iostream>
#include <math.h>
#include <string.h>
#include <cctype>
#include <assert.h>
using namespace std;

/*
    This module is intended to parse and interpret a single chemical formula of a
    compound/element. The final product of this module is a parseformula object.
    The comments inside the definition of structure parseformula gives more information on the output.
*/

struct parseformula {
    string str_arr[100]; // stores the formula of elements present in a compound/element into an array
    int num_arr[100];    // for a compound/element this stores the number of atoms
    // of the corresponding elements in str_arr into an array
    int elem_types;      // number of different types of elements present
};

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


