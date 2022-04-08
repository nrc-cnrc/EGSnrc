#include <iomanip>
#include <iostream>
#include <fstream>
#include "modules/datatables.h"
#include "modules/routine/scof.h" 
#include "modules/parseDataFile.h"
#include "modules/routine/bspol.h"
#include "modules/newtonbisec.h"
#include "modules/routine/formulaCalculations.h"
#include "modules/solverHelpers/headerfile.h"
#include "modules/solverHelpers/cutoff.cpp"
using namespace std;

// estarCalculation(NEP -> DETERMINES KMAT)
// estarCalculation(mediaDensity -> density)

// WE NEED TO PASS IN numOfAtoms as a an array of floats as it is defined as a float in GET_MEDIA_INPUT;
// however we later convert numOfAtoms from float -> int;
int estarCalculation(int isCompound, int NEP, float mediaDensity, string *elementArray, double *massFraction, float *numOfAtoms, double *densityCorr) {
    //------------------------------------------------//
    int knmat;

    if (isCompound == 1) { // 1 means compound and 0 means not compound
        knmat = 1;
    } else if (NEP == 1) {
        knmat = 0;
    } else {
        knmat = 2;
    }
    HelperFunctions hf;
    formula_calc fc;
    // cout << "element(0) or compound(1) or mixture(2)?\n";
    // cin >> knmat;
    double rho;
    string elem_name;
    // cout << "give density of element pls:\n";
    rho = mediaDensity;
    if (rho <= 0) {
        cout << "\n***************\n";
        cout << "Error! Density must be greater than 0";
        cout << "\n***************\n";
        return 9;
    };

    /////--> MUST PASS FORMULA AND RHO HERE *** MODIFY CALCULATE

    fc = getDataFromFormulae(knmat, rho, elementArray, massFraction, numOfAtoms, NEP); // pass formula array here // pass mass fractiom here too // that will get passed to mixtures then //  before that make sure to make NA Na
    cout << "I value is: " << fc.pot << "\n";
    double estarIval = fc.pot; // This is the I-value obtained by calculation
    // cout << "input custom i-val? 0->no 1->yes\n";
    
    int p = 0;
    double ival;
    // cin >> p;
    // if (p==1) {
    //     cout << "give ival: ";
    //     cin >> ival;
    //     if (ival <= 0) {
    //         cout << "\n***************\n";
    //         cout << "Error! Density I-value be greater than 0";
    //         cout << "\n***************\n";
    //         return 9;
    //     };
    //     fc.pot = ival;
    //     cout << "new i-val is :" << fc.pot << "\n";
    // }
    //------------------------------------------------//
    int lkmax = 113; // number of elements in the energy grid sizeof(er)/sizeof(*er)

    //================================================//
    // define q here. Furher information about q is given in 2.1 of report.
    int numq = 50;
    long double temp_qfac = log(10)/numq;
    long double qfac = exp(temp_qfac);

    double qbeg = 1e-04;
    long double q[1200];
    int lmax = 1101;
    q[0] = qbeg;
    int i = 0;

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
    i = 0;
    double at[50]; // mass number
    double g[50]; // This is the weight*atmic_number/atomic_mass 
    double a; // mass number (defined for convenience)
    int jz; // atomic number
    while (i < mmax) {
        jz = fc.jz[i]; 
        at[i] = atb[jz-1];
        double z = fc.jz[i];
        a = at[i];
        g[i] = fc.wt[i]*(z/a); // This is the weight*atmic_number/atomic_mass
        i = i + 1;
    }
    
    double zav = fc.zav;
    double hom = 28.81593*sqrt(rho*zav); // this is equation 4 of Sternheimer 1984
    double phil = 2.0*log(fc.pot/hom);   // this is equation 7 of Sternheimer 1984 with a slight modification.
                                         // Please refer to the report to understand the modification.
    double cbar = phil + 1.0;

    for (int i = 0; i < mmax; i++) {
        g[i] = g[i]/zav;
    };

    int nbas = 0; // index used to compute the oscillator strength for each oscillator
    int nmax;
    int record;
    double sum;
    double f[1000]; 
    double en[1000];
    
    for (int m = 0; m < mmax; m++) {
        int iz = fc.jz[m]; // atomic number
        data ds;
        ds = parseData();
        record = iz-1; // this is the index used for simplicity (atomic number - 1)
        nmax = ds.nmax[record];

        if (iz == 6 and mmax==1) { // elemental carbon is dealt differently
            ds.nc[record][nmax -1] = 1;
            ds.nc[record][nmax] = -1;
            ds.bd[record][nmax] = ds.bd[record][nmax-1];
            nmax = nmax + 1;
        };

        if (ds.nc[record][nmax-1] <0) {
            // This condition is true ONLY when the element is a metallic conductor.
            // * example: Please look at data.txt in the nc section. More information is given in the report on 
            // where to find the nc section. You will see that for metalllic conductors the last number of nb[] is 
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
    
    //=================================================================//
    /*
        The code snippet here is used to solve equation 8 of Sternheimer 1984 using
        Newton's Method to solve for root (adjustment_factor^2 of the paper).
        Note that the equation constructed here is a bit different from eqation 8.
        The differences are explained in the report.
    */
    double root; //root is the adjustment_factor^2 we want to find 
    double fun;  // this stores the function value
    double der;  // der is derivative of fun with respect to root
    double trm;  // this is one term in the function
    root = 1.0; 
    double droot = 1; // initialization to ensure loop runs at least once
    while (abs(droot)-0.00001 > 0) {
        fun = -phil;
        der = 0.0;
        for (int n = 0; n < nmax; n++) {
            trm = root*eps[n] + alf[n]*f[n];
            fun = fun +f[n]*log(trm);
            der = der + f[n]*eps[n]/trm; //  der is derivative of fun with respect to root
        };
        
        droot = fun/der;

        root = root - droot; 
    };
    //=================================================================//
    double factor = sqrt(root); //  this is the adjustment factor
    

    for (int n = 0; n < nmax; n++) {
        eps[n] = root*eps[n]; // after we find the adjustment factor we can write down the square of
                              // equation 3 of Sternheimer 1984 explicitly
    };

    // here we computer cutoff below which density effect is 0. However this cutoff is not used in estar.
    // Please refer to section 4 of the report for further details. 
    double ycut = hf.cutoff(en, eps, f, nmax);
    
    // ========================================================== //
    /*
        In this snippet we mainly find d, which is equation 1 of Sternheimer 1984
        with l^2 being replaced by q[n]. However the formulation of d[n] is slightly different 
        from the formulation in the paper. This has been described in detail in section 2.2 of the 
        report. Furthermore, I have discussed about yql and yq in 2.1 of the report. 
        Note that if you replace l^2 with q[n] in equation 7 (section 2.2 of report) you will get d[n]
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
        yq[n] = 1/sum;
        
        yql[n] = log(yq[n]);
        sum = 0.0;
        for (int m = 0; m < nmax; m++) {
            arg = 1 + q[n]/(eps[m] + alf[m] * f[m]);
            sum =  sum + f[m]*log(arg);
        };
        d[n] = sum - q[n]/(yq[n] + 1.0);    
    };

    // ========================================================== //


    double rmass = 0.510999906; // this is the rest mass of an electron https://en.wikipedia.org/wiki/Electron_mass

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
    double e; 
    double tau;
    double y;
    double delta;
    double yl;
    double dlt[lkmax];
    double tol = 0.000000001; // for newton bisection
    double xroot;
    double nb_density_corr[lkmax];
    double nb_density; // density factor from newton bisection method
    /*
        solver = 1 := use approximation in estar
        solver = 2 := use newton bisection (section 2.2 of report)
    */
    int solver = 1;
    bspol bp;

    for (int i = 0; i < lkmax; i++) {
        e = er[i];
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
                } else if (solver == 2) {
                    xroot = newton_bisec(q[bp.lb_index], q[bp.ub_index], tol, tau, f, eps, nmax);
                    double yqn = 0; 
                    sum = 0.0;
                    for (int m = 0; m < nmax; m++) {
                        arg = 1 + xroot/(eps[m] + alf[m] * f[m]);
                        sum =  sum + f[m]*log(arg);
                        yqn = yqn + f[m]/(eps[m] + xroot);
                    };
                    yqn = 1/yqn;
                    nb_density = sum - xroot/(yqn + 1.0);
                    delta =  nb_density;
                } else {
                    cout << "\n***************\n";
                    cout << "Solver option incorrect!\n";
                    cout << "\n***************\n";
                    return 9;
                }
                
            } else {
                cout << "\n***************\n";
                cout << "energy is too high and out of range";
                cout << "\n***************\n";
                return 9;
            }
        }
        dlt[i] = delta;
    }; 
    //---------------------------------------------//


    for (int i = 0; i < lkmax; i++) {

        densityCorr[i] = dlt[i];
    }
    
    return 0;
    
};


// int testFunc(string *elemArray) {
//     cout << "length is " << elemArray[0] << "\n";
//     cout << "length2 is " << elemArray[1] << "\n";
//     return 0;
// }


// int main() {
//     string formulaArr[2] = {"Cr", "Fe"};
//     double massFraction[2] = {11.9, 20};
//     estarCalculation(2, 2, formulaArr, massFraction);
//     return 0;
// }