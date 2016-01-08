/*
###############################################################################
#
#  EGSnrc egs++ IAEA format phase-space source
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


/*! \file iaea_phsp_source.cpp
 *  \brief An IAEA phase-space file source
 *  \BW
 */

#include "iaea_phsp.h"
#include "iaea_phsp_source.h"
#include "egs_input.h"
#include "egs_functions.h"

IAEA_PhspSource::IAEA_PhspSource(const string &phsp_file,
                                 const string &Name, EGS_ObjectFactory *f) : EGS_BaseSource(Name,f) {
    init();
    openFile(phsp_file);
}

void IAEA_PhspSource::init() {
    otype = "IAEA_PhspSource";
    Xmin = -1e30;
    Xmax = 1e30;
    Ymin = -1e30;
    Ymax = 1e30;
    is_valid = false;
    mode2 = false;
    swap_bytes = false;
    the_file_name = "no file";
    filter_type = -1;
    particle_type = 2;
    description = "Invalid IAEA phase space source";
    Nread = 0;
    count = 0;
    Nrecycle = 0;
    Npos = 0;
    Nlast = 0;
    wmin = -1e30;
    wmax = 1e30;
    Nreuse_g = 1;
    Nreuse_e = 1;
    Nreuse = 1;
    Nuse = 2;
    first = true;
}

void IAEA_PhspSource::openFile(const string &phsp_file) {
    the_file_name = "no file";
    is_valid = false;
    int rwmode=1; //read only
    iaea_fileid=0; //this is just an array index for the iaea routines, not an actual
    //unit no., as in Fortran, so it can really be any integer value
    //not above MAX_NUM_SOURCES (30)

    //below is required because iaea opening routine gets file name as char array
    int len = phsp_file.length();
    char phsp_name_tmp[len+1];
    phsp_file.copy(phsp_name_tmp,len,0);
    //now put a null character on the end
    phsp_name_tmp[len]='\0';

    //use iaea function for opening phase space file
    iaea_new_source(&iaea_fileid,phsp_name_tmp,&rwmode,&iaea_iostat,len);
    if (iaea_iostat==105) {
        egsWarning("IAEA_PhspSource::openFile: no header file name supplied.\n");
        return;
    }
    else if (iaea_iostat==-96) {
        egsWarning("IAEA_PhspSource::openFile: failed to open header file %s.IAEAheader\n"
                   " for reading\n",phsp_file.c_str());
        return;
    }
    else if (iaea_iostat==-94) {
        egsWarning("IAEA_PhspSource::openFile: failed to open phase space file %s.IAEAphsp\n"
                   " for reading\n",phsp_file.c_str());
        return;
    }
    else if (iaea_iostat==-1) {
        egsWarning("IAEA_PhspSource::openFile: failed to initialize phase space source %s\n",phsp_file.c_str());
        return;
    }
    else if (iaea_iostat==-91) {
        egsWarning("IAEA_PhspSource::openFile: failed to get record contents from header file %s.IAEAheader\n",phsp_file.c_str());
        return;
    }
    else if (iaea_iostat <0) {
        egsWarning("IAEA_PhspSource::openFile: I/O error ocurred on opening phase space file %s\n",phsp_file.c_str());
        return;
    }

    //now check file size and byte order

    swap_bytes = false;
    iaea_check_file_size_byte_order(&iaea_fileid,&iaea_iostat);
    if (iaea_iostat==-1) {
        egsWarning("IAEA_PhspSource::openFile: error reading file size/byte order from header %s.IAEAheader\n",phsp_file.c_str());
        return;
    }
    else if (iaea_iostat==-4) {
        egsWarning("IAEA_PhspSource::openFile: byte mismatch between phase space file and machine.  Will swap bytes.\n");
        swap_bytes = true;
    }
    else if (iaea_iostat==-3 || iaea_iostat==-5) {
        egsWarning("IAEA_PhspSource::openFile: mismatch between file size in header and actual file size of %s\n",phsp_file.c_str());
        return;
    }

    //now get some info from the header
    EGS_I64 n, n_photon, pinc;
    float emax, zposn;
    int iaea_type=-1; //for getting no. of particles
    iaea_get_max_particles(&iaea_fileid,&iaea_type,&n);
    if (n<0) {
        egsWarning("IAEA_PhspSource::openFile: failed to get total no. of particles from %s.IAEAheader\n",phsp_file.c_str());
        return;
    }
    iaea_type=1; //for getting no. of photons
    iaea_get_max_particles(&iaea_fileid,&iaea_type,&n_photon);
    if (n_photon<0) {
        egsWarning("IAEA_PhspSource::openFile: failed to get no. of photons from %s.IAEAheader\n",phsp_file.c_str());
        return;
    }
    //now get max. energy
    iaea_get_maximum_energy(&iaea_fileid,&emax);
    if (emax<0) {
        egsWarning("IAEA_PhspSource::openFile: failed to get max. energy from %s.IAEAheader\n",phsp_file.c_str());
        return;
    }
    //no. of particles incident from original source
    iaea_get_total_original_particles(&iaea_fileid,&pinc);
    if (pinc<0) {
        egsWarning("IAEA_PhspSource::openFile: failed to get no. of original particles from %s.IAEAheader\n",phsp_file.c_str());
        return;
    }

    //get number of extra float and extra long variables so we can set array dimensions
    iaea_get_extra_numbers(&iaea_fileid,&n_extra_floats,&n_extra_longs);
    if (n_extra_floats==-1 || n_extra_longs==-1) {
        egsWarning("IAEA_PhspSource::openFile: failed to get no. of extra floats and longs stored in %s.IAEAphsp\n",phsp_file.c_str());
        return;
    }
    //determine if Zlast is stored in the file and, if so, its array index, i_zlast
    int extrafloat_types[n_extra_floats], extralong_types[n_extra_longs];
    iaea_get_type_extra_variables(&iaea_fileid,&iaea_iostat,extralong_types,extrafloat_types);
    if (iaea_iostat==-1) {
        egsWarning("IAEA_PhspSource::openFile: failed to get Mode of data %s.IAEAheader\n",phsp_file.c_str());
        return;
    }
    mode2=false;
    for (int i=0; i< n_extra_floats; i++) {
        if (extrafloat_types[i]==3) {
            mode2=true;
            i_zlast=i;
            break;
        }
    }

    //determine array index of latch
    latch_stored=false;
    for (int i=0; i< n_extra_longs; i++) {
        if (extralong_types[i]==2) {
            latch_stored=true;
            i_latch=i;
            break;
        }
    }
    if (!latch_stored) {
        egsWarning("IAEA_PhspSource::openFile: LATCH is not stored in data in %s.IAEAphsp\n",phsp_file.c_str());
    }

    Npos = 0;
    Nlast = n;
    Nfirst = 1;
    // at this point the position should be at the first particle in the file
    Emax = emax;
    Pinc = pinc;
    Nparticle = n;
    Nphoton = n_photon;
    is_valid = true;
    the_file_name = phsp_file;
}

IAEA_PhspSource::IAEA_PhspSource(EGS_Input *input, EGS_ObjectFactory *f) :
    EGS_BaseSource(input,f) {
    init();
    string fname;
    int err = input->getInput("iaea phase space file",fname);
    if (err) {
        egsWarning("IAEA_PhspSource: no 'iaea phase space file' input\n");
        return;
    }
    openFile(fname);
    if (!isValid()) {
        egsWarning("IAEA_PhspSource: errors while opening the phase space file"
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
        Nreuse_g = ntmp;
    }
    err = input->getInput("reuse electrons",ntmp);
    if (!err && ntmp > 0) {
        Nreuse_e = ntmp;
    }
    description = "IAEA phase space source from ";
    description += the_file_name;
}

EGS_I64 IAEA_PhspSource::getNextParticle(EGS_RandomGenerator *, int &q,
        int &latch, EGS_Float &E, EGS_Float &wt, EGS_Vector &x, EGS_Vector &u) {
    /*
    if( Nuse >= Nreuse ) {
        do { readParticle(); } while ( rejectParticle() );
    }
    */
    int nstat,extrainttemp[n_extra_longs];
    float extrafloattemp[n_extra_floats];
    if (Nuse >= Nreuse) {  //get a new particle
        if ((++Npos) > Nlast) {
            egsWarning("IAEA_PhspSource::getNextParticle(): reached the end of the "
                       "phase space file chunk (%lld)\n  will start from the beginning "
                       "of the chunk (%lld) but this "
                       "implies that uncertainty estimates will be inaccurate\n",
                       Nlast,Nfirst);
            iaea_set_record(&iaea_fileid,&Nfirst,&iaea_iostat);
            if (iaea_iostat<0) {
                egsFatal("IAEA_PhspSource::getNextParticle(): error restarting phase space chunk\n");
            }
            Nrecycle++;
            Npos = Nfirst;
        }
        iaea_get_particle(&iaea_fileid,&nstat,&p.q,&p.E,&p.wt,&p.x,&p.y,&p.z,&p.u,&p.v,&p.w,extrafloattemp,extrainttemp);
        ++Nread;
        if (latch_stored) {
            p.latch = extrainttemp[i_latch];
        }
        if (mode2) {
            p.zlast = extrafloattemp[i_zlast];
        }
        if (swap_bytes) {
            egsSwapBytes(&p.q);
            egsSwapBytes(&nstat);
            egsSwapBytes(&p.zlast);
            egsSwapBytes(&p.latch);
            egsSwapBytes(&p.E);
            egsSwapBytes(&p.wt);
            egsSwapBytes(&p.x);
            egsSwapBytes(&p.y);
            egsSwapBytes(&p.z);
            egsSwapBytes(&p.u);
            egsSwapBytes(&p.v);
            egsSwapBytes(&p.w);
        }
        //do check here because we need the swapped version of nstat
        if (nstat<0) {
            egsFatal("IAEA_PhspSource::getNextParticle(): error reading particle number %i\n",Npos);
        }
        //convert charge from iaea type
        if (p.q==1) {
            p.q=0;
        }
        else if (p.q==2) {
            p.q=-1;
        }
        else if (p.q==3) {
            p.q=1;
        }
        else {
            egsFatal("IAEA_PhspSource::getNextParticle: unknown charge on particle\n");
        }
        if (!latch_stored) {
            p.latch = 0;    //need some non-nonsense value in case there is a LATCH filter
        }
        //note: we don't have to convert to p.E to K.E. because that's what IAEA format stores
        if (first || nstat>0) {
            count++;    //increment primary history counter
        }
        first= false;
        //store Nreuse
        if (p.q) {
            Nreuse = Nreuse_e;
        }
        else {
            Nreuse = Nreuse_g;
        }
        //reset Nuse
        Nuse = 0;
        p.wt /= Nreuse;
    }

    //energy, wt, position and direction cosines
    x.x = p.x;
    x.y = p.y;
    x.z = p.z;
    u.x = p.u;
    u.y = p.v, u.z = p.w;
    wt=p.wt;
    if (rejectParticle()) {
        wt = 0;
    }
    E=p.E;
    q=p.q;
    //latch = 0;
    latch= p.latch;
    //rejectParticle uses BeamParticle, p
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

void IAEA_PhspSource::setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun) {
    if (nstart < 0 || nrun < 1 || nstart + nrun > Nparticle) {
        egsWarning("IAEA_PhspSource::setSimulationChunk(): illegal attempt "
                   "to set the simulation chunk between %lld and %lld ignored\n",
                   nstart+1,nstart+nrun);
        return;
    }
    Nfirst = nstart+1;
    Nlast = nstart + nrun;
    Npos = nstart;
    iaea_set_record(&iaea_fileid,&Nfirst,&iaea_iostat);
    if (iaea_iostat<0) {
        egsWarning("IAEA_PhspSource::setSimulationChunk(): error setting phase space chunk\n");
        return;
    }
    egsInformation("IAEA_PhspSource: using phsp portion between %lld and %lld\n",
                   Nfirst,Nlast);
}

bool IAEA_PhspSource::rejectParticle() const {
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
    if (filter_type == 1) {
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

void IAEA_PhspSource::setFilter(int type, int nbit1, int nbit2, const int *bits) {
    if (type < 0 || type > 3) {
        egsWarning("IAEA_PhspSource::setFilter: invalid filter type %d\n",type);
        return;
    }
    int ntot = nbit1;
    if (type == 0) {
        ntot += nbit2;
    }
    if (ntot > 29) {
        egsWarning("IAEA_PhspSource::setFilter: maximum number of bits is "
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

    IAEA_PHSP_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<IAEA_PhspSource>(input,f,"iaea phsp source");
    }

}
