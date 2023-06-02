/*
###############################################################################
#
#  EGSnrc egs++ dynamic source headers
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
#  Author:          Blake Walters, 2017
#
#  Contributors:    Reid Townson
#                   Alexandre Demelo
#
###############################################################################
*/


/*! \file egs_dynamic_source.h
 *  \brief A source with simulated time-varying rotations/translations
 *  \BW
 */

#ifndef EGS_DYNAMIC_SOURCE_
#define EGS_DYNAMIC_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_shapes.h"
#include <string>
#include <sstream>


#ifdef WIN32

    #ifdef BUILD_DYNAMIC_SOURCE_DLL
        #define EGS_DYNAMIC_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_DYNAMIC_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_DYNAMIC_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_DYNAMIC_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_DYNAMIC_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_DYNAMIC_SOURCE_EXPORT
        #define EGS_DYNAMIC_SOURCE_LOCAL
    #endif

#endif

/*! \brief A source with time-varying rotations/translations

  \ingroup Sources

The dynamic source allows the user to simulate dynamic motion of
any other source.  The user specifies a number of control points,
where each control point comprises a set of incident polar coordinates
plus a monitor unit index.  The polar coordinates are:
(xiso,yiso,ziso) = coordinates of isocentre of rotation (cm)
dsource = length of vector from isocentre to source origin (cm).
          With no rotations, +ve dsource is along the -Z axis.
theta = angle of rotation of dsource about the Y-axis (deg).
        +ve values define clockwise rotations.  Angle is
        defined relative to the -Z axis.
phi = angle of rotation of dsource about Z-axis (deg).
      +ve values define clockwise rotations.  Angle is defined
      relative to the +X-axis.
phicol = angle of rotation of source about -dsource (deg).  +ve
         value defines clockwise rotations.
The time index controls dynamic motion as described
below.
The generic input is:
\verbatim
:start source:
    library = egs_dynamic_source
    name = some_name
    source name = the name of a previously defined source
    synchronize motion = yes or no (default)
    :start motion:
       control point = xiso(1) yiso(1) ziso(1) dsource(1) theta(1) phi(1) phicol(1) time(1)
       control point = xiso(2) yiso(2) ziso(2) dsource(2) theta(2) phi(2) phicol(2) time(2)
       .
       .
       .
       control point = xiso(N) yiso(N) ziso(N) dsource(N) theta(N) phi(N) phicol(N) time(N)
    :stop motion:
:stop source:
\endverbatim
Control points must be defined such that time(i+1)>=time(i), where time(i)
is the value of time for control point i.  The time(i) are automatically
normalized by time(N), where N is the number of control points.
Continuous, dynamic motion between control points is simulated by choosing a random
number, R, on (0,1] and, for time(i)<R<=time(i+1), setting incident source
coordinate, P, where P is one of xiso, yiso, ziso, dsource, theta,
phi, or phicol, using:
P=P(i)+[P(i+1)-P(i)]/[time(i+1)-time(i)]*[R-time(i)]
Note that this scheme for generating incident source coordinates really
only makes sense if time(1)=0.0.  However, the source can function
with time(1)>0.0, in the case where a user desires to eliminate particles
associated with a range of time values, but there will be a lot of
warning messages.

A simple example is shown below.  This first defines a monoenergetic
(1 MV) photon source in the Z-direction collimated to a 2x2 field
centred on the Z-axis.  The control points place the source a
distance, dsource, of 100 cm above the isocentre at (0,0,0).  Control
points 1 and 2 rotate the source clockwise around the Y-axis (phi=0)
through theta=0-360 degrees, while control points 3 and 4 rotate
the source clockwise around the Z-axis (phi=90 degrees) through
phi=0-360 degrees.  Note that time(2)-time(1)=time(4)-time(3), so the rotations
around Z and Y are carried out for an equal number of incident photons.
If the source being made to move dynamically supplies its own monitor
unit index (iaea_phsp_source and egs_beam_source only), then the dynamic
motion can be synchronized with the motion of component modules
(MLC's, jaws) within the source by setting "synchronize motion"
to "yes".
\verbatim
:start source definition:
    :start source:
        library = egs_parallel_beam
        name = my_parallel_source
        :start shape:
            library = egs_rectangle
            rectangle = -.1 -.1 .1 .1
        :stop shape:
        direction = 0 0 1
        charge = 0
        :start spectrum:
            type = monoenergetic
            energy = 1.0
        :stop spectrum:
    :stop source:
    :start source:
        library = egs_dynamic_source
        name = my_source
        source name = my_parallel_source
        :start motion:
            control point = 0 0 0 100 0 0 0 0
            control point = 0 0 0 100 360 0 0 0.5
            control point = 0 0 0 100 90 0 0 0.5
            control point = 0 0 0 100 90 360 0 1.0
        :stop motion:
    :stop source:

    simulation source = my_source

:stop source definition:
\endverbatim
*/

class EGS_DYNAMIC_SOURCE_EXPORT EGS_DynamicSource :
    public EGS_BaseSource {

public:


    struct EGS_ControlPoint {

        EGS_Vector iso; //isocentre position
        EGS_Float dsource; //source-isocentre distance
        EGS_Float theta; //angle of rotation about Y-axis
        EGS_Float phi;  //angle of rotation about Z-axis
        EGS_Float phicol; //angle of rotation in source plane
        EGS_Float time; //monitor unit index for control point
    };

    /*! \brief Construct a dynamic source using \a Source as the
    source and cpts as the control points.  Not sure if this
    will ever be used but here just in case.
    */
    EGS_DynamicSource(EGS_BaseSource *Source, vector<EGS_ControlPoint> cpts,
                      const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSource(Name,f), source(Source), valid(true) {
        //do some checks on cpts
        if (cpts.size()<2) {
            egsWarning("EGS_DynamicSource: not enough or missing control points.\n");
            valid = false;
        }
        else {
            if (cpts[0].time > 0.0) {
                egsWarning("EGS_DynamicSource: time index of control point 1 > 0.0.  This will generate many warning messages.\n");
            }
            int npts = cpts.size();
            for (int i=0; i<npts; i++) {
                if (i>0 && cpts[i].time < cpts[i-1].time) {
                    egsWarning("EGS_DynamicSource: time index of control point %i < time index of control point %i\n",i,i-1);
                    valid = false;
                }
                if (cpts[i].time<0.0) {
                    egsWarning("EGS_DynamicSource: time index of control point %i < 0.0\n",i);
                    valid = false;
                }
            }
            //normalize time values
            for (int i=0; i<npts-1; i++) {
                cpts[i].time /= cpts[npts-1].time;
            }
        }
        setUp();
    };

    /*! \brief Construct a dynamic source from the user input */
    EGS_DynamicSource(EGS_Input *, EGS_ObjectFactory *f=0);

    ~EGS_DynamicSource() {
        EGS_Object::deleteObject(source);
    };

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u) {
        int err = 1;
        EGS_ControlPoint ipt;
        EGS_I64 c;
        while (err) {
            c = source->getNextParticle(rndm,q,latch,E,wt,x,u);
            if (sync) {
                ptime = source->getTimeIndex();
                if (ptime<0) {
                    egsWarning("EGS_DynamicSource: You have selected synchronization of dynamic source with %s\n",source->getObjectName().c_str());
                    egsWarning("However, this source does not return time values for each particle.  Will turn off synchronization.\n");
                    sync = false;
                }
            }
            if (!sync) {
                ptime = rndm->getUniform();
            }
            setTimeIndex(ptime); //this is added for the storing of time index in a single location. Technically stored as a basesource attribute not dynamic source
            err = getCoord(ptime,ipt);
        }

        //translate source in Z
        x.z=x.z-ipt.dsource;
        //get the rotation matrices
        ipt.phicol *= M_PI/180;
        ipt.theta *= M_PI/180;
        ipt.phi *= M_PI/180;
        EGS_RotationMatrix Rcol(EGS_RotationMatrix::rotZ(ipt.phicol));
        EGS_RotationMatrix Rtheta(EGS_RotationMatrix::rotY(ipt.theta));
        EGS_RotationMatrix Rphi(EGS_RotationMatrix::rotZ(ipt.phi));
        //apply rotations in specific order and then translate relative
        //to the isocentre
        u=Rphi*Rtheta*Rcol*u;
        x=Rphi*Rtheta*Rcol*x + ipt.iso;
        return c;
    };
    EGS_Float getEmax() const {
        return source->getEmax();
    };
    EGS_Float getFluence() const {
        return source->getFluence();
    };
    bool storeState(ostream &data) const {
        return source->storeState(data);
    };
    bool setState(istream &data) {
        return source->setState(data);
    };
    bool addState(istream &data_in) {
        return source->addState(data_in);
    };
    void resetCounter() {
        source->resetCounter();
    };

    bool isValid() const {
        return (valid && source != 0);
    };

    void setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun, int npar, int nchunk) {
        source->setSimulationChunk(nstart, nrun, npar, nchunk);
    };

    void containsDynamic(bool &hasdynamic);

protected:

    EGS_BaseSource *source; //!< The source being rotated

    vector<EGS_ControlPoint> cpts;  //control point

    int ncpts;  //no. of control points

    bool valid; //is this a valid source

    bool sync; //set to true if source motion synched with time read from
    //iaea phsp or beam simulation source

    int getCoord(const EGS_Float rand, EGS_ControlPoint &ipt);

    EGS_Float ptime; //time index corresponding to particle
    //could just be a random number.

    void setUp();

};

#endif
