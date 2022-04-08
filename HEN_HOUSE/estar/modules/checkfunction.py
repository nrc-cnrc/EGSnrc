from math import isclose
from tracemalloc import stop

def checking_function():
    with open('modules/test/estaroutput.density') as estar_data:
        estar_array = estar_data.read().split()
        nu_elems = len(estar_array)
    i = 0
    while (i<nu_elems):
        estar_array[i] = float(estar_array[i])
        i = i + 1

    with open('modules/test/cppout.density') as cpp_data:
        cpp_array = cpp_data.read().split()
    i = 0
    while (i<nu_elems):
        cpp_array[i] = float(cpp_array[i])
        i = i + 1

    i = 0
    while (i<nu_elems):
        if (not isclose(estar_array[i], cpp_array[i], abs_tol=1e-4)): #1e-4)):
            print("\n***************")
            print("Line number ", i+1, "does not match\n***************\n")
            print("\n*** output not matching error ***\n")
            if __name__ == "__main__":
                return 2
            val = input("Do want to continue? (0-> yes 1-> no): ")
            if (val != "0"):
                returnErrorCode = 1
                #assert returnCode == 0, "Program stopped"
                return returnErrorCode
            i = i + 1
        else:
            i = i + 1
    return 0

if __name__ == "__main__":
    assert checking_function() == 0, "files did not match somewhere"
    print("files matched :)")
    # added so that during debugging we can easily compare files