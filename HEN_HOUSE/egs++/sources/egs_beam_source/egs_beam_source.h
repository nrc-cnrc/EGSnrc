/*
###############################################################################
#
#  EGSnrc egs++ beam source headers
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Frederic Tessier
#
###############################################################################
*/


/*! \file egs_beam_source.h
 *  \brief A BEAM simulation source
 *  \IK
 */

#ifndef EGS_BEAM_SOURCE_
#define EGS_BEAM_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_functions.h"

#include <fstream>
using namespace std;

#ifdef WIN32

    #ifdef BUILD_BEAM_SOURCE_DLL
        #define EGS_BEAM_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_BEAM_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_BEAM_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_BEAM_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_BEAM_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_BEAM_SOURCE_EXPORT
        #define EGS_BEAM_SOURCE_LOCAL
    #endif

#endif

class EGS_Library;
typedef void (*InitFunction)(const int *, const int *,
                             const char *, const char *, const char *,
                             const char *, const char *, int,int,int,int,int);
typedef void (*FinishFunction)();
typedef void (*SampleFunction)(EGS_Float *, EGS_Float *, EGS_Float *,
                               EGS_Float *, EGS_Float *, EGS_Float *, EGS_Float *, EGS_Float *,
                               EGS_I32 *, EGS_I32 *, EGS_I64 *, EGS_I32 *);
typedef void (*MaxEnergyFunction)(EGS_Float *);

/*! \brief A BEAM simulation source

  \ingroup Sources

A BEAM simulation source is a source that dynamically loads a BEAM
user code compiled into a shared library and then uses this
to peform a full BEAM simulation returning particles crossing
the BEAM scoring plane. This is explained in more details in a
paper by E. Tonkopi et al.

A beam simulation source is defined using the following input:
\verbatim
library = egs_beam_source
beam code = the name of the BEAMnrc user code
pegs file = the name of the PEGS file to be used in the BEAMnrc simulation
input file = the name of the input file specifying the BEAMnrc simulation
cutout = x1 x2 y1 y2 (optional)
particle type = all or electrons or photons or positrons or charged (optional)
weight window = wmin wmax
\endverbatim
If the \c cutout key is present, all particles not passing through the
rectangle specified by its left-upper and right-lower corners
<code>(x1,y1)</code> and <code>(x2,y2)</code> will be rejected.
With the optional <code>particle type</code> one can select a particular
type of particles (default is \c all). If the <code>weight window</code>
key is present, particles having statistical weights less than \c wmin
or greater than \c wmax will be rejected. This is useful to \em e.g.
reject "phat" particles from a simulation using DBS.

<b>BEWARE:</b> When restarting calculations using this source, one must make
sure that the <code>RESTART</code> calculation option is defined in
both, the source and the application input files.

*/
class EGS_BEAM_SOURCE_EXPORT EGS_BeamSource : public EGS_BaseSource {

public:

    /*! \brief Create a BEAM simulation source from the input \a inp */
    EGS_BeamSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_BeamSource();

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u);
    EGS_Float getEmax() const {
        return Emax;
    };
    EGS_Float getFluence() const {
        return count;
    };
    bool storeState(ostream &data) const {
        return egsStoreI64(data,count);
    };
    bool setState(istream &data) {
        return egsGetI64(data,count);
    };
    bool addState(istream &data) {
        EGS_I64 tmp;
        bool res = egsGetI64(data,tmp);
        count += tmp;
        return res;
    };
    void resetCounter() {
        count = 0;
    };

    bool isValid() const {
        return is_valid;
    };

    void setCutout(EGS_Float xmin, EGS_Float xmax, EGS_Float ymin,
                   EGS_Float ymax) {
        Xmin = xmin;
        Xmax = xmax;
        Ymin = ymin;
        Ymax = ymax;
    };

protected:

    EGS_Library    *lib;    //!< The BEAMnrc user code library
    FinishFunction finish;  /*!< The function to be called at the end of the
                                 simulation */
    SampleFunction sample;  //!< The function that returns the next particle

    bool        is_valid;
    string      the_file_name;
    ifstream    the_file;
    EGS_Float   Emax;
    EGS_I64     count;

    // filters
    int         particle_type;
    EGS_Float   Xmin, Xmax, Ymin, Ymax;
    EGS_Float   wmin, wmax;

    // temporary particle storage
    int         q_save, latch_save;
    EGS_Float   E_save, wt_save;
    EGS_Vector  x_save, u_save;

    // reusing particles
    int         n_reuse_photon, n_reuse_electron;
    int         i_reuse_photon, i_reuse_electron;

};

#endif
