/*
###############################################################################
#
#  EGSnrc egs++ phase space scoring object
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


/*! \file egs_phsp_scoring.cpp
 *  \brief A phase space scoring ausgab object: implementation
 *  \BW
 */

#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "egs_phsp_scoring.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "iaea_phsp.h"

EGS_PhspScoring::EGS_PhspScoring(const string &Name,
                                 EGS_ObjectFactory *f) :
    EGS_AusgabObject(Name,f), phsp_index(0), store_max(1000), phsp_file(0),
    count(0), countg(0), emin(1.e30), emax(-1.e30), first_flush(true), is_restart(false) {
    otype = "EGS_PhspScoring";
}

EGS_PhspScoring::~EGS_PhspScoring() {
}

void EGS_PhspScoring::setApplication(EGS_Application *App) {
    EGS_AusgabObject::setApplication(App);
    if (!app) {
        return;
    }

    char buf[512];//useful character buffer
    //set up the stack of particles to output to the phase space file
    //1000 particles at a time
    p_stack = new Particle[store_max];

    description = "\n*******************************************\n";
    description +=  "Phase Space Scoring Object (";
    description += name;
    description += ")\n";
    description += "*******************************************\n";
    if (score_type==0) {
        description += "\n Will output phase space for particles crossing surfaces of geometry:\n";
        description += phsp_geom->getName();
        description += "\n Particles scored on: ";
        if (scoredir == 0) {
            description += "entering and exiting phase space geometry";
        }
        else if (scoredir == 1) {
            description += "entering phase space geometry";
        }
        else if (scoredir == 2) {
            description += "exiting phase space geometry";
        }
    }
    else if (score_type==1) {
        description += "\n Will output phase space for the following exit/entry region pairs:\n";
        description += " Exit regions:\n";
        for (int i=0; i<fromreg.size(); i++) {
            sprintf(buf,"%d ",fromreg[i]);
            description += buf;
        }
        description += "\n Entry regions:\n";
        for (int i=0; i<fromreg.size(); i++) {
            sprintf(buf,"%d ",toreg[i]);
            description += buf;
        }
        description += "\n";
        //now, for every region, set up a (possibly empty) array of entry regions
        int nreg = app->getnRegions();
        for (int ir=0; ir<nreg; ir++) {
            vector<int> tolist;
            for (int i=0; i<fromreg.size(); i++) {
                if (fromreg[i]==ir) {
                    tolist.push_back(toreg[i]);
                }
            }
            from_to.push_back(tolist);
        }
    }

    //construct name of phase space file -- opening to occur later
    if (phspoutdir == "") {
        phspoutdir = app->getAppDir();
    }
    if (oformat==0) {
        description += "\n Data will be output in EGSnrc format.\n";
        if (app->getNparallel()>0) {
            sprintf(buf,"%s_w%d.egsphsp1",getObjectName().c_str(),app->getIparallel());
        }
        else {
            sprintf(buf,"%s.egsphsp1",getObjectName().c_str());
        }
        phsp_fname=egsJoinPath(phspoutdir,buf);
        description += "\n Phase space file name:\n";
        description += phsp_fname;
    }
    else if (oformat==1) {
        description += "\n Data will be output in IAEA format.\n";
        if (app->getNparallel()>0) {
            sprintf(buf,"%s_w%d.1",getObjectName().c_str(),app->getIparallel());
        }
        else {
            sprintf(buf,"%s.1",getObjectName().c_str());
        }
        phsp_fname=egsJoinPath(phspoutdir,buf);

        //need to add a null terminator
        len = phsp_fname.length();
        phsp_fname_char = new char[len+1];
        phsp_fname.copy(phsp_fname_char,len,0);
        phsp_fname_char[len]='\0'; //null terminator on string

        //iaea particle charge
        iaea_q_type[0]=2; //e-
        iaea_q_type[1]=1; //photon
        iaea_q_type[2]=3; //e+

        iaea_id = 1;

        //set up extra float and extra long array indices
        latch_ind = 2;//type of latch as defined by iaea
        iaea_n_extra_long=1; //only store latch
        iaea_i_latch=0; // position of latch in array
        if (score_mu) {
            iaea_n_extra_float=1;
            mu_ind = 0; //set type to generic float
            iaea_i_mu=0; //position of mu in array
        }
        else {
            iaea_n_extra_float=0; //no extra floats
        }

        description += "\n Phase space file names:\n";
        description += "  Header file: " + phsp_fname + ".IAEAheader\n";
        description += "  Data file: " + phsp_fname + ".IAEAphsp";
        string xyzname[3] = {"X", "Y", "Z"};
        for (int i=0; i<3; i++) {
            if (xyz_is_const[i]) {
                ostringstream xyz;
                xyz << xyzscore[i];
                description += "\n Data scored at constant " + xyzname[i] + " = " + xyz.str() + " cm";
            }
        }
    }
    description += "\n Particles scored: ";
    if (ocharge == 0) {
        description += "all";
    }
    else if (ocharge == 1) {
        description += "photons";
    }
    else if (ocharge == 2) {
        description += "charged";
    }
    if (oformat ==1 && score_mu) {
        description += "\n mu will be scored (if available)";
    }
    if (oformat ==0 && score_mc) {
        description += "\n will score multiple crossers (and descendents)";
    }
}

//final buffer flush and then close file
//we want to output some data about particles scored
//may ultimately want to allow the user to define scoring zones for output
void EGS_PhspScoring::reportResults() {
    flushBuffer();
    if (oformat == 1) { //iaea
        int iaea_iostat;
        iaea_destroy_source(&iaea_id,&iaea_iostat);
        if (iaea_iostat<0) {
            egsFatal("\n EGS_PhspScoring: Error closing phase space file.\n");
        }
    }
    else if (oformat == 0) {
        phsp_file.close();
    }
    egsInformation("\n======================================================\n");
    egsInformation("Phase Space Scoring Object(%s)\n",name.c_str());
    egsInformation("======================================================\n");
    if (oformat==1) {
        egsInformation("\n IAEA format phase space output:\n");
        egsInformation(" Header file: %s.IAEAheader\n",phsp_fname.c_str());
        egsInformation(" Data file: %s.IAEAphsp\n",phsp_fname.c_str());
    }
    else if (oformat == 0) {
        egsInformation("\n EGSnrc format phase space output:\n");
        egsInformation(" Data file: %s\n",phsp_fname.c_str());
    }
    float emintmp;
    if (count == countg) {
        emintmp = 0.0;
    }
    else {
        emintmp = emin;
    }
    egsInformation("Summary of scored data:\n");
    egsInformation("=> total no. of particles = %lld \n", count);
    egsInformation("=> no. of photons = %lld \n", countg);
    egsInformation("=> max. k.e. of all particles = %g MeV\n",emax);
    egsInformation("=> min. k.e. of charged particles = %g MeV\n",emintmp);
    egsInformation("=> no. of primary histories represented = %lld\n",last_case);
    egsInformation("\n======================================================\n");
}

//store 1000 particles at a time in p_stack
//if we're at 1000, actually write the particles to the file and update
//the header info
//also, keep track of phase space file counters, min., max. energy
void EGS_PhspScoring::storeParticle(EGS_I64 ncase) {

    //if user requested mu scoring, check if mu is available
    if (score_mu && app->getMU() < 0) {
        egsWarning("\nEGS_PhspScoring: User requested mu scoring, but mu is inavailable with this source.\n");
        egsWarning("Turning off mu scoring.\n");
        score_mu=false;
    }

    //counters, min. and max. k.e.
    EGS_Float prm = app->getRM();
    count++;
    if (app->top_p.q==0) {
        countg++;
    }
    if (app->top_p.E-abs(app->top_p.q)*prm > emax) {
        emax = app->top_p.E-abs(app->top_p.q)*prm;
    }
    if (app->top_p.q != 0 && app->top_p.E - prm < emin) {
        emin = app->top_p.E - prm;
    }

    //store particle data in p_stack
    //set -ve energy marker if this is a new primary hist.
    double E = app->top_p.E;
    if (ncase != last_case) {
        E = -E;
        last_case = ncase;
    }
    p_stack[phsp_index].E = E;
    p_stack[phsp_index].wt = app->top_p.wt;
    p_stack[phsp_index].x = app->top_p.x.x;
    p_stack[phsp_index].y = app->top_p.x.y;
    p_stack[phsp_index].z = app->top_p.x.z;
    p_stack[phsp_index].u = app->top_p.u.x;
    p_stack[phsp_index].v = app->top_p.u.y;
    p_stack[phsp_index].w = app->top_p.u.z;
    p_stack[phsp_index].q = app->top_p.q;
    if (score_mu) {
        p_stack[phsp_index].mu = app->getMU();
    }
    p_stack[phsp_index++].latch = app->top_p.latch;

    if (phsp_index > store_max - 1) {
        //write store_max particles to the file and reset phsp_index counter
        flushBuffer();
    }
}

//open the phase space file for writing/appending data
void EGS_PhspScoring::openPhspFile() const {
//the file has already been named at this point
    if (oformat==0) { //EGSnrc format
        if (is_restart) {
            phsp_file.open(phsp_fname.c_str(),ios::binary|ios::out|ios::in);
            if (!(phsp_file))
                egsFatal("\nEGS_PhspScoring: Failed to open phase space file %s for appending.\n",
                         phsp_fname.c_str());
            //check that total no. of particles in header = total no. read from .egsdat file
            unsigned int count4;
            phsp_file.seekg(5,ios::beg);
            phsp_file.read((char *) &count4, sizeof(unsigned int));
            if (count4 != countprev) {
                egsFatal("\nEGS_PhspScoring: Particle no. mismatch between %s and .egsdat file.\n",phsp_fname.c_str());
            }
            //go to the end of the file
            phsp_file.seekp(0,ios::end);
            if (phsp_file.tellp() != 28 + countprev*sizeof(egs_phsp_write_struct)) {
                egsFatal("\nEGS_PhspScoring: File size mismatch in %s and .egsdat file.\n",phsp_fname.c_str());
            }
        }
        else {
            phsp_file.open(phsp_fname.c_str(),ios::binary|ios::out);
            if (!(phsp_file))
                egsFatal("\nEGS_PhspScoring: Failed to open phase space file %s for writing.\n",
                         phsp_fname.c_str());
            //always MODE0 files--i.e. no ZLAST
            phsp_file.write("MODE0",5);
            //leave space for/skip over the rest of the header
            phsp_file.seekp(28,ios::beg);
        }
    }
    else if (oformat == 1) {  //IAEA format
        int iaea_iostat;
        iaea_id = 1; //numerical index indicating this is the 1st file associated with this object scored
        //hard coded to 1
        if (is_restart) {
            int rwmode = 3;
            iaea_new_source(&iaea_id,phsp_fname_char,&rwmode,&iaea_iostat,len);
            if (iaea_iostat < 0) {
                egsFatal("\nEGS_PhspScoring: Failed to open phase space file %s.IAEAphsp for appending.\n",phsp_fname.c_str());
            }
            //check for consistency with total no. of scored particles as read from .egsdat file
            EGS_I64 nparticle;
            int type = -1;
            int iaea_iostat;
            iaea_get_max_particles(&iaea_id,&type,&nparticle);
            if (nparticle != countprev) {
                egsFatal("\nEGS_PhspScoring: Particle no. mismatch between %s.IAEAphsp and .egsdat file.\n",phsp_fname.c_str());
            }
            iaea_check_file_size_byte_order(&iaea_id,&iaea_iostat);
            if (iaea_iostat != 0) {
                egsFatal("\nEGS_PhspScoring: Byte order/file size mismatch in %s.IAEAphsp.\n",phsp_fname.c_str());
            }
        }
        else {
            int rwmode = 2;
            iaea_new_source(&iaea_id,phsp_fname_char,&rwmode,&iaea_iostat,len);
            if (iaea_iostat < 0) {
                egsFatal("\nEGS_PhspScoring: Failed to open phase space file %s.IAEAphsp for writing.\n",phsp_fname.c_str());
            }
        }


        //set up constant variables
        for (int i=0; i<3; i++) {
            if (xyz_is_const[i]) {
                int index = i;
                float constval = xyzscore[i];
                iaea_set_constant_variable(&iaea_id,&index,&constval);
            }
        }
        //set up extra floats and int indices and types
        //need to store below in _tmp variables because this is a const function
        int latch_ind_tmp = latch_ind;
        int iaea_n_extra_long_tmp=iaea_n_extra_long;
        int iaea_i_latch_tmp=iaea_i_latch;
        int mu_ind_tmp = mu_ind;
        int iaea_i_mu_tmp = iaea_i_mu;
        int iaea_n_extra_float_tmp=iaea_n_extra_float;

        iaea_set_extra_numbers(&iaea_id,&iaea_n_extra_float_tmp,&iaea_n_extra_long_tmp);
        iaea_set_type_extralong_variable(&iaea_id,&iaea_i_latch_tmp,&latch_ind_tmp);
        if (score_mu) {
            iaea_set_type_extrafloat_variable(&iaea_id,&iaea_i_mu_tmp,&mu_ind_tmp);
        }
    }
}

//write phsp_index particles to phase space file
//update header and reset phsp_index
int EGS_PhspScoring::flushBuffer() const {

    if (first_flush) {
        openPhspFile();    //here's where we open the phase space file
    }
    //awkward logic because we do not know if this is a restart
    //until after initialization
    first_flush = false;

    if (oformat == 1) { //iaea format
        for (int j=0; j<phsp_index; j++) {
            //fairly transparent, could probably put a lot of this in a separate method
            //undo -ve energy marker and use n_stat to indicate new primary hist.
            int n_stat = p_stack[j].E < 0 ? 1 : 0;
            float E = abs(p_stack[j].E);
            //convert charge to iaea type
            int type = iaea_q_type[p_stack[j].q+1];

            //store latch in iaea_extra_long
            EGS_I32 *iaea_extra_long = new EGS_I32[iaea_n_extra_long];
            iaea_extra_long[iaea_i_latch]=p_stack[j].latch;
            float *iaea_extra_float = new float[iaea_n_extra_float];
            if (score_mu) {
                iaea_extra_float[iaea_i_mu] = p_stack[j].mu;
            }

            //now store double precision values in single precision reals
            float wt = p_stack[j].wt;
            float x = p_stack[j].x;
            float y = p_stack[j].y;
            float z = p_stack[j].z;
            float u = p_stack[j].u;
            float v = p_stack[j].v;
            float w = p_stack[j].w;
            //now actually write data

            iaea_write_particle(&iaea_id,&n_stat,&type,&E,&wt,&x,&y,&z,&u,&v,&w,iaea_extra_float,iaea_extra_long);
            if (n_stat < 0) {
                egsFatal("\nEGS_PhspScoring: Failed to write particle data to phase space file.");
            }
        }
        //update header with no. of primary histories
        EGS_I64 last_case_tmp = last_case;
        iaea_set_total_original_particles(&iaea_id,&last_case_tmp);
        //update header file
        int iaea_iostat;
        iaea_update_header(&iaea_id,&iaea_iostat);
        if (iaea_iostat<0) {
            egsFatal("\nEGS_PhspScoring: Failed to update phase space header file.");
        }
    }
    else if (oformat == 0) {  //EGSnrc format
        if (!phsp_file) {
            egsFatal("\nEGS_PhspScoring: phase space file is not open for writing.");
        }
        //don't forget that phsp_index is incremented after every particle written to p_stack
        for (int j=0; j<phsp_index; j++) {
            egs_phsp_write_struct ws(p_stack[j]);
            phsp_file.write((char *) &ws, sizeof(ws));
        }
        //store position of end of file
        iostream::off_type pos = (count+1)*sizeof(egs_phsp_write_struct);
        //update header
        phsp_file.seekp(5,ios::beg);
        unsigned int count4 = count, countg4 = countg;
        float pinc = last_case;
        float emintmp;
        if (countg4==count4) {
            emintmp = 0.0;
        }
        else {
            emintmp = emin;
        }
        phsp_file.write((char *) &count4, sizeof(unsigned int));
        phsp_file.write((char *) &countg4, sizeof(unsigned int));
        phsp_file.write((char *) &emax, sizeof(float));
        phsp_file.write((char *) &emintmp, sizeof(float));
        phsp_file.write((char *) &pinc, sizeof(float));
        phsp_file.seekp(pos,ios::beg);
    }

    phsp_index=0;

    return 0;
};

bool EGS_PhspScoring::storeState(ostream &data) const {
    if (!egsStoreI64(data,count)) {
        return false;
    }
    if (!egsStoreI64(data,countg)) {
        return false;
    }
    //update phase space file at the end of each batch
    flushBuffer();
    return true;
}

bool  EGS_PhspScoring::setState(istream &data) {
    if (!egsGetI64(data,count)) {
        return false;
    }
    if (!egsGetI64(data,countg)) {
        return false;
    }
    countprev = count;
    is_restart = true;
    return true;
}

bool  EGS_PhspScoring::addState(istream &data) {
    EGS_I64 tmp;
    if (!egsGetI64(data,tmp)) {  //return false;
        egsWarning("error while reading count\n");
        return false;
    }
    count += tmp;
    if (!egsGetI64(data,tmp)) {  //return false;
        egsWarning("error while reading countg\n");
        return false;
    }
    countg += tmp;
    return true;
}

//*********************************************************************
// Process input for this ausgab object
//**********************************************************************
extern "C" {

    EGS_PHSP_SCORING_EXPORT EGS_AusgabObject *createAusgabObject(EGS_Input *input,
            EGS_ObjectFactory *f) {
        const static char *func = "createAusgabObject(phsp_scoring)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }
        string str;
        EGS_BaseGeometry *phspgeom;
        int iscoremc = 0; //default to not score multiple crossers
        vector <int> from_reg, to_reg;
        int stype = 0; //default is to use scoring geom
        int phspouttype;
        int ptype;
        int sdir=0;
        int imuscore = 0;
        float xyzconst[3];
        bool xyzisconst[3] = {false, false, false};
        string gname;
        string outdir;
        int err01 = input->getInput("phase space geometry",gname);
        if (err01) {
            stype = 1;
        }
        else {
            phspgeom = EGS_BaseGeometry::getGeometry(gname);
            if (!phspgeom) {
                egsWarning("\nEGS_PhspScoring: %s does not name an existing geometry.\n"
                           "Will assume you want to use exit/entry region pairs.\n",gname.c_str());
                stype = 1;
            }
            else {
                if (input->getInput("score particles on", str) < 0) {
                    egsInformation("EGS_PhspScoring: No input for scoring direction.\n");
                    egsInformation("Will score on entry and exit from phase space geometry.\n");
                    sdir = 0;
                }
                else {
                    //get scoring direction
                    vector<string> allowed_sdir;
                    allowed_sdir.push_back("entry and exit");
                    allowed_sdir.push_back("entry");
                    allowed_sdir.push_back("exit");
                    sdir = input->getInput("score particles on",allowed_sdir,-1);
                    if (sdir < 0) {
                        egsFatal("\nEGS_PhspScoring: Invalid scoring direction.\n");
                    }
                }
            }
        }
        if (stype==1) {
            // user wants to use exit/entry region pairs
            int err05 = input->getInput("from regions",from_reg);
            int err06 = input->getInput("to regions",to_reg);
            if (err05 || err06) {
                egsFatal("\nEGS_PhspScoring: Missing/incorrect input for scoring method\n"
                         "(scoring geometry or pairs of exit/entry regions)\n");
            }
            else {
                //run some checks on exit/entry region pairs
                vector<int>::iterator p,p1;
                if (from_reg.size() > to_reg.size()) {
                    p = from_reg.begin();
                    egsWarning("\nEGS_PhspScoring: Mismatch in no. of exit/entry regions.\n"
                               "Will only score for matched pairs.\n");
                    p += to_reg.size();
                    from_reg.erase(p,p+from_reg.size()-to_reg.size());
                }
                else if (to_reg.size() > from_reg.size()) {
                    p = to_reg.begin();
                    egsWarning("\nEGS_PhspScoring: Mismatch in no. of exit/entry regions.\n"
                               "Will only score for matched pairs.\n");
                    p += from_reg.size();
                    to_reg.erase(p,p+to_reg.size()-from_reg.size());
                }
                //now go through and look for exit region = entry region
                int i=0;
                while (i<from_reg.size()) {
                    if (from_reg[i]==to_reg[i]) {
                        egsInformation("\nEGS_PhspScoring: Cannot have entry region = exit region (reg no. %d)\n",from_reg[i]);
                        egsInformation("Will delete this pair\n");
                        p=from_reg.begin()+i;
                        p1 =to_reg.begin()+i;
                        from_reg.erase(p);
                        to_reg.erase(p1);
                    }
                    else {
                        //advance counter
                        i++;
                    }
                }
            }
        }
        //now get common inputs for both scoring methods
        if (input->getInput("output format", str) < 0) {
            egsInformation("EGS_PhspScoring: No input for output format type.  Will default to EGSnrc.\n");
            phspouttype = 0;
        }
        else {
            vector<string> allowed_oformat;
            allowed_oformat.push_back("EGSnrc");
            allowed_oformat.push_back("IAEA");
            phspouttype = input->getInput("output format", allowed_oformat, -1);
            if (phspouttype < 0) {
                egsFatal("\nEGS_PhspScoring: Invalid output format.\n");
            }
            //see if the user wants to specify constant X/Y/Z for IAEA format
            if (phspouttype == 1) {
                int err02 = input->getInput("constant X",xyzconst[0]);
                int err03 = input->getInput("constant Y",xyzconst[1]);
                int err04 = input->getInput("constant Z",xyzconst[2]);
                if (!err02) {
                    xyzisconst[0] = true;
                }
                if (!err03) {
                    xyzisconst[1] = true;
                }
                if (!err04) {
                    xyzisconst[2] = true;
                }
                //see if user wants to score mu (if available)
                //default is not to score
                if (!input->getInput("score mu", str)) {
                    vector<string> allowed_muscore;
                    allowed_muscore.push_back("no");
                    allowed_muscore.push_back("yes");
                    imuscore = input->getInput("score mu",allowed_muscore,-1);
                    if (imuscore < 0) {
                        egsWarning("\nEGS_PhspScoring: Invalid input for mu scoring.  Will not score mu.\n");
                        imuscore = 0;
                    }
                }
            }
        }
        if (phspouttype == 0) {
            //see if user wants to score multiple crossers
            if (!input->getInput("score multiple crossers", str)) {
                vector<string> allowed_scoremc;
                allowed_scoremc.push_back("no");
                allowed_scoremc.push_back("yes");
                iscoremc = input->getInput("score multiple crossers",allowed_scoremc,-1);
                if (iscoremc < 0) {
                    egsWarning("\nEGS_PhspScoring: Invalid input for score multiple crossers.  Will not score.\n");
                    iscoremc = 0;
                }
            }
        }
        if (input->getInput("output directory",outdir) < 0) {
            outdir="";
        }
        if (input->getInput("particle type", str) < 0) {
            egsInformation("EGS_PhspScoring: No input for particle type.  Will score all.\n");
            ptype = 0;
        }
        else {
            //get particle type
            vector<string> allowed_ptype;
            allowed_ptype.push_back("all");
            allowed_ptype.push_back("photons");
            allowed_ptype.push_back("charged");
            ptype = input->getInput("particle type",allowed_ptype,-1);
            if (ptype < 0) {
                egsFatal("\nEGS_PhspScoring: Invalid particle type.\n");
            }
        }

        //=================================================

        /* Setup phsp scoring object with input parameters */
        EGS_PhspScoring *result = new EGS_PhspScoring("",f);
        result->setName(input);
        if (stype==0) {
            result->setGeom(phspgeom);
        }
        else if (stype==1) {
            result->setEntryExitReg(from_reg,to_reg);
        }
        result->setOType(phspouttype);
        result->setXYZconst(xyzisconst,xyzconst);
        result->setOutDir(outdir);
        result->setParticleType(ptype);
        result->setScoreDir(sdir);
        result->setMuScore(imuscore);
        result->setScoreMC(iscoremc);
        return result;
    }
}
