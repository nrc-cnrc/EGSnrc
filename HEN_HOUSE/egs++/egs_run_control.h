/*
###############################################################################
#
#  EGSnrc egs++ run control headers
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
#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#                   Blake Walters
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_run_control.h
 *  \brief EGS_RunControl and EGS_JCFControl class header file
 *  \IK
 */

#ifndef EGS_RUN_CONTROL_
#define EGS_RUN_CONTROL_

#include "egs_libconfig.h"
#include "egs_timer.h"

#include <iostream>
using namespace std;

class EGS_Application;
class EGS_Input;

/*! \brief A simple run control object for advanced EGSnrc C++ applications.

  \ingroup egspp_main

  In EGSnrc applications derived from EGS_AdvancedApplication the program
  execution is controlled by a 'run control object' (RCO). The purpose of
  the RCO is to tell the shower loop into how many 'chunks' the simulation
  should be split, how many particles to run per simulation chunk, into how
  many batches to split a simulation chunk, etc. In this way it is easy
  for EGSnrc C++ application developers to either use one of the RCO's
  provided with egspp or to implement their own RCO's with alternative
  implementations or strategies for {\em e.g.} parallel job processing.

  <p>Terminology
     - Simulations are split into 'chunks'. For simple simulations
       (no parallel runs, etc.) there is a single simulation chunk
       with the number of histories specified in the input file.
     - Each simulation chunk is split into 'batches'. The batches are
       not required for statistical analysis (by using the provided
       \link EGS_ScoringArray scoring classes \endlink
       it is easy to has a history-by-history uncertainty
       estimation). Instead, simulation chunks are split into batches
       so that the progress of the simulation can be reported after the
       completion of a batch and the current results can be stored into a
       data file. By default there are 10 batches per simulation chunk

  <p>Three RCO's are provided with egspp:
   - A 'simple' RCO implemented in EGS_RunControl. This RCO is used by default
     for single job control. This RCO provides the ability to run simulations
     with a user specified number of particles and up to a user specified
     statistical uncertainty. It also provides the ability to restrict
     the simulation to a user specified maximum CPU time, to perform restarted
     simulations or to simply analyze the results of a previous run or to
     combine the results of previously performed parallel runs.
   - A \link EGS_JCFControl 'job control file' (JCF) RCO \endlink.
     This RCO is used by default for parallel runs. It has all the functionality
     of the 'simple' RCO plus additional methods to control parallel execution
     via a 'job control file'.
   - A \link EGS_UniformRunControl uniform RCO \endlink.
     This RCO distributes the histories uniformly among all jobs.
     It can be an alternative to the JCF RCO when a JCF cannot be used. This can
     happen when jobs do not start sequentially. In such cases the JCF may not be
     available if job number 1 is not started first.
*/
class EGS_EXPORT EGS_RunControl {

public:

    /*! \brief Creates an RCO for the application \a app.

    This constructors obtains the user input to the application
    (available to the application as an EGS_Input object) and looks
    for
    */
    EGS_RunControl(EGS_Application *app);

    /*! \brief Destructor.  */
    virtual ~EGS_RunControl();

    /*! \brief Set the number of particles to be simulated to \a n */
    void setNcase(EGS_I64 n) {
        if (n > 0) {
            ncase = n;
        }
    };

    /*! \brief Set the number of batches to \a n */
    void setNbatch(int n) {
        if (n > 0) {
            nbatch = n;
        }
    };

    /*! \brief Set the maximum CPU time for the simulation to \a t */
    void setMaxTime(EGS_Float t) {
        maxt = t;
    };

    /*! \brief Set the required statistical uncertainty to \a a */
    void setRequiredUncertainty(EGS_Float a) {
        accu = a;
    };

    /*! \brief Returns the total number of particles to be simulated */
    EGS_I64 getNcase() const {
        return ncase;
    };

    /*! \brief Returns the number of batches per simulation chunk */
    int     getNbatch() const {
        return nbatch;
    };

    /*! \brief Returns the number of simulation chunks */
    int     getNchunk() const {
        return nchunk;
    };

    /*! \brief Starts the simulation.

      Returns zero if the simulation was successfully started,
      a positive value if no error occured but no histories are to be run
      (e.g. combine and analyze runs) and a negative value if some
      error occured.
    */
    virtual int     startSimulation();

    /*! \brief Returns the number of histories to run in the next
      simulation chunk.

      In a simple run this is simply the number of histories specified
      in the input file. For parallel runs the simulation is split
      into smaller 'chunks' and this function returns how many histories
      are in such a chunk. This function is called from within the
      runSimulation() function of EGS_Application.
    */
    virtual EGS_I64 getNextChunk() {
        return getNcase() - ndone;
    };

    /*! \brief Finish the simulation.

     This function is called from within the finishSimulation() method
     of EGS_Application and should return 1 of the follwoing 3 exit codes:
       -  -1 indicates that some error occured
       -   0 indicates that everything was OK and the application should
           simply exit.
       -   1 indicates that everything was OK and that this job is the
           last job in a parallel run.
    */
    virtual int     finishSimulation();

    /*! \brief Start a new batch

    This function is called from within the shower loop before starting each
    new batch. Returns \c true, if the simulation is to proceed,
    \c false if the simulation is to be terminated immediately.
    */
    virtual bool    startBatch(int,EGS_I64);

    /*! \brief Finish a batch

    This function is called from within the shower loop after a batch has been
    finished. Returns \c true, if the simulation is to proceed,
    \c false if the simulation is to be terminated immediately.
    */
    virtual bool    finishBatch();

    virtual void    describeRCO();
    virtual bool    storeState(ostream &data);
    virtual bool    setState(istream &data);
    virtual bool    addState(istream &data);
    virtual void    resetCounter();
    virtual bool    getCombinedResult(double &, double &) const {
        return false;
    };

    virtual EGS_I64 getNdone() const {
        return ndone;
    };

    virtual void    setNdone(EGS_I64 Ndone) {
        ndone = Ndone;
    };

    virtual void    incrementNdone() {
        ++ndone;
    };

    virtual EGS_Float getCPUTime() const {
        return cpu_time+previous_cpu_time;
    };

    /*! \brief Define RCO types */
    enum RCOType {
        simple,   //!< single job or multiple independent jobs
        uniform,  //!< parallel jobs with same numbe of histories
        balanced  //!< parallel jobs with balanced load via JCF
    };

    static EGS_RunControl *getRunControlObject(EGS_Application *);

    int             geomErrorCount, geomErrorMax;

protected:

    EGS_Application *app;
    EGS_Input       *input;

    EGS_I64         ncase;  // number of histories.
    EGS_I64         ndone;  // histories done so far.
    EGS_Float       maxt;   // maximum time to run in hours.
    EGS_Float       accu;   // statistical uncertainty sought.
    int             nbatch; // number of batches.
    int             restart;// =0 => fresh calculation
    // =1 => restart calculation
    // =2 => analyze results
    // =3 => combine parallel run
    int             nchunk; // number of simulation "chunks"

    RCOType         rco_type; //!< RCO type to use

    EGS_Timer       timer;
    EGS_Float       cpu_time;
    EGS_Float       previous_cpu_time;

};

class EGS_FileLocking;

/*! \brief A 'job control file' (JCF) RCO.

   \ingroup egspp_main

   The JCF RCO is used by default for controlling parallel job execution.
   It is called a JCF-RCO because execution is controlled by a
   file placed in the user code directory, which is created by the first
   parallel job and contains information such as the number of particles
   remaining to be simulated, number of jobs running, combined result of
   all parallel jobs, etc. This RCO objects requires that file locking
   works on the file system containing the \c EGS_HOME directory because
   the JCF is locked prior to being modified by one of the jobs to prevent
   multiply jobs modifying the file at the same time. For more details
   see PIRS-877.

*/

class EGS_EXPORT EGS_JCFControl : public EGS_RunControl {

public:

    EGS_JCFControl(EGS_Application *, int Nbuf=1024);
    ~EGS_JCFControl();
    void setNchunkForParallel(int n) {
        if (n > 0) {
            nchunk = n;
        }
    };
    int  startSimulation();
    EGS_I64 getNextChunk();
    int  finishSimulation();
    bool getCombinedResult(double &, double &) const;

protected:

    EGS_I64 nleft, ntot;
    double  tsum, tsum2, tcount, norm;
    double  last_sum, last_sum2, last_count;
    unsigned long start_time;
    int    njob;
    int    npar;
    int    ipar;
    int    ifirst;
    bool   first_time;
    bool   removed_jcf;
    int    nbuf;
    char   *buf;

    EGS_FileLocking *p;

    bool   createControlFile();
    bool   openControlFile();
    bool   closeControlFile();
    bool   lockControlFile();
    bool   unlockControlFile();
    bool   rewindControlFile();
    bool   readControlFile();
    bool   writeControlFile();
    virtual bool writeControlString();
    virtual bool readControlString();

};

/*! \brief A job control object for homogeneous computing environments (HCE).

   \ingroup egspp_main

   The uniform RCO is used for controlling parallel job execution
   in computing environments (CE) with identical hardware, software
   and communication layer (aka homogeneous CE):

   - Number of histories 'ncase' split equally among all jobs.

   - Assume last job finishes last and then cycles 5 times by default
     ('check_intervals' variable) for a period of time defined to
     be 1 s by default ('milliseconds' variable). Defaults can be changed
     via the 'run control' input block using:

       interval wait time  = time in ms (default 1 s)
       number of intervals = an_integer_value (default 5)

   - The last job combines the parallel runs by default. Since the last job
     could finish before some of the other jobs, users can set another
     job or several jobs to be 'watcher' jobs. In principle it is enough to
     define one 'watcher' job that waits long enough for all jobs to complete.
     To change the default, use the following key:

     watcher jobs = job_i,..., job_j

   - If requested, a run-completion check can be made every cycle
     by checking that the number of *.egsdat files equals the number
     of parallel jobs submitted. This could speed things up by not having
     to wait for all checking cycles. However, it could also be the case
     that some jobs might have failed, in which case, after the checking
     cycles complete, only the available *.egsdat files will be combined.

     This option can be set via the 'run control' input block using:

     check jobs completed = yes|no # default is 'no'

     When this option is enabled, each job erases at the beginning of the run
     its corresponding *.egsdat file if it exists.

*/

class EGS_EXPORT EGS_UniformRunControl : public EGS_RunControl {

public:

    EGS_UniformRunControl(EGS_Application *app);
    ~EGS_UniformRunControl() {};

    void  describeRCO();

    int  startSimulation();

    /*!  \brief Uses 'watcher' jobs to determine if the simulation has finished.

    If the current job is a 'watcher' job, it waits for some time before issuing
    the signal to recombine all available parallel jobs. These 'watcher' jobs
    can also produce intermediate results while waiting. If all jobs complete while
    waiting, the 'watcher' job combines all results and exits.

    */
    int  finishSimulation();

protected:

    int milliseconds;   // time interval for checking
    // if all jobs finished (default 1000 ms)

    int check_intervals;// Number of intervals to check
    // if all jobs done (default 5)

    int    njob;
    int    npar;
    int    ipar;
    int    ifirst;
    bool   check_egsdat;// If true, and a 'watcher' job, produce intermediate results
    bool   watcher_job; // If true, job is a 'watcher'
};

#endif
