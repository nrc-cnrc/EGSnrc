/*
###############################################################################
#
#  EGSnrc functions to load a VCU shared library
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
#  Authors:         Tony Popescu, 2010
#                   Julio Lobo, 2010

#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#
###############################################################################
#
#  Functions to load a VCU library and to resolve the init, sample 
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

static char vculib_init_name1[] = "vculib_init";
static char vculib_init_name2[] = "vculib_init_";
static char vculib_init_name3[] = "vculib_init__";
static char vculib_init_name4[] = "VCULIB_INIT";
static char vculib_init_name5[] = "VCULIB_INIT_";
static char vculib_init_name6[] = "VCULIB_INIT__";
char *vculib_init_name = vculib_init_name1;

static char vculib_sample_name1[] = "vculib_sample";
static char vculib_sample_name2[] = "vculib_sample_";
static char vculib_sample_name3[] = "vculib_sample__";
static char vculib_sample_name4[] = "VCULIB_SAMPLE";
static char vculib_sample_name5[] = "VCULIB_SAMPLE_";
static char vculib_sample_name6[] = "VCULIB_SAMPLE__";
char *vculib_sample_name = vculib_sample_name1;

static char vculib_finish_name1[] = "vculib_finish";
static char vculib_finish_name2[] = "vculib_finish_";
static char vculib_finish_name3[] = "vculib_finish__";
static char vculib_finish_name4[] = "VCULIB_FINISH";
static char vculib_finish_name5[] = "VCULIB_FINISH_";
static char vculib_finish_name6[] = "VCULIB_FINISH__";
char *vculib_finish_name = vculib_finish_name1;

#endif


DLL_HANDLE handleVCU = 0;

//these functions are called from dynamic library (compiled in VCU code)
typedef void (*InitFunction)(const char *, float *);
typedef void (*FinishFunction)();
typedef void (*SampleFunction)(EGS_Float *, EGS_Float *, EGS_Float *, 
		     EGS_Float *, EGS_Float *, EGS_Float *, EGS_Float *, 
		     EGS_Float *, EGS_I32 *, EGS_I32 *, EGS_I64 *, 
		     EGS_I32 *, EGS_Float *);

InitFunction   VCUinit = 0;
SampleFunction VCUsample = 0;
FinishFunction VCUfinish = 0;

void unload_VCUlibrary() {
    if( handleVCU ) 
#ifdef WIN32
        FreeLibrary(handleVCU);
#else
        dlclose(handleVCU);
#endif
    handleVCU = 0;
    VCUinit = 0; VCUsample = 0; VCUfinish = 0;
}

#ifdef __cplusplus 
extern "C" {
#endif

void MYF77OBJ_(sample_vcusource,SAMPLE_VCUSOURCE)(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist,
	EGS_I32 *still_in_container, EGS_Float *frMUindex){
    if( !handleVCU || !VCUsample ) {
        printf("You have to load and initialize the library first\n");
        exit(1);
    }
    VCUsample(e,x,y,z,u,v,w,wt,iq,latch,nhist,still_in_container,
	      frMUindex);
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION sample_vcusource_(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist,
        EGS_I32 *still_in_container, EGS_Float *frMUindex) {
    sample_vcusource(e,x,y,z,u,v,w,wt,iq,latch,nhist,
                     still_in_container, frMUindex);
}
void C_CONVENTION sample_vcusource__(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist,
        EGS_I32 *still_in_container, EGS_Float *frMUindex) {
    sample_vcusource(e,x,y,z,u,v,w,wt,iq,latch,nhist,
                     still_in_container, frMUindex);
}
void C_CONVENTION SAMPLE_VCUSOURCE(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist,
        EGS_I32 *still_in_container, EGS_Float *frMUindex) {
    sample_vcusource(e,x,y,z,u,v,w,wt,iq,latch,nhist,
                     still_in_container, frMUindex);
}
void C_CONVENTION SAMPLE_VCUSOURCE_(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist,
        EGS_I32 *still_in_container, EGS_Float *frMUindex) {
    sample_vcusource(e,x,y,z,u,v,w,wt,iq,latch,nhist,
                     still_in_container, frMUindex);
}
void C_CONVENTION SAMPLE_VCUSOURCE__(
        EGS_Float *e, EGS_Float *x, EGS_Float *y, EGS_Float *z,
        EGS_Float *u, EGS_Float *v, EGS_Float *w, EGS_Float *wt,
        EGS_I32 *iq, EGS_I32 *latch, EGS_I64 *nhist,
        EGS_I32 *still_in_container, EGS_Float *frMUindex) {
    sample_vcusource(e,x,y,z,u,v,w,wt,iq,latch,nhist,
                     still_in_container, frMUindex);
}
#endif


void MYF77OBJ_(finish_vcusource,FINISH_VCUSOURCE)() {
    if( VCUfinish ) {
        VCUfinish(); unload_VCUlibrary();
    }
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION finish_vcusource_()  { finish_vcusource(); }
void C_CONVENTION finish_vcusource__() { finish_vcusource(); }
void C_CONVENTION FINISH_VCUSOURCE()   { finish_vcusource(); }
void C_CONVENTION FINISH_VCUSOURCE_()  { finish_vcusource(); }
void C_CONVENTION FINISH_VCUSOURCE__() { finish_vcusource(); }
#endif

int MYF77OBJ_(init_vcusource,INIT_VCUSOURCE)(
        float *survival_ratio,const char *config_name,
	const char *hhouse,const char *ehome,const char *vcu_code,
	const char* ifile,int ncn,int nhh,int neh,int nvc,int nif) {
    
    int i, lcn, lhh, leh, lvc, lif;
    int len, in_len;
    char *libname;
    char *input_file;
    
    EGS_Float e,x,y,z,u,v,w,wt;
    EGS_I32   iq,latch,iphat;
    EGS_I64   nhist;

    for(lcn=ncn; lcn>0; lcn--) if( !isspace(config_name[lcn-1]) ) break;
    for(lhh=nhh; lhh>0; lhh--) if( !isspace(hhouse[lhh-1]) ) break;
    for(leh=neh; leh>0; leh--) if( !isspace(ehome[leh-1]) ) break;
    for(lvc=nvc; lvc>0; lvc--) if( !isspace(vcu_code[lvc-1]) ) break;
    for(lif=nif; lif>0; lif--) if( !isspace(ifile[lif-1]) ) break;

    // remove blanks from the end of the input file name 
    input_file = (char*) malloc(lif+1);
    for (i=0; i<lif; i++) input_file[i] = ifile[i];
    input_file[lif] = 0;
    // printf("\nifile:%s\ninput_file:%s\n",ifile,input_file);
 
    // get the name of the library (libparticleDmlc.so)
    len = leh + 1 + lcn + 1 + strlen(lib_pre) + lvc + strlen(lib_ext) + 5;
    libname = (char *) malloc(len);
    len=0;
    for(i=0; i<leh; i++) libname[len++] = ehome[i];
    libname[len] = 0;
    len = strlen(libname);
    if( libname[len-1] != fs ) libname[len++] = fs;
    libname[len] = 0;
    strcat(libname,bindir); len = strlen(libname);
    libname[len++] = fs;
    libname[len] = 0;
    len = strlen(libname);
    for(i=0; i<lcn; i++) libname[len++] = config_name[i];
    libname[len] = 0;
    len = strlen(libname);
    libname[len++] = fs; 
    libname[len] = 0;
    strcat(libname,lib_pre); len = strlen(libname);
    for(i=0; i<lvc; i++) libname[len++] = vcu_code[i];
    libname[len] = 0;
    len = strlen(libname);
    libname[len] = 0;
    strcat(libname,lib_ext);

    unload_VCUlibrary();
#ifdef WIN32
    handleVCU = LoadLibrary(libname);
#else
    handleVCU = dlopen(libname,RTLD_LAZY);
#endif

    if( !handleVCU ) {
        printf("Failed to open library %s\n",libname);
#ifndef WIN32
        printf("Error was: %s\n",dlerror());
#endif
        free(libname); return 1;
    }

#ifdef MAKE_WIN_DISTRIBUTION
    VCUinit = (InitFunction)
        GET_PROC_ADDRESS(handleVCU,vculib_init_name);
    VCUsample = (SampleFunction)
        GET_PROC_ADDRESS(handleVCU,vculib_sample_name);
    VCUfinish = (FinishFunction)
        GET_PROC_ADDRESS(handleVCU,vculib_finish_name);
#else
    VCUinit = (InitFunction)
        GET_PROC_ADDRESS(handleVCU,"_Z7initvcuPKcPf");
    VCUsample = (SampleFunction)
        GET_PROC_ADDRESS(handleVCU,
             "_Z13vculib_samplePdS_S_S_S_S_S_S_PiS0_S0_S0_S_");
    VCUfinish = (FinishFunction)
        GET_PROC_ADDRESS(handleVCU,"_Z13vculib_finishv");
#endif


    if( !VCUinit ) {
        printf("VCUlib: failed to resolve the init function\n");
        return 2;
    }
    if( !VCUsample ) {
        printf("VCUlib: failed to resolve the sample function\n");
        return 3;
    }
    if( !VCUfinish ) {
        printf("VCUlib: failed to resolve the finish function\n");
        return 4;
    }

    //call library function initvcu
    VCUinit(input_file, survival_ratio);

    free(libname);
    free(input_file);

    return 0;
}
#ifdef MAKE_WIN_DISTRIBUTION
int C_CONVENTION init_vcusource_(
             float *survival_ratio,
             const char *config_name, const char *hhouse,
             const char *ehome,const char *vcu_code,
	     const char* ifile,
             int ncn,int nhh,int neh,int nvc,int nif) {
  vculib_init_name = vculib_init_name2;
  vculib_finish_name = vculib_finish_name2;
  vculib_sample_name = vculib_sample_name2;
  return init_vcusource(survival_ratio,config_name,hhouse,
                        ehome,vcu_code,ifile,ncn,nhh,neh,nvc,nif);
}
int C_CONVENTION init_vcusource__(
             float *survival_ratio,
             const char *config_name, const char *hhouse,
             const char *ehome,const char *vcu_code,
	     const char* ifile,
             int ncn,int nhh,int neh,int nvc,int nif) {
  vculib_init_name = vculib_init_name3;
  vculib_finish_name = vculib_finish_name3;
  vculib_sample_name = vculib_sample_name3;
  return init_vcusource(survival_ratio,config_name,hhouse,
                        ehome,vcu_code,ifile,ncn,nhh,neh,nvc,nif);
}
int C_CONVENTION INIT_VCUSOURCE(
             float *survival_ratio,
             const char *config_name, const char *hhouse,
             const char *ehome,const char *vcu_code,
	     const char* ifile,
             int ncn,int nhh,int neh,int nvc,int nif) {
  vculib_init_name = vculib_init_name4;
  vculib_finish_name = vculib_finish_name4;
  vculib_sample_name = vculib_sample_name4;
  return init_vcusource(survival_ratio,config_name,hhouse,
                        ehome,vcu_code,ifile,ncn,nhh,neh,nvc,nif);
}
int C_CONVENTION INIT_VCUSOURCE_(
             float *survival_ratio,
             const char *config_name, const char *hhouse,
             const char *ehome,const char *vcu_code,
	     const char* ifile,
             int ncn,int nhh,int neh,int nvc,int nif) {
  vculib_init_name = vculib_init_name5;
  vculib_finish_name = vculib_finish_name5;
  vculib_sample_name = vculib_sample_name5;
  return init_vcusource(survival_ratio,config_name,hhouse,
                        ehome,vcu_code,ifile,ncn,nhh,neh,nvc,nif);
}
int C_CONVENTION INIT_VCUSOURCE__(
             float *survival_ratio,
             const char *config_name, const char *hhouse,
             const char *ehome,const char *vcu_code,
	     const char* ifile,
             int ncn,int nhh,int neh,int nvc,int nif) {
  vculib_init_name = vculib_init_name6;
  vculib_finish_name = vculib_finish_name6;
  vculib_sample_name = vculib_sample_name6;
  return init_vcusource(survival_ratio,config_name,hhouse,
                        ehome,vcu_code,ifile,ncn,nhh,neh,nvc,nif);
}
#endif

#ifdef __cplusplus 
}
#endif

