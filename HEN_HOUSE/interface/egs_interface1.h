/*
###############################################################################
#
#  EGSnrc simple C interface headers v1
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
#  Author:          Iwan Kawrakow, 2003
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Blake Walters
#
###############################################################################
*/


/*! \file egs_interface2.h
  \brief This file defines the C/C++ interface to the EGSnrc mortran
         back-end
  \IK

  C/C++-structures are defined for the most commonly used mortran
  common blocks such as the \link EGS_Stack particle stack \endlink,
  the \link EGS_Bounds transport threshold energies\endlink, etc.
  The general naming convention is that for a common block
  \c XXX the C-structure will be called EGS_Xxx. For a detailed
  description of EGSnrc common blocks see PIRS-701. Note, however,
  that various common blocks are replaced in \c egs_c_interface2.macros
  to remove all dependences on the geometry and to combine
  various transport parameter otpions into a single common
  block so that fewer common blocks need to be "exported".

  Not all
  common are "exported". If a definition of a C-structure for
  a given common block that you need to access in your
  user code is missing, simply follow the way this file
  defines the structures and define it in your user code.
  In principle the mortran common blocks can be accessed
  directly from C/C++. However, due to name mangling of Fortran
  symbols undertaken by the Fortran compiler, a common block
  named \c xxx may result in a symbol called <code>xxx, xxx_, xxx__,
  XXX, XXX_</code> or \c XXX__, depending on the name mangling scheme
  used by the Fortran compiler. This is handled by the macros
  \c %F77_OBJ(name,NAME) and \c F77_OBJ_(name,NAME) defined in
  \c %egs_config1.h, which is created during the installation of
  the EGSnrc system. With these macros one could access \em e.g. the
  particle energies using
  \verbatim
  F77_OBJ(stack,STACK).E
  \endverbatim
  Because the above is quite tedious, this file also defines the
  addresses of the mortran common blocks as pointers to
  the corresponding C-structures with the general naming convention that
  \c the_xxx points to the common block \c XXX, \em e.g.
  \c the_stack points to the \c STACK common block,
  \c the_bounds to the \c BOUNDS common block, etc.

  This file also provides declarations of various EGSnrc mortran functions
  and subroutines and corresponding pre-processor macros for
  easier access (the name mangling also applies to function names
  and therefore to call the EGSnrc subroutine \c %shower(), one must
  use \c F77_OBJ(shower,SHOWER)() ). Most of these functions are
  only relevant for developing EGSnrc applications in C. For
  EGSnrc C++ applications it is much easier to derive from the
  EGS_SimpleApplication or EGS_AdvancedApplication classes, which
  make the direct access to most of these functions unnecessary.
*/

#ifndef EGS_INTERFACE1_
#define EGS_INTERFACE1_

#include "egs_config1.h"

/* Note: The C-preprocessor macros MXMED and MXSTACK must be defined in the
   file array_sizes.h. */
#include "array_sizes.h"

/*! Number of possible calls to the
    egsAusgab() or \link EGS_Application::ausgab() ausgab() \endlink
    functions */
#define MXAUS 29  /* there are 29 different calls to ausgab */

/*! \brief A structure corresponding to the the EGSnrc particle stack
  common block \c STACK.

  Note that the stack size \c MXSTACK is defined in a file
  \c %array_sizes.h that must be present in the user code directory.
  The mortran common block can be accessed via #the_stack.
 */
struct EGS_Stack {
  /*! Array with particle energies in MeV including rest energy */
  double    E[MXSTACK];
  /*! Arrays with particle x-positions in cartesian coordinates */
  EGS_Float x[MXSTACK];
  /*! Arrays with particle y-positions in cartesian coordinates */
  EGS_Float y[MXSTACK];
  /*! Arrays with particle z-positions in cartesian coordinates */
  EGS_Float z[MXSTACK];
  /*! Arrays with particle x- direction cosines */
  EGS_Float u[MXSTACK];
  /*! Arrays with particle y- direction cosines */
  EGS_Float v[MXSTACK];
  /*! Arrays with particle z- direction cosines */
  EGS_Float w[MXSTACK];
  /*! Array with nearest distances to a boundary */
  EGS_Float dnear[MXSTACK];
  /*! Array with statistical weights */
  EGS_Float wt[MXSTACK];
  /*! Particle charges */
  EGS_I32   iq[MXSTACK];
  /*! Particle geometry regions */
  EGS_I32   ir[MXSTACK];
  /*! Latch variables. Can be used to store some additional information
      about the particles used at run time
   */
  EGS_I32   latch[MXSTACK];
  /*! The initial particle latch variable */
  EGS_I32   latchi;
  /*! Number of particles currently on the stack */
  EGS_I32   np;
  /*! Number of particles on the stack before the last interaction occured */
  EGS_I32   npold;
};

/*! \brief A structure corresponding to the EGSnrc transport threshold
  energies common block \c BOUNDS

    Note that the original \c BOUNDS common block contains
    transport threshold energies on a region-by-region basis.
    To remove this dependence on the geometry (which determines
    the number of regions), \c BOUNDS is replaced with a
    new definition in \c egs_c_interface2.macros removing the
    region dependence but introducing \c ecut_new and \c pcut_new.
    To obtain the original functionality of a region-by-region
    threshold enerty variation, these variables can be set
    to the desired values each time the transport of a new particle
    begins and each time a particle enters a new region.
    The best way to do this is to set up that the \c %ausgab()
    function is called before a step is taken (\em i.e. \c iarg=0)
    and to set \c ecut_new or \c pcut_new to the desired value
    in \c %ausgab(), if a new region will be entered after the step.
    In addition, \c ecut or \c pcut must be set to the actual value
    in the current region when the transport of a new particle
    begins. When the transport of a new particle begins, the
    function egsStartParticle is called. User codes written in
    C must provide an implementation of this function. User
    codes in C++ deriving from EGS_AdvancedApplication must
    implement the \link EGS_AdvancedApplication::startNewParticle()
    startNewParticle() \endlink function. Region-by-region variation
    of the transport thresholds is not possible in simple
    C++ applications deriving from EGS_SimpleApplication.
    Note that when setting transport threshold energies you
    mist make sure that they are higher then, or equal to,
    the particle production threshold energies \c AE (electrons) and
    \c AP (photons), otherwise program crashes are likely.
    \c AE and \c AP are the particle production thresholds that are
    determined by the PEGS data being used. They are available from
    struct EGS_Thresh for all media in the simulation geometry.
 */
struct EGS_Bounds {
    /*! Electron/positron transport threshold energy in MeV, including rest
        energy */
    EGS_Float ecut;
    /*! Photon transport threshold energy in MeV */
    EGS_Float pcut;
    /*! Electron/positron transport threshold energy in the new region */
    EGS_Float ecut_new;
    /*! Photon transport threshold energy in the new region */
    EGS_Float pcut_new;
    /*! Transport distance in vacuum */
    EGS_Float vacdst;
};

/*! \brief A structure corresponding to the particle production
  threshold energies common block \c THRESH.

  Note that the maximum number of media \c MXMED is defined in the
  file \c %array_sizes.h that must be present in the user code area.
  The \c THRESH common block also contains \c rmt2 (two times electron
  rest energy in MeV) and \c rmsq (electron rest energy squared),
  that actually have nothing to do with thresholds.
  Unfortunately, it is too much effort to remove them from
  \c THRESH and put them where they belong (\c rmt2 and \c rmsq are
  used in many places).
  */
struct EGS_Thresh {
    /*! Two times electron rest energy in MeV (approximately 1.022) */
    EGS_Float  rmt2;
    /*! electron rest energy squared */
    EGS_Float  rmsq;
    /*! Photon production threshold for each medium. This is also the
        minimum energy of the photon cross section data from PEGS
     */
    EGS_Float  ap[MXMED];
    /*! Electron/positron production threshold for each medium.
        This is also the minimum energy for the electron positron cross
        section data from PEGS */
    EGS_Float  ae[MXMED];
    /*! The upper energy of the photon cross section data for each medium */
    EGS_Float  up[MXMED];
    /*! The upper energy of the electron cross section data for each medium */
    EGS_Float  ue[MXMED];
    /*! Moller threshold energy (total) */
    EGS_Float  te[MXMED];
    /*! Moller threshold energy (kinetic) */
    EGS_Float  thmoll[MXMED];
};

/*!  \brief A structure corresponding to the \c EPCONT common block.

    The \c EPCONT common block contains various variables of interest
    for user scoring and for interacting with the user geometry
 */
struct EGS_Epcont {
  /*! Energy being deposited locally */
  double    edep;
  /*! Step length to an interaction */
  EGS_Float tstep;
  /*! Step length after step-size restrictions */
  EGS_Float tustep;
  /*! Straight line distance between initial and final position without
      geometry constraints */
  EGS_Float ustep;
  /*! Curved transport distance after geometry constraints */
  EGS_Float tvstep;
  /*! Straight line transport distance after geometry constraints */
  EGS_Float vstep;
  /*! Ratio of mass density in current region to the default mass density
      of the medium filling the region */
  EGS_Float rhof;
  /*! Old particle energy */
  EGS_Float eold;
  /*! New particle energy */
  EGS_Float enew;
  /*! Electron kinetic energy */
  EGS_Float eke;
  /*! ln(eke) */
  EGS_Float elke;
  /*! ln(photon energy) */
  EGS_Float gle;
  /*! Electron range to \c AE in the current medium */
  EGS_Float e_range;
  /*! Position of the particle after the step */
  EGS_Float x_final, y_final, z_final;
  /*! Direction of the particle after the step,
      only set for electrons/positrons */
  EGS_Float u_final, v_final, w_final;
  /*! Should the particle be discarded ?
      - 0 means no discard
      - >0 means discard immediately
      - <0 means discard after the step.

      Typically set by the geometry routines.
  */
  EGS_I32   idisc;
  /*! Old geometry region */
  EGS_I32   irold;
  /*! New geometry region */
  EGS_I32   irnew;
  /*! Array of flags for calls to \c %ausgab() */
  EGS_I32   iausfl[MXAUS];
};

/*! \brief A structure corresponding to the \c ET_control common block.

  The \c ET_control common block contains varibles that determine
  electron transport options.
 */
struct EGS_EtControl {
    /*! Maximum geometrical step-size restriction */
    EGS_Float smaxir;
    /*! smax restriction in the new region */
    EGS_Float smax_new;
    /*! Maximum fractional energy loss per step */
    EGS_Float estepe;
    /*! Maximum first elastic moment per step */
    EGS_Float ximax;
    /*! Distance from a boundary in elastic MFP to switch to single
        scattering, if \c exact_bca is \c true, or to turn off lateral
        deflections, if exact_bca is \c false.
     */
    EGS_Float skindepth_for_bca;
    /*! Condensed history transport algorithm
        (0 means EGSnrc default a.k.a. PRESTA-II, 1 means PRESTA-I) */
    EGS_I32   transport_algorithm;
    /*! Boundary crossing algorithm (0 = exact, 1 = BCA a la PRESTA-I) */
    EGS_I32   bca_algorithm;
    /*! Using exact boundary crossing ?
      Note that exact_bca is actually a variable of type logical.
      As all tested Fortran compilers make a 32 bit variable out of this,
      we declare it here as EGS_I32. I'm sure this will eventually be a
      hard to find bug if this changes... */
    EGS_I32   exact_bca;
};

/*! \brief A structure corresponding to the \c MEDIA common block

  Contains default mass densities for all media and the number of media.
  */
struct EGS_Media {
    /*! Default mass densities */
    EGS_Float rho[MXMED];
    /*! Prefix for photon cross section data files */
    char      pxsec[16];
    /*! Prefix for eii cross section data files */
    char      eiixsec[16];
    /*! Prefix for compton cross section data files */
    char      compxsec[16];
    /*! Number of media in the geometry */
    EGS_I32   nmed;
};

/*! \brief A structure corresponding to the \c rayleigh_inputs common block

  Contains media and FF file names for custom Rayleigh scattering
  */
struct EGS_Rayleigh {
    /*! Media names for which to read in custom FF */
    char  ff_media[MXMED][24];
    /*! Custom FF file names */
    char ff_file[MXMED][128];
};

/*! \brief A structure corresponding to the \c USEFUL common block.

  Contains useful stuff such as electron rest energy, current medium
  index, etc.
*/
struct EGS_Useful {
  double    pzero,      //!< Precise zero
            prm,        //!< Precise electron rest energy in MeV
            prmt2;      //!< 2*prm
  EGS_Float rm,         //!< Electron rest energy
            rhor,       //!< Mass density ratio
            rhor_new;   //!< Mass density ratio in the new region
  EGS_I32   medium,     //!< Current medium
            medium_new, //!< Medium in the new region
            medold;     //!< Old medium
};

/*! \brief A structure corresponding to the \c xsection_options common block

  Contains all cross section options available for EGSnrc. Note that this
  common block is created specifically for the C/C++ interface using
  mortran's replacement capabilities by removing the cross section option
  variables from their respective common blocks (\em e.g. \c ibrdst is
  normally in \c BREMPR) and putting them together into a cross section
  options common block, so that not so many mortran common blocks have
  to be exported as C structures (especially in view of the fact that
  most of the information in these common blocks is private to the
  EGSnrc system and should not be touched by the user).

  Note that in C++ applications derived from EGS_SimpleApplication and
  EGS_AdvancedApplication all cross section options can be set in
  the input file by including
  \verbatim
  :start MC transport parameter:
      Bound Compton scattering = On/Off
      Rayleigh scattering = On/Off
      ...
  :stop MC transport parameter:
  \endverbatim
  (see the documentation of the data members for the keys needed for
  all options).
  A similar effect can be obtained in C applications by using the
  egsGetTransportParameter() function.
*/
struct EGS_XOptions {

  /*! Determines the angular distribution of bremstrahlung. If set to
    2, Eq. 2BS of the review article by Koch and Motz will be used. If set
    to 1, the leading term of 2BS will be used (the default).
    If set to zero, the photons
    will inherit the direction of the electron so that the user can
    implement their own angular distribution by calling \c %ausgab() after
    the bremsstrahlung event and setting the angles of the photons.

    Can be set in the input file using
    \verbatim
    Brems angular sampling= Simple or KM
    \endverbatim
  */
  EGS_I32   ibrdst;

  /*! Determines the angular distribution of pair particles. If set to 2,
    the Schiff formula will be used (Eq. 3D-2003 of the review article by
    Motz, Olsen and Koch), if set to 1 the leading term of the Schiff formula
    will be used (the default), if set to 0 the original EGS4 approach
    of polar angle = ratio of rest energy to total energy is employed.

    Can be set in the input file using
    \verbatim
    Pair angular samplin= Off or Simple or KM
    \endverbatim
  */
  EGS_I32   iprdst;

  /*! Determines the bremsstrahlung cross sections differential in the
    photon energy to be used for sampling the photon energy.
    If set to 0, the Bethe-Heitler high energy approximation will be
    used, if set to 1 the NIST tabulations provided by Steve Seltzer
    will be employed. Default is 0.

    Can be set in the input file using
    \verbatim
    Brems cross sections= BH or NIST
    \endverbatim
  */
  EGS_I32   ibr_nist;

  /*! Determines if spin effects will be taken into account in electron
    and positron elastic scattering (1=yes, the default, 0=no)

    Can be set in the input file using
    \verbatim
    Spin effects= On or Off
    \endverbatim
   */
  EGS_I32   spin_effects;

  /*! Bound Compton scattering flag. If set to 0, Compton scattering will
    be modeled according to Klein-Nishina, if set to 1 (the default),
    Compton scattering will be modeled according to the relativistic
    impulse approximation that takes into account binding and
    Doppler broadenning.

    Can be set in the input file using
    \verbatim
    Bound Compton scattering= On or Off
    \endverbatim
  */
  EGS_I32   ibcmp;

  /*! Reyleigh scattering flag. If set to 0 (the default), no
    Reyleigh scattering will be done. Note that if you turn on
    Reyleigh scattering by setting this flag to 1, all PEGS4 data
    sets must include Reyleigh data

    Can be set in the input file using
    \verbatim
    Reyleigh scattering= On or Off
    \endverbatim
  */
  EGS_I32   iraylr;

  /*! Atomic relaxations flag. If set to 1 (the default), vacancies
   created in shells with binding energies above 1 keV will be relaxed
   by fluorscent, Auger and Coster-Kronig transitions. Vacancies
   can be currently created after photo-absorption, bound Compton scattering
   and electron inelastic scattering (if eii_flag is not 0).
   If set to 0, the binding energy will be given to the photo-electron
   or deposited locally.
   */
  EGS_I32   iedgfl;

  /*! Determines the angular distribution of photo-electrons. If set to
    1 (the default), the angle of photo-electrons will be sampled from
    the Sauter distribution. If set to 0, photo-electrons inherit the
    direction of the incident photon.

    Can be set in the input file using
    \verbatim
    Atomic relaxations= On or Off
    \endverbatim
  */
  EGS_I32   iphter;

  /*! Pair cross sections flag. If set to 0 (the default), the energy
    of the pair particles will be sampled from the first Born approximation
    cross section in its high-energy appoximation derived by Bethe-Heitler.
    If set to 1, energies will be sampled from the NRC tabulations
    based on the exact cross sections (tabulations are available
    up to 85 MeV, above 85 MeV the Bethe-Heitler cross sections are
    very close to the exact cross sections).

    Can be set in the input file using
    \verbatim
    Pair cross sections= BH or NRC
    \endverbatim
  */
  EGS_I32   pair_nrc;

  /*! Triplet production flag. If set to 0 (the default), triplet events
    will be simulated as pair events. If set to 1, triplet event will
    be explicitely simulated using the Borsellino cross section

    Can be set in the input file using
    \verbatim
    Triplet production= On or Off
    \endverbatim
  */
  EGS_I32   itriplet;

  /*! Flag for radiative corrections for Compton scattering.
      If set to 0 (the default), Compton scattering is modelled according
      to Klein-Nishina or RIA, depending on ibcmp. If set to 1 and
      rad_compton.mortran is compiled with the other EGSnrc mortran sources,
      radiative corrections are taken into account in next-to-leading
      order.

      Can be set in the input file using
      \verbatim
      Radiative Compton corrections= On or Off
      \endverbatim
                    */
  EGS_I32   radc_flag;

  /*! Electron impact ionization (EII) flag. If set to 0 (the default),
   no EII occurs. If set to 1, EII is simulated using cross sections
   based on unpublished work by Kawrakow

    Can be set in the input file using
    \verbatim
    Electron Impact Ionization= On or Off
    \endverbatim
   */
  EGS_I32   eii_flag;
};

/*! \brief A structure corresponding to the \c egs_vr common block

 The \c egs_vr common block contains variables that can turn on/off
 internally implemented variabce reduction techniques.
*/
struct EGS_VarianceReduction {
    /*! Maximum energy for which it is allowed to use range rejection.
      Note that range rejection is an approximation because it ignores
      the possibility of bremstrahlung creation that mey leave the region
      of range-discard (and therefore actually not a real variance
      reduction technique as per definition, a variance reduction
      technique is a technique that improves the simulation
      efficiency without changing the result in a statistically significant
      way). One can decrease the associated error by
      decreasing e_max_rr (when e_max_rr is sufficiently small, the
      fraction of energy transfered to bremsstrahlung becomes
      negligible).
    */
    EGS_Float e_max_rr;

    /*! Maximum range rejection energy in the new region */
    EGS_Float e_max_rr_new;

    /*! Probability for Russian Roulette (RR) (i_play_RR must be set to 1
      to play RR). */
    EGS_Float prob_RR;

    /*! Number of times to split radiative events */
    EGS_I32   nbr_split;

    /*! If set to 1, Russian Roulette will be played with electrons created
      in photon interactions using a survival probability #prob_RR.
     This is useful together with radiative splitting (\em i.e.
     #nbr_split > 1)
    */
    EGS_I32   i_play_RR;

    /*! If Roussian Roulette of electrons created in photon interactions is
      on (#i_play_RR is 1), i_survived_RR is set to the number of electrons
      surviving the RR game.
    */
    EGS_I32   i_survived_RR;

    /*! Counts warnings in RR (if #prob_RR is less than 0). */
    EGS_I32   n_RR_warning;

    /*! Turns on range rejection if set to 1 (default is 0).
      \sa #e_max_rr */
    EGS_I32   i_do_rr;
};

/*! \brief A structure corresponding to the \c egs_io common block

  C++ applications deriving from
  EGS_AdvancedApplication should not use this structure directly
  but EGS_Application::inputFile(), etc., instead.
  Parallel runs are implemented using a
  \link EGS_RunControl run control object \endlink
*/
struct EGS_IO {
    char file_extensions[20][10];
    EGS_I32 file_units[20];
    char user_code[64];
    char input_file[256];
    char output_file[256];
    char pegs_file[256];
    char hen_house[128];
    char egs_home[128];
    char work_dir[128];
    char host_name[64];
    EGS_I32 n_parallel, i_parallel, first_parallel, n_max_parallel,
            n_chunk, n_files, i_input, i_log, i_incoh, i_nist_data,
            i_mscat, i_photo_cs, i_photo_relax, xsec_out, is_batch,
            is_pegsless;
};


/*! \brief The address of the mortran \c STACK common block as a
    pointer to a C-structure of type EGS_Stack */
extern __extc__ struct EGS_Stack             *the_stack;

/*! \brief The address of the morrtan \c BOUNDS common block as a
    pointer to a C-structure of type EGS_Bounds */
extern __extc__ struct EGS_Bounds            *the_bounds;

/*! \brief The address of the mortran \c THRESH common block as a
    pointer to a C-structure of type EGS_Thresh */
extern __extc__ struct EGS_Thresh            *the_thresh;

/*! \brief The address of the mortran \c EPCONT common block as a
    pointer to a C-structure of type EGS_Epcont */
extern __extc__ struct EGS_Epcont            *the_epcont;

/*! \brief The address of the mortran \c Et_Control common block as a
    pointer to a C-structure of type EGS_EtControl */
extern __extc__ struct EGS_EtControl         *the_etcontrol;

/*! \brief The address of the mortran \c MEDIA  common block as a
    pointer to a C-structure of type EGS_Media  */
extern __extc__ struct EGS_Media             *the_media;

/*! \brief The address of the mortran \c USEFUL common block as a
    pointer to a C-structure of type EGS_Useful */
extern __extc__ struct EGS_Useful            *the_useful;

/*! \brief The address of the mortran cross section options common block as a
  pointer to a C-structure of type EGS_XOptions */
extern __extc__ struct EGS_XOptions          *the_xoptions;

/*! \brief The address of the mortran \c egs_io common block as a
    pointer to a C-structure of type EGS_IO */
extern __extc__ struct EGS_IO                *the_egsio;

/*! \brief The address of the mortran \c egs_vr common block as a
  pointer to a C-structure of type EGS_VarianceReduction */
extern __extc__ struct EGS_VarianceReduction *the_egsvr;

/*! \brief The address of the mortran rayleigh_inputs common block as a
  pointer to a C-structure of type EGS_Rayleigh */
extern __extc__ struct EGS_Rayleigh *the_rayleigh;

/* ******************* EGSnrc interface functions *************************/


/*! \brief Initializes the EGSnrc mortran back-end

  This function initializes default values for the various
  cross section options and transport parameter settings,
  creates a temporary working directory where all user code
  output is stored at run time, and opens necessary Fortran I/O
  units. It must be called before accessing any EGSnrc data
  from user codes written in C. This is not necessary
  in C++ application classes as the function is
  called in the EGS_SimpleApplication constructor and from
  the default implementation of the
  \link EGS_AdvancedApplication::initEGSnrcBackEnd()
  initEGSnrcBackEnd() \endlink function of EGS_AdvancedApplication.
  \a argc and \a argv are the command line arguments.
 */
extern __extc__ void egsInit(int argc, char **argv);

/*! Shorthand notation for the \c egs_finish mortran subroutine */
#define egsFinish F77_OBJ_(egs_finish,EGS_FINISH)

/*! \brief Finish a simulation.

  This function moves all output generated during the run from
  the temporary working directory to the user code directory.
  It must be called from user codes written in C before the
  program exits. For the C++ interface it is called from the
  default implementations of EGS_SimpleApplication::finish() and
  EGS_AdvancedApplication::finishRun(). */
extern __extc__ void egsFinish(void);

/*! Shorthand notation for the \c egs_add_medium mortran function */
#define egsAddMedium F77_OBJ_(egs_add_medium,EGS_ADD_MEDIUM)

/*! \brief Add a medium to the list of media

  This function adds the medium named \a medname to the list of
  media to the mortran back-end (\a length is the length of \a medname).
  It can be used to add media before calling the egsHatch() function
  to initialize the cross section data. In the C++ interface
  media found in the geometry are automatically added to the
  EGSnrc mortran back-end by the EGS_SimpleApplication constructor and
  by the EGS_AdvancedApplication::initCrossSections() function.
*/
extern __extc__ EGS_I32 egsAddMedium(const char *medname, EGS_I32 length);

/*! Shorthand notation for the \c HATCH subroutine (which is renamed to
  \c egs_hatch for the C/C++ interface using mortran's replacemant
  capabilities) */
#define egsHatch F77_OBJ_(egs_hatch,EGS_HATCH)
/*! \brief Initialize cross section data from a PEGS4 data file

  This function must be called from user codes written in C after
  all media are defined (\em i.e. added using egsAddMedium()) and
  before the first call to egsShower(). to initialize the cross section
  data needed at run time from a PEGS4 data file. In the C++ interface
  this function is automatically called by the EGS_SimpleApplication
  constructor and by the default implementation of
  EGS_AdvancedApplication::initCrossSections().
 */
extern __extc__ void egsHatch(void);

/*! Shorthand notation for the \c SHOWER subroutine (which is renamed to
    \c egs_shower for the C/C++ interface using mortran's replacemant
    capabilities) */
#define egsShower F77_OBJ_(egs_shower,EGS_SHOWER)
/*! \brief Transport the particles currently on the particle stack.

  This function transports all particles currently on the
  particle stack (see EGS_Stack and #the_stack). To use it,
  fill the particle stack #the_stack with one or more particles
  and call it. For user codes written in C++ this function is
  only relevant if the user wishes to re-implement the
  EGS_AdvancedApplication::shower() function.
 */
extern __extc__ void egsShower(void);

/****************************************************************************
            Functions for using the EGSnrc random number generator
            Only relevant for user codes written in C
 ****************************************************************************/

/*! Shorthand notation for the \c egs_init_default_rng mortran subroutine */
#define egsRandomDefaultInit F77_OBJ_(egs_init_default_rng,EGS_INIT_DEFAULT_RNG)
/*! \brief Initialize the EGSnrc RNG using default values.

    This function should only be used from user codes written in C.
    Application classes derived from EGS_SimpleApplication or
    EGS_AdvancedApplication should use
    EGS_SimpleApplication::rndm or EGS_AdvancedApplication::rndm
    instead, or construct their own
    \link EGS_RandomGenerator RNG object \endlink
 */
extern __extc__ void egsRandomDefaultInit();

/*! Shorthand notation for the \c egs_init_rng function */
#define egsRandomInit F77_OBJ_(egs_init_rng,EGS_INIT_RNG)
/*! \brief Initialize the EGSnrc RNG.

    This function initializes the EGSnrc RNG. It should only be
    used from user codes written in C. C++ applications should
    use EGS_RandomGenerator objects instead.
    The meaning of the parameters \a s1 and \a s2 depends on
    the RNG being used (RANMAR vs RANLUX).
    If RANLUX is used, \a s1 is
    the luxury level, \a s2 the initial seed.
    For RANMAR, the two arguments are initial seeds.
 */
extern __extc__ void egsRandomInit(const EGS_I32 *s1, const EGS_I32 *s2);

/*! Shorthand notation for the \c egs_get_rndm function */
#define egsRandomGet F77_OBJ_(egs_get_rndm,EGS_GET_RNDM)
/*! \brief Get a single random number stored at the location pointed to
    by \a ranno.

    This function should only be used from the C interface,
    where egsRandomInit() or egsRandomDefaultInit() must have been
    called before its first use. C++ application should use
    EGS_RandomGenerator objects instead, either
    EGS_SimpleApplication::rndm or EGS_AdvancedApplication::rndm
    or their own RNG object.
 */
extern __extc__ void egsRandomGet(EGS_Float *ranno);

/*! Shorthand notation for the \c egs_fill_rndm_array mortran subroutine */
#define egsFillRandomArray F77_OBJ_(egs_fill_rndm_array,EGS_GET_RNDM_ARRAY)

/*! \brief Get \a n random numbers stored in the array pointed to by \a rarray

  This function should only be used from the C interface after the RNG
  has been initialized using egsRandomInit() or egsRandomDefaultInit().
  C++ objects should use EGS_RandomGenerator objetcs instead,
  either EGS_SimpleApplication::rndm or EGS_AdvancedApplication::rndm
  or their own RNG object.
 */
extern __extc__ void egsFillRandomArray(const EGS_I32 *n, EGS_Float *rarray);

/*****************************************************************************
       Some utility functions
 *****************************************************************************/

/*! Shorthand notation for the \c egs_tot_time mortran function */
#define egsElapsedTime F77_OBJ_(egs_tot_time,EGS_TOT_TIME)
/*! \brief Get elapsed time

  This function returns the elapsed time in seconds since the last call
  (if \a flag = 0) or since the first call (if \a flag = 1).
 */
extern __extc__ EGS_Float egsElapsedTime(int *flag);

/*! Shorthand notation for the \c egs_etime mortran function */
#define egsCpuTime F77_OBJ_(egs_etime,EGS_ETIME)
/*! \brief Get the CPU time used since the process started

  A better way to measure the CPU time in C++ applications is to use
  an EGS_Timer object.
 */
extern __extc__ EGS_Float egsCpuTime(void);

/*! Shorthand notation for the \c get_transport_parameter mortran subroutine */
#define egsGetTransportParameter F77_OBJ_(get_transport_parameter,GET_TRANSPORT_PARAMETER)
/*! \brief Get the transport parameter settings from the input file

  This function gets transport parameters related input from a section
  in the input file delimited by <code>:start MC transport parameter:
  :stop MC transport parameter:</code> The settings are echoed to the
  Fortran IO unit \a ounit. In C++ applications a better way to parse
  the input file is to use an EGS_Input object.
 */
extern __extc__ void egsGetTransportParameter(const int *ounit);


/*****************************************************************************
  Fuctions that must be implemented by user codes written in C
 *****************************************************************************/

/*! Shorthand notation for the \c egs_howfar subroutine */
#define egsHowfar F77_OBJ_(egs_howfar,EGS_HOWFAR)
/*! \brief Calculate distance to the next boundary along the current
           direction of motion.

  This function must be implemented in user codes written in C.
  (implementation is automatically provided for C++ applications
  deriving from EGS_SimpleApplication or EGS_AdvancedApplication).
  The intended step length is available in #the_epcont->ustep,
  particle position, direction, region number, etc. from
  the top particle on the particle stack #the_stack.
  This function must calculate the distance \c t to the next boundary along the
  particle direction and
   - If t > the_epcont->ustep, just return
   - If t <= the_epcont->ustep, set the_epcont->ustep to \c t, set
     the_epcont->irnew to the new region index and set
     the_useful->medium_new to
     the new medium. If you wish to have a transport cutoff higher
     than the particle production threshold of the medium in the
     region, set the_bounds->ecut_new or the_bounds->pcut_new to the
     desired value(depending on particle charge). If the current particle
     is an electron and you want to restrict the step size in the new
     region, set the_etcontrol->smax_new to the desired value.
     If you want to have a non-default mass density in the new region,
     set the_useful->rhor_new to the desired value (ratio of desired mass
     density to the default mass density).
   - If you want to discard the particle immediately, set
     the_useful->idisc to a positive value.
   - If you want to discard the particle after the step is completed,
     set the_useful->idisc to a negative value.
 */
extern __extc__ void egsHowfar(void);

/*! Shorthand notation for the \c egs_hownear subroutine */
#define egsHownear F77_OBJ_(egs_hownear,EGS_HOWNEAR)
/*! \brief Calculate minimum perpendicular distance to any
           surrounding boundary and return it in \a tperp.

  This function must be implemented by user codes written in C
  (implementation is automatically provided for C++ applications
  deriving from EGS_SimpleApplication or EGS_AdvancedApplication).
  If it is impossible to calculate \a tperp, you must set tperp to 0.
  In this case the transport will be determined by the setings of
  EGS_EtControl::transport_algorithm,
  EGS_EtControl::bca_algorithm and
  EGS_EtControl::skin_depth_for_bca (see PIRS-701 for details).
 */
extern __extc__ void egsHownear(EGS_Float *tperp);

/*! Shorthand notation for the \c egs_ausgab subroutine */
#define egsAusgab F77_OBJ_(egs_ausgab,EGS_AUSGAB)
/*! \brief User scoring function

  This function must be implemented by user codes written in C
  to perform the actual scoring. The value of \a iarg depends
  on the event that triggered the call (see PIRS-701 for details).
  C++ applications deriving from EGS_SimpleApplication or
  EGS_AdvancedApplication must implement the respective
  \c %ausgab() virtual function.
*/
extern __extc__ void egsAusgab(EGS_I32 *iarg);

/*! Shorthand notation for the \c egs_start_particle subroutine */
#define egsStartParticle F77_OBJ_(egs_start_particle,EGS_START_PARTICLE)
/*! \brief Start the transport of a new particle.

  This function is called just before the transport of a new
  particle begins (this may be either directly a source particle or
  just the next particle on the stack). User codes written in C
  must implement this function to set
  - the_useful->medium to the medium in the region the particle is in.
  - the_useful->rhor to the ratio of the region mass density to the
    default mass density (if you want to use a non-default mass density)
  - the_bounds->ecut or the_bounds->pcut to the transport cut-off energy
    (depending on the particle charge). Note that e/pcut must
    be greater than, or equal to, the_thresh->ae/p of the current medium
  - the_etcontrol->smaxir to the maximum geometrical step-size restriction,
    if the particle is an electron and you wish to impose such restrictions.
  Simple C++ applications deriving from EGS_SimpleApplication can not
  have a region-by-region variation of transport threshold energies,
  smax or non-default mass densities and an implementation is
  automatically provided. An implementation is also provided for
  advanced C++ applications deriving from EGS_AdvancedApplication.
  Such applications can implement region-by-region variation of
  these quantities by re-implementing
  EGS_AdvancedApplication::startNewParticle().
 */
extern __extc__ void egsStartParticle(void);

#endif

