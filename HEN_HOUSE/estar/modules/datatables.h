#include <iomanip>
#include <iostream>
#include <map>
using namespace std;

// fixed energy grid with 113 elements
double er[] = {1.00e-03,1.25e-03,1.50e-03,1.75e-03,2.00e-03,2.50e-03,
               3.00e-03,3.50e-03,4.00e-03,4.50e-03,5.00e-03,5.50e-03,
               6.00e-03,7.00e-03,8.00e-03,9.00e-03,1.00e-02,1.25e-02,
               1.50e-02,1.75e-02,2.00e-02,2.50e-02,3.00e-02,3.50e-02,
               4.00e-02,4.50e-02,5.00e-02,5.50e-02,6.00e-02,7.00e-02,
               8.00e-02,9.00e-02,1.00e-01,1.25e-01,1.50e-01,1.75e-01,
               2.00e-01,2.50e-01,3.00e-01,3.50e-01,4.00e-01,4.50e-01,
               5.00e-01,5.50e-01,6.00e-01,7.00e-01,8.00e-01,9.00e-01,
               1.00e+00,1.25e+00,1.50e+00,1.75e+00,2.00e+00,2.50e+00,
               3.00e+00,3.50e+00,4.00e+00,4.50e+00,5.00e+00,5.50e+00,
               6.00e+00,7.00e+00,8.00e+00,9.00e+00,1.00e+01,1.25e+01,
               1.50e+01,1.75e+01,2.00e+01,2.50e+01,3.00e+01,3.50e+01,
               4.00e+01,4.50e+01,5.00e+01,5.50e+01,6.00e+01,7.00e+01,
               8.00e+01,9.00e+01,1.00e+02,1.25e+02,1.50e+02,1.75e+02,
               2.00e+02,2.50e+02,3.00e+02,3.50e+02,4.00e+02,4.50e+02,
               5.00e+02,5.50e+02,6.00e+02,7.00e+02,8.00e+02,9.00e+02,
               1.00e+03,1.25e+03,1.50e+03,1.75e+03,2.00e+03,2.50e+03,
               3.00e+03,3.50e+03,4.00e+03,4.50e+03,5.00e+03,5.50e+03,
               6.00e+03,7.00e+03,8.00e+03, 9.00e+03, 1.00e+04
              }; // 1.00e+04

// atb[] contains the atomic mass of each of the elements in the per_table dictionary
// and indexed by atomic number
// for example atb[n-1] gives the atomic mass of the atom with atomic number n
double atb[] = {1.007940,       4.0026020,      6.9410,         9.0121820,
                10.8110,        12.0110,        14.006740,      15.99940,
                18.99840320,    20.17970,       22.9897680,     24.30500,
                26.9815390,     28.08550,       30.9737620,     32.0660,
                35.45270,       39.9480,        39.09830,       40.0780,
                44.9559100,     47.880,         50.94150,       51.99610,
                54.938050,      55.8470,        58.933200,      58.690,
                63.5460,        65.390,         69.7230,        72.610,
                74.921590,      78.960,         79.9040,        83.800,
                85.46780,       87.620,         88.905850,      91.2240,
                92.906380,      95.940,         97.90720,       101.070,
                102.90550,      106.420,        107.86820,      112.4110,
                114.820,        118.7100,       121.750,        127.600,
                126.904470,     131.290,        132.905430,     137.3270,
                138.90550,      140.1150,       140.907650,     144.240,
                144.91270,      150.360,        151.9650,       157.250,
                158.925340,     162.500,        164.930320,     167.260,
                168.934210,     173.040,        174.9670,       178.490,
                180.94790,      183.850,        186.2070,       190.20,
                192.220,        195.080,        196.966540,     200.590,
                204.38330,      207.20,         208.980370,     208.98240,
                209.98710,      222.01760,      223.01970,      226.02540,
                227.02780,      232.03810,      231.035880,     238.02890,
                237.04820,      239.05220,      243.06140,      247.07030,
                247.07030,      251.07960,      252.0830,       257.09510
               };

// The dictionary (per_table) contains the elements of the periodic table (atomic numbers 1 - 100).
// In NIST ESTAR, elements with
// atomic numbers 1 -98 were included (source: https://physics.nist.gov/PhysRefData/Star/Text/method.html).
map<string, int>per_table =     {{"H", 1}, {"He", 2}, {"Li",3}, {"Be",4},
    {"B",5}, {"C",6}, {"N",7}, {"O",8}, {"F",9}, {"Ne",10}, {"Na",11}, {"Mg",12},
    {"Al",13}, {"Si",14}, {"P",15}, {"S",16}, {"Cl",17}, {"Ar",18}, {"K",19}, {"Ca",20},
    {"Sc",21}, {"Ti",22}, {"V",23}, {"Cr",24}, {"Mn",25}, {"Fe",26}, {"Co",27}, {"Ni",28},
    {"Cu",29}, {"Zn",30}, {"Ga",31}, {"Ge",32}, {"As",33}, {"Se",34}, {"Br",35}, {"Kr",36},
    {"Rb",37}, {"Sr",38}, {"Y",39}, {"Zr",40}, {"Nb",41}, {"Mo",42}, {"Tc",43}, {"Ru",44},
    {"Rh",45}, {"Pd",46}, {"Ag",47}, {"Cd",48}, {"In",49}, {"Sn",50}, {"Sb",51}, {"Te",52},
    {"I",53}, {"Xe",54}, {"Cs",55}, {"Ba",56}, {"La",57}, {"Ce",58}, {"Pr",59}, {"Nd",60},
    {"Pm",61}, {"Sm",62}, {"Eu",63}, {"Gd",64}, {"Tb",65}, {"Dy",66}, {"Ho",67}, {"Er",68},
    {"Tm",69}, {"Yb",70}, {"Lu",71}, {"Hf",72}, {"Ta",73}, {"W",74}, {"Re",75}, {"Os",76},
    {"Ir",77}, {"Pt",78}, {"Au",79}, {"Hg",80}, {"Tl",81}, {"Pb",82}, {"Bi",83}, {"Po",84},
    {"At",85}, {"Rn",86}, {"Fr",87}, {"Ra",88}, {"Ac",89}, {"Th",90}, {"Pa",91}, {"U",92},
    {"Np",93}, {"Pu",94}, {"Am",95}, {"Cm",96}, {"Bk",97}, {"Cf",98}, {"Es",99}, {"Fm",100}
};


//*************************************************************
/*
    The I-value of an element depends on whether the element
    is part of a compound or not. poth[] is the array
    containing I-values of elements when the elements are
    present in their elemental form. potgas[] and potcon[]
    and are I-values of elements when they are part of
    a compound.
    Further details are given below
*/

// poth is an array of length 100 and is again indexed by atomic number
// the table contais the I values of elements in the
// condensed phase (ICRU 37 - table 4.3). For example (poth[99] is the I-value of Fermium)
// * I-val of graphite in (ICRU 37 - table 4.3) is 78 eV. However in ESTAR 81 eV is used (poth[5]).
// * I-val of chlorine in (ICRU 37 - table 4.3) is 174 eV. However in ESTAR 159.29 eV is used (poth[16]) (perhaps this was an improvement).
double poth[] = {19.2,41.8,40.0,63.7,76.0,81.0,82.0,95.0,
                 115.,137.,149.,156.,166.,173.,173.,180.,159.29,
                 188.,190.,191.,216.,233.,245.,257.,272.,286.,
                 297.,311.,322.,330.,334.,350.,347.,348.,357.,
                 352.,363.,366.,379.,393.,417.,424.,428.,441.,
                 449.,470.,470.,469.,488.,488.,487.,485.,491.,
                 482.,488.,491.,501.,523.,535.,546.,560.,574.,
                 580.,591.,614.,628.,650.,658.,674.,684.,694.,
                 705.,718.,727.,736.,746.,757.,790.,790.,800.,
                 810.,823.,823.,830.,825.,794.,827.,826.,841.,
                 847.,878.,890.,902.,921.,934.,939.,952.,966.,
                 980.,994.
                };

// potgas represents the I-value of elements (atomic number 1-9)
// when the elements (atomic number 1-9) are part of a compound in a gaseous state (ICRU 37 - table 5.1)
double potgas[] = {19.2,41.8,34.0,38.6,49.0,70.0,82.0, 97.0, 115.0};
// potcon represents the I-value of elements (atomic number 1-9)
// when the elements (atomic number 1-9) are part of a compound in a solid/liquid state (ICRU 37 - table 5.1)
double potcon[] = {19.2,41.8,45.2,72.0,85.9,81.0,82.0, 106.0, 112.0};

//************************************************************
