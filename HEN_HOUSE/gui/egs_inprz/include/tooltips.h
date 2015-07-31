/*
###############################################################################
#
#  EGSnrc egs_inprz tooltips headers
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
#  Author:          Ernesto Mainegra-Hing, 2001
#
#  Contributors:    Blake Walters
#
###############################################################################
*/


#ifndef TOOLTIPS_H
#define TOOLTIPS_H

// Tool tip definitions for General Tab

#define USER_CODE_AREA "Select default area for user code location." \
                     "\nIf $HOME/egsnrc or $HEN_HOUSE are selected, "  \
                     "\ndialog boxes will automatically start looking there." \
                     "\nOther starts looking in $HOME."
#define PEGS4_AREA   "Select default area for pegs4 data location." \
                     "\nIf $HOME/egsnrc or $HEN_HOUSE are selected, "  \
                     "\ndialog boxes will automatically start looking in" \
                     "\n$HOME/egsnrc/pegs4/data or $HEN_HOUSE/pegs4/data." \
                     "\nOther starts looking in $HOME.  If you opt to run" \
                     "\nin PEGSless mode, then cross-sections are calculated" \
                     "\non the fly, and the media in your simulation must be" \
                     "\nspecified either directly in the .egsinp file or in" \
                     "\na material data file.  Media and other parameters for" \
                     "\ncross-section calculations are specified in the " \
                     "\nMedia Definition tab."
#define RUN_MODE     "Select running mode"
#define RUN_STAND_ALONE "Run user code interactively in local machine"
#define RUN_PARALLEL  "Run user code using  a batch system." \
                                          "\n Make sure the latter is installed since inputRZ" \
                                          "\n checks for script pprocess or egs_batch_run."
#define PPROCESS_PARAM  "Define parameters for use with pprocess."
#define NUMBER_OF_JOBS  "If > 1, jobs processed in parallel using pprocess"
#define QUEUE_TYPE  "Type of calculation:\n"\
                                      "short  = 20 min \n"\
                                      "medium = 2 hrs \n "\
                                      "long = 40 days (default)."
#define START_JOB  "When parallel processing, jobs will be labelled name_wN,\n "\
                                    "where N starts with this number."
#define OPEN_INPUT_FILE  "Load egsnrc input file using an Open File Dilaog.\n"\
                                               "This is more general than the combo box, allowing\n"\
			     "to open files in other locations."
#define OPEN_PEGS_FILE  "Load pegs4 data file using an Open File Dilaog.\n"\
                                               "This is more general than the combo box, allowing\n"\
			     "to open files in other locations."

/* removed to separate file to give users redifining option
static const char* queues[] = {
"20 minutes",
"2 hours",
"40 days (default)"
};
*/

static const char* comp_type[] = {
"process mortran source into fortran and"
"\ncompile fortran source.(default)",
"Just process mortran source code into fortran.",
"Compile the fortran source."
};

// Tool tip definitions for I/O Control Tab

#define NO_RND_NUM "Do not store initial random numbers"
#define LAST_RND_NUM "Store initial random numbers of last history"
#define ALL_RND_NUM  "Store all initial random numbers"
#define DEP_RND_NUM  "Store initial random numbers of histories" \
                                         "\n depositing energy in the cavity"
#define DOSE_REGIONS  "Specify which regions to output the dose scored." \
                                          "\nThis is only useful if there are a large number" \
			"\nof regions and one only cares about a few of them."\
			"\nValues greater than the actual number of regions in"\
			"\nthe present problem are reduced to the correct values."

#define SLOTE_FLU "If equal 0.0, one must define energy bins for output in the table  below.\n"\
		     " If SLOTE > 0.0, set up equal energy bins of width SLOTE. Doubled if needed \n"\
		     " to cover whole spectrum.\n"\
		     "If SLOTE < 0.0, set up equal log. bins of width abs(SLOTE). \n"\
		     "If SLOTE = -999, set up $EBIN bins, top 10% linear for SPR calculations."

#define TOP_BIN_FLU "tops of output bins in MeV starting with the lowest, \n"\
		         " end with value < or = last value."

static const char* out_cav[] = {
"short output - just the cavity summary"
"\nand the dose grid.",

"cavity summary, dose grid and details for"
"\neach cavity zone."
};

static const char* out_dos[] = {
"short output - just dose grid(DG)",

"output dose summary only (DS)<br>"
"(<i>no kerma output </i>)",

"output material summary grid(MG)"
"\nand dose grid (DG).",

"output material summary grid(MG)<br>"
"and dose summary (DS).<br>"
"(<i>no kerma output </i>)",

"output material summary grid(MG), "
"\ndose grid (DG) and dose summary (DS)."
};

static const char* spectrum_type[] = {

"Print total fluence spectra  only.",

"Print total fluence spectra and electron <br>"
"spectra, excluding <i> knock-on </i> electrons <br>"
"and descendants.",

"As option <i> electron primaries </i> but also"
"<br>excludes bremsstrahlung secondaries and "
"<br>descendants.",

"Print spectra excluding all secondary or"
"<br>scattered particles.",

"Print spectra for <i> knock-on </i> electrons"
"<br>and their descendants."
};

static const char* electron_transport[] = {

"Normal electron transport (discrete interactions).",

"no discrete interactions (used for CDSA calculations"
"<br>but note that special data sets are also needed"
"<br>to do a full CSDA calculation."
};

static const char* irestart[] = {

"First run for this input file.",

"Restart a previous run, <i> i.e. </i> add more histories.",

"Just read in the raw data and do the statistical"
"\nanalysis( gives no timing - restart 100 histories"
"\nto get the same effect and more info)",

"Read starting random numbers from a file <br>"
"(<i> e.g. </i>FOR OUTPUT TO A GRAPHICS PACKAGE).",

"Post-process distributed runs (all files <br>"
"named <i> filenamebase_w# </i>)."
};

static const char* iwatch[] = {

"Normal output.",

"Output on every discrete interaction.",

"Output on every electron/photon step as well.",

"Prints out only when energy is deposited.",

"Prints out file for graphics."
};

// Tool tip definitions for Monte Carlo Tab

#define CPU_TIME  "Execution will terminate cleanly as long" \
                                 "\nas one batch finishes within this CPU time. "

#define ACCURACY "Finish the run if this accuracy is obtained at the"\
		      "\nend of a batch prior to the total number of histories"\
		      "\nbeing run."

#define RND_GEN  "Which generator used is selected in the "\
		    "\nEGSnrc user code configuration file."

#define RND_1 "RANLUX: Luxury level between 1 and 4 (default 1)"\
	           "\nRANMAR: seed between 1 and 31328 (default 1802)"

#define RND_2 "RANLUX: Sequence to use, from 1 to 1,073,741,824 ( = 2<sup>30</sup> )<br>"\
	           "RANMAR: Seed between 1 and 30081 (default 9937)"

#define SLOTE "Defines the output energy bins. If > 0.0, equal size bins"\
	           "\nof this width in MeV used. It will get increased by  factors"\
	           "\nof two until the whole spectrum input gets covered."\
	           "\nIf < 0.0, input tops of individual energy bins for pulse "\
	           "\nheight distribution (see second column in table below)."

#define DELTAE	"DOSRZnrc analyses peak efficiencies using a bin width"\
	           "\nof 2*DELTAE about each peak and two background regions"\
	           "\nof width DELTAE above and below the peak."\
	           "\nDefault value is 0.005 MeV (meaningless for electrons "\
	           "\nand positrons)."

#define BIN_TOP "Tops of energy bins, lowest energy first."

#define SENSITIVE_VOL "Region numbers(IRL) of sensitive volume"

#define INDEPENDENT "\n(this columns are absolutely independent)"

#define KERMA  "Score kerma and ratio of kerma to dose" \
                            "\nin each region. "

#define REGENERATION "If checked, calculation performed with regeneration"	\
			"\nof the parent photon after they have interacted. A"\
			"\ntypical setting when FANO conditions are examined."

static const char* ifull_cav[] = {
"Just calculate total dose and that due\n"
"to stoppers and discards.",

"Calculate total dose, that due\n"
"to stoppers and discards,\n"
"Aatt and Ascat.",

"Calculate total dose, that due\n"
"to stoppers and discards,\n"
"Aatt, Ascat and also Ap.",

"Calculate total dose, that due\n"
"to stoppers and discards,\n"
"Aatt, Ascat, Ap plus Afl \n"
"and <s>g,w as well"
};

static const char* ifull_dos[] = {
"Just calculate total dose and that due"
"\nto stoppers and discards.",

"Calculate total dose, that due"
"\nto stoppers and discards,"
"\nand analyze the total dose"
"\nper entrance region.",

"Score a pulse height distribution in"
"\nthe volume specified after the"
"\nmaterial inputs.",

"Score the scatter fraction instead of"
"\nstoppers.  Only for incident photons."
"\nDose after Compton and for fluorescent"
"\nphotons if followed."
};

// Tool tip definitions for Geometry Tab

#define GROUPS "Input groups of slabs of equal thickness."
#define INDIVIDUAL "Verbose input of the geometry and media."
#define CAVITY "Just input the wall thickness, cavity and electrode"\
                           "\ndimensions and materials via Cavity Tab (next)."\
	             "\nGeometry is then set up automatically."

static const char* media_regions[] = {
"using the IRL region numbers",
"using the radius number IX and plane number IZ"
};

// Tool tip definitions for Cavity Tab

// Tool tip definitions for Source Tab

static const char* imode[] = {
"7 variables/record: X,Y,U,V,E,WT,LATCH",

"8 variables/record: the above + ZLAST"

};

static const char* sources[] = {
"PARALLEL BEAM INCIDENT FROM THE FRONT\n"
"( positive Z-axis side).",

"POINT SOURCE ON AXIS INCIDENT FROM FRONT" ,

"BROAD PARALLEL BEAM INCIDENT FROM FRONT\n"
"( positive Z-axis side). Beam has unit area and large scoring area",

"UNIFORM ISOTROPICALLY RADIATING DISK OF FINITE SIZE.\n"
"(must be allowed for in the geometrical definitions)",

"FOR CENTRAL AXIS FLUENCE VS BEAM RADIUS.\n"
"Note: this source option treats the cylindrical radii as\n"
"beam radii. The largest radius must be infinite and the\n"
"phantom must be homogeneous (at least in each layer).",

"PARALLEL BEAM INCIDENT FROM THE SIDE\n"
"( positive Y-axis side).",

"POINT SOURCE INCIDENT FROM THE SIDE",

"POINT SOURCE OFF AXIS",

"PARALLEL BEAM FROM ANY ANGLE",

"POINT SOURCE ON AXIS INCIDENT FROM FRONT. All events below radius\n"
"RMINBM are terminated by the source routine by giving them zero weight.",

"POINT SOURCE OFF AXIS. Similar to source 12 but uses an alternative\n "
"sampling of points on the surface of the RZ-geomtry. The motivation was\n "
"to check source 12 was OK and the effect of varying weights from the source\n "
"on the statistical uncertainty (contrary to source 12, source 15 produces\n "
"essentially constant weights if the geometry-to-source distance is large compared\n"
"to the geometry dimension, a typical situation for ion chamber simulations)",

"EXTENDED (CIRC. OR RECT.) SOURCE OFF AXIS",
"RADIAL DISTRIBUTION INPUT. Parallel beam incident from front with radial distribution. ",

"FULL BEAM PHASE-SPACE BEAM DATA, INCIDENT ON FRONT FACE",

"FULL BEAM PHASE-SPACE BEAM DATA FROM ANY ANGLE, INSIDE OR OUTSIDE",

"BEAM TREATMENT HEAD SIMULATION AS SOURCE INCIDENT FROM AN ANGLE,\n"
"INSIDE OR OUTSIDE PHANTOM. PARTICLES ARE READ DIRECTLY FROM A BEAM\n"
"SIMULATION COMPILED AS A SHARED LIBRARY."
};

#define RAD_DIS_LOCAL "Radial distribution is to be input locally through the .egsinp file"

#define RAD_DIS_EXTERNAL "Radial distribution is to be input via an external file"

#define SOURCE16_TEMP3 "source radius if TEMP4 <= 0 (emitting position picked uniformly\n" \
                                              " within circle) or half-size of the radiating rectangle in x-direction\n"\
                                              "if both, TEMP3 and TEMP4 >= 0."

#define SOURCE16_TEMP4 "source radius if TEMP3 <= 0 (emitting position picked uniformly\n" \
                                              " within circle) or half-size of the radiating rectangle in y-direction\n"\
                                              "if both, TEMP3 and TEMP4 >= 0."

#define PHSP_FILE "The name of the phase-space file to be used as a source.\n"\
                  "Path remembered by the GUI when using the Open File dialog,\n"\
                  "although not shown in combo box. \n"

#define BEAM_CODE "The name of the accelerator code being used as a source\n"\
                  "including the BEAM prefix (ie BEAM_accelname). This code\n"\
                  "must have been compiled as a shared library and exist as\n"\
                  "libBEAM_accelname.so (for Linux/Unix) or libBEAM_accelname.dll\n"\
                  " (for Windows) in directory $EGS_HOME/bin/config.\n"

#define BEAM_INP "Name of a working input file (without extension) for\n"\
                 "the BEAM_accelname code. This file must trun on output\n"\
                 "to a phase-space file at the scoring plane. Particles\n"\
                 "reaching scoring plane are used as incident particles\n"\
                 "in the user-code. Input file must exist in "\
                 "$EGS_HOME/BEAM_accelname."

#define BEAM_PEGS "Name of pegs4 data set (without extension) used by\n" \
                  "the BEAM simulation. The data set must exist in either\n" \
                  "$HEN_HOUSE/pegs4/data or in $EGS_HOME/pegs4/data."

#define BEAM_MIN_WEIGHT "Min. weight of particles to use from\n" \
                        "the BEAM simulation (defaults to -1E30)"
#define BEAM_MAX_WEIGHT "Max. weight of particles to use from\n" \
                        "the BEAM simulation (defaults to 1E30)"

#define BEAM_DIST "Perpendicular distance of the phase-space\n" \
                  "plane to the point of rotation in cm."


#define BEAM_ANGLE "Angle of rotation in degrees. The rotation\n" \
                   "is performed around an axis that is parallel\n" \
                   "to the x-axis and passes through the point\n" \
                   "(x,y,z)=(0,0,ZOFFSET)."

#define BEAM_ZOFFSET "Point of rotation. If |ZOFFSET| > 1e4,\n" \
                     "the centre of the geometry is taken as\n" \
                     "the point of rotation (but note that\n" \
                     "the maximum value allowed by the input\n" \
                     "routine is 1e6, so that |ZOFFSET| must\n" \
                     "be between 1e4 and 1e6 to use the centre\n" \
                     "of the geometry automatically)."

#define BEAM_XOFFSET "X offset of scoring plane in BEAM\n" \
                     "simulation (cm).  Offsets are applied before\n" \
                     "rotating the source."

#define BEAM_YOFFSET "Y offset of scoring plane in BEAM\n" \
                     "simulation (cm).  Offsets are applied before\n" \
                     "rotating the source."




// Tool tip definitions for Transport Parameters Tab

#define GLOBAL_PCUT "Global photon transport cut off energy (in MeV)."
#define GLOBAL_ECUT "Global electron transport cut off energy" \
                    "\n(in MeV, includes electron rest energy)."

#define GLOBAL_SMAX "Global maximum step-size restriction for electron" \
                    "\ntransport (in cm). If using default EGSnrc" \
                    "\nelectron-step algorithm, no SMAX-restriction necessary."

#define ESTEPE_TIP "Maximum fractional energy loss per step." \
               "\nDefault is 0.25 (25%)."

#define XIMAX_TIP "Maximum first elastic scattering moment per step." \
                  "\nNEVER use value greater than 1 as this is beyond"  \
                  "\nthe range of MS data available."

#define SKINDEPTH_TIP "distance from a boundary at which the algorithm will" \
                      "\n go into single scattering mode (in elastic MFP)"

#define SPIN_EFFECTS "Turns off/on spin effects for electron elastic scattering." \
                     "\nABSOLUTELY necessary for good backscattering calculations."

#define BCA_TIP "Two selections possible:" \
                "\nEXACT, means crossing boundaries in a single scattering mode" \
                "\nPRESTA-I, means boundaries crossed a la PRESTA," \
                "\ni.e. lateral correlations turned off and MS forced at boundaries."

#define PAIR_ANG_SAMPLING "If Off: pairs set in motion at angle m/E." \
                          "\nIf Simple: leading term of angular distribution turned on." \
                          "\nIf KM: turns on 2BS from Koch and Motz"

#define BREMS_ANG_SAMPLING "If KM: complete modified Koch-Motz 2BS used." \
                           "\nIf Simple: only leading term used."

#define BOUND_COMPTON "If On, Compton scattering treated in the Impuls Approximation," \
                      "\nelse treated with Klein-Nishina. Make sure to turn it on for" \
                      "\nlow energy applications. Default is On."
#define PE_ANG_SAMPLING "If Off, photo-electrons get direction of `mother' photon," \
                        "\nelse Sauter's formula used (which is, striktly speaking," \
                        "\nvalid only for  K-shell photo-absorption)."

#define RAYLEIGH_SCAT "Turns on/off coherent (Rayleigh) scattering." \
                      "\nDefault is Off. Should be turned on for low energy" \
                      "\napplications but needs PEGS4 data set with data."

#define RELAXATION_TIP "If On, shell vacancies relaxed via emission of fluorescent X-Rays," \
                       "\nAuger and Koster-Cronig electrons (element and shell sampled from"  \
                       "\nappropriate cross sections). Make sure to turn this option on for" \
                       "\nlow energy applications. Default is On."

#define ESTEPE_ALGORITHM "Algorithm used to take into account lateral and longitudinal" \
                         "\ncorrelations in a condensed history step."

#define BREMS_XSECTION "If BH:  Bethe-Heitler brems. cross sections used." \
                       "\nIf NIST: NIST brems cross section used (basis for" \
                       "\nICRU radiative stopping powers)."

#define PHOTON_XSECTION "Select photon cross section compilation." \
                        "\n   si: Default photon cross section (Storm&Israel)" \
                        "\n xcom: Data taken directly from the XCOM library" \
                        "\n epdl: Data taken directly from the EPDL97 library" \
                        "\nPEGS4: Data taken directly from the PEGS4 file" \
                        "\n\nThe prefix for any other photon cross setion compilation" \
                        "\nplaced in $HEN_HOUSE/data/ in the form prefix_*.data" \
                        "\nwill also be automatically detected."

#define EII_XSECTION "On: EII modeled using EGSnrc's implementation by Kawrakow" \
                     "\nOff: No EII modeled" \
                     "\ncasnati: Casnati's empirical fit to experimental data" \
                     "\nik: Kawrakow's semi-empirical theory" \
                     "\nkolbenstvedt: Revised Kolbenstvedt relativistic theory" \
                     "\ngryzinski: Gryzinski's ab-initio classical theory" \
                     "\npenelope: Bote and Salvat semi-empirical theory" \
                     "\n\nThe suffix for any other EII cross setion compilation" \
                     "\nplaced in $HEN_HOUSE/data/ in the form eii_suffix.data" \
                     "\nwill also be automatically detected."

// Tool tip definitions for Transp Param. by Regions Tab

// Tool tip definitions for Variance Reduction Tab

#define RANGE_REJECTION "There are 2 components to range rejection." \
		       "\nOne uses the EGSnrc range rejection below ESAVEIN"\
		       "\nand terminates any charged particle which cannot get"\
		       "\nout of its local region."\
		       "\nThe second component terminates any charged particle"\
		       "\nwhich cannot reach the cylinder enclosing the cavity region"\
		       "\nand any other region of the same material as the cavity."\
		       "\nThis cylinder is determined automatically."

#define RR_ESAVEIN "If electron range rejection is on, discard an electron when E< ESAVEIN." \
                                    "\nThis ignores bremsstrahlung losses below ESAVEIN.This parameter" \
                                    "\nmust be input even if not used and if left 0, it effectively turns off range"\
                                    "\nrejection."\
	                      "\nNote: ESAVEIN is total energy (i.e. Ekin + 0.511 MeV)"

#define PHOTON_FORCING "Force photons to interact in the geometry."
#define START_PHOTON_FORCING "Start forcing photons at this interaction number"
#define STOP_PHOTON_FORCING "Number of photons interactions after which to stop"\
			                 "\nforcing photon interactions."
#define PHOTON_SPLITTING "# of times to split a photon, if &lt; 2, normal transport.<br>"\
			       "Overrides PHOTON FORCING if &gt;= 2. Can only<br>"\
			       "be &gt;= 2 if IFULL = <b>dose and stoppers</b> or <br>"\
			       "IFULL = <b>Aatt and Ascat</b>"

#define EXPONENTIAL_TRANSPORT "Parameter for pathlength biasing <0 FOR SHORTENING"\
			                     "\nIf 0.0, no biasing done."

#define RUSSIAN_ROULETTE "Play russian roulette as any photon crosses a defined plane."\
			        "\nIf any of the parameters below is zero, no Russian roulette played."

#define RUSSIAN_ROULETTE_DEPTH "Play russian roulette as any photon crosses the plane"\
				       "\nZ = Russian Roulette Depth."
#define RUSSIAN_ROULETTE_FRACTION "Survival probability for Russian Roulette. If the photon"\
				       "\nsurvives, its weight is increased by inverse of this fraction."

#define CS_ENHANCEMENT_CAVRZNRC "The cross section enhancement technique in CAVRZnrc is applied globally to the entire geometry."\
	                                                        "\nIf this input is set > 1  all other input concerning photon forcing, splitting, exp. transform, etc.,"\
	                                                        "\n  is ignored. The calculation result corresponds  ALWAYS to 'Aatt and Ascat', but only Awall is"\
		                                          "\ncalculated. Scoring is done on a history-by-history basis for the whole cavity only."

#define CS_ENHANCEMENT_DOSRZNRC "Cross section enhancement in DOSRZnrc allows to increase the photon cross section"\
		                                         "\nof a material in an arbitrary region in the geometry by a given enhancement factor, and"\
		                                         "\nhence to increase the interaction density by that factor. Useful to calculate energy"\
				             "\ndeposition in a relatively small part of the geometry."

#define CS_ENHANCEMENT_FACTOR "Scales the photon cross section by this factor in a specified set of regions (DOSRZnrc)"\
		                                     "\nor in the entire geometry (CAVRZnrc)."

#define BREMS_SPLITTING "Splitting Bremsstrahlung photons is a very powerful technique for brems. beams."\
			     "\nIn accelerator modelling it has been shown that splitting factors of 20 - 40 are optimal"\
			     "\nand can save a factor of 4 in computing time."

#define BREMS_SPLITTING_FACTOR "Number of times to split Bremsstrahlung photons."

#define CHARGED_PARTICLE_RR "This form of Russian Roulette is meant to complement the use of bremsstrahlung splitting"\
			             "\nfor those cases where only the photons are of significant interest. It is designed to ensure"\
			             "\nthat charged particles carry their natural weight."

// Tool tip definitions for Plot Tab

#define RADIAL_PLOT "Radial IX: plot vs depth in radial region"

#define PLANAR_PLOT "Planar IZ: plot vs radius in planar slabs"

// Tool tip definitions for Media Definition Tab--only long ones are included here

#define MATERIAL_DATA_FILE "The full name of a file containing the specifications for the"\
                           "\nmedia used in your geometry."

#define INPMEDIA_DEF "You have the option of specifying media directly in the .egsinp file."\
                     "\nParameters specified here supersede those for a medium of the same name"\
                     "\nin the material data file."

#define MEDIUM_NAME "specify medium name.  If defining a new medium, select 'define new medium',"\
                    "\ninput the name of the medium, then hit <return> to add the medium to the list."\
                    "\nTo delete a medium, delete its name, select away from the medium, and it will"\
                    "\n be deleted from the list."

#define DENSITY_FILE "specify a density correction file which, when applied to calculated cross-sections,"\
                     "\nresults in agreement with stopping powers published in ICRU37.  The browser"\
                     "\nstarts in the $HEN_HOUSE/pegs4/density_corrections parent directory and allows"\
                     "\nyou to enter either the 'compounds' or 'elements' subdirectory."\
                     "\nOmit the directory path and '.density' extension from the filename."

#define STERNCID "The Sternheimer-Seltzer-Berger ID for the medium.  If this matches the ID in"\
                 "\nan internal lookup table, pre-calculated density effect parameters are applied"\
                 "\nto collision stopping powers.  Superseded if a density correction file is specified."

#endif

