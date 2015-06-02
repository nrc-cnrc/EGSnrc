/*
###############################################################################
#
#  EGSnrc functions to load a BEAMnrc shared library
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
#  Author:          Iwan Kawrakow, 2004
#  
#  Contributors:    Blake Walters
#                   Ernesto Mainegra-Hing
#                   Frederic Tessier
#
###############################################################################
#
#  Functions to load a BEAMnrc library and to resolve the init, sample 
#  and finish functions.
#
###############################################################################
*/


#include "egs_config1.h"

/*
 To be able to provide pre-compiled object files for 
 Windows, we use the following.
*/

#ifdef MAKE_WIN_DISTRIBUTION
#ifdef MY_STDCALL
#define MYF77OBJ(fname,FNAME) __stdcall fname
#define MYF77OBJ_(fname,FNAME) __stdcall fname
#define C_CONVENTION __stdcall
#else
#define MYF77OBJ(fname,FNAME) fname
#define MYF77OBJ_(fname,FNAME) fname
#define C_CONVENTION
#endif
#else
#define MYF77OBJ(fname,FNAME) F77_OBJ(fname,FNAME)
#define MYF77OBJ_(fname,FNAME) F77_OBJ_(fname,FNAME)
#endif

/*
#include "egs_config1.h"
*/

#ifdef WIN32  
#include <windows.h>
#define DLL_HANDLE HMODULE
#define GET_PROC_ADDRESS GetProcAddress
static char fs = '\\';
static char lib_pre[] = "";
static char lib_ext[] = ".dll";
#else
#include <dlfcn.h>
#include <unistd.h>
#define DLL_HANDLE void*
#define GET_PROC_ADDRESS dlsym
static char fs = '/';
static char lib_pre[] = "lib";
static char lib_ext[] = ".so";
#endif

static char bindir[] = "bin";

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 The following set of macros is for automatically creating the 
 appropriate function name
*/

#define STRINGIFY(s) HELP_S(s)
#define HELP_S(s) #s
#define F77_NAME(fname,FNAME) STRINGIFY(F77_OBJ(fname,FNAME))
#define F77_NAME_(fname,FNAME) STRINGIFY(F77_OBJ_(fname,FNAME))

#ifdef MAKE_WIN_DISTRIBUTION

static char beamlib_init_name1[] = "beamlib_init";
static char beamlib_init_name2[] = "beamlib_init_";
static char beamlib_init_name3[] = "beamlib_init__";
static char beamlib_init_name4[] = "BEAMLIB_INIT";
static char beamlib_init_name5[] = "BEAMLIB_INIT_";
static char beamlib_init_name6[] = "BEAMLIB_INIT__";
char *beamlib_init_name = beamlib_init_name1;

static char beamlib_sample_name1[] = "beamlib_sample";
static char beamlib_sample_name2[] = "beamlib_sample_";
static char beamlib_sample_name3[] = "beamlib_sample__";
static char beamlib_sample_name4[] = "BEAMLIB_SAMPLE";
static char beamlib_sample_name5[] = "BEAMLIB_SAMPLE_";
static char beamlib_sample_name6[] = "BEAMLIB_SAMPLE__";
char *beamlib_sample_name = beamlib_sample_name1;

/*JL: added to allow motion in BEAM libraries*/
static char beamlib_motionsample_name1[] = "beamlib_motionsample";
static char beamlib_motionsample_name2[] = "beamlib_motionsample_";
static char beamlib_motionsample_name3[] = "beamlib_motionsample__";
static char beamlib_motionsample_name4[] = "BEAMLIB_MOTIONSAMPLE";
static char beamlib_motionsample_name5[] = "BEAMLIB_MOTIONSAMPLE_";
static char beamlib_motionsample_name6[] = "BEAMLIB_MOTIONSAMPLE__";
char *beamlib_motionsample_name = beamlib_motionsample_name1;

/*JL: added to give phsp particles to BEAM libraries and allow motion*/
static char beamlib_phspmotionsample_name1[] = "beamlib_phspmotionsample";
static char beamlib_phspmotionsample_name2[] = "beamlib_phspmotionsample_";
static char beamlib_phspmotionsample_name3[] = "beamlib_phspmotionsample__";
static char beamlib_phspmotionsample_name4[] = "BEAMLIB_PHSPMOTIONSAMPLE";
static char beamlib_phspmotionsample_name5[] = "BEAMLIB_PHSPMOTIONSAMPLE_";
static char beamlib_phspmotionsample_name6[] = "BEAMLIB_phspMOTIONSAMPLE__";
char *beamlib_phspmotionsample_name = beamlib_phspmotionsample_name1;

static char beamlib_finish_name1[] = "beamlib_finish";
static char beamlib_finish_name2[] = "beamlib_finish_";
static char beamlib_finish_name3[] = "beamlib_finish__";
static char beamlib_finish_name4[] = "BEAMLIB_FINISH";
static char beamlib_finish_name5[] = "BEAMLIB_FINISH_";
static char beamlib_finish_name6[] = "BEAMLIB_FINISH__";
char *beamlib_finish_name = beamlib_finish_name1;


static char beamlib_maxenergy_name1[] = "beamlib_max_energy";
static char beamlib_maxenergy_name2[] = "beamlib_max_energy_";
static char beamlib_maxenergy_name3[] = "beamlib_max_energy__";
static char beamlib_maxenergy_name4[] = "BEAMLIB_MAX_ENERGY";
static char beamlib_maxenergy_name5[] = "BEAMLIB_MAX_ENERGY_";
static char beamlib_maxenergy_name6[] = "BEAMLIB_MAX_ENERGY__";
char *beamlib_maxenergy_name = beamlib_maxenergy_name1;

#endif

DLL_HANDLE handle = 0;

typedef void (*InitFunction)(const int *, const int *,
                         const char *, const char *, const char *,
                         const char *, const char *, int,int,int,int,int);
typedef void (*FinishFunction)();
typedef void (*SampleFunction)(EGS_Float *, EGS_Float *, EGS_Float *,
           EGS_Float *, EGS_Float *, EGS_Float *, EGS_Float *, EGS_Float *,
           EGS_I32 *, EGS_I32 *, EGS_I64 *, EGS_I32 *);
/*JL: added to allow motion in BEAM libraries*/
typedef void (*MotionSampleFunction)(EGS_Float *, EGS_Float *, EGS_Float *,
           EGS_Float *, EGS_Float *, EGS_Float *, EGS_Float *, EGS_Float *,
           EGS_I32 *, EGS_I32 *, EGS_I64 *, EGS_I32 *,EGS_Float *);
/*JL: added to give phsp particles to BEAM and allow motion*/
typedef void (*PhspMotionSampleFunction)(EGS_Float *, EGS_Float *, EGS_Float *, 
		     EGS_Float *, EGS_Float *, EGS_Float *, EGS_Float *, 
		     EGS_Float *, EGS_I32 *, EGS_I32 *, EGS_I64 *, 
		     EGS_I32 *,EGS_I32 *, EGS_Float *);
typedef void (*MaxEnergyFunction)(EGS_Float *);

InitFunction   libinit = 0;
SampleFunction libsample = 0;
/*T Popescu and J Lobo: added to allow motion in BEAM libraries*/
MotionSampleFunction libmotionsample = 0;
/*JL: added to give phsp particles to BEAM and allow motion*/
PhspMotionSampleFunction libphspmotionsample = 0;
FinishFunction libfinish = 0;
MaxEnergyFunction libmaxenergy = 0;

void unload_library() {
    if( handle ) 
#ifdef WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
    handle = 0;
    libinit = 0; libsample = 0; libmotionsample=0; libfinish = 0;
}

#ifdef __cplusplus 
extern "C" {
#endif

void MYF77OBJ_(sample_beamsource,SAMPLE_BEAMSOURCE)(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat) {
    if( !handle || !libsample ) {
        printf("You have to load and initialize the library first\n");
        exit(1);
    }
    libsample(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat);
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION sample_beamsource_(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat) {
    sample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat);
}
void C_CONVENTION sample_beamsource__(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat) {
    sample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat);
}
void C_CONVENTION SAMPLE_BEAMSOURCE(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat) {
    sample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat);
}
void C_CONVENTION SAMPLE_BEAMSOURCE_(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat) {
    sample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat);
}
void C_CONVENTION SAMPLE_BEAMSOURCE__(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat) {
    sample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat);
}
#endif

/*JL: added to allow motion in BEAM libraries*/
void MYF77OBJ_(motionsample_beamsource,MOTIONSAMPLE_BEAMSOURCE)(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, EGS_Float *muindex) {
    if( !handle || !libmotionsample ) {
        printf("You have to load and initialize the library first\n");
        exit(1);
    }
    libmotionsample(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,muindex);
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION motionsample_beamsource_(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, EGS_Float *muindex) {
    motionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,muindex);
}
void C_CONVENTION motionsample_beamsource__(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, EGS_Float *muindex) {
    motionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,muindex);
}
void C_CONVENTION MOTIONSAMPLE_BEAMSOURCE(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, EGS_Float *muindex) {
    motionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,muindex);
}
void C_CONVENTION MOTIONSAMPLE_BEAMSOURCE_(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, EGS_Float *muindex) {
    motionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,muindex);
}
void C_CONVENTION MOTIONSAMPLE_BEAMSOURCE__(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, EGS_Float *muindex) {
    motionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,muindex);
}
#endif

/*JL: added to give phsp particles to BEAM allow motion in BEAM libraries*/
void MYF77OBJ_(phspmotionsample_beamsource,PHSPMOTIONSAMPLE_BEAMSOURCE)(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, 
	EGS_I32 *still_in_container, EGS_Float *muindex) {
    if( !handle || !libphspmotionsample ) {
        printf("You have to load and initialize the library first\n");
        exit(1);
    }
    libphspmotionsample(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,still_in_container,muindex);

}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION phspmotionsample_beamsource_(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat,
	EGS_I32 *still_in_container, EGS_Float *muindex) {
    phspmotionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,still_in_container,muindex);
}
void C_CONVENTION phspmotionsample_beamsource__(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, 
	EGS_I32 *still_in_container, EGS_Float *muindex) {
    phspmotionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,still_in_container,muindex);
}
void C_CONVENTION PHSPMOTIONSAMPLE_BEAMSOURCE(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, 
	EGS_I32 *still_in_container, EGS_Float *muindex) {
    phspmotionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,still_in_container,muindex);
}
void C_CONVENTION PHSPMOTIONSAMPLE_BEAMSOURCE_(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, 
	EGS_I32 *still_in_container, EGS_Float *muindex) {
    phspmotionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,still_in_container,muindex);
}
void C_CONVENTION PHSPMOTIONSAMPLE_BEAMSOURCE__(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist, EGS_I32 *iphat, 
	EGS_I32 *still_in_container, EGS_Float *muindex) {
    phspmotionsample_beamsource(e,x,y,z,u,v,w,wt,iq,latch,nhist,iphat,still_in_container,muindex);
}
#endif

void MYF77OBJ_(maxenergy_beamsource,MAXENERGY_BEAMSOURCE)(EGS_Float *E) {
    if( libmaxenergy ) libmaxenergy(E);
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION maxenergy_beamsource_(EGS_Float *E) 
       { maxenergy_beamsource(E); }
void C_CONVENTION maxenergy_beamsource__(EGS_Float *E) 
       { maxenergy_beamsource(E); }
void C_CONVENTION MAXENERGY_BEAMSOURCE(EGS_Float *E) 
       { maxenergy_beamsource(E); }
void C_CONVENTION MAXENERGY_BEAMSOURCE_(EGS_Float *E) 
       { maxenergy_beamsource(E); }
void C_CONVENTION MAXENERGY_BEAMSOURCE__(EGS_Float *E) 
       { maxenergy_beamsource(E); }
#endif

void MYF77OBJ_(finish_beamsource,FINISH_BEAMSOURCE)() {
    if( libfinish ) {
        libfinish(); unload_library();
    }
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION finish_beamsource_() { finish_beamsource(); }
void C_CONVENTION finish_beamsource__() { finish_beamsource(); }
void C_CONVENTION FINISH_BEAMSOURCE() { finish_beamsource(); }
void C_CONVENTION FINISH_BEAMSOURCE_() { finish_beamsource(); }
void C_CONVENTION FINISH_BEAMSOURCE__() { finish_beamsource(); }
#endif

int MYF77OBJ_(init_beamsource,INIT_BEAMSOURCE)(
        const int *i_par, const int *i_logunit, const char *config_name, 
        const char *hhouse, const char *ehome, const char *beam_code,
        const char *pfile, const char *ifile, 
        int ncn, int nhh, int neh, int nbc, int npf, int nif) {
    int i, lcn, lhh, leh, lbc, lpf, lif;
    int len;
    char *libname;
    EGS_Float e,x,y,z,u,v,w,wt;
    EGS_I32   iq,latch,iphat;
    EGS_I64   nhist;

    /*
    printf("In init_beamsource: nhh=%d neh=%d nbc=%d npf=%d nif=%d\n",
            nhh,neh,nbc,npf,nif);
    */

    for(lcn=ncn; lcn>0; lcn--) if( !isspace(config_name[lcn-1]) ) break;
    for(lhh=nhh; lhh>0; lhh--) if( !isspace(hhouse[lhh-1]) ) break;
    for(leh=neh; leh>0; leh--) if( !isspace(ehome[leh-1]) ) break;
    for(lbc=nbc; lbc>0; lbc--) if( !isspace(beam_code[lbc-1]) ) break;
    for(lpf=npf; lpf>0; lpf--) if( !isspace(pfile[lpf-1]) ) break;
    for(lif=nif; lif>0; lif--) if( !isspace(ifile[lif-1]) ) break;

    len = leh + 1 + lcn + 1 + strlen(lib_pre) + 
        lbc + strlen(lib_ext) + 5;
    libname = (char *) malloc(len);
    for(i=0; i<leh; i++) libname[i] = ehome[i];
    len = leh;
    if( libname[len-1] != fs ) libname[len++] = fs;
    for(i=0; i<strlen(bindir); i++) libname[len+i] = bindir[i];
    len += strlen(bindir); libname[len++] = fs; 
    for(i=0; i<lcn; i++) libname[len++] = config_name[i];
    /*
    libname[len] = 0;
    strcat(libname,CONFIG_NAME); len = strlen(libname);
    */
    libname[len++] = fs; libname[len] = 0;
    strcat(libname,lib_pre); len = strlen(libname);
    for(i=0; i<lbc; i++) libname[len+i] = beam_code[i];
    len += lbc; /* libname[len++] = '_';  */
    libname[len] = 0;
    /* strcat(libname,CONFIG_NAME); */
    strcat(libname,lib_ext);

    unload_library();
#ifdef WIN32
    handle = LoadLibrary(libname);
#else 
    handle = dlopen(libname,RTLD_LAZY);
#endif
    if( !handle ) {
        printf("Failed to open library %s\n",libname);
#ifndef WIN32
        printf("Error was: %s\n",dlerror());
#endif
        free(libname); return 1;
    }

#ifdef MAKE_WIN_DISTRIBUTION
    libinit = (InitFunction)
        GET_PROC_ADDRESS(handle,beamlib_init_name);
    libsample = (SampleFunction)
        GET_PROC_ADDRESS(handle,beamlib_sample_name);
    libmotionsample = (MotionSampleFunction)
        GET_PROC_ADDRESS(handle,beamlib_motionsample_name);
    libphspmotionsample = (PhspMotionSampleFunction)
        GET_PROC_ADDRESS(handle,beamlib_phspmotionsample_name);
    libfinish = (FinishFunction)
        GET_PROC_ADDRESS(handle,beamlib_finish_name);
    libmaxenergy = (MaxEnergyFunction) 
        GET_PROC_ADDRESS(handle,beamlib_maxenergy_name);
#else
    libinit = (InitFunction)
        GET_PROC_ADDRESS(handle,F77_NAME_(beamlib_init,BEAMLIB_INIT));
    libsample = (SampleFunction)
        GET_PROC_ADDRESS(handle,F77_NAME_(beamlib_sample,BEAMLIB_SAMPLE));
    libmotionsample = (MotionSampleFunction)
        GET_PROC_ADDRESS(handle,F77_NAME_(beamlib_motionsample,BEAMLIB_MOTIONSAMPLE));
    libphspmotionsample = (PhspMotionSampleFunction)
        GET_PROC_ADDRESS(handle,F77_NAME_(beamlib_phspmotionsample,BEAMLIB_PHSPMOTIONSAMPLE)); 
    libfinish = (FinishFunction)
        GET_PROC_ADDRESS(handle,F77_NAME_(beamlib_finish,BEAMLIB_FINISH));
    libmaxenergy = (MaxEnergyFunction)
      GET_PROC_ADDRESS(handle,F77_NAME_(beamlib_max_energy,BEAMLIB_MAX_ENERGY));
#endif

/*
#ifdef WIN32
    libinit = (InitFunction) 
        GetProcAddress(handle,F77_NAME_(beamlib_init,BEAMLIB_INIT));
    libsample = (SampleFunction) 
        GetProcAddress(handle,F77_NAME_(beamlib_sample,BEAMLIB_SAMPLE));
    libfinish = (FinishFunction) 
        GetProcAddress(handle,F77_NAME_(beamlib_finish,BEAMLIB_FINISH));
#else
    libinit = (InitFunction) 
        dlsym(handle,F77_NAME_(beamlib_init,BEAMLIB_INIT));
    libsample = (SampleFunction) 
        dlsym(handle,F77_NAME_(beamlib_sample,BEAMLIB_SAMPLE));
    libfinish = (FinishFunction) 
        dlsym(handle,F77_NAME_(beamlib_finish,BEAMLIB_FINISH));
#endif
*/

    if( !libinit ) {
        printf("Failed to resolve the init function\n");
        return 2;
    }
    if( !libsample ) {
        printf("Failed to resolve the sample function\n");
        return 3;
    }
    if( !libmotionsample ) {
        printf("Failed to resolve the motion sample function\n");
        return 3;
    }
    if( !libphspmotionsample ) {
        printf("Failed to resolve the phsp motion sample function\n");
        return 3;
    }
    if( !libfinish ) {
        printf("Failed to resolve the finish function\n");
        return 4;
    }
    libinit(i_par,i_logunit,hhouse,ehome,beam_code,pfile,ifile,nhh,neh,nbc,npf,nif);

    free(libname);

    return 0;
}
#ifdef MAKE_WIN_DISTRIBUTION
int C_CONVENTION init_beamsource_(const int *i_par, const int *i_logunit, 
        const char *config_name, const char *hhouse, const char *ehome, 
        const char *beam_code, const char *pfile, const char *ifile, 
        int ncn, int nhh, int neh, int nbc, int npf, int nif) {
  beamlib_init_name = beamlib_init_name2;
  beamlib_finish_name = beamlib_finish_name2;
  beamlib_sample_name = beamlib_sample_name2;
  beamlib_motionsample_name = beamlib_motionsample_name2;
  beamlib_phspmotionsample_name = beamlib_phspmotionsample_name2;
  beamlib_maxenergy_name = beamlib_maxenergy_name2;
  return init_beamsource(i_par,i_logunit,config_name,hhouse,ehome,beam_code,pfile,ifile,
          ncn,nhh,neh,nbc,npf,nif);
}
int C_CONVENTION init_beamsource__(const int *i_par, const int *i_logunit,
        const char *config_name, const char *hhouse, const char *ehome, 
        const char *beam_code, const char *pfile, const char *ifile, 
        int ncn, int nhh, int neh, int nbc, int npf, int nif) {
  beamlib_init_name = beamlib_init_name3;
  beamlib_finish_name = beamlib_finish_name3;
  beamlib_sample_name = beamlib_sample_name3;
  beamlib_motionsample_name = beamlib_motionsample_name3;
  beamlib_phspmotionsample_name = beamlib_phspmotionsample_name3;
  beamlib_maxenergy_name = beamlib_maxenergy_name3;  
  return init_beamsource(i_par,i_logunit,config_name,hhouse,ehome,beam_code,pfile,ifile,
          ncn,nhh,neh,nbc,npf,nif);
}
int C_CONVENTION INIT_BEAMSOURCE(const int *i_par, const int *i_logunit,
        const char *config_name, const char *hhouse, const char *ehome, 
        const char *beam_code, const char *pfile, const char *ifile, 
        int ncn, int nhh, int neh, int nbc, int npf, int nif) {
  beamlib_init_name = beamlib_init_name4;
  beamlib_finish_name = beamlib_finish_name4;
  beamlib_sample_name = beamlib_sample_name4;
  beamlib_motionsample_name = beamlib_motionsample_name4;
  beamlib_phspmotionsample_name = beamlib_phspmotionsample_name4;
  beamlib_maxenergy_name = beamlib_maxenergy_name4;  
  return init_beamsource(i_par,i_logunit,config_name,hhouse,ehome,beam_code,pfile,ifile,
          ncn,nhh,neh,nbc,npf,nif);
}
int C_CONVENTION INIT_BEAMSOURCE_(const int *i_par, const int *i_logunit,
        const char *config_name, const char *hhouse, const char *ehome, 
        const char *beam_code, const char *pfile, const char *ifile, 
        int ncn, int nhh, int neh, int nbc, int npf, int nif) {
  beamlib_init_name = beamlib_init_name5;
  beamlib_finish_name = beamlib_finish_name5;
  beamlib_sample_name = beamlib_sample_name5;
  beamlib_motionsample_name = beamlib_motionsample_name5;
  beamlib_phspmotionsample_name = beamlib_phspmotionsample_name5;
  beamlib_maxenergy_name = beamlib_maxenergy_name5;  
  return init_beamsource(i_par,i_logunit,config_name,hhouse,ehome,beam_code,pfile,ifile,
          ncn,nhh,neh,nbc,npf,nif);
}
int C_CONVENTION INIT_BEAMSOURCE__(const int *i_par, const int *i_logunit,
        const char *config_name, const char *hhouse, const char *ehome, 
        const char *beam_code, const char *pfile, const char *ifile, 
        int ncn, int nhh, int neh, int nbc, int npf, int nif) {
  beamlib_init_name = beamlib_init_name6;
  beamlib_finish_name = beamlib_finish_name6;
  beamlib_sample_name = beamlib_sample_name6;
  beamlib_motionsample_name = beamlib_motionsample_name6;
  beamlib_phspmotionsample_name = beamlib_phspmotionsample_name6;
  beamlib_maxenergy_name = beamlib_maxenergy_name6;  
  return init_beamsource(i_par,i_logunit,config_name,hhouse,ehome,beam_code,pfile,ifile,
          ncn,nhh,neh,nbc,npf,nif);
}
#endif

#ifdef __cplusplus 
}
#endif

#ifdef TEST

int main() {

    EGS_Float e,x,y,z,u,v,w,wt;
    EGS_I32   iq,latch,iphat;
    EGS_I64   nhist;
    int       i, ipar=0;
    char      ans;

#ifdef WIN32
    int res = F77_OBJ_(init_beamsource,INIT_BEAMSOURCE)(
            &ipar,
            "r:/HEN_HOUSE/", "r:/egsnrc_mp/",
            "BEAM_Elekta_nrc","my700","y1a",
            strlen("r:/HEN_HOUSE/"),strlen("r:/egsnrc_mp/"),
            strlen("BEAM_Elekta_nrc"),strlen("my700"),strlen("y1a"));
#else
    int res = F77_OBJ_(init_beamsource,INIT_BEAMSOURCE)(
            &ipar,
            "/home/iwan/HEN_HOUSE/", "/home/iwan/egsnrc_mp/",
            "BEAM_Elekta_nrc","my700","y1a",
            strlen("/home/iwan/HEN_HOUSE/"),strlen("/home/iwan/egsnrc_mp/"),
            strlen("BEAM_Elekta_nrc"),strlen("my700"),strlen("y1a"));
#endif

    while(1) {
        for(i=0; i<20; i++) {
            F77_OBJ_(sample_beamsource,SAMPLE_BEAMSOURCE)(&e,&x,&y,&z,
                    &u,&v,&w,&wt,&iq,&latch,&nhist,&iphat);
 printf("Particle: E=%g q=%d wt=%g x=(%g,%g,%g) u=(%g,%g,%g) nh=%d iphat=%d\n",
                    e,iq,wt,x,y,z,u,v,w,nhist,iphat);
        }
        printf("\nGet another 20? (y/n): ");
        scanf("%c",&ans);
        if( ans != 'y' ) break;
    }

    libfinish();

    return 0;
}

#endif
