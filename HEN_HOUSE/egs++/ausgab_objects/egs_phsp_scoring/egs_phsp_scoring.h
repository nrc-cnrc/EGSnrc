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
#  Contributors:    Reid Townson
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

This ausgab object generates phase space data for particles using one of two methods:
1. Scores particles exiting and/or entering all surfaces of a predefined geometry.
   The geometry can be a component of the simulation geometry or coincident with a surface
   in the simulation geometry.
2. Scores particles on exiting one user-specified region and entering another.
   The user can specify multiple exit/entry region pairs.

Phase space data can be scored in one of 2 possible formats:

EGSnrc format: E,x,y,u,v,wt,latch
IAEA format: iq,E,[x],[y],[z],u,v,wt,latch,[mu]

Note that in IAEA format, the user has the option of specifying a fixed x, y, and/or z coordinate
of the scoring plane/line/point, in which case the fixed coordinates shall not be scored for each
particle but will be specified in the header (.IAEAheader) file.  Also, in this format, the
user has the option of scoring the synchronization parameter, mu, passed from the source.

Can be used in any C++ user code by entering the proper input block in
the ausgab object definition block.

\verbatim
:start ausgab object definition:
      :start ausgab object:
          library                  = egs_phsp_scoring
          name                     = some_name
          output format            = EGSnrc or IAEA
          constant X               = X value (cm) at which all particles are scored (IAEA format only)
          constant Y               = Y value (cm) at which all particles are scored (IAEA format only)
          constant Z               = Z value (cm) at which all particles are scored (IAEA format only)
          particle type            = all, photons, or charged
          score mu                 = yes or no [default] (IAEA format only)
          score multiple crossers  = no (default) or yes (EGSnrc format only)
          output directory         = name of output directory

     and one of two methods of scoring particles:

     Method 1: score particles on entry to/exit from a predefined geometry
          phase space geometry     = name of previously defined geometry
          score particles on       = entry, exit, entry and exit [default]

     Method 2: score particles on exiting one region and entering another
          from regions             = list of exit region numbers
          to regions               = list of entry region numbers

      :stop ausgab object:
:stop ausgab object definition:
\endverbatim

Phase space data is output to the file some_name.egsphsp1 (EGSnrc format)
or some_name.1.IAEAphsp and some_name.1.IAEAheader (IAEA format)
Note that if the user specifies constant X/Y/Z, then particles are all assumed to be scored at the
same X/Y/Z with this(ese) values output to .IAEAheader instead of being output for each particle
to the .IAEAphsp file.

Particles of the type indicated by the "particle type" input are scored.  If this input
is omitted, then all particles are scored.

In IAEA-format phase space files, the user can opt to score the synchronization parameter, mu.
This option will automatically be turned off if the parameter is not available from the
source.  Currently, this parameter can only be passed from egs_beam_source (only if the accelerator
includes synchronized CMs), iaea_phsp_source (if scored using a BEAMnrc simulation with
synchronized CMs or scored using this ausgab object with mu scoring turned on) and
egs_dynamic_source (always available).

The default is to not score multiple crossers (and their descendents) and, indeed, this
is, by definition, the protocol for IAEA format phase space files.  However, if the user is scoring
data in EGSnrc format, then they have the option to include multiple crossers and their
descendents.  Note that the marker for a particle having been scored is to set bit 31 of the
particle's latch high.  Thus, the marker is associated with the particle, not the scoring
object.  This has implications if the user wishes to have multiple phase space scoring
objects in one run: a particle scored by one phase space scoring object will not subsequently
be scored by another, unless the user has set "score multiple crossers = yes" in the
latter.

If "output directory" is omitted or left blank then the output directory defaults to the
application directory (i.e. $EGS_HOME/appname).

When using a phase space scoring geometry, particles can be scored on entering the phase space geometry,
exiting the geometry, or both (the default).  Be aware of how the "inside" and "outside" of the geometry
are defined when using this option.

When using pairs of exit/entry regions, the number of regions specified by the "to regions" input
must equal that of the "from regions" input.  If the exit region number is equal to the entry
region number, then the pair is deleted prior to the run.

A note on parallel runs:
If a phase space file is being written during a parallel run, then each job, i, outputs its phase
space data to some_name_wi.[egsphsp1][.1.IAEAheader/phsp]. Thus, the naming scheme is the same as
that for other output files from a parallel run.  These phase space files are not added automatically
when the results of a parallel run are combined.  The user must either use the addphsp tool or
program their own concatenation routine.

Example:
The following example input illustrates the two phase space scoring methods.  In both
cases, phase space is scored in planes perpendicular to the axis of a water/air cylinder.
\verbatim
:start geometry definition:
      :start geometry:
          library = egs_planes
          type = EGS_Zplanes
          positions = 21.0
          name = scoreplane
      :stop geometry:

      :start geometry:
          library = egs_planes
          type = EGS_Zplanes
          positions = 0.0 10.0 21.0
          name = cutplane
      :stop geometry:

      :start geometry:
          library = egs_cylinders
          type = EGS_ZCylinders
          name = cylinders
          radii = 1.0 5.0 15.0
          midpoint = 0
          :start media input:
             media = H2O521ICRU AIR521ICRU
             set medium = 0 0
             set medium = 1 1
             set medium = 2 0
          :stop media input:
      :stop geometry:

      :start geometry:
           name = maingeom
           library = egs_cdgeometry
           base geometry = cutplane
           set geometry = 0 cylinders
           set geometry = 1 cylinders
      :stop geometry:

      simulation geometry = maingeom
  :stop geometry definition:

  :start ausgab object definition:
      :start ausgab object:
        library = egs_phsp_scoring
        name = test
        from regions = 0 2
        to regions = 3 5
        output format = IAEA
        constant Z = 10.0
        particle type = all
        score mu = no
      :stop ausgab object:

      :start ausgab object:
        library = egs_phsp_scoring
        name = test2
        phase space geometry = scoreplane
        output format = EGSnrc
        particle type = all
        score particles on = entry
        score multiple crossers = yes
      :stop ausgab object:
:stop ausgab object definition:
\endverbatim
The first ausgab object, "test," uses scoring method 2 (exit/entry region pairs) to
score phase space data, in IAEA format, for all particles crossing a plane perpendicular
to the cylinder and at the mid-point of the cylinder (Z=10 cm).  Data will be scored
for particles within the innermost cylinder (exit/entry regions 0/3) and within the outermost
annulus (exit/entry regions 2/5).  Note that the constant Z value of the scoring plane is input,
so data will be in 2D format.  Also note that IAEA format, by convention, does not score multiple
crossers.  The output will be to test.1.IAEAheader (header file) and test.1.IAEAphsp (phase space data).

The second ausgab object, "test2," uses scoring method 1 (predefined scoring geometry) to
score phase space data, in EGSnrc format, for all particles crossing a single plane coincident with
the bottom (Z=21 cm) of the cylinder.  The scoring plane is given by the geometry, "scoreplane,"
which defines a single plane perpendicular to Z at Z=21 cm.  Recall that a single plane defines
a single region on the +ve normal side of the plane.  In this example, a particle is considered "outside"
scoreplane if Z < 21.0 and inside if Z > 21.0 and, thus, scoring particles on "entry" will collect
data for all particles leaving out the bottom of the cylinder.  In this case, however, the specification
of scoring direction is moot, since particles leaving the bottom of the cylinder are effectively leaving
the simulation geometry.  Note that the option to score multiple crossers has been turned on.  Otherwise,
particles previously scored in "test" would not be scored by "test2."  Phase space data will be output
to test2.egsphsp1.
*/

class EGS_PHSP_SCORING_EXPORT EGS_PhspScoring : public EGS_AusgabObject {

public:

    EGS_PhspScoring(const string &Name="", EGS_ObjectFactory *f = 0);

    ~EGS_PhspScoring();

    int processEvent(EGS_Application::AusgabCall iarg) {
        //only score if particle has correct charge
        if (ocharge==0 || 1+abs(app->top_p.q)==ocharge) {
            EGS_Vector x = app->top_p.x;
            int ir = app->top_p.ir;
            int latch = app->top_p.latch;
            //only score if: 1) it has not been scored before or
            //2) we are scoring multiple crossers (EGSnrc format only)
            if (!(latch & bsmc()) || (oformat==0 && score_mc)) {
                if (score_type==0) {  //using scoring geometry
                    if (iarg == 0) {
                        phsp_before = phsp_geom->isInside(x);
                    }

                    if (iarg == 5) {
                        phsp_after = phsp_geom->isInside(x);
                        if (phsp_after != phsp_before) {
                            if (scoredir == 0 || (scoredir == 1 && phsp_after) ||
                                    (scoredir == 2 && phsp_before)) {
                                storeParticle(current_case);
                            }
                            //set bit 31 to flag this as having been scored
                            latch = (latch | bsmc());
                            app->setLatch(latch);
                            return 0;
                        }
                    }
                }
                else if (score_type==1) { //pairs of exit/entry regions
                    if (iarg == 0) {
                        ir_before = ir;
                    }

                    if (iarg == 5) {
                        ir_after = ir;
                        if (from_to[ir_before].size()>0 && ir_before != ir_after) {
                            for (int i=0; i< from_to[ir_before].size(); i++) {
                                if (ir_after == from_to[ir_before][i]) {
                                    storeParticle(current_case);
                                    latch = (latch | bsmc());
                                    app->setLatch(latch);
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        return 0;
    };

    int processEvent(EGS_Application::AusgabCall iarg, int ir) {
        //same as above, we don't need the region no.
        if (ocharge==0 || 1+abs(app->top_p.q)==ocharge) {
            EGS_Vector x = app->top_p.x;
            int latch = app->top_p.latch;
            //only score if: 1) it has not been scored before or
            //2) we are scoring multiple crossers (EGSnrc format only)
            if (!(latch & bsmc()) || (oformat==0 && score_mc)) {
                if (score_type==0) {  //using scoring geometry
                    if (iarg == 0) {
                        phsp_before = phsp_geom->isInside(x);
                    }

                    if (iarg == 5) {
                        phsp_after = phsp_geom->isInside(x);
                        if (phsp_after != phsp_before) {
                            if (scoredir == 0 || (scoredir == 1 && phsp_after) ||
                                    (scoredir == 2 && phsp_before)) {
                                storeParticle(current_case);
                            }
                            //set bit 31 to flag this as having been scored
                            latch = (latch | bsmc());
                            app->setLatch(latch);
                            return 0;
                        }
                    }
                }
                else if (score_type==1) { //pairs of exit/entry regions
                    if (iarg == 0) {
                        ir_before = ir;
                    }

                    if (iarg == 5) {
                        ir_after = ir;
                        if (from_to[ir_before].size()>0 && ir_before != ir_after) {
                            for (int i=0; i< from_to[ir_before].size(); i++) {
                                if (ir_after == from_to[ir_before][i]) {
                                    storeParticle(current_case);
                                    latch = (latch | bsmc());
                                    app->setLatch(latch);
                                    return 0;
                                }
                            }
                        }
                    }
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
        score_type=0;
        phsp_geom = phspgeom;
    }

    void setEntryExitReg(const vector <int> from_reg, const vector <int> to_reg) {
        score_type=1;
        fromreg=from_reg;
        toreg=to_reg;
    }

    void setOType(const int phspouttype) {
        oformat = phspouttype;
    }

    //set output directory
    void setOutDir(const string outdir) {
        phspoutdir =  outdir;
    }

    void setParticleType(const int ptype) {
        ocharge = ptype;
    }

    void setScoreDir(const int sdir) {
        scoredir = sdir;
    }

    void setMuScore(const int imuscore) {
        if (imuscore == 1) {
            score_mu = true;
        }
        else {
            score_mu = false;
        }
    }

    void setScoreMC(const int iscoremc) {
        if (iscoremc==1) {
            score_mc = true;
        }
        else {
            score_mc = false;
        }
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
        EGS_Float E, x, y, z, u, v, w, wt, mu;
    };

    //functions, struct and variables used to write EGSnrc format phsp files
    static unsigned int bclr() {
        return ~((1 << 30) | (1 << 29));
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
            if (p.q == -1) {
                latch = (latch | bsqe());
            }
            else if (p.q == 1) {
                latch = (latch | bsqp());
            }
            E = p.E;
            x = p.x;
            y = p.y;
            u = p.u;
            v = p.v;
            wt = p.w >= 0 ? p.wt : -p.wt;
        };
    };

    bool score_mc; //set to true to score multiple crossers and their descendents

    //variables specific to IAEA format
    mutable int iaea_id; //file id--mutable so we can write to it during storeState
    int latch_ind; //IAEA id for latch variable (set to 2)
    int mu_ind; //IAEA id for mu--no ID for now so set to generic float (0)
    int iaea_n_extra_float, iaea_n_extra_long; //no. of extra floats, ints
    int iaea_i_latch, iaea_i_mu; //indices of indicated variables in arrays
    int iaea_q_type[3]; //easy conversion from q to iaea type
    bool xyz_is_const[3]; //set to true if scoring at constant X/Y/Z
    float xyzscore[3]; //constant X/Y/Z scoring values
    int len; //length of name
    char *phsp_fname_char; //need file name in char format
    bool score_mu; //set to true if scoring mu
    float pmu; //mu value associated with particle

    //back to variables common to EGSnrc and IAEA formats

    Particle *p_stack; //the stored particle stack

    //below used to set bit 31 to denote the particle has been scored
    static unsigned int bsmc() {
        return (1 << 31);
    }

    mutable int phsp_index; //index in p_stack array -- mutable so we can change it in storeState
    int store_max; //max. no. of particles to store in p_stack
    mutable fstream phsp_file; //output file -- mutable so we can write to it during storeState
    EGS_I64 count; //total no. of particles in file
    EGS_I64 countg; //no. of photons in file
    float emin; //min. k.e. of charged particles in file
    float emax; //max. k.e. of particles in phsp file
    mutable bool first_flush; //first time writing to file in this run -- mutable so we can change it in storeState

    bool is_restart; //true if this is a restart
    EGS_I64 countprev; //no. of particles written to file before restart

    EGS_I64    last_case;   //last primary history scored
    EGS_I64    current_case; //current primary history

    int oformat;           //0 for EGSnrc format, 1 for IAEA format

    int ocharge;           //particle type for output: 0--all; 1--photons; 2--charged particles

    string phsp_fname; //name of phase space file

    string phspoutdir; //output directory

    int score_type;        //0 if scoring on exiting/entering a phase space geometry
    //1 if using pairs of exit/entry regions

    //for method 1: scoring using predefined geometry
    EGS_BaseGeometry *phsp_geom; //geometry on entrance to/exit from which phase space data is scored
    int scoredir;           //scoring direction: 0--on entry and exit; 1--on entry; 2--on exit
    bool phsp_before; //true if inside scoring geometry before step
    bool phsp_after;  //true if inside scoring geometry after step

    //for method 2: scoring using exit/entry region pairs
    vector <int> fromreg;          //array of exit regions
    vector <int> toreg;            //array of entry regions
    vector <vector <int> > from_to;   //from a given global exit region, an array of possible entry regions
    int ir_before, ir_after;       //reg. no. before and after step
};

#endif
