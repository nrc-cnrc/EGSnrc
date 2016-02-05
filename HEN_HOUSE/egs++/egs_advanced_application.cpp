/*
###############################################################################
#
#  EGSnrc egs++ advanced application
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
#                   Blake Walters
#
###############################################################################
*/


/*! \file egs_advanced_application.cpp
 *  \brief EGS_AdvancedApplication implementation
 *  \IK
 *
 *  Also provides implementations of the C-style functions needed to link
 *  against the mortran back-end egsHowfar(), egsHownear(), egsAusgab(),
 *  and egsStartParticle().
 */
#include "egs_advanced_application.h"
#include "egs_functions.h"
#include "egs_interface2.h"
#include "egs_run_control.h"
#include "egs_base_source.h"
#include "egs_input.h"
#include "egs_interpolator.h"
#include "egs_rndm.h"
#include "egs_ausgab_object.h"

#include <string>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

using namespace std;

#ifdef DEBUG_APPLICATION
#define CHECK_GET_APPLICATION(a,b) \
    EGS_Application *a = EGS_Application::activeApplication(); \
    if( !a ) egsFatal("%s: no active application!\n",b);
#else
#define CHECK_GET_APPLICATION(a,b) \
    EGS_Application *a = EGS_Application::activeApplication();
#endif

string EGS_AdvancedApplication::base_revision = "$Revision: 1.39 $";

/*
extern __extc__ struct EGS_Stack F77_OBJ(stack,STACK);
extern __extc__ struct EGS_Bounds F77_OBJ(bounds,BOUNDS);
extern __extc__ struct EGS_Thresh F77_OBJ(thresh,THRESH);
extern __extc__ struct EGS_Epcont F77_OBJ(epcont,EPCONT);
extern __extc__ struct EGS_EtControl F77_OBJ_(et_control,ET_CONTROL);
extern __extc__ struct EGS_Media F77_OBJ(media,MEDIA);
extern __extc__ struct EGS_Useful F77_OBJ(useful,USEFUL);
extern __extc__ struct EGS_XOptions F77_OBJ_(xsection_options,XSECTION_OPTIONS);
extern __extc__ struct EGS_IO F77_OBJ_(egs_io,EGS_IO);
extern __extc__ struct EGS_VarianceReduction F77_OBJ_(egs_vr,EGS_VR);

struct EGS_Stack        *the_stack     = & F77_OBJ(stack,STACK);
struct EGS_Bounds       *the_bounds    = & F77_OBJ(bounds,BOUNDS);
struct EGS_Thresh       *the_thresh    = & F77_OBJ(thresh,THRESH);
struct EGS_Epcont       *the_epcont    = & F77_OBJ(epcont,EPCONT);
struct EGS_EtControl    *the_etcontrol = & F77_OBJ_(et_control,ET_CONTROL);
struct EGS_Media        *the_media     = & F77_OBJ(media,MEDIA);
struct EGS_Useful       *the_useful    = & F77_OBJ(useful,USEFUL);
struct EGS_XOptions     *the_xoptions  = & F77_OBJ_(xsection_options,XSECTION_OPTIONS);
struct EGS_IO           *the_egsio     = & F77_OBJ_(egs_io,EGS_IO);
struct EGS_VarianceReduction *the_egsvr = & F77_OBJ_(egs_vr,EGS_VR);
*/

#define egsGetRNGPointers F77_OBJ_(egs_get_rng_pointers,EGS_GET_RNG_POINTERS)
extern __extc__ void egsGetRNGPointers(EGS_I32 *, EGS_I32 *);
#define egsGetRNGArray F77_OBJ_(egs_get_rng_array,EGS_GET_RNG_ARRAY)
extern __extc__ void egsGetRNGArray(EGS_Float *);
#define egsSetRNGState F77_OBJ_(egs_set_rng_state,EGS_SET_RNG_STATE)
extern __extc__ void egsSetRNGState(const EGS_I32 *, const EGS_Float *);
#define egsGetSteps F77_OBJ_(egs_get_steps,EGS_GET_STEPS)
extern __extc__ void egsGetSteps(double *, double *);
#define egsSetSteps F77_OBJ_(egs_set_steps,EGS_SET_STEPS)
extern __extc__ void egsSetSteps(const double *, const double *);
#define egsOpenUnits F77_OBJ_(egs_open_units,EGS_OPEN_UNITS)
extern __extc__ void egsOpenUnits(const EGS_I32 *);
#define egsGetElectronData F77_OBJ_(egs_get_electron_data,EGS_GET_ELECTRON_DATA)
extern __extc__ void egsGetElectronData(void (*func)(EGS_I32 *,EGS_Float *,
                                        EGS_Float *,EGS_Float *,EGS_Float *),const EGS_I32 *,const EGS_I32 *);
#define egsGetPhotonData F77_OBJ_(egs_get_photon_data,EGS_GET_PHOTON_DATA)
extern __extc__ void egsGetPhotonData(void (*func)(EGS_I32 *,EGS_Float *,
                                      EGS_Float *,EGS_Float *,EGS_Float *),const EGS_I32 *,const EGS_I32 *);

static EGS_Float *__help1, *__help2, *__help3, *__help4;
static EGS_I32 *__ihelp;
static void __help_get_data(EGS_I32 *nbin,EGS_Float *emin, EGS_Float *emax,
                            EGS_Float *a, EGS_Float *b) {
    __ihelp = nbin;
    __help1 = emin;
    __help2 = emax;
    __help3 = a;
    __help4 = b;
}

static void __init_interpolator(int q, int imed, int type,
                                EGS_Interpolator &i) {
    if (q) {
        egsGetElectronData(__help_get_data,&imed,&type);
    }
    else {
        egsGetPhotonData(__help_get_data,&imed,&type);
    }
    i.initialize(*__ihelp,*__help1,*__help2,__help3,__help4);
}

void EGS_AdvancedApplication::setAusgabCall(AusgabCall call, bool on_or_off) {
    \
    EGS_Application::setAusgabCall(call,on_or_off);
    the_epcont->iausfl[call] = (int) on_or_off;
}

static EGS_LOCAL void __egs_iovar(int nio, int ns,
                                  const char *source, char *var) {
    if (ns > nio-1) {
        egsFatal("Mortran variable is not long enough to hold %s\n",source);
    }
    int j;
    for (j=0; j<ns; j++) {
        var[j] = source[j];
    }
    var[ns] = 0;
    for (j=ns+1; j<nio; j++) {
        var[j] = ' ';
    }
}

static EGS_LOCAL char __write_buf[8192];
extern __extc__ void F77_OBJ_(egs_write_string,EGS_WRITE_STRING)(int *,
        const char *,int);
void EGS_LOCAL __write_to_fortran_file(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    vsprintf(__write_buf,msg,ap);
    va_end(ap);
    int ounit=the_egsio->i_log;
    F77_OBJ_(egs_write_string,EGS_WRITE_STRING)(&ounit,__write_buf,
            strlen(__write_buf));
}
void EGS_LOCAL __write_to_fortran_file_and_exit(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    vsprintf(__write_buf,msg,ap);
    va_end(ap);
    int ounit=the_egsio->i_log;
    F77_OBJ_(egs_write_string,EGS_WRITE_STRING)(&ounit,__write_buf,
            strlen(__write_buf));
    exit(1);
}

extern __extc__  void F77_OBJ_(egs_set_defaults,EGS_SET_DEFAULTS)();
extern __extc__  void F77_OBJ_(egs_init1,EGS_INIT1)();
int EGS_AdvancedApplication::initEGSnrcBackEnd() {
    F77_OBJ_(egs_set_defaults,EGS_SET_DEFAULTS)();
    __egs_iovar(64,app_name.size(),app_name.c_str(),the_egsio->user_code);
    __egs_iovar(128,hen_house.size(),hen_house.c_str(),the_egsio->hen_house);
    __egs_iovar(128,egs_home.size(),egs_home.c_str(),the_egsio->egs_home);
    __egs_iovar(256,input_file.size(),input_file.c_str(),the_egsio->input_file);
    __egs_iovar(256,final_output_file.size(),final_output_file.c_str(),
                the_egsio->output_file);
    __egs_iovar(256,pegs_file.size(),pegs_file.c_str(),the_egsio->pegs_file);
    the_egsio->i_parallel = i_parallel;
    the_egsio->n_parallel = n_parallel;
    the_egsio->is_pegsless = is_pegsless;
    if (run) {
        the_egsio->n_chunk = run->getNchunk();
    }
    the_egsio->is_batch = (int) batch_run;
    F77_OBJ_(egs_init1,EGS_INIT1)();
    /*
    if( batch_run ) {
        egsSetInfoFunction(Information,__write_to_fortran_file);
        egsSetInfoFunction(Warning,__write_to_fortran_file);
        egsSetInfoFunction(Fatal,__write_to_fortran_file_and_exit);
    }
    */
    io_flag = 1;
    return 0;
}

/* The following 2 functions were used in an attempt to redirect
   all fortran I/O to use egsInformationn a degsWarning.
   Unfortunately, there were segmentation violations in the
   fortran I/O functions that I don't understand.

extern __extc__  void F77_OBJ_(egs_info_output,EGS_INFO_OUTPUT)
    (char *msg, int len) {
    int j; for(j=len-1; j>=0; j--) if( !isspace(msg[j]) ) break;
    if( j<len-1 ) ++j;
    char c = msg[j]; msg[j] = 0;
    egsInformation("%s\n",msg);
    msg[j] = c;
}

extern __extc__  void F77_OBJ_(egs_warning_output,EGS_WARNING_OUTPUT)
    (char *msg, int len) {
    int j; for(j=len-1; j>=0; j--) if( !isspace(msg[j]) ) break;
    if( j<len-1 ) ++j;
    char c = msg[j]; msg[j] = 0;
    egsWarning("%s\n",msg);
    msg[j] = c;
}
*/

#ifndef SKIP_DOXYGEN
/*!
  \brief A helper class used in EGS_AdvancedApplication::initCrossSections()
  and EGS_AdvancedApplication::describeSimulation().

  \internwarning

*/
class EGS_LOCAL EGS_TransportProperty {

    string name;
    vector<string> options;
    vector<string> *str_v;
    vector<EGS_Float> *f_v;
    string     charinp;
    int        type;
    EGS_I32   *int_input;
    EGS_Float *float_input;
    int        len,nitem;
    char      *char_input;

public:

    /* Integer input type = 0 */
    EGS_TransportProperty(const char *Name, EGS_I32 *val) :
        name(Name), type(0), int_input(val) {};
    /* Real input type = 1 */
    EGS_TransportProperty(const char *Name, EGS_Float *val) :
        name(Name), type(1), float_input(val) {};
    /* Character string input type = 3 */
    EGS_TransportProperty(const char *Name, int L, char *val) :
        name(Name), type(3), len(L), char_input(val)  {};
    /* Character string array input type = 4 */
    EGS_TransportProperty(const char *Name,int L,int N,vector<string> *str):
        name(Name), str_v(str), type(4), len(L), nitem(N) {};
    /* Real array input type = 5 */
    EGS_TransportProperty(const char *Name,int N,vector<EGS_Float> *f):
        name(Name), f_v(f), type(5), nitem(N) {};
    /* Integer input with allowed values => type = 2 */
    void addOption(const char *opt) {
        options.push_back(opt);
        type = 2;
    };
    void getInput(EGS_Input *input) {
        if (type == 1) {
            EGS_Float aux;
            int err = input->getInput(name,aux);
            if (!err) {
                *float_input = aux;
            }
        }
        else if (type == 0) {
            EGS_I32 aux;
            int err = input->getInput(name,aux);
            if (!err) {
                *int_input = aux;
            }
        }
        else if (type == 3) {
            int err = input->getInput(name,charinp);
            if (!err) {
                int l = charinp.size();
                if (l > len) {
                    l = len;
                }
                int j;
                for (j=0; j<l; j++) {
                    char_input[j] = charinp[j];
                }
                for (j=l; j<len; j++) {
                    char_input[j] = ' ';
                }
            }
            else {
                charinp = string(char_input).substr(0,len);
            }
        }
        else if (type == 4) {
            int err = input->getInput(name,*str_v);
            if (!err) {
                vector<string> str = *str_v;
                str_v->clear();
                int number = str.size();
                if (number>nitem) {
                    str.erase(str.begin()+nitem,str.end());
                    number=nitem;
                }
                for (int i=0; i<number; i++) {
                    if (str[i].size()>len) {
                        str[i] = str[i].substr(0,len);
                    }
                }
                *str_v = str;
            }
        }
        else if (type == 5) {
            int err = input->getInput(name,*f_v);
            if (!err) {
                vector<EGS_Float> fv = *f_v;
                f_v->clear();
                int number = fv.size();
                if (number>nitem) {
                    fv.erase(fv.begin()+nitem,fv.end());
                    number=nitem;
                }
                *f_v = fv;
            }
        }
        else {
            bool is_ok;
            EGS_I32 iaux = input->getInput(name,options,0,&is_ok);
            if (is_ok) {
                *int_input = iaux;
            }
            egsWarning("Property %s: %d\n",name.c_str(),*int_input);
        }
    };
    void info(int nc) {
        egsInformation("%s",name.c_str());
        for (int j=name.size(); j<nc; j++) {
            egsInformation(" ");
        }
        if (type == 0) {
            egsInformation("%d\n",*int_input);
        }
        else if (type == 1) {
            egsInformation("%g\n",*float_input);
        }
        else if (type == 3) {
            egsInformation("%s\n",(string(char_input).substr(0,len)).c_str());
        }
        else if (type == 5) {
            for (vector<EGS_Float>::iterator it = f_v->begin() ; it != f_v->end(); ++it) {
                egsInformation("%g ",*it);
            }
            egsInformation("\n");
        }
        else {
            egsInformation("%s\n",options[*int_input].c_str());
        }
    };
    int size() {
        if (type == 1) {
            return sizeof(EGS_Float);
        }
        else if (type == 3) {
            return len;
        }
        else if (type == 4) {
            return nitem;
        }
        else if (type == 5) {
            return f_v->size();
        }
        else {
            return sizeof(EGS_I32);
        }
    };

};
#endif

EGS_AdvancedApplication::EGS_AdvancedApplication(int argc, char **argv) :
    EGS_Application(argc,argv), nmed(0), n_rng_buffer(0), final_job(false), io_flag(0) { }

EGS_AdvancedApplication::~EGS_AdvancedApplication() {
    if (n_rng_buffer > 0) {
        delete [] rng_buffer;
    }
}

void EGS_AdvancedApplication::describeSimulation() {
    EGS_Application::describeSimulation();
    if (final_job) {
        helpInit(0,false);
    }
}

int EGS_AdvancedApplication::initCrossSections() {
    egsWarning("In initCrossSections(): spin effects = %d\n",
               the_xoptions->spin_effects);
    EGS_Input *transportp = input->getInputItem("MC transport parameter");
    if (!transportp) {
        transportp = input->getInputItem("transport parameter");
    }
    return helpInit(transportp,true);
}

extern "C" void F77_OBJ_(set_elastic_parameter,SET_ELASTIC_PARAMETER)();

int EGS_AdvancedApplication::helpInit(EGS_Input *transportp, bool do_hatch) {
    if (!geometry) {
        egsWarning("initCrossSections(): no geometry?\n");
        return 1;
    }
    EGS_BaseGeometry::setActiveGeometryList(app_index);
    int nmed_new = EGS_BaseGeometry::nMedia(), j;
    if (nmed_new < 1) {
        egsWarning("initCrossSections(): no media in this geometry?\n");
        return 2;
    }
    int nmed_old = nmed;
    nmed = nmed_new;
    int *ind = new int [nmed];
    for (j=0; j<nmed; j++) {
        const char *medname = geometry->getMediumName(j);
        int len = strlen(medname);
        ind[j] = egsAddMedium(medname, len);
    }

    vector<string> ff_media, ff_names;
    string eii_xsect;
    vector<EGS_Float> efield_v, bfield_v;
    //
    // The Intel compiler uses -1 for .true. => all logical
    // variables don't work!
    // This is a fix for this
    //
    if (the_xoptions->spin_effects != 0) {
        the_xoptions->spin_effects=1;
    }
    //if( the_xoptions->exact_bca != 0 ) the_xoptions->exact_bca=1;

    //
    // See if there are transport parameters set in the input file.
    //
    EGS_TransportProperty efield("Electric Field",3,&efield_v);
    EGS_TransportProperty bfield("Magnetic Field",3,&bfield_v);
    EGS_TransportProperty estepem("EM ESTEPE",&the_emf->EMLMTIN);
    EGS_TransportProperty ecut("Global Ecut",&the_bounds->ecut);
    EGS_TransportProperty pcut("Global Pcut",&the_bounds->pcut);
    EGS_TransportProperty smax("Global Smax",&the_etcontrol->smaxir);
    EGS_TransportProperty estep("ESTEPE",&the_etcontrol->estepe);
    EGS_TransportProperty ximax("Ximax",&the_etcontrol->ximax);
    EGS_TransportProperty skind("Skin depth for BCA",
                                &the_etcontrol->skindepth_for_bca);
    EGS_TransportProperty pxsec("Photon cross sections",16,the_media->pxsec);
    EGS_TransportProperty pxsec_out("Photon cross-sections output",&the_egsio->xsec_out);
    pxsec_out.addOption("Off");
    pxsec_out.addOption("On");
    EGS_TransportProperty cxsec("Compton cross sections",16,the_media->compxsec);
    EGS_TransportProperty bc("Bound Compton scattering",&the_xoptions->ibcmp);
    bc.addOption("Off");
    bc.addOption("On");
    bc.addOption("Simple");
    bc.addOption("norej");
    EGS_TransportProperty radc("Radiative Compton corrections",
                               &the_xoptions->radc_flag);
    radc.addOption("Off");
    radc.addOption("On");
    /* Photonuclear input parameters */
    EGS_TransportProperty photonucxsec("Photonuclear cross sections",16,the_media->photonucxsec);
    EGS_TransportProperty photonuc("Photonuclear attenuation",&the_xoptions->iphotonuc);
    photonuc.addOption("Off");
    photonuc.addOption("On");

    EGS_TransportProperty rayl("Rayleigh scattering",&the_xoptions->iraylr);
    rayl.addOption("Off");
    rayl.addOption("On");
    rayl.addOption("custom");
    EGS_TransportProperty ff_med("ff media names",24,MXMED,&ff_media);
    EGS_TransportProperty ff_files("ff file names",128,MXMED, &ff_names);
    EGS_TransportProperty relax("Atomic relaxations",&the_xoptions->iedgfl);
    relax.addOption("Off");
    relax.addOption("On");
    EGS_TransportProperty iphter("Photoelectron angular sampling",
                                 &the_xoptions->iphter);
    iphter.addOption("Off");
    iphter.addOption("On");
    EGS_TransportProperty spin("Spin effects",&the_xoptions->spin_effects);
    spin.addOption("Off");
    spin.addOption("On");
    EGS_TransportProperty trip("Triplet production",&the_xoptions->itriplet);
    trip.addOption("Off");
    trip.addOption("On");
    EGS_TransportProperty eii("Electron Impact Ionization",16,the_media->eiixsec);
    EGS_TransportProperty brem("Brems cross sections",&the_xoptions->ibr_nist);
    brem.addOption("BH");
    brem.addOption("NIST");
    brem.addOption("NRC");
    EGS_TransportProperty bang("Brems angular sampling",&the_xoptions->ibrdst);
    bang.addOption("Simple");
    bang.addOption("KM");
    EGS_TransportProperty pair("Pair cross sections",&the_xoptions->pair_nrc);
    pair.addOption("BH");
    pair.addOption("NRC");
    EGS_TransportProperty pang("Pair angular sampling",&the_xoptions->iprdst);
    pang.addOption("Off");
    pang.addOption("Simple");
    pang.addOption("KM");
    pang.addOption("Uniform");
    pang.addOption("Blend");
    EGS_TransportProperty tran("Electron-step algorithm",
                               &the_etcontrol->transport_algorithm);
    tran.addOption("EGSnrc");
    tran.addOption("PRESTA-I");
    tran.addOption("PRESTA-II");
    tran.addOption("default");
    EGS_TransportProperty bca("Boundary crossing algorithm",
                              &the_etcontrol->bca_algorithm);
    bca.addOption("Exact");
    bca.addOption("PRESTA-I");

    if (transportp) {
        efield.getInput(transportp);
        bfield.getInput(transportp);
        estepem.getInput(transportp);
        ecut.getInput(transportp);
        pcut.getInput(transportp);
        smax.getInput(transportp);
        the_etcontrol->smax_new = the_etcontrol->smaxir;
        estep.getInput(transportp);
        ximax.getInput(transportp);
        skind.getInput(transportp);
        pxsec.getInput(transportp);
        pxsec_out.getInput(transportp);
        cxsec.getInput(transportp);
        photonucxsec.getInput(transportp);
        photonuc.getInput(transportp);
        bc.getInput(transportp);
        radc.getInput(transportp);
        rayl.getInput(transportp);
        if (the_xoptions->iraylr>1) {
            ff_med.getInput(transportp);
            ff_files.getInput(transportp);
            setRayleighData(ff_media,ff_names);
        }
        relax.getInput(transportp);
        iphter.getInput(transportp);
        spin.getInput(transportp);
        trip.getInput(transportp);
        eii.getInput(transportp);
        setEIIData(eii.size());
        brem.getInput(transportp);
        bang.getInput(transportp);
        pair.getInput(transportp);
        pang.getInput(transportp);
        tran.getInput(transportp);
        if (the_etcontrol->transport_algorithm > 1) {
            the_etcontrol->transport_algorithm = 0;
        }
        bca.getInput(transportp);
    }

    if (do_hatch) {

        egsHatch();
        F77_OBJ_(set_elastic_parameter,SET_ELASTIC_PARAMETER)();

        the_bounds->ecut_new = the_bounds->ecut;
        the_bounds->pcut_new = the_bounds->pcut;

        if (nmed_old > 0) {
            delete [] i_ededx;
            delete [] i_pdedx;
            delete [] i_esig;
            delete [] i_psig;
            delete [] i_ebr1;
            delete [] i_pbr1;
            delete [] i_pbr2;
            delete [] i_gmfp;
            delete [] i_gbr1;
            delete [] i_gbr2;
            delete [] i_cohe;
        }
        if (nmed > 0) {
            i_ededx = new EGS_Interpolator [nmed];
            i_pdedx = new EGS_Interpolator [nmed];
            i_esig  = new EGS_Interpolator [nmed];
            i_psig  = new EGS_Interpolator [nmed];
            i_ebr1  = new EGS_Interpolator [nmed];
            i_pbr1  = new EGS_Interpolator [nmed];
            i_pbr2  = new EGS_Interpolator [nmed];
            i_gmfp  = new EGS_Interpolator [nmed];
            i_gbr1  = new EGS_Interpolator [nmed];
            i_gbr2  = new EGS_Interpolator [nmed];
            i_cohe  = new EGS_Interpolator [nmed];
            i_photonuc = new EGS_Interpolator [nmed];
            for (int imed=0; imed<nmed; imed++) {
                int type;
                int imed1 = imed+1;
                type = 3;
                __init_interpolator(1,imed1,type,i_ededx[imed]);
                type = 4;
                __init_interpolator(1,imed1,type,i_pdedx[imed]);
                type = 1;
                __init_interpolator(1,imed1,type,i_esig[imed]);
                type = 2;
                __init_interpolator(1,imed1,type,i_psig[imed]);
                type = 5;
                __init_interpolator(1,imed1,type,i_ebr1[imed]);
                type = 6;
                __init_interpolator(1,imed1,type,i_pbr1[imed]);
                type = 7;
                __init_interpolator(1,imed1,type,i_pbr2[imed]);
                type = 1;
                __init_interpolator(0,imed1,type,i_gmfp[imed]);
                type = 2;
                __init_interpolator(0,imed1,type,i_gbr1[imed]);
                type = 3;
                __init_interpolator(0,imed1,type,i_gbr2[imed]);
                type = 4;
                __init_interpolator(0,imed1,type,i_cohe[imed]);
                type = 5;
                __init_interpolator(0,imed1,type,i_photonuc[imed]);
            }
        }
    }

    egsInformation("\n\nThe following media are defined:\n"
                   "================================\n\n");
    EGS_Float Emax = source ? source->getEmax() : 0;
    bool data_ok = true;
    for (j=0; j<nmed; j++) {
        int imed = ind[j]-1;
        egsInformation("%3d  %-24s AE=%7.4f AP=%7.4f %d\n",j,
                       geometry->getMediumName(j),the_thresh->ae[imed],
                       the_thresh->ap[imed],imed);
        if (Emax > the_thresh->ue[imed]-0.511 ||
                Emax > the_thresh->up[imed]) {
            egsInformation("  upper limits (UE=%g UP=%g) not enough for "
                           "maximum source energy (%g)\n",the_thresh->ue[imed],
                           the_thresh->up[imed],Emax);
            data_ok = false;
        }
    }
    if (!data_ok) {
        delete [] ind;
        return 3;
    }

    egsInformation("\n\nTransport parameter and cross section options:\n"
                   "==============================================\n");
    int nc = 50;
    if (!isspace(the_media->pxsec[0])) {
        pxsec.info(nc);
    }
    if (!isspace(the_media->compxsec[0])) {
        cxsec.info(nc);
    }
    pcut.info(nc);
    pair.info(nc);
    pang.info(nc);
    trip.info(nc);
    bc.info(nc);
    radc.info(nc);
    rayl.info(nc);
    relax.info(nc);
    iphter.info(nc);
    photonuc.info(nc);
    photonucxsec.info(nc);
    egsInformation("\n");
    ecut.info(nc);
    brem.info(nc);
    bang.info(nc);
    spin.info(nc);
    eii.info(nc);
    smax.info(nc);
    estep.info(nc);
    ximax.info(nc);
    bca.info(nc);
    skind.info(nc);
    tran.info(nc);
    if (efield.size()==3) {
        efield.info(nc);
        the_emf->ExIN=efield_v[0];
        the_emf->EyIN=efield_v[1];
        the_emf->EzIN=efield_v[2];
    }
    if (bfield.size()==3) {
        bfield.info(nc);
        the_emf->BxIN=bfield_v[0];
        the_emf->ByIN=bfield_v[1];
        the_emf->BzIN=bfield_v[2];
    }
    if (efield.size()==3 || bfield.size()==3) {
        estepem.info(nc);
    }

    egsInformation("==============================================\n\n");

    delete [] ind;
    return 0;
}

/**********************************************************************
   Set media and corresponding ff file names for custom Rayleigh data

   Since the_xoptions->iraylr is reset to 1 here, output will only
   indicate coherent scatter is on, but not whether custom input
   requested. Perhaps should make EGS check for iraylm(medium)>0
   instead of iraylm(medium)=1. The latter has been kept over
   the years for historic reasons.
                                                    EMH April 6th 2011
**********************************************************************/
void EGS_AdvancedApplication::setRayleighData(
    const vector<string> &str_medium,
    const vector<string> &str_file) {
    the_xoptions->iraylr=1;
    int i,k;
    for (i=0; i<str_medium.size(); i++) {
        for (k=0; k<str_medium[i].size(); k++) {
            the_rayleigh->ff_media[i][k]= str_medium[i][k];
        }
    }
    for (i=0; i<str_file.size(); i++) {
        for (k=0; k<str_file[i].size(); k++) {
            the_rayleigh->ff_file[i][k]=str_file[i][k];
        }
    }
}

/**********************************************************************
   Set EII flag and xsection file name.

   The EII input is of a mixed type, i.e., one should be able to turn
   this option on/off, but there is also the possibility of using an
   arbitrary EII xsection compilation, including the available EII
   xsections with EGSnrc.
                                                    EMH April 6th 2011
**********************************************************************/
void EGS_AdvancedApplication::setEIIData(EGS_I32 len) {
    string str_eii,off,on;
    the_xoptions->eii_flag = 1;
    string ik="ik";
    for (int i=0; i<len; i++) {
        str_eii += toupper(the_media->eiixsec[i]);
        off+=' ';
        on+=' ';
    }
    off[0]='O';
    off[1]='F';
    off[2]='F';
    on[0]='O';
    on[1]='N';
    if (str_eii == off) {
        the_xoptions->eii_flag = 0;
    }
    else if (str_eii == on) {
        strcpy(the_media->eiixsec,ik.c_str());
    }
}

#ifdef GDEBUG
    #define MAX_STEP 100000
    EGS_Vector steps_x[MAX_STEP], steps_u[MAX_STEP];
    int steps_ireg[MAX_STEP], steps_inew[MAX_STEP];
    EGS_Float steps_ustepi[MAX_STEP], steps_ustepf[MAX_STEP];
    int steps_n = 0;
#endif

int EGS_AdvancedApplication::shower() {
    if (!p.wt) {
        return 0;
    }
    the_stack->E[0] = (p.q) ? p.E + the_useful->rm : p.E;
    the_stack->x[0] = p.x.x;
    the_stack->y[0] = p.x.y;
    the_stack->z[0] = p.x.z;
    the_stack->u[0] = p.u.x;
    the_stack->v[0] = p.u.y;
    the_stack->w[0] = p.u.z;
    the_stack->dnear[0] = 0;
    the_stack->wt[0] = p.wt;
    the_stack->ir[0] = p.ir + 2;
    the_stack->iq[0] = p.q;
    the_stack->latch[0] = p.latch;
    the_stack->np = 1;
#ifdef GDEBUG
    steps_n = 0;
#endif
    egsShower();
    return 0;
}

void EGS_AdvancedApplication::finishRun() {
    egsFinish();
    //egsSetDefaultIOFunctions();
    io_flag = 0;
    // so that if extra output is being produced somewhere, it does not
    // go to fort.6 as it would without this as all fortran units are
    // closed after egsFinish().
}

int EGS_AdvancedApplication::finishSimulation() {
    int err = EGS_Application::finishSimulation();
    egsInformation("finishSimulation(%s) %d\n",app_name.c_str(),err);
    if (err <= 0) {
        return err;
    }
    //egsInformation("\n\n********** I'm last job! **********\n\n");
    //return 0;
    // if err is positive, this is the last job in a parallel run
    // => we have to combine the results
    // EGS_Application::finishSimulation() has already called
    // our finishRun() which has called egs_finish => all data has been
    // moved to the user code directory and the working directory has been
    // reset to be the user code directory. We must now reset the
    // output_file name and re-open units.
    output_file = final_output_file;
    the_egsio->i_parallel = 0;
    int flag = 0;
    egsOpenUnits(&flag);
    // The following is necessary because finishRun() was called from
    // EGS_Application::finishSimulation() and this resets I/O
    // to stdout and stderr. But if we are here, we are combining
    // the results of a parallel run and we want the output to go
    // into the combined run log file.
    //egsSetInfoFunction(Information,__write_to_fortran_file);
    //egsSetInfoFunction(Warning,__write_to_fortran_file);
    //egsSetInfoFunction(Fatal,__write_to_fortran_file_and_exit);
    io_flag = 1;
    final_job = true;
    describeUserCode();
    describeSimulation();
    err = combineResults();
    if (err) {
        return err;
    }
    run->finishSimulation();
    for (int j=0; j<a_objects_list.size(); ++j) {
        a_objects_list[j]->reportResults();
    }
    outputResults();
    finishRun();
    return 0;
}

int EGS_AdvancedApplication::outputData() {
    int err = EGS_Application::outputData();
    if (err) {
        return err;
    }
    EGS_I32 np, ip;
    egsGetRNGPointers(&np,&ip);
    if (np < 1) {
        return 11;
    }
    if (np > 10000000) {
        egsWarning("EGS_AdvancedApplication::outputData(): egsGetRNGPointers"
                   " returns a huge number? (%d)\n",np);
        return 12;
    }
    EGS_Float *array = new EGS_Float [np];
    egsGetRNGArray(array);
    (*data_out) << "  " << np << "  " << ip << endl;
    for (int j=0; j<np; j++) {
        (*data_out) << array[j] << " ";
    }
    (*data_out) << endl;
    double ch_steps, all_steps;
    egsGetSteps(&ch_steps,&all_steps);
    (*data_out) << ch_steps << "  " << all_steps << endl;
    delete [] array;
    return data_out->good() ? 0 : 13;
}

int EGS_AdvancedApplication::readData() {
    int err = EGS_Application::readData();
    if (err) {
        return err;
    }
    int np, ip;
    (*data_in) >> np >> ip;
    if (np < 1) {
        return 11;
    }
    if (np > 10000000) {
        egsWarning("EGS_AdvancedApplication::readData(): got huge size "
                   "for the mortran random array? (%d)\n",np);
        return 12;
    }
    EGS_Float *array = new EGS_Float [np];
    for (int j=0; j<np; j++) {
        (*data_in) >> array[j];
    }
    if (!data_in->good()) {
        return 13;
    }
    egsSetRNGState(&ip,array);
    delete [] array;
    double ch_steps, all_steps;
    (*data_in) >> ch_steps >> all_steps;
    egsSetSteps(&ch_steps,&all_steps);
    return data_in->good() ? 0 : 13;
}

int EGS_AdvancedApplication::addState(istream &data) {
    int err = EGS_Application::addState(data);
    if (err) {
        return err;
    }
    int np, ip;
    data >> np >> ip;
    if (np < 1) {
        egsWarning("Got np=%d\n",np);
        return 11;
    }
    if (np > 10000000) {
        egsWarning("EGS_AdvancedApplication::addState(): got huge size "
                   "for the mortran random array? (%d)\n",np);
        return 12;
    }
    EGS_Float *array = new EGS_Float [np];
    for (int j=0; j<np; j++) {
        data >> array[j];
    }
    if (!data.good()) {
        return 13;
    }
    egsSetRNGState(&ip,array);
    delete [] array;
    double ch_steps, all_steps;
    data >> ch_steps >> all_steps;
    double ch_steps_old, all_steps_old;
    egsGetSteps(&ch_steps_old,&all_steps_old);
    ch_steps += ch_steps_old;
    all_steps += all_steps_old;
    egsSetSteps(&ch_steps,&all_steps);
    return data.good() ? 0 : 13;
}

void EGS_AdvancedApplication::resetCounter() {
    EGS_Application::resetCounter();
    double ch_steps = 0, all_steps = 0;
    egsSetSteps(&ch_steps,&all_steps);
}

EGS_I64 EGS_AdvancedApplication::randomNumbersUsed() const {
    EGS_I64 nused = EGS_Application::randomNumbersUsed();
    EGS_I32 np, ip;
    egsGetRNGPointers(&np,&ip);
    if (ip <= np) {
        nused -= (np+1-ip);
    }
    return nused;
}

void EGS_AdvancedApplication::appInformation(const char *msg) {
    if (!msg) {
        return;
    }
    if (!io_flag) {
        EGS_Application::appInformation(msg);
    }
    else {
        int ounit = the_egsio->i_log;
        F77_OBJ_(egs_write_string,EGS_WRITE_STRING)(&ounit,msg,strlen(msg));
        checkDeviceFull(stdout);
    }
}

void EGS_AdvancedApplication::appWarning(const char *msg) {
    if (!msg) {
        return;
    }
    if (!io_flag) {
        EGS_Application::appWarning(msg);
    }
    else {
        int ounit = the_egsio->i_log;
        F77_OBJ_(egs_write_string,EGS_WRITE_STRING)(&ounit,msg,strlen(msg));
        checkDeviceFull(stdout);
    }
}

void EGS_AdvancedApplication::appFatal(const char *msg) {
    if (!io_flag) {
        EGS_Application::appFatal(msg);
    }
    else {
        if (msg) {
            int ounit = the_egsio->i_log;
            F77_OBJ_(egs_write_string,EGS_WRITE_STRING)(&ounit,msg,strlen(msg));
        }
        exit(1);
    }
}

void EGS_AdvancedApplication::getElectronSteps(double &ch_steps,
        double &all_steps) const {
    egsGetSteps(&ch_steps,&all_steps);
}

void EGS_AdvancedApplication::startNewParticle() {
    if (geometry->hasRhoScaling()) {
        int ireg = the_stack->ir[the_stack->np-1] - 2;
        EGS_Float rho = geometry->getRelativeRho(ireg);
        the_useful->rhor = rho;
        the_useful->rhor_new = rho;
    }
    else {
        the_useful->rhor = 1;
        the_useful->rhor_new = 1;
    }
}

void EGS_AdvancedApplication::enterNewRegion() {
    if (geometry->hasRhoScaling()) {
        int ireg = the_epcont->irnew-2;
        if (ireg >= 0) {
            EGS_Float rho = geometry->getRelativeRho(ireg);
            the_useful->rhor_new = rho;
        }
    }
    else {
        the_useful->rhor_new = 1;
    }
}

void EGS_AdvancedApplication::saveRNGState() {
    rndm->saveState();
    int np,ip;
    egsGetRNGPointers(&np,&ip);
    if (n_rng_buffer > 0) {
        if (np != n_rng_buffer) egsFatal("EGS_AdvancedApplication::saveRNGState:\n"
                                             "  new buffer size (%d) is not the same as previous size(%d) ?\n",
                                             np,n_rng_buffer);
    }
    else {
        rng_buffer = new EGS_Float [np];
        n_rng_buffer = np;
    }
    i_rng_buffer = ip;
    egsGetRNGArray(rng_buffer);
}

void EGS_AdvancedApplication::resetRNGState() {
    rndm->resetState();
    if (n_rng_buffer > 0) {
        egsSetRNGState(&i_rng_buffer,rng_buffer);
    }
}

//************************************************************
// Utility functions for use with ausgab dose scoring objects
//************************************************************
// Returns density for medium ind
EGS_Float EGS_AdvancedApplication::getMediumRho(int ind) {
    return the_media->rho[ind];
}
// Returns edep
EGS_Float EGS_AdvancedApplication::getEdep() {
    return the_epcont->edep;
}
//************************************************************

extern __extc__ void egsHowfar() {
    CHECK_GET_APPLICATION(app,"egsHowfar()");
    int np = the_stack->np-1;
    int ireg = the_stack->ir[np]-2;
    if (ireg < 0 || the_stack->wt[np] == 0) {
        the_epcont->idisc = 1;
        return;
    }
    int newmed;
    int inew = app->howfar(ireg,
                           EGS_Vector(the_stack->x[np],the_stack->y[np],the_stack->z[np]),
                           EGS_Vector(the_stack->u[np],the_stack->v[np],the_stack->w[np]),
                           the_epcont->ustep,&newmed);
#ifdef GDEBUG
    EGS_Float tsave = the_epcont->ustep;
    if (steps_n < MAX_STEP) {
        steps_x[steps_n] =
            EGS_Vector(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
        steps_u[steps_n] =
            EGS_Vector(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
        steps_ireg[steps_n] = ireg;
        steps_inew[steps_n] = inew;
        steps_ustepi[steps_n] = tsave;
        steps_ustepf[steps_n++] = the_epcont->ustep;
    }
    if (the_epcont->ustep < -1e-4) {
        egsWarning("Negative step: %g\n",the_epcont->ustep);
        for (int j=0; j<steps_n; j++)
            egsWarning("%d x=(%g,%g,%g) u=(%g,%g,%g) ireg=%d inew=%d ustepi=%g"
                       " ustepf=%g\n",j,steps_x[j].x,steps_x[j].y,steps_x[j].z,
                       steps_u[j].x,steps_u[j].y,steps_u[j].z,
                       steps_ireg[j],steps_inew[j],steps_ustepi[j],
                       steps_ustepf[j]);
        exit(1);
    }
#endif
    if (inew != ireg) {
        the_epcont->irnew = inew+2;
        the_useful->medium_new = newmed+1;
        app->enterNewRegion();
        if (inew < 0) {
            the_epcont->idisc = -1; // i.e. discard after the step.
            the_useful->medium_new = 0;
        }
    }
}

extern __extc__ void egsHownear(EGS_Float *tperp) {
    CHECK_GET_APPLICATION(app,"egsHownear()");
    int np = the_stack->np-1;
    int ireg = the_stack->ir[np]-2;
    *tperp = app->hownear(ireg,EGS_Vector(the_stack->x[np],the_stack->y[np],
                                          the_stack->z[np]));
}

extern __extc__ void egsFillRandomArray(const EGS_I32 *n, EGS_Float *rarray) {
    CHECK_GET_APPLICATION(app,"egsFillRandomArray()");
    app->fillRandomArray(*n,rarray);
}

//extern __extc__ int egsAusgab(const EGS_I32 *iarg) {
//    CHECK_GET_APPLICATION(app,"egsAusgab()");
//    return app->ausgab(*iarg);
//}
extern __extc__ void egsAusgab(EGS_I32 *iarg) {
    CHECK_GET_APPLICATION(app,"egsAusgab()");
    //*iarg = app->ausgab(*iarg);
    int np = the_stack->np-1;
    app->top_p.E = the_stack->E[np];
    app->top_p.wt = the_stack->wt[np];
    app->top_p.x = EGS_Vector(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
    app->top_p.u = EGS_Vector(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
    app->top_p.q = the_stack->iq[np];
    app->top_p.ir = the_stack->ir[np]-2;
    app->top_p.latch = the_stack->latch[np];
    app->Np = the_stack->np-1;
    *iarg = app->userScoring(*iarg);
}

extern __extc__ void egsStartParticle() {
    CHECK_GET_APPLICATION(app,"egsStartParticle()");
    int np = the_stack->np - 1;
    int ir = the_stack->ir[np]-2;
    if (ir < 0) {
        the_epcont->idisc = 1;
        return;
    }
    the_epcont->idisc = 0;
    the_useful->medium = app->getMedium(ir)+1;
    //egsInformation("start particle: ir=%d medium=%d\n",ir,the_useful->medium);
    app->startNewParticle();
}


