/*
###############################################################################
#
#  EGSnrc egs++ phase-space source
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
#                   Reid Townson
#                   Hannah Gallop
#
###############################################################################
*/


/*! \file egs_phsp_source.cpp
 *  \brief A phase-space file source
 *  \IK
 */

#include "egs_phsp_source.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_application.h"

static bool EGS_PHSP_SOURCE_LOCAL inputSet = false;

EGS_PhspSource::EGS_PhspSource(const string &phsp_file,
                               const string &Name, EGS_ObjectFactory *f) : EGS_BaseSource(Name,f) {
    init();
    openFile(phsp_file);
}

void EGS_PhspSource::init() {
    otype = "EGS_PhspSource";
    recl = 0;
    Xmin = -veryFar;
    Xmax = veryFar;
    Ymin = -veryFar;
    Ymax = veryFar;
    is_valid = false;
    record = 0;
    mode2 = false;
    swap_bytes = false;
    the_file_name = "no file";
    filter_type = -1;
    particle_type = 2;
    description = "Invalid phase space source";
    Nread = 0;
    count = 0;
    Nrestart = 0;
    Npos = 0;
    Nlast = 0;
    wmin = -veryFar;
    wmax = veryFar;
    Nrecycle_g = 0;
    Nrecycle_e = 0;
    Nrecycle = 0;
    Nuse = -1;
    first = true;
}

void EGS_PhspSource::openFile(const string &phsp_file) {
    if (the_file.is_open()) {
        the_file.close();
    }
    the_file_name = "no file";
    is_valid = false;
    recl = 0;
    if (record) {
        delete [] record;
        record = 0;
    }
    the_file.open(phsp_file.c_str(),ios::binary | ios::in);
    if (!the_file.is_open()) {
        egsWarning("EGS_PhspSource::openFile: failed to open binary file %s"
                   " for reading\n",phsp_file.c_str());
        return;
    }
    string cmode;
    char auxc;
    for (int i=0; i<5; i++) {
        the_file.get(auxc);
        if (the_file.eof() || !the_file.good()) {
            egsWarning("EGS_PhspSource::openFile: an I/O error occured "
                       "while reading the first record of %s\n",phsp_file.c_str());
            return;
        }
        cmode += auxc;
    }
    if (cmode == "MODE0") {
        mode2 = false;
        recl = 28;
    }
    else if (cmode == "MODE2") {
        mode2 = true;
        recl = 32;
    }
    else {
        egsWarning("EGS_PhspSource::openFile: the file %s is not a MODE0 or"
                   " MODE2 file\n",phsp_file.c_str());
        return;
    }
    record = new char [recl];
    int n, n_photon;
    float emax, emin, pinc;
    the_file.read((char *) &n, sizeof(int));
    the_file.read((char *) &n_photon, sizeof(int));
    the_file.read((char *) &emax, sizeof(float));
    the_file.read((char *) &emin, sizeof(float));
    the_file.read((char *) &pinc, sizeof(float));
    if (the_file.eof() || !the_file.good()) {
        egsWarning("EGS_PhspSource::openFile: an I/O error occured "
                   "while reading the first record of %s\n",phsp_file.c_str());
        recl = 0;
        return;
    }
    swap_bytes = false;
    if (n <= 0 || n_photon < 0 || n_photon > n || emin < 0 || emax < 0 ||
            emax < emin || pinc < 0) {
        swap_bytes = true;
        egsSwapBytes(&n);
        egsSwapBytes(&n_photon);
        egsSwapBytes(&emin);
        egsSwapBytes(&emax);
        egsSwapBytes(&pinc);
        if (n <= 0 || n_photon < 0 || n_photon > n || emin < 0 || emax < 0 ||
                emax < emin || pinc < 0) {
            egsWarning("EGS_PhspSource::openFile: phase space file header"
                       " contains meaningless values with and without byte swaping:\n");
            if (n <= 0) {
                egsWarning("  number of particles: %d\n",n);
            }
            if (n_photon < 0) {
                egsWarning("  number of photons: %d\n",n_photon);
            }
            if (n_photon > n) egsWarning("  number of photons (%d) is "
                                             "greater than number of particles (%d)\n",n_photon,n);
            if (emin < 0) {
                egsWarning("  minimum energy: %g\n",emin);
            }
            if (emax < 0) {
                egsWarning("  maximum energy: %g\n",emax);
            }
            if (emin > emax) {
                egsWarning("  emin > emax: %g %g\n",emin,emax);
            }
            if (pinc < 0) {
                egsWarning("  incident particles: %g\n",pinc);
            }
            recl = 0;
            return;
        }
    }
    // at this points we have passed a set of checks and think that
    // we have some meaningful information about number of particles, etc.
    // to be completely sure, it is a good idea to read the last particle
    // in the file and check for errors.
    istream::off_type nend = n;
    nend = nend*recl;
    //the_file.seekg(recl*n,ios::beg);
    the_file.seekg(nend,ios::beg);
    the_file.read(record,recl*sizeof(char));
    if (the_file.bad() || the_file.fail()) {
        egsWarning("EGS_PhspSource::openFile: failed to read the last"
                   " particle in the file, this indicates some unknown error condition\n");
        recl = 0;
        return;
    }
    the_file.clear();
    the_file.seekg(recl,ios::beg);
    Npos = 0;
    Nlast = n;
    Nfirst = 1;
    // at this point the position should be at the first particle in the file
    Emax = emax;
    Emin = emin;
    Pinc = pinc;
    Nparticle = n;
    Nphoton = n_photon;
    is_valid = true;
    the_file_name = phsp_file;
}

EGS_PhspSource::EGS_PhspSource(EGS_Input *input, EGS_ObjectFactory *f) :
    EGS_BaseSource(input,f) {
    init();
    string fname;
    int err = input->getInput("phase space file",fname);
    if (err) {
        egsWarning("EGS_PhspSource: no 'phase space file' input\n");
        return;
    }
    openFile(fname);
    if (!isValid()) {
        egsWarning("EGS_PhspSource: errors while opening the phase space file"
                   " %s\n",fname.c_str());
        return;
    }
    vector<EGS_Float> cutout;
    err = input->getInput("cutout",cutout);
    if (!err && cutout.size() == 4) {
        setCutout(cutout[0],cutout[1],cutout[2],cutout[3]);
    }
    vector<string> ptype;
    ptype.push_back("electrons");
    ptype.push_back("photons");
    ptype.push_back("positrons");
    ptype.push_back("all");
    ptype.push_back("charged");
    particle_type = input->getInput("particle type",ptype,3)-1;
    vector<int> the_filter;
    vector<string> the_latch;
    err = input->getInput("filter type",the_filter);
    int err1 = input->getInput("latch setting",the_latch);
    if (!err && !err1) {
        if (the_filter[0] >= 0 && the_filter[0] <= 3) {
            int nbit1 = the_latch[0].size();
            int nbit2 = 0;
            if (!the_filter[0]) {
                nbit2 = the_latch[1].size();
            }
            if (nbit1 + nbit2 > 0) {
                int *the_bits = new int [nbit1+nbit2];
                for (int j=0; j<nbit1+nbit2; j++) {
                    if (j < nbit1) {
                        the_bits[j]=(int)(the_latch[0][j] == '1');
                    }
                    else {
                        the_bits[j] = (int)(the_latch[1][j-nbit1] == '1');
                    }
                }
                setFilter(the_filter[0],nbit1,nbit2,the_bits);
                delete [] the_bits;
            }
        }
    }
    vector<EGS_Float> wwindow;
    err = input->getInput("weight window",wwindow);
    if (!err && wwindow.size() == 2) {
        wmin = wwindow[0];
        wmax = wwindow[1];
    }
    int ntmp;
    err = input->getInput("reuse photons",ntmp);
    if (!err && ntmp > 0) {
        Nrecycle_g = ntmp;
    }
    else {
        err = input->getInput("recycle photons",ntmp);
        if (!err && ntmp > 0) {
            Nrecycle_g = ntmp;
        }
    }
    err = input->getInput("reuse electrons",ntmp);
    if (!err && ntmp > 0) {
        Nrecycle_e = ntmp;
    }
    else {
        err = input->getInput("recycle electrons",ntmp);
        if (!err && ntmp > 0) {
            Nrecycle_e = ntmp;
        }
    }
    description = "Phase space source from ";
    description += the_file_name;
}

EGS_I64 EGS_PhspSource::getNextParticle(EGS_RandomGenerator *, int &q,
                                        int &latch, EGS_Float &E, EGS_Float &wt, EGS_Vector &x, EGS_Vector &u) {
    if (!recl) egsFatal("EGS_PhspSource::readParticle(): the file is not "
                            "open yet\n");
    /*
    if( Nuse >= Nrecycle ) {
        do { readParticle(); } while ( rejectParticle() );
    }
    */
    if (Nuse > Nrecycle || Nuse < 0) {
        readParticle();
    }
    x.x = p.x;
    x.y = p.y;
    x.z = 0;
    u.x = p.u;
    u.y = p.v;
    EGS_Float aux = p.u*p.u+p.v*p.v;
    if (aux < 1) {
        aux = sqrt(1-aux);
    }
    else {
        aux = 0;
    }
    if (p.wt > 0) {
        u.z = aux;
        wt = p.wt;
    }
    else {
        u.z = -aux;
        wt = -p.wt;
    }
    if (rejectParticle()) {
        wt = 0;
    }
    E = p.E;
    q = p.q;
    latch = 0; //latch = p.latch;
    ++Nuse;
    return count;
}

#ifndef SKIP_DOXYGEN
/*!  \brief A union used in byte swaping

  \interwarning
*/
union __egs_data32 {
    int   i;
    float f;
    char  c[4];
};
#endif

void EGS_PhspSource::setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun, int npar, int nchunk) {
    //determine the simulation chunk and use this to calculate first/last particles
    //in the phase space chunk
    EGS_I64 particlesPerChunk = Nparticle/(npar*nchunk);
    int ichunk = nstart/nrun;
    if (ichunk > npar*nchunk-1) {
        //remainder of histories, reuse the last chunk of the phsp
        //there may be a more clever strategy
        egsWarning("EGS_PhspSource: Remainder of histories will reuse the last chunk of the phase space source.\n");
        ichunk = npar*nchunk-1;
    }
    Nfirst = ichunk*particlesPerChunk+1;
    if (ichunk == npar*nchunk-1) {
        //last chunk of the phsp file, just go to the end of the file
        Nlast = Nparticle;
    }
    else {
        Nlast = Nfirst-1+particlesPerChunk;
    }
    Npos = Nfirst-1; //we increment Npos before attempting to read a particle
    istream::off_type pos = Nfirst*recl;
    the_file.seekg(pos,ios::beg);
    egsInformation("EGS_PhspSource: using phsp portion between %lld and %lld\n",
                   Nfirst,Nlast);
}

void EGS_PhspSource::readParticle() {
    if ((++Npos) > Nlast) {
        egsWarning("EGS_PhspSource::readParticle(): reached the end of the "
                   "phase space file chunk (%lld)\n  will start from the beginning "
                   "of the chunk (%lld) but this "
                   "implies that uncertainty estimates will be inaccurate\n",
                   Nlast,Nfirst);
        istream::off_type pos = Nfirst*recl;
        the_file.seekg(pos,ios::beg);
        Nrestart++;
        Npos = Nfirst;
    }
    the_file.read(record,recl*sizeof(char));
    ++Nread;
    if (the_file.eof() || !the_file.good())
        egsFatal("EGS_PhspSource::readParticle(): I/O error while reading "
                 "phase space file\n");
    __egs_data32 *data;
    data = (__egs_data32 *) &record[0];
    p.latch = data->i;
    data = (__egs_data32 *) &record[4];
    p.E = data->f;
    data = (__egs_data32 *) &record[8];
    p.x = data->f;
    data = (__egs_data32 *) &record[12];
    p.y = data->f;
    data = (__egs_data32 *) &record[16];
    p.u = data->f;
    data = (__egs_data32 *) &record[20];
    p.v = data->f;
    data = (__egs_data32 *) &record[24];
    p.wt = data->f;
    if (swap_bytes) {
        egsSwapBytes(&p.latch);
        egsSwapBytes(&p.E);
        egsSwapBytes(&p.wt);
        egsSwapBytes(&p.x);
        egsSwapBytes(&p.y);
        egsSwapBytes(&p.u);
        egsSwapBytes(&p.v);
    }
    if (p.latch & 1073741824) {
        p.q = -1;
    }
    else if (p.latch & 536870912) {
        p.q = 1;
    }
    else {
        p.q = 0;
    }
    if (p.E < 0) {
        count++;
        p.E = -p.E;
    }
    else if (first) {
        ++count;
    }
    first = false;
    if (p.q) {
        p.E -= EGS_Application::activeApplication()->getRM();
        Nrecycle = Nrecycle_e;
    }
    else {
        Nrecycle = Nrecycle_g;
    }
    Nuse = 0;
    p.wt /= (Nrecycle+1);
}

bool EGS_PhspSource::rejectParticle() const {
    if (particle_type < 2 && p.q != particle_type) {
        return true;
    }
    if (particle_type == 3 && !p.q) {
        return true;
    }
    if (p.x < Xmin || p.x > Xmax || p.y < Ymin || p.y > Ymax) {
        return true;
    }
    if (p.wt < wmin || p.wt > wmax) {
        return true;
    }
    if (p.latch & 2147483648UL) {
        return true;
    }
    if (filter_type < 0) {
        return false;
    }
    if (filter_type == 0) {
        bool r1;
        if (filter1) {
            r1 = !(p.latch & filter1);
        }
        else {
            r1 = false;
        }
        bool r2 = (p.latch & filter2);
        return (r1 || r2);
    }
    if (filter_type == 1) {
        return (p.latch & filter1);
    }
    int l = p.latch >> 24;
    bool res = l & filter1;
    return filter_type == 3 ? res : !res;
}

void EGS_PhspSource::setFilter(int type, int nbit1, int nbit2, const int *bits) {
    if (type < 0 || type > 3) {
        egsWarning("EGS_PhspSource::setFilter: invalid filter type %d\n",type);
        return;
    }
    int ntot = nbit1;
    if (type == 0) {
        ntot += nbit2;
    }
    if (ntot > 29) {
        egsWarning("EGS_PhspSource::setFilter: maximum number of bits is "
                   "limited to 29, you requested %d\n",ntot);
        return;
    }
    if (!ntot) {
        return;
    }
    filter_type = type;
    if (filter_type == 0) {
        int i;
        filter1 = 0;
        filter2 = 0;
        for (i=0; i<ntot; i++) {
            if (bits[i]) {
                unsigned int aux = 1;
                aux = (aux << i);
                if (i < nbit1) {
                    filter1 += aux;
                }
                else {
                    filter2 += aux;
                }
            }
        }
    }
    else if (filter_type == 1) {  // TODO
    }
    else if (filter_type == 2) {  // TODO
    }
    else if (filter_type == 3) {  // TODO
    }
}

extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseSourceInputs(false, false);

        srcBlockInput->getSingleInput("library")->setValues({"EGS_Phsp_Source"});

        // Format: name, isRequired, description, vector string of allowed values
        srcBlockInput->addSingleInput("phase space file", true, "The name of the phase space file.");
        srcBlockInput->addSingleInput("particle type", true, "The type of particle", {"all", "charged", "electrons", "positrons", "photons"});
        srcBlockInput->addSingleInput("cutout", false, "A rectagular cutout defined by x1, x2, y1, y2");
        srcBlockInput->addSingleInput("weight window", false, "wmin, wmax, the min and max particle weights to use. If the particle is not in this range, it is rejected.");
        srcBlockInput->addSingleInput("recyle photons", false, "The number of time to recycle each photon");
        srcBlockInput->addSingleInput("recycle electrons", false, "The number of times to recycle each electron");
    }

    EGS_PHSP_SOURCE_EXPORT string getExample() {
        string example;
        example =
{R"(
    # Example of egs_phsp_soure
    #:start source:
        name = my_source
        library = egs_phsp_source
        phase space file = ../BEAM_EX16MVp/EX16MVp.egsphsp1
        particle type = all
        cutout = -1 1 -2 2
        recycle photons = 10
        recycle electrons = 10
    :stop source:
)"};
        return example;
    }

    EGS_PHSP_SOURCE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return srcBlockInput;
    }

    EGS_PHSP_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_PhspSource>(input,f,"phsp source");
    }

}
