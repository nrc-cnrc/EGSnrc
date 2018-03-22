/*
###############################################################################
#
#  EGSnrc egs++ phase space scoring object headers
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
#  Author:          Blake Walters, 2018
#
###############################################################################
#
#  A phase space output tool.  See .cpp file for more details.
#
###############################################################################
*/


/*! \file egs_phsp_scoring.h
 *  \brief A phase space scoring ausgab object
 *  \BW
 */

#ifndef EGS_PHSP_SCORING_
#define EGS_PHSP_SCORING_

#include "egs_ausgab_object.h"
#include "egs_application.h"
#include "egs_scoring.h"
#include "egs_base_geometry.h"

#ifdef WIN32

    #ifdef BUILD_PHSP_SCORING_DLL
        #define EGS_PHSP_SCORING_EXPORT __declspec(dllexport)
    #else
        #define EGS_PHSP_SCORING_EXPORT __declspec(dllimport)
    #endif
    #define EGS_PHSP_SCORING_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_PHSP_SCORING_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_PHSP_SCORING_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_PHSP_SCORING_EXPORT
        #define EGS_PHSP_SCORING_LOCAL
    #endif

#endif

/*! \brief A phase space scoring object: header

\ingroup AusgabObjects

*/

class EGS_PHSP_SCORING_EXPORT EGS_PhspScoring : public EGS_AusgabObject {

public:

    EGS_PhspScoring(const string &Name="", EGS_ObjectFactory *f = 0);

    ~EGS_PhspScoring();

    int processEvent(EGS_Application::AusgabCall iarg) {
      if (ocharge==0 || 1+abs(app->top_p.q)==ocharge) {
        EGS_Vector x = app->top_p.x;

        if (iarg == 0) phsp_before = phsp_geom->isInside(x);

        if (iarg == 5) {
            phsp_after = phsp_geom->isInside(x);
            if (phsp_after != phsp_before) {
              if (scoredir == 0 || (scoredir == 1 && phsp_after) ||
                  (scoredir == 2 && phsp_before))
                      storeParticle(current_case);
            }
        }
      }
      return 0;
    };

    int processEvent(EGS_Application::AusgabCall iarg, int ir) {
        //same as above, we don't need the region no.
      if (ocharge==0 || 1+abs(app->top_p.q)==ocharge) {
        EGS_Vector x = app->top_p.x;

        if (iarg == 0) phsp_before = phsp_geom->isInside(x);

        if (iarg == 5) {
            phsp_after = phsp_geom->isInside(x);
            if (phsp_after != phsp_before) {
                //store the particle
             if (scoredir == 0 || (scoredir == 1 && phsp_after) ||
                  (scoredir == 2 && phsp_before))
                   storeParticle(current_case);
            }
        }
      }
      return 0;
    };

    bool needsCall(EGS_Application::AusgabCall iarg) const {
        if (iarg == 0 || iarg == 5) {
            return true;
        }
        else {
            return false;
        }
    };

    //below gets called from startNewShower if current_case != last_case
    //don't update last_case yet
    void setCurrentCase(EGS_I64 ncase) {
        current_case=ncase;
    };

    void setGeom(EGS_BaseGeometry *phspgeom) {
        phsp_geom = phspgeom;
    }

    void setOType(const int phspouttype) {
        oformat = phspouttype;
    }

    void setOutDir(const string outdir) {
        phspoutdir =  outdir;
    }

    void setParticleType (const int ptype) {
        ocharge = ptype;
    }

    void setScoreDir (const int sdir) {
        scoredir = sdir;
    }

    //method below pertains only to IAEA format
    //set array element 0/1/2 of xyz_is_constant equal to true
    //if scoring at a constant X/Y/Z value and store the
    //constant value in element 0/1/2 of array xyzscore
    void setXYZconst(bool xyzisconst[3], float xyzconst[3]) {
       for (int i=0; i<3; i++) {
         xyz_is_const[i] = xyzisconst[i];
         xyzscore[i]=xyzconst[i];
       }
    }

    //set output directory

    void storeParticle(EGS_I64 ncase);

    int flushBuffer() const;

    void openPhspFile() const;

    void setApplication(EGS_Application *App);

    void reportResults();

    bool storeState(ostream &data) const;
    bool setState(istream &data);
    bool addState(istream &data);

protected:

    struct Particle {
        int  q, latch;
        EGS_Float E, x, y, z, u, v, w, wt;
    };

    //functions, struct and variables used to write EGSnrc format phsp files
    static unsigned int bclr() {
          return ~( (1 << 31) | (1 << 30) | (1 << 29) );
    }
    static unsigned int bsqe() {
           return (1 << 30);
    }
    static unsigned int bsqp() {
           return (1 << 29);
    }
    struct egs_phsp_write_struct {
       int   latch;
       float E;
       float x,y;
       float u,v;
       float wt;
       egs_phsp_write_struct() {};
       egs_phsp_write_struct(const Particle &p) {
        latch = (p.latch & bclr());
        if( p.q == -1 ) latch = (latch | bsqe());
        else if( p.q == 1 ) latch = (latch | bsqp());
        E = p.E; x = p.x; y = p.y; u = p.u; v = p.v;
        wt = p.w >= 0 ? p.wt : -p.wt;
       };
    };
    mutable fstream phsp_file; //output file -- mutable so we can write to it during storeState
    EGS_I64 count; //total no. of particles in file
    EGS_I64 countg; //no. of photons in file
    float emax; //max. k.e. of particles in phsp file
    float emin; //min. k.e. of charged particles in file

    //variables specific to IAEA format
    mutable int iaea_id; //file id--mutable so we can write to it during storeState

    int latch_ind; //IAEA id for latch variable (set to 2)
    int iaea_n_extra_float, iaea_n_extra_long; //no. of extra floats, ints
    int iaea_i_latch, iaea_i_muindex; //indices of indicated variables in arrays...may not need these
    int iaea_q_type[3]; //easy conversion from q to iaea type
    bool xyz_is_const[3]; //set to true if scoring at constant X/Y/Z
    float xyzscore[3]; //constant X/Y/Z scoring values
    const string xyzname[3] = {"X", "Y", "Z"};
    int len; //length of name
    char *phsp_fname_char; //need file name in char format

    //back to variables common to EGSnrc and IAEA formats

    Particle *p_stack; //the stored particle stack

    bool phsp_before; //true if inside scoring geometry before step
    bool phsp_after;  //true if inside scoring geometry after step

    int store_max; //max. no. of particles to store in p_stack
    mutable int phsp_index; //index in p_stack array -- mutable so we can change it in storeState
    mutable bool first_flush; //first time writing to file in this run -- mutable so we can change it in storeState

    bool is_restart; //true if this is a restart
    EGS_I64 countprev; //no. of particles written to file before restart

    EGS_I64    last_case;   //last primary history scored
    EGS_I64    current_case; //current primary history

    EGS_BaseGeometry *phsp_geom; //geometry on entrance to/exit from which phase space data is scored
    int oformat;           //0 for EGSnrc format, 1 for IAEA format

    int ocharge;           //particle type for output: 0--all; 1--photons; 2--charged particles

    int scoredir;           //scoring direction: 0--on entry and exit; 1--on entry; 2--on exit

    string phsp_fname; //name of phase space file

    string phspoutdir; //output directory

    const EGS_Float prm = 0.5109989461; //precise rest mass
                                  //may eventually want to get this from
                                  //the_useful somehow
};

#endif
