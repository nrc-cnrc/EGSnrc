/*
###############################################################################
#
#  EGSnrc egs++ radiative splitting object headers
#  Copyright (C) 2018 National Research Council Canada
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
#  Author:          Ernesto Mainegra-Hing, 2018
#
#  Contributors:    Blake Walters, 2021
#
###############################################################################
#
#  A general radiative splitting tool.
#
#  TODO:
#
#  - Testing/debugging directional radiative splitting (DRS)
#  - Implement e- splitting
#  - Implement beampp-style DBS (DRSf)
#
###############################################################################
*/


/*! \file egs_radiative_splitting.h
 *  \brief A radiative splitting ausgab object: header
 *  \EM
 */

#ifndef EGS_RADIATIVE_SPLITTING_
#define EGS_RADIATIVE_SPLITTING_

#include "egs_ausgab_object.h"
#include "egs_application.h"
#include "egs_scoring.h"
#include "egs_base_geometry.h"
#include "egs_rndm.h"
#include "egs_interpolator.h"
#include "egs_transformations.h"

#ifdef WIN32

#ifdef BUILD_RADIATIVE_SPLITTING_DLL
#define EGS_RADIATIVE_SPLITTING_EXPORT __declspec(dllexport)
#else
#define EGS_RADIATIVE_SPLITTING_EXPORT __declspec(dllimport)
#endif
#define EGS_RADIATIVE_SPLITTING_LOCAL

#else

#ifdef HAVE_VISIBILITY
#define EGS_RADIATIVE_SPLITTING_EXPORT __attribute__ ((visibility ("default")))
#define EGS_RADIATIVE_SPLITTING_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define EGS_RADIATIVE_SPLITTING_EXPORT
#define EGS_RADIATIVE_SPLITTING_LOCAL
#endif

#endif

/*! \brief A radiative splitting object: header

\ingroup AusgabObjects

This ausgab object can be used to increase the efficiency of radiative
events. This ausgab object is specified via:
\verbatim
:start ausgab object:
    library   = egs_radiative_splitting
    name      = some_name
    splitting type   = uniform (default) or directional (based on BEAMnrc DBS)
    splitting = the splitting number (n_split)

 The following inputs apply to directional splitting only:
    field size = radius of splitting field centred on Z-axis (cm) -- required
    ssd = distance of splitting field from Z=0 along +Z-axis (cm) -- required
    e-/e+ split regions = region number(s) for e-/e+ splitting.  On entering this(ese) region(s), phat charged particles will be split n_split times.
                         If set to 0 or omitted, charged particles will not be split.
    radially redistribute e-/e+ = "yes" or "no" (default) -- if "yes", evenly distribute split e-/e+ in a circle of radius sqrt(x(np)^2+y(np)^2) about the Z-axis
    Z of russian roulette plane = Z below which russian roulette is not played on low-weight charged particles resulting from e-/e+ splitting (cm) -- optimally set to some value < Z at which particles enter the splitting region(s)
    Below is an optional input for an affine transform to be applied to the directional splitting cone defined by field size and ssd:
    :start transformation:
       rotation = optional 3D rotation vector to be applied to the splitting cone (applied first)
       translation = optional 3D translation to be applied to the splitting cone
    :stop transformation:

:stop ausgab object:
\endverbatim

TODO:
 - Testing/debugging directional radiative splitting (DRS)
 - Implement e-/e+ splitting
 - Implement beampp-style DRS (DRSf)

*/

class EGS_RADIATIVE_SPLITTING_EXPORT EGS_RadiativeSplitting : public EGS_AusgabObject {

public:

    /*! Splitting algortihm type */
    enum {
        URS, // EGSnrc Uniform Radiative Splitting
        DRS, // Directional Radiative Splitting (based on BEAMnrc DBS)
        DRSf // Directional Radiative Splitting (based on beampp DBS)
    };

    EGS_RadiativeSplitting(const string &Name="", EGS_ObjectFactory *f = 0);

    ~EGS_RadiativeSplitting();

    void setApplication(EGS_Application *App);

    void initializeData() {
        //set bit 0 of the first particle in each history to 1
        //Until a more general solution is implemented (extra_stack), this
        //bit determines whether the particle (and its descendents) has(ve)
        //been split (i.e. is/are phat) or not
        //Note: This means we lose the ability of this bit as a flag for
        //brem events
        EGS_Particle p = app->getParticleFromStack(0);
        int latch = p.latch;
        latch = latch | (1 << 0);
        app->setLatch(latch);
    }

    int doInteractions(int iarg, int &killed);

    int doSmartBrems();

    void doSmartCompton(int nsample);

    void getCostMinMax(const EGS_Vector &xx, const EGS_Vector &uu,
                       EGS_Float &ro, EGS_Float &ct_min, EGS_Float &ct_max);

    void getBremsEnergies();

    void killThePhotons(EGS_Float fs, EGS_Float ssd, int n_split, int npstart, int kill_electrons);

    void selectAzimuthalAngle(EGS_Float &cphi, EGS_Float &sphi);

    void uniformPhotons(int nsample, int n_split, EGS_Float fs, EGS_Float ssd, EGS_Float energy);

    void initSmartKM(EGS_Float Emax);

    void doUphi21(EGS_Float sinthe, EGS_Float costhe, EGS_Vector &u);

    void setSplitting(const int &n_s) {
        nsplit = n_s;
    };

    void setSplitType(const int &type) {
        split_type = type;
    };

    void initDBS(const EGS_Float field_rad, const EGS_Float field_ssd, const vector<int> splitreg, const int irad, const EGS_Float zrr, const EGS_AffineTransform *t);

    bool needsCall(EGS_Application::AusgabCall iarg) const override {
        if ( split_type == DRS || split_type == DRSf ) {
            if ( iarg == EGS_Application::BeforeBrems ||
                    iarg == EGS_Application::BeforeAnnihFlight ||
                    iarg == EGS_Application::BeforeAnnihRest ||
                    iarg == EGS_Application::BeforePair ||
                    iarg == EGS_Application::BeforeCompton ||
                    iarg == EGS_Application::BeforePhoto ||
                    iarg == EGS_Application::BeforeRayleigh ||
                    iarg == EGS_Application::FluorescentEvent ||
                    iarg == EGS_Application::BeforeTransport) {
                return true;
            }
            if ( ireg_esplit.size()>0 && iarg == EGS_Application::AfterTransport) {
                return true;
            }
        }
        return false;
    };

    int processEvent(EGS_Application::AusgabCall iarg) {
        if ( split_type > URS && iarg >= EGS_Application::BeforeTransport )
        {
            if( !doInteractions(iarg,killed) )
            {
                return 0;
            }
        }
        return 0;
    };

    int processEvent(EGS_Application::AusgabCall iarg, int ir) {
        if (split_type >  URS && iarg >= EGS_Application::BeforeTransport)
        {
            if( !doInteractions(iarg,killed) )
            {
                return 0;
            }
        }
        return 0;
    };

    //function to rotate/translate particle position and rotate particle direction
    void transformP(EGS_Particle &p, const EGS_AffineTransform *t) {
        if (t)
        {
            t->transform(p.x);
            t->rotate(p.u);
        }
    }

    //function to inverse rotate/translate particle position and inverse rotate direction
    void inverseTransformP(EGS_Particle &p, const EGS_AffineTransform *t) {
        if (t)
        {
            t->inverseTransform(p.x);
            t->rotateInverse(p.u);
        }
    }

protected:

    int split_type; //0 = uniform, 1 = DBS, 2 = BEAMnrc DBS
    /* Maximum splitting limited to 2,147,483,647 */
    int nsplit;
    int killed; //no of photons killed
    EGS_Float be_factor = 0; //Brems enhancement factor--currently set to zero and not used
    EGS_Float fs; //radius of splitting field
    EGS_Float ssd; //ssd at which splitting field is defined
    vector<int> ireg_esplit; //numbers of regions on entering which phat charged particles are split
    int irad_esplit; //set to 1 to radially redistribute split e-/e+
    EGS_Float zrr_esplit; //Z value below which Russian Roulette will not be played with split e-/e+

    bool use_cyl_sym = false; //set to true to use cylindrical symmetry, hard coded as false for now
    EGS_Float zcyls; //Z below which cylindrical symmetry does not exist

    //interpolators for smart brems estimates of KM distribution
    EGS_Interpolator *f_KM_max;
    EGS_Interpolator **f_KM_a;
    EGS_Interpolator **f_KM_b;
    EGS_Float        *a_KM, *b_KM;
    EGS_Float        *y2_KM;
    EGS_Float        *zbr_KM;
    int              nmed_KM;

    bool flag_fluor = false;

    EGS_AffineTransform *T; //optional transformation of the splitting cone

    vector<EGS_Particle> particle_stack; //store a stack of brems particles in do_smart_brems
    vector<EGS_Float> dnear_stack; //similar for dnear

    int imed;

    const char *dbs_err_msg =
        "Stack size exceeded in BEAMpp_DBS::%s()\n"
        "Increase MXSTACK (currently %d) in array_sizes.h and retry\n";
};

#endif
