# This module is supposed to be run independently
# The purpose is to find the ratio of the density data in estaroutput.density
# and cppout.density and then to plot a graph of this ratio against the standard energy grid

# Note that this module sould be ran for a particular set of density files which can be obtained
# by running estar.f or the c++ file or any of the testfiles which produce the output inside 
# estaroutput.density or cppout,density

# (Note) Need to be careful about the 0 case
import matplotlib.pyplot as plt

class Data:
    # standard energy grid 
    energyGrid = [1.00e-03,1.25e-03,1.50e-03,1.75e-03,2.00e-03,2.50e-03,
             3.00e-03,3.50e-03,4.00e-03,4.50e-03,5.00e-03,5.50e-03,
             6.00e-03,7.00e-03,8.00e-03,9.00e-03,1.00e-02,1.25e-02,
             1.50e-02,1.75e-02,2.00e-02,2.50e-02,3.00e-02,3.50e-02,
             4.00e-02,4.50e-02,5.00e-02,5.50e-02,6.00e-02,7.00e-02,
             8.00e-02,9.00e-02,1.00e-01,1.25e-01,1.50e-01,1.75e-01,
             2.00e-01,2.50e-01,3.00e-01,3.50e-01,4.00e-01,4.50e-01,
             6.00e+00,7.00e+00,8.00e+00,9.00e+00,1.00e+01,1.25e+01,
             1.50e+01,1.75e+01,2.00e+01,2.50e+01,3.00e+01,3.50e+01,
             4.00e+01,4.50e+01,5.00e+01,5.50e+01,6.00e+01,7.00e+01,
             8.00e+01,9.00e+01,1.00e+02,1.25e+02,1.50e+02,1.75e+02,
             2.00e+02,2.50e+02,3.00e+02,3.50e+02,4.00e+02,4.50e+02,
             5.00e+02,5.50e+02,6.00e+02,7.00e+02,8.00e+02,9.00e+02,
             1.00e+03,1.25e+03,1.50e+03,1.75e+03,2.00e+03,2.50e+03,
             3.00e+03,3.50e+03,4.00e+03,4.50e+03,5.00e+03,5.50e+03,
             6.00e+03,7.00e+03,8.00e+03,9.00e+03,1.00e+04]
# cutoff seemes to be 0.2 in the energy grid
def plottingFunction():
    data = Data()
    with open('modules/test/estaroutput.density') as estar_data:
        estar_array = estar_data.read().split()
        num_elems = len(estar_array)
        num_elems = len(data.energyGrid)
        #print(num_elems)
    i = 0
    while (i<num_elems):
        estar_array[i] = float(estar_array[i])
        if (estar_array[i] == 0):
            estar_array[i] = 0.00000000000001
        i = i + 1

    with open('modules/test/cppout.density') as cpp_data:
        cpp_array = cpp_data.read().split()
    i = 0
    while (i<num_elems):
        cpp_array[i] = float(cpp_array[i])
        if (cpp_array[i] == 0):
            cpp_array[i] = 0.00000000000001
        i = i + 1

    # find ratio
    i = 0

    ratio = [0]*len(data.energyGrid)
    while (i<num_elems):
        ratio[i] = estar_array[i]/cpp_array[i]
        if (cpp_array[i] == 0.00000000000001):
            ratio[i] = -1
        print(ratio[i])
        i = i + 1
    print("For the list, minumum is ", min(ratio), " and maximum is ", max(ratio))
    print(data.energyGrid)
    plt.plot(data.energyGrid, ratio, marker="o", markersize=5, markeredgecolor="red", markerfacecolor="green")
    plt.xlabel("Energy (MeV)")
    plt.ylabel("Ratio:- EGSnrc_DensityData/ESTAR_DensityData")
    plt.xscale("log")
    plt.show()
    return 0

if __name__ == "__main__":
    plottingFunction()