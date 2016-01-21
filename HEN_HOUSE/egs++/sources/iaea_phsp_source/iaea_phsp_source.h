/*
###############################################################################
#
#  EGSnrc egs++ IAEA format phase-space source headers
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
#  Author:          Blake Walters, 2013
#
#  Contributors:
#
###############################################################################
*/


/*! \file iaea_phsp_source.h
 *  \brief An IAEA format phase-space file source
 *  \BW
 */

#ifndef IAEA_PHSP_SOURCE_
#define IAEA_PHSP_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_alias_table.h"

#include <fstream>
using namespace std;

#ifdef WIN32

    #ifdef BUILD_IAEA_PHSP_SOURCE_DLL
        #define IAEA_PHSP_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define IAEA_PHSP_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define IAEA_PHSP_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define IAEA_PHSP_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define IAEA_PHSP_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define IAEA_PHSP_SOURCE_EXPORT
        #define IAEA_PHSP_SOURCE_LOCAL
    #endif

#endif

/*! \brief An IAEA phase-space file source.

  \ingroup Sources

An IAEA format phase-space file source reads and delivers particles from a
IAEA phase-space file. The phase-space file contains iq, E, x, y, u, v, w, latch, (Zlast).
The format also allows z to be defined for each particle.  In the case
of a planar source (e.g. output by BEAMnrc), the constant z where the file was scored
is read from the header of the file and applied to every particle.  Note that this is
different from an EGS phase space source, which always sets the incident z equal to zero.  Thus, for
the IAEA phase space source, you have to be aware of the z where the source was scored when
defining the position of the source relative to the chamber/phantom geometry.
An IAEA phase-space file source is defined as follows:
\verbatim
:start source:
    library = iaea_phsp_source
    name = some_name
    iaea phase space file = base name of the phase space file (no extension)
    particle type = one of photons, electrons, positrons, all, or charged
    cutout = x1 x2 y1 y2  (optional)
:stop source:
\endverbatim
The optional \c cutout key permits to set a rectangular cutout
defined by its upper-left and lower-right corners <code>x1,y1</code> and
<code>x2,y2</code>
(all particles not within this rectangle will be thrown away).
The <code>particle type</code> key permits to select a subset of particles
based on the particle charge. No filters based on the value of the
\c latch variable are implemented yet but such filters will be added
in future versions of the library. Note that an iaea phase-space source
can be used as the source in a
\link EGS_TransformedSource transformed source \endlink permitting in this way
arbitrary transformations to be applied to the particle positions and
directions. It is worth noting that,
together with a transformation, the iaea phase-space source
can reproduce the functionality of any iaea phase-space file based source
in the RZ series of user codes and in DOSXYZnrc.

\todo Fully implement latch filters
\todo Add weight window filter
\todo Add particle recycling (\em i.e. same particle returned Nrecycle times)
*/
class IAEA_PHSP_SOURCE_EXPORT IAEA_PhspSource : public EGS_BaseSource {

public:

    /*! \brief Constructor

    Construct a phase-space file source delivering particles from the
    IAEA format phase-space file \a phsp_file.
    */
    IAEA_PhspSource(const string &phsp_file,
                    const string &Name="", EGS_ObjectFactory *f=0);

    /*! \brief Constructor

    Construct a phase-space file source from the information pointed to by
    \a inp. */
    IAEA_PhspSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~IAEA_PhspSource() { };

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u);
    void setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun);
    EGS_Float getEmax() const {
        return Emax;
    };
    EGS_Float getFluence() const {
        double aux = ((double) Nread)/((double) Nparticle);
        return Pinc*aux;
    };
    bool storeState(ostream &data) const {
        data << endl;
        bool res = egsStoreI64(data,Nread);
        if (!res) {
            return res;
        }
        data << "  ";
        res = egsStoreI64(data,Nfirst);
        if (!res) {
            return res;
        }
        data << "  ";
        res = egsStoreI64(data,Nlast);
        if (!res) {
            return res;
        }
        data << "  ";
        res = egsStoreI64(data,Npos);
        if (!res) {
            return res;
        }
        data << "  ";
        res = egsStoreI64(data,count);
        if (!res) {
            return res;
        }
        data << "  ";
        return res;
    };
    bool setState(istream &data) {
        first = false;
        bool res = egsGetI64(data,Nread);
        if (!res) {
            return res;
        }
        res = egsGetI64(data,Nfirst);
        if (!res) {
            return res;
        }
        res = egsGetI64(data,Nlast);
        if (!res) {
            return res;
        }
        res = egsGetI64(data,Npos);
        if (!res) {
            return res;
        }
        Npos++;
        iaea_set_record(&iaea_fileid,&Npos,&iaea_iostat);
        res = egsGetI64(data,count);
        return res;
    };
    bool addState(istream &data) {
        EGS_I64 tmp_Nread = Nread, tmp_count = count;
        bool res = setState(data);
        Nread += tmp_Nread;
        count += tmp_count;
        return res;
    };
    void resetCounter() {
        Nread = 0;
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
    void setFilter(int, int, int, const int *);

protected:

    bool        is_valid;
    string      the_file_name; //!< The phase-space file name
    bool        mode2;         //!< \c true, if a MODE2 file (i.e. storing Zlast)
    bool        latch_stored;  //!< true if LATCH is stored in data
    bool        swap_bytes;    /*!< \c true, if phase-space file was generated
                                on a CPU with different endianness */
    EGS_Float   Emax,    //!< Maximum k.e. (obtained from the phsp file)
                Pinc;    //!< Number of incident particles that created the file
    EGS_I64     Nparticle, //!< Number of particles in the file
                Nphoton,   //!< Number of photons in the file
                Nread,     //!< Number of particles read so far
                Npos,      //!< Next record to be read
                Nfirst,    //!< first record this source can use
                Nlast,     //!< Last record this source can use
                count;     /*!< Particles delivered so far (may be less than
                           Nread because some particles were rejected */
    int         Nrecycle;  //!< Number of times the file was recycled.
    int         Nreuse_g;  //!< Number of times to reuse a photon
    int         Nreuse_e;  //!< Number of times to reuse a charged particle
    int         Nreuse;    //!< Number of times to reuse current particle
    int         Nuse;      //!< Number of times current particle was used so far
    int         iaea_iostat; //!< iostat on read/write of iaea phsp file
    int         iaea_fileid; //!< phsp file unit no.
    int         n_extra_floats; //!< no. of extra floats stored in phsp file
    int         n_extra_longs; //!< no. of extra longs stored in phsp file
    int         i_zlast;   //!< index of zlast in extra_longs array
    int         i_latch;   //!< index of latch in extra_floats array

    bool        first;

    // filters
    int         particle_type;
    int         filter_type;
    unsigned long filter1, filter2;
    EGS_Float   Xmin, Xmax, Ymin, Ymax;
    EGS_Float   wmin, wmax; // weight window

    void openFile(const string &);
    void init();

#ifndef SKIP_DOXYGEN
    struct EGS_LOCAL BeamParticle {
        int  latch, q;
        float E, u, v, w, x, y, z, wt, zlast;
    };
    BeamParticle  p;
#endif

    inline bool rejectParticle() const;

};

#endif
