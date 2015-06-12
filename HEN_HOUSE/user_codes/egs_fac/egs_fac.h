/*
###############################################################################
#
#  EGSnrc egs++ egs_fac application headers
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_FAC_APPLICATION_
#define EGS_FAC_APPLICATION_

#include "egs_advanced_application.h"

class EGS_FACSimulation;
class EGS_FACCorrelation;
class EGS_Interpolator;
class EGS_RangeRejection;
class EGS_AxCalculator;

class APP_EXPORT EGS_FACApplication : public EGS_AdvancedApplication {

public:

    /*! Constructor */
    EGS_FACApplication(int argc, char **argv);

    /*! Destructor.  */
    ~EGS_FACApplication();

    /*! Describe the application.  */
    void describeUserCode() const;

    /*! Describe the simulation */
    void describeSimulation();

    /*! Initialize scoring.  */
    int initScoring();

    /*! Accumulate quantities of interest at run time */
    int ausgab(int iarg);

    /*! Simulate a single shower.
        We need to do special things and therefore reimplement this method.
     */
    int simulateSingleShower();

    /*! Output intermediate results to the .egsdat file. */
    int outputData();

    /*! Read results from a .egsdat file. */
    int readData();

    /*! Reset the variables used for accumulating results */
    void resetCounter();

    /*! Add simulation results */
    int addState(istream &data);

    /*! Output the results of a simulation. */
    void outputResults();

    /*! Get the current simulation result.  */
    void getCurrentResult(double &sum, double &sum2, double &norm,
            double &count);

    /*! simulate a shower */
    int shower() {
        expmfp[0] = 1; is_fat[0] = 0;
        return EGS_AdvancedApplication::shower();
    };

    /* Select photon mean-free-path */
    void selectPhotonMFP(EGS_Float &dpmfp);

    int rangeDiscard(EGS_Float tperp, EGS_Float range) const;

protected:

    /*! Start a new shower.  */
    int startNewShower();

private:

    int              ngeom;     // number of simulation cases to calculate
                                // quantities of interest
    int              ig;        // current geometry index

    EGS_FACSimulation **sim;    // simulation cases

    int              ncor;      // number of correlated geometries
    EGS_FACCorrelation **corr;  // correlated geometries

    int              nax;       // number of Ax calculators
    EGS_AxCalculator **Ax;      // the Ax calculators

    EGS_Interpolator  *muen;    // mu_en interpolator

    EGS_Float        *expmfp;   // attenuation unweighting
    int              *is_fat;   // electron "fatness" flag

    EGS_Float        fsplit;    // photon splitting number
    EGS_Float        fspliti;   // inverse photon splitting number
    EGS_Float        kerma_fac; // normalization factor for kerma scoring

    EGS_RangeRejection *rrej;   // range rejection object

    EGS_GeometryIntersections *gsections; // for getting geometry intersections
    EGS_Float                 *idist;     // distances to interactions
    EGS_Float                 *ilambda;   // mfp's to interactions
    int                       *iindex;    // indeces of interaction sites
    int                       *itype;     // interaction types
    int                        ngsec;     // size of gsections
    int                        med_cv;    // medium in CV
    bool                       increase_scatter;

    static string revision;

};

#endif
