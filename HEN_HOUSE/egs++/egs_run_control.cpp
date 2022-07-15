/*
###############################################################################
#
#  EGSnrc egs++ run control
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
#                   Hubert Ho
#                   Ernesto Mainegra-Hing
#                   Blake Walters
#                   Marc Chamberland
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_run_control.cpp
 *  \brief EGS_RunControl and EGS_JCFControl implementation
 *  \IK
 */

#include "egs_run_control.h"
#include "egs_application.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_library.h"

#include <vector>
#include <ctime>
#include <cstdio>

using namespace std;

vector<EGS_Library *> rc_libs;
static int n_run_controls = 0;

EGS_RunControl::EGS_RunControl(EGS_Application *a) : geomErrorCount(0),
    geomErrorMax(0), app(a), input(0), ncase(0), ndone(0), maxt(-1), accu(-1),
    nbatch(10), restart(0), nchunk(1), cpu_time(0), previous_cpu_time(0),
    rco_type(simple) {
    n_run_controls++;
    if (!app) egsFatal("EGS_RunControl::EGS_RunControl: it is not allowed\n"
                           " to construct a run control object on a NULL application\n");
    input = app->getInput();
    if (!input) {
        egsWarning("EGS_RunControl::EGS_RunControl: the application has no"
                   " input\n");
        return;
    }
    input = input->takeInputItem("run control");
    if (!input) {
        egsWarning("EGS_RunControl::EGS_RunControl: no 'run control' "
                   "input\n");
        return;
    }
    double ncase_double;
    int err = input->getInput("number of histories", ncase_double);
    if (err) {
        err = input->getInput("ncase", ncase_double);
        if (err)
            egsWarning("EGS_RunControl: missing/wrong 'ncase' or "
                       "'number of histories' input\n");
    }
    ncase = EGS_I64(ncase_double);
    /*****************************************************
     * Split histories into different parallel jobs.
     * For the JCFO ncase reset to total as it is handled
     * via the lock file mechanism dispatching smaller
     * of histories chunks.
     *****************************************************/
    if (app->getNparallel()) {
        ncase /= app->getNparallel();
    }
    err = input->getInput("nbatch",nbatch);
    if (err) {
        nbatch = 10;
    }
    err = input->getInput("max cpu hours allowed",maxt);
    if (err) {
        maxt = -1;
    }
    err = input->getInput("statistical accuracy sought",accu);
    if (err) {
        accu = -1;
    }
    err = input->getInput("geometry error limit", geomErrorMax);
    if (err) {
        geomErrorMax = 0;
    }

    vector<string> ctype;
    ctype.push_back("first");
    ctype.push_back("restart");
    ctype.push_back("analyze");
    ctype.push_back("combine");
    restart = input->getInput("calculation",ctype,0);
}

EGS_RunControl::~EGS_RunControl() {
    if (input) {
        delete input;
    }
    n_run_controls--;
    if (!n_run_controls) {
        while (rc_libs.size() > 0) {
            delete rc_libs[rc_libs.size()-1];
            rc_libs.pop_back();
        }
    }
}

void EGS_RunControl::describeRCO() {
    egsInformation(
        "Run Control Object (RCO):\n"
        "=========================\n");
    switch (rco_type) {
    case simple:
        egsInformation("  type = simple\n");
        break;
    case balanced:
        egsInformation("  type = balanced (JCF)\n");
        break;
    case uniform:
        egsInformation("  type = uniform\n");
        break;
    }
}

bool EGS_RunControl::storeState(ostream &data) {
    if (!egsStoreI64(data,ndone)) {
        return false;
    }
    data << "  " << (cpu_time+previous_cpu_time) << endl;
    return data.good();
}

bool EGS_RunControl::setState(istream &data) {
    EGS_I64 ndone1;
    if (!egsGetI64(data,ndone1)) {
        return false;
    }
    ndone += ndone1;
    ncase += ndone1;
    data >> previous_cpu_time;
    return data.good();
}

bool EGS_RunControl::addState(istream &data) {
    EGS_Float previous_cpu_time_save = previous_cpu_time;
    if (!setState(data)) {
        return false;
    }
    previous_cpu_time += previous_cpu_time_save;
    return true;
}

void EGS_RunControl::resetCounter() {
    previous_cpu_time = 0;
    cpu_time = 0;
    timer.start();
    ncase = 0;
    ndone = 0;
}

int EGS_RunControl::startSimulation() {
    if (restart == 1 || restart == 2) {
        if (app->readData()) {
            return -1;
        }
        if (restart == 2) {
            ncase = ndone;
            egsInformation("\n\nResult analysis only\n\n");
            return 1;
        }
    }
    else if (restart == 3) {
        app->describeSimulation();
        egsInformation("\n\nCombine results only\n\n");
        egsInformation("calling combineResults()\n");
        int err = app->combineResults();
        ncase = ndone;
        return err ? -1 : 2;
    }
    app->describeSimulation();
    time_t tinfo = time(0);
    egsInformation("\n\nStarting simulation on %s\n",
                   asctime(localtime(&tinfo)));
    if (restart == 0) {
        egsInformation("    Fresh simulation of %lld histories\n\n\n",ncase);
    }
    else {
        egsInformation("    Restarted simulation with %lld old and %lld"
                       " new histories\n\n\n",ndone,ncase-ndone);
    }
    timer.start();
    return 0;
}

bool EGS_RunControl::startBatch(int ibatch, EGS_I64 ncase_per_batch) {
    if (!ibatch) egsInformation(
            "  Batch             CPU time        Result   Uncertainty(%c)\n"
            "==========================================================\n",'%');
    if (maxt > 0 && ndone > 0) {
        EGS_Float time_per_shower = (cpu_time + previous_cpu_time)/ndone;
        EGS_Float extra_time = time_per_shower*ncase_per_batch;
        if (cpu_time + extra_time > maxt*3600) {
            egsWarning("\n\n*** Not enough time to finish another batch\n"
                       "    => terminating simulation.\n\n");
            return false;
        }
    }
    egsInformation("%7d",ibatch+1);
    ndone += ncase_per_batch;
    return true;
}

bool EGS_RunControl::finishBatch() {
    cpu_time = timer.time();
    int out = app->outputData();
    if (out) {
        egsWarning("\n\noutputData() returned error code %d ?\n",out);
    }
    double sum, sum2, norm, count;
    app->getCurrentResult(sum,sum2,norm,count);
    double f, df;
    if (sum > 0 && sum2 > 0 && norm > 0 && count > 1) {
        f = sum*norm/count;
        df = count*sum2/(sum*sum)-1;
        if (df > 0) {
            df = 100*sqrt(df/(count-1));
        }
        else {
            df = 100;
        }
    }
    else {
        f = 0;
        df = 100;
    }
    egsInformation("        %12.2f %14g %14.2f\n",cpu_time,f,df);
    if (df < 100 && accu > 0 && df < accu) {
        char c = '%';
        egsWarning("\n\n*** Reached the requested uncertainty of %g%c\n"
                   "    => terminating simulation.\n\n",accu,c);
        return false;
    }
    return true;
}

EGS_UniformRunControl::EGS_UniformRunControl(EGS_Application *a) :
    EGS_RunControl(a), njob(0), npar(app->getNparallel()),
    ipar(app->getIparallel()), ifirst(app->getFirstParallel()),
    milliseconds(1000), check_intervals(5), check_egsdat(true),
    watcher_job(false) {

    rco_type = uniform;

    if (input) {

        /*Change waiting time to check for parallel run completion*/
        int dummy;
        int err = input->getInput("interval wait time", dummy);
        if (!err) {
            milliseconds = dummy;
        }

        /*Change how many times to check for parallel run completion*/
        err = input->getInput("number of intervals", dummy);
        if (!err) {
            check_intervals = dummy;
        }

        /* Define watcher jobs to check for parallel run completion*/
        vector<int> w_jobs;
        err = input->getInput("watcher jobs", w_jobs);
        if (!err) {
            for (int i = 0; i < w_jobs.size(); i++) {
                if (ipar == w_jobs[i]) {
                    watcher_job = true;
                    break;
                }
            }
        }
        else { // use defaults
            /* last job is watcher job */
            if (ipar == ifirst + npar - 1) {
                watcher_job = true;
            }
            else {
                watcher_job = false;
            }
        }

        /* Request checking parallel run completion */
        vector<string> check_options;
        check_options.push_back("yes");
        check_options.push_back("no");
        int ichk = input->getInput("check jobs completed",check_options,0);
        if (ichk != 0) {
            check_egsdat = false;    // true by default
        }

    }
    else { // use defaults if no RCO input found
        /* last job is watcher job */
        if (ipar == ifirst + npar - 1) {
            watcher_job = true;
        }
    }
}

int EGS_UniformRunControl::startSimulation() {


    /* Check run completion based on *egsdat files requires erasing
       existing files from previous runs.
     */
    if (check_egsdat) {
        char buf[512];
        sprintf(buf,"%s_w%d.egsdat",app->getFinalOutputFile().c_str(), ipar);
        string datFile = egsJoinPath(app->getAppDir(),buf);
        if (remove(datFile.c_str()) == 0) {
            egsWarning("EGS_UniformRunControl: %s deleted\n",
                       datFile.c_str());
        }
    }

    return EGS_RunControl::startSimulation();
}

void EGS_UniformRunControl::describeRCO() {

    EGS_RunControl::describeRCO();

    if (watcher_job) {
        if (check_egsdat) {
            egsInformation(
                "   Watcher job: remains running after completion checking\n"
                "                for other jobs finishing every %d s for %d s!\n",
                milliseconds/1000, check_intervals*milliseconds/1000);
        }
        else {
            egsInformation(
                "   Option to check for finishing jobs is OFF!\n\n");
        }
    }

}

#ifdef WIN32

    #include <io.h>
    #include <stdio.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/locking.h>
    #include <windows.h>

    #define OPEN_FILE _open
    #define CLOSE_FILE _close
    #define CREATE_FLAGS _O_CREAT | _O_EXCL | _O_RDWR, _S_IREAD | _S_IWRITE
    #define OPEN_FLAGS _O_RDWR,_S_IREAD | _S_IWRITE
    #define WAIT_FOR_FILE _sleep(1000)
    #define WRITE_FILE _write
    #define READ_FILE _read

#else

    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <errno.h>
    #include <string.h>
    #include <stdio.h>

    #define OPEN_FILE open
    #define CLOSE_FILE close
    #define CREATE_FLAGS O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR
    #define OPEN_FLAGS O_RDWR
    #define WAIT_FOR_FILE sleep(1)
    #define WRITE_FILE write
    #define READ_FILE read

#endif

#ifndef SKIP_DOXYGEN

/*!  \brief Class implementing file locking

  \internwarning
*/
class EGS_LOCAL EGS_FileLocking {
public:
    int  fd;
    bool is_locked;
    int  ntry;
#ifndef WIN32
    struct flock fl_write, fl_unlock;
#endif
    EGS_FileLocking() : fd(-1), is_locked(false), ntry(15) {
#ifndef WIN32
        fl_write.l_type = F_WRLCK;
        fl_write.l_whence = SEEK_SET;
        fl_write.l_start = 0;
        fl_write.l_len = 0;
        fl_unlock.l_type = F_UNLCK;
        fl_unlock.l_whence = SEEK_SET;
        fl_unlock.l_start = 0;
        fl_unlock.l_len = 0;
#endif
    };
    ~EGS_FileLocking() {
        if (fd > 0) {
            CLOSE_FILE(fd);
        }
    };
    bool createControlFile(const char *fname) {
        is_locked = false;
        if (fd > 0) {
            CLOSE_FILE(fd);
        }
        fd = OPEN_FILE(fname,CREATE_FLAGS);
        egsWarning("createControlFile: file=%s fd=%d\n",fname,fd);
        if (fd < 0) {
            egsWarning("createControlFile(): open failed! (fd=%d)\n",fd);
#ifndef WIN32
            perror("System error was");
#endif
        }
        return lockControlFile();
    };
    bool openControlFile(const char *fname) {
        is_locked = false;
        if (fd > 0) {
            CLOSE_FILE(fd);
        }
        for (int t=0; t<ntry; t++) {
            fd = OPEN_FILE(fname,OPEN_FLAGS);
            if (fd > 0) {
                break;
            }
            WAIT_FOR_FILE;
        }
        return (fd > 0);
    };
    bool closeControlFile() {
        if (fd > 0) {
            int res = CLOSE_FILE(fd);
            fd = -1;
            return !res;
        }
        return true;
    };
    bool lockControlFile() {
        if (is_locked) {
            return true;
        }
        if (fd < 0) {
            return false;
        }
#ifdef WIN32
        long np = _lseek(fd,0L,SEEK_SET);
        if (np) {
            egsWarning("lockControlFile: _lseek returned %d?\n",np);
            return false;
        }
        int res = _locking(fd,_LK_LOCK,1000000L);
        if (!res) {
            is_locked = true;
            return true;
        }
        return false;
#else
        for (int i1=0; i1<5; i1++) {
            for (int i2=0; i2<12; i2++) {
                int res = fcntl(fd,F_SETLK,&fl_write);
                if (!res) {
                    is_locked = true;
                    return true;
                }
                WAIT_FOR_FILE ;
            }
            egsWarning("lockControlFile: failed to lock file for "
                       "12 seconds...\n");
        }
        return false;
#endif
    };
    bool unlockControlFile() {
        if (!is_locked) {
            return true;
        }
        if (fd < 0) {
            return false;
        }
#ifdef WIN32
        int np = _lseek(fd,0L,SEEK_SET);
        if (np) {
            egsWarning("unlockControlFile: _lseek returned %d?\n",np);
            return false;
        }
        int res = _locking(fd,_LK_UNLCK,1000000L);
#else
        int res = fcntl(fd,F_SETLKW,&fl_unlock);
#endif
        if (!res) {
            is_locked = false;
            return true;
        }
        return false;
    };
    bool rewindControlFile() {
        if (fd < 0) {
            return false;
        }
        if (!is_locked) {
            if (!lockControlFile()) {
                return false;
            }
        }
#ifdef WIN32
        return !_lseek(fd,0,SEEK_SET);
#else
        return !lseek(fd,0,SEEK_SET);
#endif
    };
};

#endif

EGS_JCFControl::EGS_JCFControl(EGS_Application *a, int Nbuf) :
    EGS_RunControl(a), tsum(0), tsum2(0), tcount(0), norm(1), last_sum(0),
    last_sum2(0), last_count(0), njob(0), npar(app->getNparallel()),
    ipar(app->getIparallel()), ifirst(app->getFirstParallel()),
    first_time(true), removed_jcf(false), nbuf(Nbuf), p(new EGS_FileLocking) {

    rco_type = balanced;

    /* Recover initial number of histories */
    if (npar) {
        ncase *= npar;
    }

    if (input) {
        int err = input->getInput("nchunk",nchunk);
        if (err) {
            nchunk = 10;
        }
    }
    else {
        nchunk = 10;
    }
    if (nbuf < 0) {
        nbuf = 1024;
    }
    buf = new char [nbuf];
    nleft = ncase;
    ntot = 0;
    //egsInformation("EGS_JCFControl::EGS_JCFControl:\n");
    //egsInformation("  ncase = %lld nleft = %lld nchunk = %d\n",
    //        nleft,ncase,nchunk);
}

bool EGS_JCFControl::createControlFile() {
    string cfile = egsJoinPath(app->getAppDir(),app->getFinalOutputFile());
    cfile += ".lock";
    if (!p->createControlFile(cfile.c_str())) {
        egsWarning("EGS_JCFControl: failed to create or lock the "
                   " job control file %s\n\n",cfile.c_str());
        return false;
    }
    if (p->fd < 0) {
        return false;
    }
    writeControlString();
    int nwant = strlen(buf)+1;
    int nwrite = WRITE_FILE(p->fd,buf,nwant);
    if (nwrite != nwant) {
        return false;
    }
    return p->unlockControlFile();
}

bool EGS_JCFControl::openControlFile() {
    string cfile = egsJoinPath(app->getAppDir(),app->getFinalOutputFile());
    cfile += ".lock";
    if (!p->openControlFile(cfile.c_str())) {
        egsWarning("EGS_JCFControl: failed to open the "
                   " job control file %s\n\n",cfile.c_str());
        return false;
    }
    return true;
}

#ifdef NO_SSTREAM
    #include <strstream>
    #define MY_OSTREAM std::ostrstream
    #define MY_ISTREAM std::istrstream
#else
    #include <sstream>
    #define MY_OSTREAM std::ostringstream
    #define MY_ISTREAM std::istringstream
#endif

bool EGS_JCFControl::writeControlString() {
    //if( first_time ) { start_time = time(0); first_time = false; }
    if (first_time) {
        start_time = time(0);
    }
    /*
    MY_OSTREAM data(buf);
    //ostream &data = cout;
    if( !egsStoreI64(data,ntot) ) return false;
    if( !egsStoreI64(data,nleft) ) return false;
    data << " " << njob << " " << tsum << " " << tsum2 << " " << tcount << " ";
    double f = tsum*norm, df;
    if( tsum > 0 && tsum2 > 0 && norm > 0 && tcount > 1 ) {
        df = tcount*tsum2/(tsum*tsum)-1;
        if( df > 0 ) df = 100*sqrt(df/(tcount-1)); else df = 100;
    } else df = 100;
    data << f << " " << df << " " << start_time << endl;
    if( f > 0 && df < 100 ) egsInformation("\nCombined result from all "
          "parallel jobs: %g +/- %g%%\n\n",f,df);
    egsInformation("EGS_JCFControl::writeControlString: <%s>\n",buf);
    return data.good();
    */
    double f = tsum*norm, df;
    if (tsum > 0 && tsum2 > 0 && norm > 0 && tcount > 1) {
        f = tsum*norm/tcount;
        df = tcount*tsum2/(tsum*tsum)-1;
        if (df > 0) {
            df = 100*sqrt(df/(tcount-1));
        }
        else {
            df = 100;
        }
    }
    else {
        df = 100;
    }
    sprintf(buf,"%lld %lld %d %lg %lg %lg %lg %lg %ld ",ntot,nleft,njob,tsum,
            tsum2,tcount,f,df,start_time);
    return true;
}

bool EGS_JCFControl::getCombinedResult(double &f, double &df) const {
    if (tsum > 0 && tsum2 > 0 && norm > 0 && tcount > 1) {
        f = tsum*norm/tcount;
        df = tcount*tsum2/(tsum*tsum)-1;
        if (df > 0) {
            df = 100*sqrt(df/(tcount-1));
        }
        else {
            df = 100;
        }
        return true;
    }
    df = 100;
    f = 0;
    return false;
}

bool EGS_JCFControl::readControlString() {
    /*
    MY_ISTREAM data(buf);
    if( !egsGetI64(data,ntot) ) return false;
    if( !egsGetI64(data,nleft) ) return false;
    double f,df;
    data >> njob >> tsum >> tsum2 >> tcount >> f >> df >> start_time;
    return data.good();
    */
    double f,df;
    int res = sscanf(buf,"%lld %lld %d %lg %lg %lg %lg %lg %ld",
                     &ntot,&nleft,&njob,&tsum,&tsum2,&tcount,&f,&df,&start_time);
    if (res == EOF || res != 9) {
        return false;
    }
    return true;
}

int EGS_JCFControl::startSimulation() {
    int res = EGS_RunControl::startSimulation();
    if (res) {
        return res;
    }
    bool ok = (ipar == ifirst) ? createControlFile() : openControlFile();
    if (ok) {
        egsInformation("    Parallel run with %d jobs and %d chunks per "
                       "job\n\n\n",npar,nchunk);
        return 0;
    }
    return -99;
}

bool EGS_JCFControl::readControlFile() {
    if (!p->rewindControlFile()) {
        egsWarning("EGS_JCFControl: failed to rewind the job control file\n");
        return false;
    }
    int res = READ_FILE(p->fd,buf,nbuf-1);
    if (res <= 0) {
        p->unlockControlFile();
        egsWarning("EGS_JCFControl: failed to read the job control file\n");
        return false;
    }
    buf[res] = 0;
    if (!readControlString()) {
        p->unlockControlFile();
        egsWarning("EGS_JCFControl: failed to read from the control string"
                   " <%s>\n",buf);
        return false;
    }
    return true;
}

bool EGS_JCFControl::writeControlFile() {
    if (!writeControlString()) {
        egsWarning("EGS_JCFControl::writeControlFile: failed to write to the "
                   "control string\n");
        return false;
    }
    if (!p->rewindControlFile()) {
        egsWarning("EGS_JCFControl: failed to rewind the job control file\n");
        return false;
    }
    int nwant = strlen(buf)+1;
    int nwrite = WRITE_FILE(p->fd,buf,nwant);
    if (!p->unlockControlFile()) {
        egsWarning("EGS_JCFControl::writeControlFile: failed to unlock the "
                   "control file\n");
        return false;
    }
    if (nwrite != nwant) {
        egsWarning("EGS_JCFControl::getNextChunk: could write only %d "
                   "instead of %d chars to the job control file?\n",nwrite,nwant);
        return false;
    }
    return true;
}

EGS_I64 EGS_JCFControl::getNextChunk() {
    if (!readControlFile()) {
        return -1;
    }
    if (first_time) {
        first_time = false;
        njob++;
    }
    double sum, sum2, count;
    app->getCurrentResult(sum,sum2,norm,count);
    tsum += sum - last_sum;
    tsum2 += sum2 - last_sum2;
    tcount += count - last_count;
    last_sum = sum;
    last_sum2 = sum2;
    last_count = count;
    EGS_I64 nrun = ncase/(npar*nchunk);
    if (nrun < 1) {
        nrun = 1;
    }
    if (nrun > nleft) {
        nrun = nleft;
    }
    if (nrun > 0) {
        app->setSimulationChunk(ntot,nrun);
    }
    nleft -= nrun;
    ntot += nrun;
    writeControlFile();
    double f,df;
    if (accu > 0 && getCombinedResult(f,df)) {
        if (df < 100 && df < accu) {
            char c = '%';
            egsWarning("\n\n*** After combining the results of all parallel "
                       "jobs the requested\n    uncertainty of %g%c was reached: %g%c\n"
                       "    => terminating simulation.\n\n",accu,c,df,c);
            return 0;
        }
    }
    return nrun;
}

/*! \brief Suspend execution for a given time (in ms)

 Called from the uniform RCO to wait for all jobs to
 finish. Time is set by default to 1s, but user can
 change it using the input key

 interval wait time = time in ms

 in the run control input block.


void rco_sleep(const int& mscnds);
*/
void rco_sleep(const int &mscnds) {
#ifdef WIN32
    Sleep(mscnds);
#else
    usleep(mscnds * 1000);
#endif
}

int EGS_RunControl::finishSimulation() {
    cpu_time = timer.time();
    egsInformation("\n\nFinished simulation\n\n");
    egsInformation("%-40s%.2f (sec.) %.4f(hours)\n",
                   "Total cpu time for this run:",cpu_time,cpu_time/3600);
    //egsInformation("Total cpu time for this run: %g seconds (%g hours)\n\n",
    //        cpu_time, cpu_time/3600);
    if (previous_cpu_time > 0)
        egsInformation("%-40s%.2f (sec.) %.4f (hours)\n",
                       "CPU time including previous runs:",cpu_time+previous_cpu_time,
                       (cpu_time+previous_cpu_time)/3600);
    egsInformation("%-40s%-14g\n","Histories per hour:",3600.*ndone/
                   (cpu_time+previous_cpu_time));
    egsInformation("%-40s%-14lld\n","Number of random numbers used:",
                   app->randomNumbersUsed());
    double ch_steps, all_steps;
    app->getElectronSteps(ch_steps,all_steps);
    egsInformation("%-40s%-14g\n","Number of electron CH steps:",
                   ch_steps);
    //egsInformation("%-40s%14g\n","Number of all electron steps:",
    //        all_steps);
    egsInformation("%-40s","Number of all electron steps:");
    egsInformation("%-14g\n",all_steps);

    int n_par   = app->getNparallel(),
        i_par   = app->getIparallel(),
        i_first = app->getFirstParallel();
    /* If parallel run and last job, trigger the app combineResults method */
    return (n_par && i_par == i_first + n_par - 1) ? 1 : 0;
}

int EGS_UniformRunControl::finishSimulation() {
    int err = EGS_RunControl::finishSimulation();
    if (err < 0) {
        return err;
    }
    /* Check and wait for all jobs to finish */
    if (watcher_job) {
        int interval = 0, njobs_done = 0, njobs_done_old= 0;
        while (interval < check_intervals) {
            rco_sleep(milliseconds);
            if (check_egsdat) {
                njobs_done = app->howManyJobsDone();
                //egsInformation("\n-> Finished %d jobs...\n",njobs_done);
                if (njobs_done == npar - 1) {
                    watcher_job=false;//don't enter this after all jobs done!
                    break;
                }
                // Only combine if new jobs finished
                if (njobs_done_old < njobs_done) {
                    egsInformation("=> Combining %d jobs ...\n",njobs_done);
                    app->combinePartialResults();
                }
                njobs_done_old = njobs_done;
            }
            interval++;
        }
        return 1;
    }
    /*I am not a watcher job, do not combine results yet!*/
    return 0;
}

int EGS_JCFControl::finishSimulation() {
    int err = EGS_RunControl::finishSimulation();
    if (err < 0) {
        return err;
    }
    if (removed_jcf) {
        return 0;
    }
    if (!readControlFile()) {
        return -2;
    }
    njob--;
    writeControlFile();
    p->closeControlFile();
    if (njob > 0 || removed_jcf) {
        return 0;
    }
    string cfile = egsJoinPath(app->getAppDir(),app->getFinalOutputFile());
    cfile += ".lock";
#ifdef WIN32
    int res = _unlink(cfile.c_str());
#else
    int res = unlink(cfile.c_str());
#endif
    if (res) egsWarning("EGS_JCFControl::finishSimulation: failed to remove "
                            " the job control file %s\n",cfile.c_str());
    removed_jcf = true;
    return 1;
}

EGS_JCFControl::~EGS_JCFControl() {
    delete p;
}

bool EGS_JCFControl::closeControlFile() {
    return p->closeControlFile();
}

bool EGS_JCFControl::lockControlFile() {
    return p->lockControlFile();
}

bool EGS_JCFControl::unlockControlFile() {
    return p->unlockControlFile();
}

bool EGS_JCFControl::rewindControlFile() {
    return p->rewindControlFile();
}

typedef EGS_RunControl *(*EGS_RunControlCreationFunction)(EGS_Application *);

EGS_RunControl *EGS_RunControl::getRunControlObject(EGS_Application *a) {
    if (!a) {
        egsWarning("EGS_RunControl::getRunControlObject(): "
                   "null application?\n");
        return 0;
    }
    EGS_Input *inp = a->getInput();
    EGS_Input *irc = 0;
    if (inp) {
        irc = inp->getInputItem("run control");
    }
    /* If no input file, defaults to simple RCO for single runs and
       to JCF RCO for parallel runs.
    */
    if (!irc) {
        /*
        egsWarning("EGS_RunControl::getRunControlObject(): "
                "the application does not have any input\n");
        return 0;
        */
        if (a->getNparallel() > 0) {
            return new EGS_JCFControl(a);
        }
        else {
            return new EGS_RunControl(a);
        }
    }
    /*
    EGS_Input *irc = inp->getInputItem("run control");
    if( !irc ) {
        egsWarning("EGS_RunControl::getRunControlObject(): "
                "the application input has no 'run control' item\n");
        return 0;
    }
    */
    string libname;
    int err = irc->getInput("library",libname);
    EGS_RunControl *result;
    if (!err) {
        EGS_Library *lib = 0;
        for (unsigned int j=0; j<rc_libs.size(); j++) {
            if (libname == rc_libs[j]->libraryName()) {
                lib = rc_libs[j];
                break;
            }
        }
        if (!lib) {
            string dsodir = egsJoinPath("egs++","dso");
            dsodir = egsJoinPath(dsodir,CONFIG_NAME);
            dsodir = egsJoinPath(a->getHenHouse(),dsodir);
            lib = new EGS_Library(libname.c_str(),dsodir.c_str());
            lib->load();
            if (!lib->isLoaded()) {
                egsWarning("EGS_RunControl::getRunControlObject: failed to"
                           " load the library %s from %s\n",libname.c_str(),
                           dsodir.c_str());
                delete irc;
                return 0;
            }
            rc_libs.push_back(lib);
        }
        EGS_RunControlCreationFunction create =
            (EGS_RunControlCreationFunction) lib->resolve("createRunControl");
        if (!create) {
            egsWarning("EGS_RunControl::getRunControlObject: failed to"
                       " resolve the run control creation function of library %s\n",
                       libname.c_str());
            result = 0;
        }
        else {
            result = create(a);
        }
    }
    else {
        if (a->getNparallel() > 0) {
            vector<string> allowed_types;
            allowed_types.push_back("simple");
            allowed_types.push_back("uniform");
            allowed_types.push_back("balanced");
            int rco_t = irc->getInput("rco type",allowed_types,2);
            switch (rco_t) {
            case 0:
                result = new EGS_RunControl(a);
                break;
            case 1:
                result = new EGS_UniformRunControl(a);
                break;
            case 2:
                result = new EGS_JCFControl(a);
                break;
            default:
                result = new EGS_JCFControl(a);
            }
        }
        else {
            result = new EGS_RunControl(a);
        }
    }
    delete irc;
    return result;
}
