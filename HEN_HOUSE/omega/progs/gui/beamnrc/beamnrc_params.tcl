
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: parameters
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
#  Author:          Joanne Treurniet, 1998
#
#  Contributors:    Blake Walters
#                   Dave Rogers
#                   Iwan Kawrakow
#                   Ernesto Mainegra-Hing
#                   Frederic Tessier
#
###############################################################################
#
#  The contributors named above are only those who could be identified from
#  this file's revision history.
#
#  This code is part of the BEAMnrc code system for Monte Carlo simulation of
#  radiotherapy treatments units. BEAM was originally developed at the
#  National Research Council of Canada as part of the OMEGA collaborative
#  research project with the University of Wisconsin, and was originally
#  described in:
#
#  BEAM: A Monte Carlo code to simulate radiotherapy treatment units,
#  DWO Rogers, BA Faddegon, GX Ding, C-M Ma, J Wei and TR Mackie,
#  Medical Physics 22, 503-524 (1995).
#
#  BEAM User Manual
#  DWO Rogers, C-M Ma, B Walters, GX Ding, D Sheikh-Bagheri and G Zhang,
#  NRC Report PIRS-509A (rev D)
#
#  As well as the authors of this paper and report, Joanne Treurniet of NRC
#  made significant contributions to the code system, in particular the GUIs
#  and EGS_Windows. Mark Holmes, Brian Geiser and Paul Reckwerdt of Wisconsin
#  played important roles in the overall OMEGA project within which the BEAM
#  code system was developed.
#
#  There have been major upgrades in the BEAM code starting in 2000 which
#  have been heavily supported by Iwan Kawrakow, most notably: the port to
#  EGSnrc, the inclusion of history-by-history statistics and the development
#  of the directional bremsstrahlung splitting variance reduction technique.
#
###############################################################################
#
#  This script is read from the main script beamnrc_gui.  It has one procedure,
#  which reads the beamnrc_user_macros file to get the defaults set there.
#
#  It sets up the parameters used by the program for the main input parameters,
#  the arrays names, num_opts, options and help_text. 'names' contains the
#  labels displayed on the main input parameter window, 'num_opts' is the
#  number of options used in a pulldown menu for that parameter (if it is zero
#  a textbox is used for that parameter), 'options' contains the labels used
#  in the pulldown menu (if num_opts>0) and 'help_text' contains the text
#  displayed for that parameter's help button.  Note that the array 'values'
#  is used to store the current value of a main input parameter in the other
#  scripts.
#
#  It also defines the array cm_names, an array of 15 elements each containing
#  the name of a component module.  It defines the help text for the component
#  modules and the help text for the sources.
#
###############################################################################


# Help for the top window of the GUI:

set GUI_help_text {
How to use this GUI:

This GUI is meant to be used to generate BEAMnrc input files.

There are four options available to you from the main window:
1) Load a previous accelerator,
2) Specify a new accelerator,
3) Load a previous input file, and
4) Save input parameters as...

The "Load a previous accelerator" option allows you to read in\
	a set of component modules (CMs) from a file (the 'module file').  \
	All of the module files should reside in your\
	$EGS_HOME/beamnrc/spec_modules directory.  The GUI browser will\
	start you out there.  Once the accelerator is loaded, a window\
	will appear with a "Set main parameters" button and a list of\
	the CMs loaded accompanied by "Edit" buttons.  \
	These buttons allow you to set the parameters required by BEAMnrc\
	to run the simulation, such as CM geometry, materials and\
	other simulation parameters.

The "Specify a new accelerator" option allows you to build an accelerator\
	from scratch.  You MUST save the accelerator before leaving\
	the building stage, or it will not present the window for editing\
	the parameters.  Once the accelerator is saved, you can edit the\
	simulation parameters.

If you wish to load an accelerator which has already been completely\
	defined, you can start directly with "Load a previous input file".  \
	The GUI will search for the appropriate module file (which defines\
	the CMs used in the accelerator) based on the directory name of\
	the selected input parameter file, load it, then load the input file. \
	At this point you can edit the CM and simulation parameters.  \
	If you do not have an accelerator module file to load with it, \
	or if the input file is not in a directory named\
	"BEAM_<module filename>", this shortcut will not work.  You will have\
	to load or specify an accelerator, then load the input file.

The last step in using the GUI is "Save input parameters as...".  \
	Select a file name (with extension .egsinp) and click OK.  \
	That's it!  You're done.  Run BEAMnrc as usual.
}

# zero all parameter values (see BEAMnrc GUI maintenance manual
# for a list of meanings)

for {set i 1} {$i<=41} {incr i} {
    set numopts($i) 0
    set values($i) {}
    for {set j 1} {$j<32} {incr j} {
	set inc($i,1,$j) "  "
	set inc($i,2,$j) "  "
    }
}
for {set i 1} {$i<=4} {incr i} {
    set force_bdd($i) {}
}

set values(23,1) {}
set values(23,2) {}
set values(23,3) {}
set values(23,4) {}

set special_N {}
set s1opt "rectangular"
set s3opt "vertical"

set names(1) "Title";
set help_text(1) {
Title:

Type the title you wish to use for the simulation\
	in the space provided (maximum 80 characters).  \
	It is strongly recommended\
	to identify the name of the input file in the title.
}

set names(2) "Medium"
set values(2) "VACUUM"
set help_text(2) {
Medium:

In many component modules,  the region about the\
	central-axis or at the front or back of the CM is\
	assumed to be this medium.  It is thought of and\
	refered to as air, but can be anything.  Default\
	is VACUUM. The entry must exactly match the name\
	used in the .pegs4dat data file.
}

set names(3) "IWATCH Output"
set help_text(3) {
IWATCH Output:

"none" (0) is normal output, the default.
"show interactions" (1) outputs information for every discrete interaction.
"show each step" (2) outputs information for every electron/photon step.
"for EGS_Windows" (3) outputs a file for graphics.
"Special" (4) is a special setting for which selecting the number\
	N sets the program to output for every electron/photon\
	step on history N and to the default otherwise (for debugging purposes).
}
set numopts(3) 5;
set options(3,0) "none"
set options(3,1) "show interactions"
set options(3,2) "show each step"
set options(3,4) "for EGS_Windows"
set options(3,5) "Special"
set values(3) $options(3,0)

set names(4) "RNG Seed Options"
set help_text(4) {
RNG Seed Options, ISTORE:

"store RNG at start of each batch" (0) means store RNG seeds for the 1st history\
	of a batch.  This is the default.
"store RNG at start of each history" (1) means store initial RNG status (unit 2)\
	for each history being simulated.
"start run with stored RNG read in" (-1) means start first history with RNG\
	status from a file (unit 2).  This is a debugging tool.  \
	If run quits, rerun using "store RNG at start of each history", then try\
	again with Output set to "interactions" or "steps"\
	and/or the debugger on.}
set numopts(4) 3;
set options(4,0) "store RNG at start of each batch";
set options(4,1) "store RNG at start of each history"
set options(4,-1) "start run with stored RNG read in";
set values(4) $options(4,0)

set names(5) "Run option";
set help_text(5) {
Restart option, IRESTART:

"first time" (0) means that this is the first run for this data\
	set (default).  BEAMnrc initiates a new run, deleting all\
	of the output files from previous runs of the same\
	name if present (.egslog, .egslst, .egsrns, .egsphsp1, etc.).

"restart" (1) means that this is a restart of a previous run.  BEAMnrc reads\
	fluence and dose data from the previous run, the number of\
	histories already run, the time taken by the previous run, and\
	the random number seeds from the last batch of the previous run\
	from the .egsdat file; fluence and dose and their statistics\
	are averaged with those from the previous run; phase space output\
	is appended to that from the previous run; note that the number\
	of histories to run and the total CPU time allowed for the\
	simulation include the histories and CPU time from the previous run.

"create input file" (2): BEAMnrc creates the .egsinp file and then exits\
	without running the simulation.  This option isn't available\
	with the GUI since it would be redundant.

"analyze previous" (3): BEAMnrc reads dose and fluence data from a previous\
	run in the .egsdat file, performs statistical\
	analysis on the data and ouputs results to .egslst; no\
	simulation is run.

"analyze parallel jobs" (4) allows you to sum and analyze all the results of a group\
	of parallel jobs having the same base name as the input file but\
	with extension _w#, where # can be any positive integer.
}
set numopts(5) 5;
set options(5,0) "first time";
set options(5,1) "restart";
set options(5,3) "analyze previous";
set options(5,4) "analyze parallel jobs";
set values(5) $options(5,0)

set names(6) "Output Options"
set help_text(6) {
Output Options, IO_OPT:

The options are to output
    0) phase-space output at each scoring plane(the\
	    default),
    1) no phase-space output when particles cross\
	    scoring plane,
    2) no phase-space output but do data analysis for\
	    simplified source models, and
    3) phase-space output up to 100 k particle histories\
	    then do analysis only for simplified source models.
    4) output phase-space data in IAEA format at each scoring\
       plane (on Linux machines, requires a C++ compiler to compile\
       the IAEA libraries)

Naming scheme:
--------------
For BEAMnrc-format phsp files (options 0,3):
inputfile.egsphsp#, where # is the no. of the scoring plane

For IAEA-format phsp files (option 4):
inputfile.#.IAEAheader (header file) and inputfile.#.IAEAphsp (phsp data),\
where # is the no. of the scoring plane
}
set numopts(6) 5;
set options(6,0) "phase space at each scoring plane"
set options(6,1) "no phase space at scoring planes"
set options(6,2) "no phase space but do analysis"
set options(6,3) "phase space output and do analysis"
set options(6,4) "IAEA phase space at each scoring plane"
set values(6) $options(6,0)

set names(7) "Store Data Arrays";
set help_text(7) {
Store Data Arrays, IDAT:

Store data arrays for re-use (0) (takes time but is safer), \
	or don't store them (1).
}
set numopts(7) 2;
set options(7,0) "yes"
set options(7,1) "no"
set values(7) $options(7,0)

set names(8) "LATCH option"
set help_text(8) {
LATCH option:

The LATCH variable, associated with each particle in a simulation, \
	is a 32-bit variable used to track the particle's history. In\
	the GUI, there is an opportunity to define a mapping\
	from geometric regions to bits using the "Associate with LATCH bit"\
	option.  Thus, for example, it is possible that bit 5\
	corresponds to geometric region 3, and more importantly, one bit, \
	say 3, can correspond to multiple geometric regions, \
	e.g.  1,5,8.  Thus, although the JAWS may consist of 6\
	different geometric regions, they can all be associated\
	with a single bit.   Each bit is designated as follows:

bit 0
    Set to 1 if a bremsstrahlung or positron annihilation event occurs\
	    in the history; 0 otherwise(not used for 'non-inherited LATCH').
bit 1-23
    Used to record the region where a particle has been and/or interacted\
	    (Note that the bit set for a region is determined\
	    by 'Associate with LATCH bit' for that region)
bit 24-28
    Stores the region number in which a secondary particle is created; \
	    if these bits are all 0, the particle is a primary\
	    particle (not for 'non-inherited LATCH').
bit 29-30
    Store the charge of a particle when LATCH is output to a phase\
	    space file (see the User's Manual, Section 7 on phase\
	    space files). During a simulation, bit 30 is used to\
	    identify a contaminant particle but this information is\
	    not output to the phase space file.  The bit is set to 1\
	    if the particle is a contaminant particle; 0 otherwise. \
	    Note that if LATCH is not inherited, bit 30 loses its meaning.
bit 31
    Set to 1 if a particle has crossed a scoring plane more than once\
	    when LATCH is output to a phase space file (see the User's\
	    Manual, Section 7 on phase space files).

For secondary particles, recording the region number in which they were\
	created in bits 24-28 is equivalent to multiplying\
	the region number by 16777216. Thus, to retrieve the region of origin\
	of a secondary particles, the LATCH value of\
	the particle must be divided by 16777216.

The user controls the protocol for setting LATCH using the LATCH option\
	input variable. The possible settings are:

Non-Inherited LATCH (1):
    secondaries do not inherit LATCH values from the primaries\
	    that created them; bits 1-23 of a secondary particle carry\
	    no information about the regions its primary parent(s)\
	    has(ve) been. This option must NOT be used if you are scoring\
	    dose components since that option needs bit 30.

Inherited LATCH - set by passage (default) (2):
    LATCH values are passed on to secondary particles from the\
	    primaries that created them; bits 1-23 for a secondary\
	    particle include all regions in which the secondary\
	    particle has been plus those in which its primary\
	    parent(s) has(ve) been up to the point where the\
	    secondary was created; uses bits 24-28 to record where\
	    secondary particles were created and bit 0 to record\
	    whether or not a bremsstrahlung photon was involved in a\
	    particle's history.

Inherited LATCH - set by interactions (3):
    similar to the above, but for photons bits 1-23 record the regions\
	    in which the particles have interacted, rather than simply the\
	    regions in which they have been.

Please see the User's manual for more information on tracking a particle's\
	history with LATCH.
}
set numopts(8) 3;
set options(8,1) "non-inherited latch"
set options(8,2) "inherited latch - set by passage"
set options(8,3) "inherited latch - set by interactions"
set values(8) $options(8,2)

set names(9) "Score Last Z"
set help_text(9) {
Score Last Z, IZLAST:

"no" (0) means do not score ZLAST etc. (the default).
"last interaction" (1) means score the z-position of the last
    site of interaction for photons and creation of electrons
    by a photon.
"x,y,z of last interaction" (2) means score the xyz-position
    of the last site of interaction in the file $.egsgph
    to be used by EGS_Windows.  Note that for phase
    space inputs, ZLAST is passed through, but XLAST
    and YLAST are not.}
set numopts(9) 3
set options(9,0) "no";
set options(9,1) "last interaction"
set options(9,2) "x,y,z of last interaction"
set values(9) $options(9,0)

set names(10) "Number of histories";
set help_text(10) {
Number of histories, NCASE:

This parameter sets the number of histories to run for the\
	simulation.  The default number of histories is 100, which\
	is also the minimum allowed.  The number of\
	histories per batch is equal to NCASE/(ISTAT), \
	where ISTAT is the number of statistical bins. \
	Currently, ISTAT = 10. The total number of histories\
	after this run will be the sum of NCASE and the number\
	of histories from the previous run.
}
set values(10) 100

set names(11) "Initial RNG seed 1"
set help_text(11) {
Initial RNG seed 1, IXX:

The random number generator used requires 2 seeds between 1 and 31328 and\
	1 and 30081 respectively.  For each different pair of seeds, an\
	independent random number sequence is generated.  Once the seeds are\
	used to establish the state of the generator, the "seeds" output\
	in log files etc. are really just pointers between 1 and 98.
}
set values(11) 33

set names(12) "Initial RNG seed 2"
set help_text(12) {
Initial RNG seed 2, JXX:

The random number generator used requires 2 seeds between 1 and 31328 and\
	1 and 30081 respectively.  For each different pair of seeds, an\
	independent random number sequence is generated.  Once the seeds are\
	used to establish the state of the generator, the "seeds" output\
	in log files etc. are really just pointers between 1 and 98.
}
set values(12) 97

set names(13) "Maximum CPU hours allowed"
set help_text(13) {
Maximum CPU hours allowed, TIMMAX:

This is the maximum CPU time in hours allowed for this run.  \
	The default value is 0.99 hours if set to 0.  At the end of\
	each batch, an estimate is made of how much time is needed\
	to complete another batch.  If the next batch cannot be\
	completed in this time limit, BEAMnrc terminates the run and\
	analyzes the results for the shortened run.  The restart\
	feature can be used to continue the run if needed, as long\
	as data arrays are stored ("Store Data Arrays" option).
}
set values(13) 0.99

set names(14) "Bremsstrahlung Splitting";
set numopts(14) 3
set options(14,0) "none";
set options(14,1) "uniform"
set options(14,2) "directional"
set values(14) $options(14,0)

#splitting field radius
set dbrem(1) {}
#SSD
set dbrem(2) {}
#CM for e-/e+ splitting
set dbrem(3) 0
#plane in CM for e-/e+ splitting
set dbrem(4) 0
#0 to not redistribute e-/e+ radially, 1 to do so
set dbrem(5) 0
#position of Russ. Rou. plane
set dbrem(6) {}
#augmented range rejection
set dbrem(7) 0
#rejection plane
set use_rejpln {}
set z_rejpln {}
set dbrem(use_rejpln) 0
set dbrem(z_rejpln) {}

#defaults for dsb
set i_dsb 0
set splitcm_dsb 0
set dsb_delta 1.5

set help_text(dsb) {
Directional Source Biasing (DSB):

Directional source biasing can be used to increase the efficiency of isotropically \
radiating photon sources modelled using ISOURC=3.

The user selects a splitting field radius (FS), defined at an source-to-surface distance \
(SSD) encompassing the treatment field.  Primary photons directed into this field are \
split a user-selected number of times (NBRSPL).  If there is radial symmetry at the top \
of the treatment head, then further CPU time can be saved by splitting and radially redistributing \
primary photons upon entering the CM where radial symmetry ends (splitcm_dsb), thus reducing \
the number of primary photons that must be tracked above this CM.  The number of times that photons \
are split on entering splitcm_dsb is determined by the minimum linear distance between these split, radially
redistributed photons (dsb_delta). \
This input is used to divide the splitting field into radial bins, where each bin has an associated splitting \
number for primary photons directed into it.

If you are interested in electron contamination, then you must use electron (e-/e+) splitting.  Electron \
splitting uses the same algorithm as in directional bremsstrahlung splitting (DBS).  Inputs for electron splitting \
are similar to directional bremsstrahlung splitting in a MV photon beam with the exceptions that: 1) in general, secondary electron \
energies for isotropically radiating sources (e.g. Co-60) are lower than for MV photon beams, so the \
e-/e+ splitting plane and Russian Roulette plane may be closer to the SSD; 2) electron splitting may occur \
in a region of the treatment head that does not have radial symmetry, in which case you should not radially \
redistribute split e-/e+.

Note that directional bremsstrahlung splitting must be turned on to use DSB.  All DSB input parameters are \
shared with directional bremsstrahlung splitting with the exception of the additional \
inputs, splitcm_dsb and dsb_delta.  Thus, if you are \
selecting the parameters for DSB and open the window for inputting directional bremsstrahlung splitting \
you will see that the parameters there match their equivalent values in the DSB input window.

DSB has been shown to increase the efficiency of Co-60 dose calculations by a factor of up to ~400. \
Efficiency is maximized for a photon splitting number = ~20,000 and does not vary significantly for splitting \
numbers higher than this.  Also, efficiency shows little variation with the min. linear distance between \
split, radially-redistributed photons, and a value of 1.5 cm is recommended should you opt to split/radially \
redistribute photons.

For more information about DSB, see the BEAMnrc Manual.
}

set help_text(14) {
Bremstrahlung Photon Splitting, IBRSPL:

The options for Bremsstrahlung splitting are \
        "none" (0), "uniform" (1) and \
        "directional" (2), for no Brem splitting, UBS \
         and DBS, respectively.

In bremsstrahlung splitting, each bremsstrahlung event produces\
	not 1, but NBRSPL photons (NBRSPL > 1, set by the user\
	as "Number of brem photons").  Each bremsstrahlung photon has \
	weight 1/NBRSPL.  In the case of uniform bremsstrahlung splitting \
        (UBS), NBRSPL is a constant and is the same for all bremsstrahlung \
        events.  Directional bremsstrahlung \
        splitting (DBS), uses a constant value of NBRSPL but eliminates \
        (using Russian Roulette) those photons not aimed into the \
        user-defined field.  Not only bremsstrahlung events are split, but \
        also compton events, positron annihilation events and fluorescent \
        photons.  In order to avoid the CPU time required to generate and \
        play Russian Roulette with photons not aimed into the field, DBS \
        makes use of the subroutines, do_smart_brems, do_smart_compton and \
        uniform_photons, which only generate the photons that are aimed into \
        the field.

Our results show that DBS provides the greatest efficiency gain, with dose \
        efficiencies a factor of \
        up to 23 greater than with UBS, and a factor of up to 160 greater \
        than with no splitting.

For more information on UBS and DBS, click on the help button in \
        the input windows for each splitting algorithm.

}

set help_text(dbs_rr) {

Click in the "Augmented range rejection" box if you are using charged\
        particle range rejection\
        in the accelerator simulation (see "Main inputs" form) and wish to\
        make it more efficient.  This option is only available\
        if you are using charged particle splitting with DBS.  Range rejection\
        is carried out as before\
        (See the "help" for the range rejection option itself for details)\
        with the exception that all non-fat charged particles are subject to\
        Russian Roulette (survival probability=1/NBRSPL) if it is determined\
        that they cannot reach the nearest region boundary with energy>ECUT\
        REGARDLESS OF WHETHER OR NOT THEIR ENERGY IS ABOVE THE RANGE REJECTION\
        CUTOFF ENERGY (ESAVE_GLOBAL).  Note that this will not affect\
        bremsstrahlung production since particles surviving the Russian\
        Roulette have the opportunity to produce bremsstrahlung photons.\
        This option is only available when using DBS.  Preliminary studies in\
        a simulated 6 MV photon beam have shown that it can reduce CPU time\
        by ~20%.

}

set help_text(dbs_rejpln) {

If you define a rejection plane with directional bremsstrahlung splitting\
        then fat photons and electrons that are about to undergo interactions\
        at Z>= Z of the rejection plane are discarded.  This prevents\
        correlated particles from compromising statistics at a scoring plane.\
        Usually, the rejection plane is placed just above the scoring plane of\
        interest.
}

set names(41) "Brems cross-section enhancement"
set numopts(41) 2
set options(41,0) "off"
set options(41,1) "on"
set values(41) $options(41,0)
for {set i 1} {$i<=20} {incr i} {
   set bcse_meds($i) {}
}
set bcse_rr {}
set bcse_constant {}
set bcse_power {}
set num_bcse_meds 0
set help_text(41) {
    Bremsstrahlung cross-section enhancement (BCSE):

    If turned on (USE_BCSE=On) then you are asked to define the media\
    in which to use BCSE (BCSE_MEDNAME(i)--must be used in the accelerator),\
    an enhancement constant (BCSE_CONSTANT), and an enhancement power\
    (BCSE_POWER).  You also have the\
    option of turning Russian Roulette on or off.

    BCSE is a variance reduction technique which increases the effective\
    bremsstrahlung cross-section for the media\
    BCSE_MEDNAME(i).  It was designed to increase the\
    efficiency of simulations that involve x-ray production from\
    bremsstrahlung targets in both the kilovoltage and megavoltage range\
    (ie x-ray tubes and linear accelerators).  BCSE is compatible with all\
    other variance reduction techniques and is most efficient when used\
    in conjunction with UBS or directional bremsstrahlung splitting (DBS).\
    Use with UBS is recommended for 4-pi geometries, and use with DBS is\
    recommended for x-ray tube or clinical linear accelerator simulations.\
    For typical simulations, BCSE can increase efficiency by up to 5x\
    over that of simulations with no variance reduction and by an order\
    of magnitude over simulations using optimized UBS or DBS.

    If BCSE_POWER <= 0, then the bremsstrahlung cross-sections of\
    BCSE_MEDNAME(i) are enhanced by a factor of BCSE_CONSTANT.  If you set\
    BCSE_POWER > 0, then the enhancement factor varies and is computed for\
    each interacting electron based on its energy, T, using the equation:

    enhancement factor = 1 + BCSE_CONSTANT*T^(BCSE_POWER)

    BCSE also has its own Russian Roulette option, which can be used alone\
    or in conjunction with UBS.\
    If used with UBS, the Russian Roulette setting (i.e. on or off) must\
    mirror that for BCSE. \
    Russian Roulette is recommended if the user is only interested\
    in photons.  Note that BCSE Russian Roulette is not used with\
    DBS.

    Details of optimization of BCSE with UBS or DBS are given\
    in the BEAM Manual:

    Restrictions on use of BCSE:
    1.  If BCSE_MEDNAME exists in more than one region, but the user only\
        wants to use BCSE in one region, then the PEGS4 file must be\
        modified to duplicate the medium but give it a different name.\
        The different name must then be used in regions where the user does\
        not wish to turn on BCSE.

    2.  BCSE cannot be used if the electron beam incident on the bremsstrahlung\
        target has variable electron weights.
}

#    1.  Choose an optimum enhancement factor for the simulation type from\
#        the following table (efficiency is not very sensitive to exact choice\
#        of the enhancement factor):
#
#
#   4-pi geometry (kv range)----opt. BCSE_FACTOR~500
#   x-ray tube (mammography)----opt. BCSE_FACTOR~500
#   x-ray tube (diagnostic)-----opt. BCSE_FACTOR~200
#   x-ray tube (orthovoltage)---opt. BCSE_FACTOR~100
#   linac (Mv)------------------opt. BCSE_FACTOR~20
#
#
#    2.  Calculate the optimum bremsstrahlung splitting number (NBRSPL)\
#        to use in UBS or DBS using the following algorithm:
#
#      a) Perform a few short runs with the optimized enhancement factor\
#       given above with different values of NBRSPL.
#
#      b) For each run, calculate the effiency of the quantity of interest,\
#        eff(Ni), where Ni is the value of NBRSPL in the run, using:
#
#         eff(Ni)=1/Ti*si^2
#
#         where Ti is the CPU time for the run, and si\
#         is the uncertainty on the quantity of interest.
#
#      c) Fit Ni/eff(Ni) vs (Ni-1) to the polynomial:
#
#         Ni/eff(Ni)=A0+A1(Ni-1)+A2(Ni-1)^2
#
#         where A0, A1, A2 are polynomial coefficients.
#
#      d) The optimum value of NBRSPL=SQRT(A0/A2).
#
#    If using BCSE without UBS or DBS, then do not choose the enhancement\
#    factors from the above table, but do a few short runs with various values\
#    of BCSE_FACTOR (typically much larger than the values given in the table)\
#    and then use step 2, with the BCSE_FACTORs in place of the Ni, to find\
#    the optimum BCSE_FACTOR.
#

set names(40) "Split electrons or photons at CM"
set values(40) none
set numopts(40) [expr $maxvals(MAX_CMs)+1 ]
set options(40,0) "none"
for {set i 1} {$i<=$maxvals(MAX_CMs)} {incr i} {
    set options(40,$i) $i
}
set help_text(40) {
    Split electrons or photons at CM, ICM_SPLIT:

    If yes, split photons and electrons a user-specified number of times\
	    as soon as they cross the arbitrary splitting plane at the top\
	    of this CM.  It should not be set to 1, as for this case splitting\
	    does not happen.

    Splitting is designed primarily for photons and was originally\
	    introduced to allow splitting in phantoms placed at the\
	    bottom of an accelerator.  Its use near the top of the\
	    accelerator is possible but must be investigated carefully\
	    in case unwanted correlations are introduced.

    Splitting to 0 particles in not allowed; this is not a way to filter\
	    out a class of particles.  One should also not use too high\
	    a splitting number in the accelerator since it introduces\
	    correlations which may not be desireable, while gaining little.
}

set names(16) "Russian roulette";
set numopts(16) 2
set options(16,0) "off";
set options(16,1) "on"
set values(16) $options(16,0)

set names(17) "Incident particle";
set numopts(17) 3
set options(17,-1) "electron";
set options(17,0) "photon"
set options(17,1) "positron"
set options(17,2) "all"
set help_text(17) {
    Incident particle, IQIN:

    Set the charge of the incident beamnrc by selecting an\
	    incident particle.
}

set names(18) "Source number";
set numopts(18) 16;
set options(18,0) "0 - Parallel beam from the front"
set options(18,1) "1 - Point source on axis incident from front"
set options(18,2) "3 - Uniform isotropically radiating internal source "
set options(18,3) "5 - NRC swept beam source"
set options(18,4) "6 - Rectangular beam incident from the front"
set options(18,5) "7 - Scanning beam source (sawtooth)"
set options(18,6) "8 - Scanning beam for MM50 (uniform circular)"
set options(18,7) "9 - Scanning beam for MM50 (point source, discrete)"
set options(18,8) "10 - Parallel circular beam incident from side"
set options(18,9) "13 - Parallel rectangular beam incident from side"
set options(18,10) "15 - NRC swept beam with beam divergence and radial distribution"
set options(18,11) "19 - Elliptical beam with gaussian distributions in X and Y"
set options(18,12) "21 - Full phase-space beam data, incident on any CM"
set options(18,13) "23 - BEAM simulation source, incident on any CM"
set options(18,14) "24 - Phase-space source, incident from user-specified angle"
set options(18,15) "31 - Beam characterization model"
set help_text(18) {
    Source Number, ISOURC:

    Set the source number by selecting the type of source to use\
	    in the simulation.  A full description of each source\
	    is given in the BEAMnrc User's Manual, as well as in the\
	    help text for a selected source.
}
# (See the help for each source at the end of this file.)

set Ein_val {}
set spec_file {}


set names(19) "Electron range rejection"
set help_text(19) {
Range Rejection, IREJCT_GLOBAL:

The options for electron range rejection are "off" (0), "on with varying ECUTRR"\
	(1), and "on with set ECUTRR" (2).

Range rejection is used to save computing time during simulations. \
	The basic method is to calculate the range of a charged\
	particle and terminate its history (depositing all of its\
	energy at that point) if it cannot leave the current region. \
	For each medium in an accelerator, BEAMnrc calculates a table\
	of ranges to the electron transport cutoff energy\
	(ECUT; see description of variables below) as a function of\
	electron energy. These ranges are calculated using restricted\
	stopping powers and, thus, \
	represent the longest possible ranges to cutoff energy. When\
	the range rejection control variable IREJCT_GLOBAL is set\
	to 2, range rejection is performed on a region by region basis; \
	if the range to the cutoff energy is less than the distance to the\
	nearest region boundary, the history is terminated and energy\
	is deposited in the current region. On the other hand, when\
	IREJCT_GLOBAL is set to 1, range rejection is performed with\
	respect to whether or not a charged particle can reach the\
	bottom of the accelerator and still have energy  ECUT at the\
	base of the accelerator. In this latter type of range rejection, \
	ECUTRR is the minimum energy a charged particle can have as\
	it leaves the current region and still reach the bottom of the\
	accelerator with an energy greater than ECUT. ECUTRR is calculated\
	for each region; if the range to ECUTRR is less than\
	the distance to the nearest region boundary, the particle is\
	terminated and energy deposited in the current region.  \
	IREJCT_GLOBAL=1 can save more time than IREJCT_GLOBAL=2 but can\
	only be used if there is only 1 scoring plane\
	and it is at the very bottom of the accelerator. One can\
	approximate IREJCT_GLOBAL=1 for other situations by carefully\
	selecting ECUT for different regions throughout the accelerator.

Range rejection introduces an approximation because, in terminating\
	  a charged particle's history and depositing all of its\
	  energy in the current region, it is assumed that any\
	  bremsstrahlung photons that would have been created by\
	  the particle, do not leave the current region. The user\
	  can minimize inaccuracies resulting from this approximation\
	  using the input variable\
	  ESAVE_GLOBAL defining the maximum charged particle energy\
	  (in MeV) at which range rejection is considered. The\
	  choice of ESAVE_GLOBAL depends on the incident beam energy\
	  and the materials that it is passing through. ESAVE is\
	  treated internally on a region by region basis, but only\
	  in the CM SLABS does the user currently have the ability to assign\
	  individual values to each region (via ESAVEIN, this is\
	  because this CM is used for bremsstrahlung targets and we thought\
	  we might need more control).

}
set numopts(19) 3;
set options(19,0) "off"
set options(19,1) "on with varying ECUTRR"
set options(19,2) "on with set ECUTRR"
set values(19) $options(19,0)

set names(20) "Global electron cutoff (ESAVE_GLOBAL, range rejection, MeV)"
set help_text(20) {
Global electron cutoff, ESAVE_GLOBAL:

This parameter is used in conjunction with range rejection.  \
	Electron having an energy larger than this value are not considered\
	for range rejection since they may have enough energy to\
	exit the current region.
}
set values(20) 0
set values(25) $values(20)

set names(21) "Photon forcing"
set numopts(21) 2
set options(21,0) "off"
set options(21,1) "on"
set values(21) $options(21,0)
set help_text(21) {
Photon forcing, IFORCE:

BEAMnrc offers an option whereby the user can force photons\
	to interact in specified CMs within a simulation. \
	This option is useful for improving statistics of\
	scattered photons when photon interactions are sparse\
	(eg.  in thin slabs of material or in\
	material with low density). One of the main purposes\
	of implementing this option was to study the generation of\
	contaminant electrons in a photon beam.

Briefly, a photon forced to interact in a CM is ``split"\
	into a scattered photon whose weight is equal to the probability of\
	interaction and an unscattered photon carrying the remaining\
	weight. The unscattered photon proceeds as if an interaction\
	did not take place, and it cannot be forced to interact any\
	more within the specified forcing zone, which can consist of one\
	or several component modules. However, once the unscattered\
	photon gets out of the forcing zone, it may interact again\
	depending on the sampled pathlength. The scattered photon can\
	be forced again in the forcing zone depending on how\
	many interactions are allowed to be forced.

When photon forcing is selected, you will be prompted to specify:

1) The interaction number at which to start forcing (default is 1),
2) the CM number at which to start photon forcing (default is 1), and
3) the CM numer at which to stop forcing (default is the number of CMs\
	present in the accelerator).

Photon forcing parameters are passed onto secondary photons, \
	so that if the parent particle has not yet been forced to\
	interact NFMAX times, each secondary photon is forced to\
	interact the remaining number of times (ie NFMAX - # of times\
	parent particle forced) as long as it is within the forcing\
	CMs. Forcing of secondary photons does not affect the number of\
	times the parent particle is forced to interact if it is a\
	photon as well. The feature of passing forcing parameters\
	to secondary photons is particularly useful to get good\
	statistics for Bremsstrahlung photon interactions. The\
	incident electron creating\
	the photons will not be forced at all, so each\
	Bremsstrahlung photon will be forced to interact\
	NFMAX times. This feature\
	makes Photon Forcing a powerful tool for improving\
	statistics when used in conjunction with Bremsstrahlung Splitting.
}

set names(22) "Number of scoring planes";
set help_text(22) {
Number of scoring planes, NSC_PLANES:

Set the number of fluence scoring planes you want with this option.  \
	If you don't want any, set it to 0.  Otherwise, you can choose up to\
	3 scoring planes, or the number set in\
	$OMEGA_HOME/beamnrc/beamnrc_user_macros.mortran.  \

For each scoring plane, you will be prompted for more details.  \
	The plane is always at the back of a CM.  You must specify which CM\
	to place the plane behind.  You also must choose whether the scoring\
        zones on the scoring plane are to be square, annular or a grid.\
        Note that scoring zones are for the purposes of output to the .egslst\
        file only.  For square or annular zones, you must specify the number of\
	scoring zones.  If this option is set to\
	zero, BEAMnrc will automatically assign the maximum number\
	available with equal zone area.  The "Define zones" button\
	takes you to a window in which the dimensions of the scoring zones\
	are defined.  In the case of square or annular zones, the user defines\
        the half-widths or radii of the zones.  In the case of a grid, the\
        user defines the minimum and maximum X and Y dimensions of the grid\
        and the number of divisions in X (NX) and Y (NY).  Note that if\
        NX*NY (the total no. of zones in the grid) > $MAX_SC_ZONES\
        (the max. no. of allowed scoring zones as defined in\
        $OMEGA_HOME/beamnrc/beamnrc_user_macros.mortran), BEAMnrc will not\
        run.  In this case, the user must either modify the scoring grid or\
        go into $OMEGA_HOME/beamnrc/beamnrc_user_macros.mortran, increase\
        $MAX_SC_ZONES and recompile their accelerator.
}
set numopts(22) [expr $maxvals(MAX_SC_PLANES)+1]
for {set i 0} {$i<=$numopts(22)} {incr i} {
    set options(22,$i) $i
    set values(22,1,$i) {}
    set values(22,2,$i) {}
    set values(22,3,$i) {}
    for {set j 0} {$j<=$maxvals(MAX_SC_ZONES)} {incr j} {
         set values(22,4,$i,$j) {}
    }
}
set values(22) $options(22,0)
set marktype(square) "halfwidths"
set marktype(annular) "radii"

set names(23) "Dose calculation";
set help_text(23) {
Dose Calculation, ITDOSE_ON:

The ability to trace a particle's history using LATCH also allows\
	doses to be broken down into their components. In any dose\
	zone, BEAMnrc is able to break dose down in 2 ways: dose from\
	contaminant particles (identified on the basis of their charge\
	only); or dose including only and/or excluding only contributions\
	arising from particles with certain user-specified LATCH\
	bit settings (this is called ``bit filtering").

When dose components are selected, you will be prompted for which\
	particles to consider as contaminants\
	once they enter which CM (by setting LATCH bit 30 of the\
	particles to 1).  Contaminant particles are\
	identified upon entering the CM number entered if this number is\
	between 1 and the total number of CMs; if it is set to 0, \
	no contaminant dose will be calculated.

Contaminant dose is scored in every dose zone. However it is\
	traced back only to those particles identified as having\
	contaminant charge upon entering the CM number entered\
	(say, CM number 3).  \
	For example, if charged particles are selected as contaminants, \
	all particles entering CM number 3 will be marked as\
	contaminant particles and this mark will be passed onto their\
	descendants via LATCH bit 30. The dose contributed by the\
	contaminant particles and their descendants is then scored as\
	the contaminant dose.

Note that if LATCH option "non-inherited LATCH"\
	is used, LATCH values are not be transferred to descendants\
	(secondaries), and contaminant dose calculations will be\
	meaningless. Thus, the contaminant dose option is automatically\
	turned off for non-inherited LATCH.

After the contaminant type and CM number have been set, the exclusive and\
	inclusive bit filters can be defined (by entering the number of\
	filters, then clicking ">>").

Exclusive bit filters:
    If a particle has been in a region corresponding to a selected LATCH bit, \
	    that particle is not considered part of the dose.  The maximum\
	    number of exclusive filters is set in beamnrc_user_macros.mortran\
	    as $MAXIT-3 and can be changed by altering that file.  \
	    It is initally set to 12, for a maximum of 9 filters.

Inclusive bit filters:
    Inclusive filters may be a combination of inclusive and exclusive filters.  \
	    Dose is scored for a particle if it has ALL of the inclusive bits\
	    turned on and NONE of the exclusive bits.  The maximum number of\
	    inclusive filters is defined as $MAXIT-3-#exclusive filters.

    For example, if an inclusive filter is set with 2,3,5 on for the\
	    inclusive part and 1,4 for the exclusive part, then dose\
	    component I will include the contributions from the\
	    particles with any of the LATCH bits 2, 3 or 5 turned on\
	    and both LATCH bits 1 and 4 NOT set.  The status of other\
	    LATCH bits will have no effect on this dose component.

Note that bit 0 of LATCH cannot be used for the above dose component calculations.

Bit filtering of dose provides a particularly powerful tool for\
	determining dose contributions. In view of the information\
	stored in LATCH (see section above), dose contributions can\
	be separated according to what regions particles have passed\
	through/interacted in, whether the particle is a primary or\
	secondary, if the particle is a secondary then where it was created, \
	whether or not the particle is a contaminant, and any\
	combination of these.
}
set numopts(23) 2
set options(23,0) "Only total dose"
set options(23,1) "Total dose and dose components"
set values(23) $options(23,0)

set names(23,1) "Score contaminant in CM number"
set numopts(23,1) 0
set names(23,2) "Contaminant type"
set numopts(23,2) 3
set options(23,2,0) "photons"
set options(23,2,1) "charged particles"
set names(23,3) "lnexc"
set names(23,4) "lninc"

set names(24) "Z of front of 1st CM to reference plane (cm)"
set help_text(24) {
Z of front of first CM to reference plane, Z_min_CM(1):

Z coordinate of front surface for the first component module.  \
	This includes any air gap and defines the position of the front\
	of the model.  A common value will be 0.0 cm, except\
	for sources 8 and 9.  For most sources except 3, 21, 23 and 31, this\
	is also the source plane on which the particles are incident.   \
	Note that the front of all CMs is given with respect to\
	Z=0.0 cm, not Z_min_CM(1).
}
set values(24) 0.0

# Initialize srcopts
for {set i 1} {$i<=9} {incr i} {
    set srcopts($i) {}
}
set isrc_dbs 0
set src19input sigma

set numsrcopts(0) 4;
set srcoptnames(0,1) "Beam radius";
set srcoptnames(0,2) "Incident x-axis direction cosine";
set srcoptnames(0,3) "Incident y-axis direction cosine";
set srcoptnames(0,4) "Incident z-axis direction cosine";
set source_help(0) {Source 0 is a parallel beam incident from the front, on\
	the positive Z axis.}
set numsrcopts(1) 7;
set srcoptnames(1,1) "Distance of point source to front of first CM (cm)"
set srcoptnames(1,2) "Beam radius at front of first CM (cm)";
set srcoptnames(1,3) "1/2 angle about Z-axis of source (degrees)
(only applies if beam radius is 0)"
set srcoptnames(1,4) "Min. X of beam (cm)"
set srcoptnames(1,5) "Max. X of beam (cm)"
set srcoptnames(1,6) "Min. Y of beam (cm)"
set srcoptnames(1,7) "Max. Y of beam (cm)"
set numsrcopts(2) 4;
set srcoptnames(2,1) "Inner radius of vertical ring or Z of centre of\
	horizontal cylinder";
set srcoptnames(2,2) "Outer radius of vertical ring or radius of\
	horizontal cylinder";
set srcoptnames(2,3) "Z of top of vertical ring or min. X of\
	horizontal cylinder";
set srcoptnames(2,4) "Z of bottom of vertical ring or max. X of\
	horizontal cylinder";
set numsrcopts(3) 2;
set srcoptnames(3,1) "Gamma"; set srcoptnames(3,2) "Beam radius at z=0";
set numsrcopts(4) 4;
set srcoptnames(4,1) "X of beam centre";
set srcoptnames(4,2) "Y of beam centre";
set srcoptnames(4,3) "Half-width in x direction";
set srcoptnames(4,4) "Half-width in y direction";
set numsrcopts(5) 3;
set srcoptnames(5,1) "Field dimension at SSD=100cm";
set srcoptnames(5,2) "Y scans per x scan";
set srcoptnames(5,3) "Beam radius";
set numsrcopts(6) 3;
set srcoptnames(6,1) "Source to surface distance (SSD)";
set srcoptnames(6,2) "Beam radius at SSD";
set srcoptnames(6,3) "Beam spot radius";
set numsrcopts(7) 2;
set srcoptnames(7,1) "Source to surface distance (SSD)";
set srcoptnames(7,2) "Number of discrete points at the SSD";
set numsrcopts(8) 4;
set srcoptnames(8,1) "Beam radius"; set srcoptnames(8,2) "UINC";
set srcoptnames(8,3) "VINC"; set srcoptnames(8,4) "WINC";
set numsrcopts(9) 4;
set srcoptnames(9,1) "Ybeam"; set srcoptnames(9,2) "Zbeam";
set srcoptnames(9,3) "UINC"; set srcoptnames(9,4) "VINC";
set numsrcopts(10) 4;
set srcoptnames(10,1) "Half angle of cone (degrees)";
set srcoptnames(10,2) "Z of cone apex (cm)";
set srcoptnames(10,3) "Radius at which divergence is specified (cm)";
set srcoptnames(10,4) "Divergence angle of the beam (degrees)";
set numsrcopts(11) 6;
set srcoptnames(11,2) "UINC";
set srcoptnames(11,3) "VINC";
set srcoptnames(11,4) "WINC";
set srcoptnames(11,5) "Mean angular spread (deg.)"
set numsrcopts(12) 4;
set srcoptnames(12,1) "CM at which particles start";
set srcoptnames(12,2) "No. of times to recycle particles (NRCYCL)";
set srcoptnames(12,3) "No. of parallel jobs (IPARALLEL)";
set srcoptnames(12,4) "Parallel job no. (PARNUM)";
set numsrcopts(13) 1;
set srcoptnames(13,1) "CM at which particles start";
set numsrcopts(14) 4;
set srcoptnames(14,1) "CM at which particles start";
set srcoptnames(14,2) "No. of times to recycle particles (NRCYCL)";
set srcoptnames(14,3) "No. of parallel jobs (IPARALLEL)";
set srcoptnames(14,4) "Parallel job no. (PARNUM)";
set numsrcopts(15) 1;
set srcoptnames(15,1) "CM at which particles start";

set monoen "monoenergetic"
set options(monoen,0) "monoenergetic";
set options(monoen,1) "spectrum"

set spcnam15 {}
set spcnam21 {}
set spcnam31 {}
set the_beam_code {}
set the_input_file {}
set the_pegs_file {}

set ioutsp {}
set ioutsp_opts(0) "no spectrum data in output summary"
set ioutsp_opts(1) "include spectrum data in output summary"

for {set i 0} {$i < 16} {incr i} {
    for {set j 1} {$j < 5} {incr j} {
	set srcoptvals($i,$j) 0
    }
}

#################The following are the EGSnrc parameters#############

set ecut {}
set names(ecut) "Global electron cutoff energy - ECUT (MeV)"
set numopts(ecut) 0
set values(ecut) {}
set help_text(ecut) {
Global electron cutoff energy, ECUT:

ECUT defines the global cutoff energy for electron\
	transport in MeV. As soon as an electron's total energy\
	falls below the cutoff energy, its history is terminated\
	and its energy deposited in the current region. The time\
	required for a given calculation is strongly dependent\
	on the value of ECUT and thus it is important to use as\
	high a value as possible.

ECUT defaults to AE(medium) (the lowest electron energy for the data\
	in the pegs4 data.

Selection of ECUT is complex in general and is very dependent on\
	what is being calculated[16, 15]. For therapy beams, \
	ECUT can be quite high since low energy electrons contribute\
	little to dose in phantom. For what we consider detailed work, \
	we have used ECUT = 0.700 MeV but much higher may be possible. \
	However, if the dose in the monitor chamber is an\
	important part of the calculation, lower values of ECUT may be required.
As a general rule of thumb for calculations of dose distributions, \
	ECUT should be chosen so that the electron's range at\
	ECUT is less than about 1/3 of the smallest dimension in\
	a dose scoring region. This ensures energy is transported and\
	deposited in the correct region although for electrons which\
	are moving isotropically, this can be a very conservative requirement.
}

set pcut {}
set names(pcut) "Global photon cutoff energy - PCUT (MeV)"
set numopts(pcut) 0
set values(pcut) {}
set help_text(pcut) {
Global photon cutoff energy, PCUT:

PCUT defines the global cutoff energy for photon transport\
	in MeV. It is the photon equivalent of ECUT.  PCUT defaults\
        to AP(medium) (the lowest photon energy for which there is data\
        in the pegs4 file).

The exact value of PCUT is not critical in the sense that\
	low values do not take much more time. A value of 0.01 MeV\
	should generally be used.
}

set smaxir {}
set names(smaxir) "Maximum step size (cm)"
set numopts(smaxir) 0
set values(smaxir) 5
set help_text(smaxir) {
Maximum step size, SMAX:

Global (in all regions) maximum step-size\
        restriction for electron transport (in cm).\
        No SMAX restriction is necessary if the electron\
        step algorithm is PRESTA-II and the EXACT boundary\
        crossing algorithm (the default) is used.  In this case, SMAX\
        will default to 1e10.  However, if either\
        Electron-step algorithm= PRESTA-I\
             or\
        Boundary crossing algorithm= PRESTA-I,\
        then a step-size restriction is necessary, and\
        SMAX will default to 5 cm.
}

set estepe {}
set names(estepe) "Max. fractional energy loss/step"
set numopts(estepe) 0
set values(estepe) 0.25
set help_text(estepe) {
Maximum fractional energy loss/step, ESTEPE:

Note that this is a global option only, no\
        region-by-region setting is possible. If missing,\
        the defualt is 0.25 (25%).
}

set ximax {}
set names(ximax) "XImax"
set numopts(ximax) 0
set values(ximax) 0.5
set help_text(ximax) {
XImax:

Maximum first elastic scattering moment per step.\
Default is 0.5, NEVER use value greater than 1 as\
this is beyond the range of MS data available.
}

set bca_algorithm {}
set names(bca_algorithm) "Boundary crossing algorithm"
set help_text(bca_algorithm) {
Boundary crossing algorithm (bca_algorithm):

There are two selections possible: EXACT and\
        PRESTA-I.  PRESTA-I means that boundaries will\
        be crossed a la PRESTA.  That is, with lateral\
        correlations turned off at a distance given by\
        `Skin depth for BCA' (see below) from the boundary\
        and MS forced at the boundary.  EXACT means\
        the algorithm will cross boundaries in a single\
        scattering (SS) mode, the distance from a boundary\
        at which the transition to SS mode is made is\
        determined by `Skin depth for BCA' (see below).\
        Default is EXACT since the PRESTA-I BCA has been found\
        to result in significant dose overestimates when\
        CHAMBER is used as a depth-dose phantom, and the EXACT BCA\
        will not result in a significant increase in CPU time for\
        most accelerator simulations.
}
set numopts(bca_algorithm) 2
set options(bca_algorithm,0) "EXACT"
set options(bca_algorithm,1) "PRESTA-I"
set values(bca_algorithm) $options(bca_algorithm,0)

set skindepth_for_bca {}
set numopts(skindepth_for_bca) 0
set names(skindepth_for_bca) "Skin depth for BCA"
set values(skindepth_for_bca) 0
set help_text(skindepth_for_bca) {
Skin depth for BCA (skindepth_for_bca):

If Boundary crossing algorithm= PRESTA-I\
        then this is the distance from the boundary (in\
        elastic MFP) at which lateral correlations will be\
        switched off.  The default in this case is to\
        calculate a value based on the scattering power at\
        ECUT (same as PRESTA in EGS4).  If\
        Boundary crossing algorithm= EXACT (default) then this is\
        the distance from the boundary (in elastic\
        MFP) at which the algorithm will go into single\
        scattering mode and defaults to 3 mfp.\
        Note that if you choose EXACT boundary crossing and\
        set Skin depth for BCA to a very large number (e.g.\
        1e10), the entire calculation will be in SS mode.\
        If you choose PRESTA-I boundary crossing and make\
        Skin depth for BCA large, you will get default EGS4\
        behaviour (no PRESTA).\

Note that the above defaults have been choosen as a compromise\
between accuracy (EXACT BCA) and efficiency (PRESTA-I BCA).\
Note that the new transport\
mechanics of EGSnrc are maintained away from boundaries.
}

set transport_algorithm {}
set names(transport_algorithm) "Electron-step algorithm"
set help_text(transport_algorithm) {
Electron-step algorithm (transport_algorithm):

PRESTA-II (the default), the name is\
used for historical reasons\
or PRESTA-I\
Determines the algorithm used to take into account\
lateral and longitudinal correlations in a\
condensed history step.
}
set numopts(transport_algorithm) 2
set options(transport_algorithm,0) "PRESTA-II"
set options(transport_algorithm,1) "PRESTA-I"
set values(transport_algorithm) $options(transport_algorithm,0)

set eii_flag {}
set names(eii_flag) "Electron impact ionization"
set help_text(eii_flag) {
Electron impact ionization (eii_flag):
======================================

Off, On, ik, casnati, gryzinski, kolbenstvedt, penelope, default is Off.  \
Determines which, if any, electron impact ionization theory (EII) is used.

"On" or "ik"   => Kawrakow's EII theory is used. We have preserved the
                  "On" option for compatibility with old input files.
"casnati"      => Casnati's empirical fit to measured data is used.
"gryzinski"    => Gryzinski's semi-classical theory is used.
"kolbenstvedt" => Kolbenstvedt's revised theory used.
"penelope"     => Bote and Salvat theory, used in PENELOPE's EII data base.

For more details, see the EGSnrc Manual (PIRS-701).

Note: This is only of interest in kV X-ray calculations.  Otherwise, leave it\
"off".
}
set numopts(eii_flag) 2
set options(eii_flag,0) "Off"
set options(eii_flag,1) "On"
#other options will be added after looking in the users
#$HEN_HOUSE/data directory
#set options(eii_flag,2) "Casnati"
#set options(eii_flag,3) "Kolbenstvedt"
#set options(eii_flag,4) "Gryzinski"
set values(eii_flag) $options(eii_flag,0)

set photon_xsections {}
set names(photon_xsections) "Photon cross-sections"
set help_text(photon_xsections) {
Photon cross-sections (photon_xsections):

si (default), epdl, xcom, PEGS4, etc.  \
Determines which photon cross-sections are used.

By default EGSnrc uses the Storm-Israel cross-sections (si_*.data files)\
. Other photon cross section compilations provided with EGSnrc are the \
EPDL (epdl_*.data files) and XCOM (xcom_*.data files) photon cross section\
compilations. For more details about these options, see \
the EGSnrc Manual (PIRS-701).

Selecting PEGS4 forces the use of the photon cross section data in the\
PEGS4 file. This recently added option allows the use of PEGS4 files\
if the need arises. For instance, for comparison with previous calculations.\
Beware that despite the fact that the PEGS4 data is used, if Rayleigh scattering\
is turned on, a new Rayleigh angular sampling algorithm is used which should\
not have a large impact as it only removes an undersampling at large\
scattering angles observed with the original EGS4 algorithm.

You also have the option of using your own customized photon cross-section\
data.  To do this, you must create the files x_pair.data, x_photo.data,\
x_rayleigh.data and x_triplet.data (where "x" is the name of your\
cross-section data) containing your cross-sections for pair production,\
photoelectric events, Rayleigh scattering and triplet production,\
respectively.  These files must be in your $HEN_HOUSE/data directory.\
If the GUI detects that these four files are present, then "x" will appear\
as an option in the pull-down menu.
}
#other options will be added after looking in the users $HEN_HOUSE/data
#directory
set numopts(photon_xsections) 1
set options(photon_xsections,0) "xcom"
set values(photon_xsections) $options(photon_xsections,0)

set xsec_out {}
set names(xsec_out) "Photon cross-sections output"
set help_text(xsec_out) {
Photon cross-sections output (xsec_out):

Off (default) or On.  If On, then\
a file $EGS_HOME/user_code/inputfile.xsections is\
output containing photon cross-section data used.
}
set numopts(xsec_out) 2
set options(xsec_out,0) "Off"
set options(xsec_out,1) "On"
set values(xsec_out) $options(xsec_out,0)

set spin_effects {}
set names(spin_effects) "Spin effects"
set help_text(spin_effects) {
Spin effects (spin_effects):

Off, On, default is On\
Turns off/on spin effects for electron elastic\
scattering. Spin On is ABSOLUTELY necessary for\
good backscattering calculations. Will make a\
difference even in `well conditioned' situations\
(e.g. depth dose curves for RTP energy range\
electrons).
}
set numopts(spin_effects) 2
set options(spin_effects,0) "Off"
set options(spin_effects,1) "On"
set values(spin_effects) $options(spin_effects,1)

set ibrdst {}
set names(ibrdst) "Brems angular sampling"
set help_text(ibrdst) {
Brems angular sampling (IBRDST):

Simple, KM, default is Simple \
If Simple, use only the leading term of the Koch-Motz\
distribution to determine the emission angle of \
bremsstrahlung photons. If KM, complete\
modified Koch-Motz 2BS is used (modifications\
concern proper handling of kinematics at low energies,\
makes 2BS almost the same as 2BN at low energies).
}
set numopts(ibrdst) 2
set options(ibrdst,0) "Simple"
set options(ibrdst,1) "KM"
set values(ibrdst) $options(ibrdst,0)

set ibr_nist {}
set names(ibr_nist) "Brems cross sections"
set help_text(ibr_nist) {
Brems cross sections:

BH, NIST, NRC, default is BH\
If BH is selected, the Bethe-Heitler bremsstrahlung\
cross sections (Coulomb corrected above 50 MeV)\
will be used. If NIST is selected, the NIST brems\
cross section data base (which is the basis for \
the ICRU radiative stopping powers) will be employed.\
Differences are negligible for E > ,say, 10 MeV,\
but significant in the keV energy range.  If NRC is selected,\
NIST data including corrections for electron-electron\
brems will be used (typically only\
significant for low values of the atomic number Z\
and for k/T < 0.005).
}
set numopts(ibr_nist) 3
set options(ibr_nist,0) "BH"
set options(ibr_nist,1) "NIST"
set options(ibr_nist,2) "NRC"
set values(ibr_nist) $options(ibr_nist,0)

set ibcmp {}
set names(ibcmp) "Bound Compton scattering"
set help_text(ibcmp) {
Bound compton scattering (IBCMP):

Off, On, On in regions, Off in regions, Norej\
If Off, Compton scattering will be treated with\
Klein-Nishina, with On Compton scattering is\
treated in the Impulse approximation. Third option, Norej,\
uses actual bound Compton cross sections and does not\
reject any Compton interactions at run time.  If the Norej\
option is selected, then the user has the option\
of specifying their own Compton cross sections.\
Default is Off\
Make sure to use bound Compton for low energy applications.\
Not necessary above, say, 1 MeV.\

If you use "On in regions", you will be prompted to enter\
pairs of region numbers.  Each pair defines a range of regions\
in which bound compton scattering will be turned On.  Everywhere\
outside these regions it will be turned Off.\

If you use "Off in regions", you will be prompted to enter\
ranges of regions where bound compton scattering is to be turned Off.\
Everywhere else it will be turned on.

Note that the "Norej" option, if used, is applied to all regions.
}
set numopts(ibcmp) 5
set options(ibcmp,0) "Off"
set options(ibcmp,1) "On"
set options(ibcmp,2) "On in regions"
set options(ibcmp,3) "Off in regions"
set options(ibcmp,4) "Norej"
set values(ibcmp) $options(ibcmp,4)
set level(ibcmp) 0

set comp_xsections {}
set names(comp_xsections) "Compton cross sections"
set help_text(comp_xsections) {
Compton cross sections (comp_xsections):

Bound Compton cross-section data.  User-\
supplied bound Compton cross-sections in the file\
$HEN_HOUSE/data/x_compton.data, where\
"x" is the name given by the user.\
This is only used if Bound Compton scattering= Norej\
and is not available on a region-by-region basis\
(see below).  The default file (ie in the absence\
of any user-supplied data) is compton_sigma.data, which\
is included with the EGSnrc distribution.
}
#other options will be added after looking in the users $HEN_HOUSE/data
#directory
set numopts(comp_xsections) 1
set options(comp_xsections,0) "default"
set values(comp_xsections) $options(comp_xsections,0)

set radc_flag {}
set names(radc_flag) "Radiative Compton corrections"
set help_text(radc_flag) {
Radiative Compton corrections (radc_flag):

On or Off (default). If on, then\
include radiative corrections for Compton scattering.\
Equations are based on original Brown & Feynman\
equations (Phys. Rev. 85, p 231--1952).  Requires\
a change to the user codes Makefile to include\
$(EGS_SOURCEDIR)rad_compton1.mortran in the\
SOURCES (just before $(EGS_SOURCEDIR)egsnrc.mortran).
}
set numopts(radc_flag) 2
set options(radc_flag,0) "Off"
set options(radc_flag,1) "On"
set values(radc_flag) $options(radc_flag,0)

set iprdst {}
set names(iprdst) "Pair angular sampling"
set help_text(iprdst) {
Pair angular sampling (IPRDST):

Off, Simple or KM\
If off, pairs are set in motion at an angle m/E\
relative to the photon direction (m is electron rest\
energy, E the photon energy). Simple turns on\
the leading term of the angular distribution\
(this is sufficient for most applications),\
KM (comes from Koch and Motz) turns on using 2BS\
from the article by Koch and Motz.\
Default is Simple, make sure you always use \
Simple or KM
}
set numopts(iprdst) 3
set options(iprdst,0) "Off"
set options(iprdst,1) "Simple"
set options(iprdst,2) "KM"
set values(iprdst) $options(iprdst,1)

set pair_nrc {}
set names(pair_nrc) "Pair cross sections"
set help_text(pair_nrc) {
Pair cross sections (pair_nrc):

BH (default) or NRC.\
If set to BH, then use\
Bethe-Heitler pair production cross-sections.  If set\
to NRC, then use NRC pair production cross-sections\
(in file $HEN_HOUSE/data/pair_nrc1.data).  Only\
of interest at low energies, where the NRC cross-\
sections take into account the assymmetry in the\
positron-electron energy distribution.
}
set numopts(pair_nrc) 2
set options(pair_nrc,0) "BH"
set options(pair_nrc,1) "NRC"
set values(pair_nrc) $options(pair_nrc,0)

set iphter {}
set names(iphter) "Photoelectron angular sampling"
set help_text(iphter) {
Photoelectron angular sampling (IPHTER):

Off, On, On in regions, Off in regions\
If Off, photo-electrons get the direction of the\
`mother' photon, with On, Sauter's formula is\
used (which is, strictly speaking, valid only for\
K-shell photo-absorption).\
If the user has a better approach, replace the macro\
$SELECT-PHOTOELECTRON-DIRECTION;\
The only application \
encountered until now where this option made a\
small difference was a big ion chamber (cavity size\
comparable with electron range) with high-Z walls\
in a low energy photon beam.\
Default in BEAMnrc is Off\

If you use "On in regions", you will be prompted to enter\
pairs of region numbers.  Each pair defines a range of regions\
in which photoelectron angular sampling will be turned On.  Everywhere\
outside these regions photoelectron angular sampling will be turned Off.\

If you use "Off in regions", you will be prompted to enter\
ranges of regions where photoelectron angular sampling is to be turned Off.\
Everywhere else it will be turned on.
}
set numopts(iphter) 4
set options(iphter,0) "Off"
set options(iphter,1) "On"
set options(iphter,2) "On in regions"
set options(iphter,3) "Off in regions"
set values(iphter) $options(iphter,0)
set level(iphter) 0

set iraylr {}
set names(iraylr) "Rayleigh scattering"
set help_text(iraylr) {
Rayleigh scattering (IRAYLR):

Off, On, On in regions, Off in regions, custom\
If On, turn on coherent (Rayleigh) scattering.\
Default is Off. Should be turned on for low energy\
applications. Not set to On by default because\
On requires a sperial PEGS4 data set.\

If you use "On in regions", you will be prompted to enter\
pairs of region numbers.  Each pair defines a range of regions\
in which Rayleigh scattering will be turned On.  Everywhere\
outside these regions Rayleigh scattering will be turned Off.\

If you use "Off in regions", you will be prompted to enter\
ranges of regions where Rayleigh scattering is to be turned off.\
Everywhere else it will be turned on.
}
set numopts(iraylr) 5
set options(iraylr,0) "Off"
set options(iraylr,1) "On"
set options(iraylr,2) "On in regions"
set options(iraylr,3) "Off in regions"
set options(iraylr,4) "custom"
set values(iraylr) $options(iraylr,0)
set level(iraylr) 0
set num_rayl_custom 0
for {set i 1} {$i <= 200} {incr i} {
   set rayl_cust_med($i) {}
   set rayl_cust_file($i) {}
}

set iedgfl {}
set names(iedgfl) "Atomic relaxations"
set help_text(iedgfl) {
Atomic relaxations (IEDGFL):

Off, On, On in regions, Off in regions\
The default is Off. The effect of using On is twofold:\
- In photo-electric absorption events, the element\
  (if material is mixture) and the shell the photon\
  is interacting with are sampled from the appropriate\
  cross seections\
- Shell vacancies created in photo-absorption events\
  are relaxed via emission of fluorescent X-Rays,\
  Auger and Koster-Cronig electrons.\
  Make sure to turn this option on for low energy\
  applications.\

If you use "On in regions", you will be prompted to enter\
pairs of region numbers.  Each pair defines a range of regions\
in which Atomic relaxations will be turned On.  Everywhere\
outside these regions Atomic relaxations will be turned Off.\

If you use "Off in regions", you will be prompted to enter\
ranges of regions where Atomic relaxation is to be turned Off.\
Everywhere else it will be turned on.
}
set numopts(iedgfl) 4
set options(iedgfl,0) "Off"
set options(iedgfl,1) "On"
set options(iedgfl,2) "On in regions"
set options(iedgfl,3) "Off in regions"
set values(iedgfl) $options(iedgfl,0)
set level(iedgfl) 0

#################end of options for EGSnrc parameters####################

set nuser_inputs 0
# number of user inputs defined in input block enclosed in
# delimiters :Start user inputs: :Stop user inputs:



# define cm_names: SAME ORDER AS IN MANUAL

set cm_names(1) "SLABS"; set cm_names(2) "CONS3R"; set cm_names(3) "CONESTAK"
set cm_names(4) "FLATFILT"; set cm_names(5) "CHAMBER"; set cm_names(6) "JAWS"
set cm_names(7) "APPLICAT"; set cm_names(8) "CIRCAPP"; set cm_names(9) "PYRAMIDS"
set cm_names(10) "BLOCK"; set cm_names(11) "MLC"; set cm_names(12) "MESH"
set cm_names(13) "MIRROR"; set cm_names(14) "XTUBE";
set cm_names(15) "SIDETUBE"; set cm_names(16) "ARCCHM"; set cm_names(17) "MLCQ"
set cm_names(18) "VARMLC"; set cm_names(19) "MLCE"; set cm_names(20) "DYNVMLC"
set cm_names(21) "DYNJAWS"
set cm_names(22) "SYNCJAWS"
set cm_names(23) "SYNCMLCE"
set cm_names(24) "SYNCVMLC"
set cm_names(25) "SYNCHDMLC"

# HELP TEXT FOR CMs:

set help_applicat_text {
Applicat is used for a set of rectangular scrapers.  Each scraper\
	is defined by the outer region of two concentric rectangles, \
	the inner region being air.  There is a minimum air gap at the front\
	and back of the CM of 0.01 cm.  \
	The scrapers may be square or rectangular\
	and may be of different materials.  This CM may be used for modelling \
	the square applicator found in electron beams, although it does\
	not allow for a bevelled edge.  The outer boundary is a square\
	centered on the beam axis.

For each scraper in Applicat, the geometry must be specified (in centimetres).

1) The Z-coordinate of the back face of the scraper.
2) The thickness of the scaper.
3) The halfwidth in the X and Y dimensions.
4) The bar width in the X and Y dimensions.
5) The dose scoring zone for the scraper bar.
6) The bit with which the scraper is associated.
}

set help_mirror_text {
The parameters you need to set for the MIRROR CM are shown\
	to the left.  Note that the angle which between the\
	mirror and the central axis must be larger than 5 degrees.
}

set help_xtube_text {
The XTUBE component is used for the simulation of an x-ray tube\
	target.  It is ALWAYS the first CM in the geometry, so there\
	is no air gap for it.  The distance to the reference plane\
	(Z=0) should be zero.  Note that target layers are numbered\
        from the holder out.   Also, there is an option to specify\
        a rectangular extra central region, composed of a different medium, in\
        the outermost target layer.
}

set help_mesh_text {
The MESH component module is used to simulate the presence\
	of a single-layer wire mesh placed perpendicular to the beam\
	direction and in the path of the beam.  The mesh consists of\
	identical, regularly spaced, rectangular air regions separated\
	from each other by wire.  The mesh need not extend to the\
	boundary of the CM.  The CM has square symmetry.
}

set help_slabs_text {
The SLABS CM is used for the simulation of multiple\
	planes of arbitrary thickness and material, i.e. a set\
	of semi-infinite slabs.  One single slab is a special case.  \
	Slabs has square symmetry about the central axis.\
	Note that ESAVE, the maximum particle energy for range rejection, \
	can be set for each region, unlike the other CMs\
	where the global value is used.
}

set help_sidetube_text {
The SIDETUBE CM is used to model concentric cylinders\
	parallel to the X-axis.  Each cylinder is made of its\
	own medium and has an arbitrary outer radius.  The\
	medium surrounding the outer\
	cylinder and filling the cube must also be defined.  It is\
	excellent for simulating cobalt and cesium sources\
	with source #3.  Side tube has square symmetry\
	about the beam axis.
}

set help_conestak_text {
CONESTAK is used to simulate a primary collimator by\
	defining a cone geometry of arbitrary thickness, angle\
	and outer and inner materials.  It has cylindrical\
	symmetry.  Note that where one section ends the next\
	must start, as shown in the diagram.  Also, note the restriction\
        on cone radii:

        back Ri >= front Ri

        and:

        front Ri+1 >= back Ri

Thus, cone radii must always be increasing from the top layer\
        down.

The user also has\
        the option of specifying an outer wall which extends over\
        all conical layers and is comprised of the same material\
        throughout.  If the user chooses to have an outer wall, then\
        its inner radius must be specified.

}

set help_cons3r_text {
CONS3R can be used for any case with radial symmetry, \
	if there are only two regions in the radial direction. \
	Convex shapes only are allowed in the Z direction\
	in this version, not concave, i.e. for a point i at\
	(z1,r1), the next point (z2,r2) must have z2 > z1.  \
	Also the first and last points must lie at the Z\
	boundaries of the CM.  Note that the Z values are\
	measured from the front of the accelerator, not from\
	the front of the CM.
}

set help_jaws_text {
JAWS is used for a set of paired bars, which can be bars\
	in the collimator or applicator.  The bars are of arbitrary\
	thickness and material, and X or Y orientation.  \
	There must be a gap of at leat 0.01 cm between the first set\
	of jaws and the top of the CM and between sets of jaws.  \
	The bars in each pair must be composed of the same\
	material, but\
	different pairs can be different materials.  \
	Jaws has square symmetry.

The CM shown to the left is composed of a pair of X jaws stacked on a pair\
	of Y jaws.  The cross-section shown is through the X axis, \
	cutting throught the opening in the Y jaws, hence the\
	dashed rectangle representing the Y jaws.

Jaw opening coordinates can either be input explicitly or can be calculated\
        using the "Define x/y using field size and SSD" button.  When you\
        press this button, a window will pop up which allows you to specify\
        the half-width, SSD and Z focus (from which the SSD is measured) of\
        the field and the sets of jaws that you want to apply this to.  Then\
        when you press the "Update x/y coords." button, the opening coordinates\
        of the jaws will be calculated and displayed in the "Define jaws"\
        window.  Note that this method of specifying opening coordinates is\
        only valid for photon beams.
}

set help_pyramids_text {
    The PYRAMIDS component module is used to model pyramid-shaped\
	    structures comprising one or more layers in the path of\
	    the beam.  Each layer has three distinct regions: the\
	    central region (the pyramid), the surrounding region and\
	    the outer region (beyond the outer edges of the layer). \
	    The central and outer regions default to air but can also\
	    be filled with a user-defined medium (assumed the same for\
	    the central and outer regions within a layer).  PYRAMIDS is useful\
	    for modelling rectangular collimators and beam blocks.  \
	    This CM has a square outer boundary centered on the beam axis.

    The 'use symmetry' checkbox can be used if your geometry is symmetric\
	    in x and y.

    The figure to the left shows a pyramids CM with 2 layers.  \
	    Within each layer, the front and back openings of\
	    the central region are specified, as well as the X and Y\
	    outer edges of each layer.  By selecting "user-specified\
	    materials", the user can set the medium filling\
	    for the central and outer regions.  \
	    The gaps between layers are assumed to filled with air.  \
	    Layers must be separated by at least 0.01 cm.
}

set help_chamber_text {
The chamber CM models a parallel-plate ion chamber in a container\
	with top and bottom planes of arbitrary thickness and\
	material.  It also models a combination of scattering foils, \
	cylindrical collimators and an ion chamber, etc.  \
	This CM is also useful for central-axis depth-dose calculations\
	and for analysis of dose components due to particles coming\
	from different parts of an accelerator.  Chamber has\
	cylindrical symmetry.

The CM may be interpreted as a phantom by selecting 'phantom' on the\
	main CHAMBER window.
}

set help_block_text {
BLOCK is used to model a treatment block having non-rectangular\
	    and/or multiple openings.  The user specifies openings\
	    in the block material using up to 20 'subregions'.  \
	    For each subregion, the user specifies the (x,y)\
	    coordinates of its vertices at the top surface of the block\
	    material (either clockwise or counter-clockwise around\
	    the perimeter).  The inner planes of all subregions are\
	    angled with respect to the beam (Z) axis toward a\
	    user-specified 'focal point' on the beam axis.  \
	    Openings can consist of a single subregion or may\
	    require several adjoining subregions.  \
	    Not that no subregion can have an inner angle greater\
	    than 180 degrees.  The user also specifies the X and Y\
	    coordinates of the 4 outer edges of the block material, \
	    so the material need not extend to the square outer\
	    boundary of the CM.  Due to its generality, BLOCK may require\
	    up to twice the CPU time of PYRAMIDS to simulate\
	    simple rectangular geometries; thus, PYRAMIDS is recommended\
	    when there is a single rectangular opening.

    The figure to the left shows a BLOCK CM with a single opening\
	    composed of 1 subregion.  The (x,y) coordinates of this\
	    subregion (small black dots at vertices of pentagon) should\
	    be defined clockwise or counter-clockwise.  The air gap must\
	    be at least 0.01 cm thick.
}

set help_mlc_text {
The MLC CM is used to model a multi-leaf collimator.  The collimator has a\
	single layer with a user-specified number of leaves all opening\
	in either the X or Y direction.  The collimator opening is specified\
	by the coordinated of the individual leaf openings at the top\
	of the collimator, the thickness of the leaves in the Z direction, \
	and two Z 'foci' that determine the angles of the leaf side and\
	leaf end surfaces.  The outer boundary of the MLC CM is a square\
	centered on the beam axis.  Currently, the collimator body extends\
	to this outer boundary.

The diagram to the left shows an MLC with 6 leaves parallel to the X axis.  \
	The leaves must be defined in groups of those adjacent and of the\
	same maximum and minimum X or Y coordinates (in this case, X).
}

set help_circapp_text {
CIRCAPP is similar to APPLICAT, but used to model scrapers\
	    that have circular openings.  The scrapers retain\
	    their rectangular outer edges, but the opening is\
	    now defined as a circle concentric with the rectangle\
	    defining the outer edges.  Similar to APPLICAT, CIRCAPP\
	    does not allow beveled edges.  The outer boundary\
	    of CIRCAPP is a square centered on the beam axis.

    The air gap between scrapers and at the boundaries must be at least 0.01 cm.
}

set help_flatfilt_text {
FLATFILT consists of a stacked set of truncated coaxial cones\
	with an arbitrary number of cones on each level.  \
	The material in the cones need not be the same.  \
	Both the number of cones and the radii of cones\
	in each layer can be specified independently.  \
	This CM can be used to model very complex beam flattening\
	filters for photon beam simulations, including those interior\
	to a conical collimator.  FLATFILT has cylindrical symmetry.

The figure to the left shows a flattening filter with 4 layers.  \
	Within a layer, cones cannot cross.  \
	For each layer, the material for each cone and the material\
	for the region between the outermost cone and the boundary\
	must be specified.
}

set help_arcchm_text {
The figure shows an ARCCHM CM with 4 chambers.  The number of chambers\
	must be even so that they can be arrayed as shown in the y\
	cross-section, with a septum centered on the beam axis at\
	y=0.  The number of septa is always 1 less than the number of\
	chambers.  The user also specifies the z position of the front face\
	of the chambers at y=0, the radius of the arc at the front face \
	(these will be equal for a perfectly isotropic source), the arc-width\
	of the chambers and septa, the thickness of the chambers and septa, \
	the thickness of the front and back faces, and the maximum z of the\
	CM.  All other geometrical properties are determined within the\
	code and do not need to be specified.  Note that the maximum z\
	must be chosen to accomodate the full z range of the arc.

The cross-section in the x direction shows the user-specified minimum\
	and maximum x of the arc and width of the chamber x-walls.  \
	The user can specify the medium in all regions except above\
	the ends of the arc.
    }

set help_mlcq_text {
MLCQ is used to model a multi-leaf collimator with rounded leaf ends.  \
	The collimator is similar to MLC with the exception that, rather\
	than specifying a Z focus for the leaf ends, the user specified a\
	radius for the leaf ends and the Z position of the origin of this\
	radius (i.e. so the leaf ends can be angled up or down).  The\
	collimator opening is defined by specifying the X or Y (depending\
	on leaf orientation) origin of the radius for the positive and\
	negative portions of each leaf.

The figure shows the three views of MLCQ.  The top view shows the set of points\
	X0N and X0P, which are the X or Y (depending on orientation)\
	coordinates of the centre of the circle which defines the leaf ends.  \
	The cross-section parallel to the leaves shows the radius of the\
	circle defining the leaf ends, R0.  Z0 is the distance from the front\
	of the accelerator to the centre of the circle.  R0 and Z0 apply to\
	all leaves.

Note that abs(X0N) and abs(X0P) can be larger than the boundary of the CM, as\
	long as the radius of the circle accounts for this.  The points of\
	intersection of of the circle defining a leaf end with the top and\
	bottom of the CM must also be within the boundaries of the CM or\
	else the volume/mass of the collimator will be determined\
	incorrectly for dose calculations.
}

set help_varmlc_text {
VARMLC is used to model a multi-leaf collimator with either rounded ends \
        or straight leaf ends.  The main difference between VARMLC and\
        MLC or MLCQ is that VARMLC models the air gaps between leaves,\
        the tongue-in-groove mechanism by which adjacent leaves slide against\
        each other, and the driving screws at the top and bottom of each leaf.\
        VARMLC can also simulate leaves of different widths within the same\
        collimator.  In VARMLC the material beyond the leaf sides in the\
        direction perpendicular\
        to the leaves defaults to air.  This is unlike MLC and MLCQ, where\
        the material in this region defaults to the leaf material.

The figure shows four views of VARMLC.  The cross-section perpendicular to the\
        leaves (Y cross section in this case) shows the leaf widths\
        (LEAFWIDTH(1)...LEAFWIDTH(4)) and the dimensions of the air gap (LEAFGAP),\
        tongue-in-groove (WTONGUE, HTONGUE) and screw (WSCREW, HSCREW), all of which\
        are input by the user.  The tongue starts at ZTONGUE and the groove\
        at ZGROOVE.  If ZTONGUE=ZGROOVE=0, then the tongue and groove are assumed to\
        be centred at Z=ZMIN+ZTHICK/2 (ie halfway down the leaf).  Note that all side\
        surfaces are focused at ZFOCUS(1) and that all horizontal dimensions are given\
        at ZMIN (ie at the top of the leaves).   The Y cross-section also shows that\
        leaves may not be symmetric about the Z axis due to the offset caused by LEAFGAP,\
        although START may be set to give approximate symmetry in this dimension.\
        Two cross-sections parallel to the leaves (X cross-sections in this case)\
        are shown; one for the case of rounded leaf ends (ENDTYPE=0) and one for\
        straight, focused leaf ends (ENDTYPE=1).  In the case of the rounded leaf\
        ends, the user specifies the radius of the ends (LEAFRADIUS) and the min.\
        (NEG) and max. (POS) dimensions of each leaf opening at Z=ZMIN+ZTHICK/2.\
        The leaf radius is always assumed to originate at Z=ZMIN+ZTHICK/2.\
        For straight leaf ends, the user must input the focal point of the end\
        surfaces (ZFOCUS(2)) and the min. (NEG) and max. (POS) dimensions of\
        the opening in each leaf at Z=ZMIN.
}

set help_mlce_text {
MLCE is used to model an Elekta MLC with cylindrical or focused (straight)\
        leaf ends.  This CM is based on VARMLC, however, it does not model\
        carriage screws and, instead of a tongue and groove, it models the\
        interlocking steps of this kind of MLC.  All leaves have the\
        same cross-section perpendicular to the opening direction.  Leaves\
        are angled (or tilted) about the mid-point of their top surface\
        so that the leaf sides are focussed at Z=0.  The user can also rotate\
        the entire leaf bank about X=0 (if leaves parallel to Y) or Y=0\
        (if leaves parallel to X), Z=ZMIN (the top surface of the leaves)\
        so that the focus of the leaf sides is no longer along the\
        central axis.  Leaf material does not continue beyond outer edges\
        of outermost leaves.

The figure shows a cross section through the imaginary "central leaf" used\
        to define the cross-sections of all leaves.  The central leaf is\
        specified by two points (X3,ZMIN) and (X4,ZMAX), the Z positions\
        of the left (ZSTEPL) and right (ZSTEPR) steps, and the width of\
        the step (TGW).  Note that ZSTEPL>ZSTEPR in order for the steps\
        to fit together.  The central leaf is then duplicated and rotated\
        about X=0 (ORIENT=0) or Y=0 (ORIENT=1), Z=ZMIN, and translated in\
        in the X (ORIENT=0) or Y (ORIENT=1) direction to create the user-input\
        number of leaves (NUM_LEAF) in the leaf bank.  Rotation angles and\
        translation distances are determined by the user-input distance\
        between cross-section centres (SPACE) projected to SSD and the\
        requirement that leaf sides focus to Z=0.  See the manual for more\
        details.  At this point, leaf cross-sections are symmetric about\
        the Z axis (Note: NUM_LEAF must be an even number!).  The entire leaf\
        bank can then be rotated by LBROT about X=0 (ORIENT=0) or Y=0\
        (ORIENT=1), Z=ZMIN so that the sides now focus on the Z' axis.  Leaf\
        ends can be cylindrical (ENDTYPE=0) or straight (ENDTYPE=1).\
        Cylindrical leaf are specified by the radius (LEAFRAD) and Z position\
        of the origin (CIL) of the cylinder. Straight ends are specified by the\
        Z focus (ZFOCUS) of the ends.  For cylindrical ends, leaf openings are\
        defined by the X (ORIENT=1) or Y (ORIENT=0) coordinates of the cylinder\
        origins for the positive (POS) and negative (NEG) portions of the leaf.\
        For straight ends, openings are specified by the X (ORIENT=1) or Y\
        (ORIENT=0) coordinates of the ends at Z=ZMIN for the positive (POS)\
        and negative (NEG) portions of the leaf.
}

set help_dynvmlc_text {
DYNVMLC is designed to model the Varian Millenium MLC.  It is based on\
VARMLC, but, instead of every leaf having an identical cross-section\
(perpendicular to the leaf opening direction), the user specifies\
cross-sections for the 3 different types of leaves, FULL, TARGET and\
ISOCENTER found in the MLC.

The figure shows a MLC with 8 leaves (a realistic Millenium MLC would have\
120 leaves) opening in the Y direction\
(ORIENT=0).  The X cross-sections for FULL, TARGET and ISOCENTER leaves\
are shown at the top.  These cross-sections\
can be seen in more detail by using the "?" buttons in the\
"Define leaf cross-sections" window of the GUI.  When specifying these\
cross-sections the dashed X and Z grid lines shown must not change order\
 (they can overlap, however).  Also, tongue and groove Z positions must\
allow leaves to fit together in the following cominations:\
FULL/FULL, FULL/TARGET, TARGET/ISOCENTER, ISOCENTER/TARGET,\
ISOCENTER/FULL.  Note that in each\
leaf cross section you must specify the Z dimensions of a driving screw\
hole (filled with medium, MED3).  When assigning types to leaves in the leaf\
bank, TARGET/ISOCENTER leaves must always occur in pairs (this means that\
you really only have two choices when assigning leaf type).\
In the example shown, leaves 1,2,7,8 are FULL\
and leaves 3-6 are TARGET/ISOCENTER pairs.  The X cross-section through\
the leaf bank shows how the different leaves fit together.  Leaves start\
at a user-specified X (ORIENT=0) or Y (ORIENT=1) position (START) and are\
separated by a user-specified air gap (LEAFGAP).  You must also specify a\
point on the Z axis (ZFOCUS(1)) to which all leaf side surfaces are focused.\
Note that all leaf widths, along with START and LEAFGAP, are specified at\
Z=ZMIN, but will not be constant throughout the Z thickness of the MLC due\
to focusing of the leaf sides.\
The Y cross-sections show the\
two possible types of leaf ends: cylindrical (ENDTYPE=0) and straight\
(ENDTYPE=1).\
For rounded ends you must specify a radius (LEAFRADIUS), and for straight ends\
you must specify a focal point on the Z axis (ZFOCUS(2)).  The origin of the\
radius for cylindrical ends is always at Z=ZMIN+ZTHICK/2.  The dimensions\
of the opening (NEG,POS) are specified at Z=ZMIN+ZTHICK/2 for a leaf with\
cylindrical ends and at Z=ZMIN for a leaf with straight ends.  The\
Y cross-sections also show HOLEPOS, the\
distance between driving screw holes and leaf tips, which must be\
defined for each leaf type.  HOLEPOS is constant for a given leaf type\
independent of the leaf opening dimensions.

}

set help_dynvmlc_full_text {
Figure showing a cross-section through a FULL leaf and the dimensions that\
you must input:

widths (cm):
wl = leaf width (excl. tongue)
wt = tongue width
wg = groove width
wtip = tip width
wts = upper support rail
wbs = lower support rail

Z positions (cm):
ztip = Z of top of tip
zl = Z of top of leaf
zt = Z of bottom of tongue
zg = Z of bottom of groove
zth = Z of top of driving screw hole
zbh = Z of bottom of driving screw hole
zts = Z of top of support rail
zbs = Z of bottom of support rail

Note that the Z position of the bottom of the leaf is always\
ZMIN+ZTHICK.

Widths and Z positions must be set up so that the dashed grid lines\
shown in the figure do not change order.  They can, however, overlap.  This\
means, for example that wt <= wtip <= wts-wbs and that\
ztip <= zl <= zt.

You must also ensure that FULL and TARGET leaves can fit together by\
ensuring that zg >= Z of bottom of tongue of TARGET leaves and that FULL and\
ISOCENTER leaves fit together by ensuring that zt <= Z of bottom of groove\
of ISOCENTER leaves.
}

set help_dynvmlc_tar_text {
Figure showing a cross-section through a TARGET leaf and the dimensions that\
you must input:

widths (cm):
wl = leaf width (excl. tongue)
wt = tongue width
wg = groove width
wtip = tip width
wts = upper support rail
wbs = lower support rail

Z positions (cm):
zts = Z of top of support rail
zbs = Z of bottom of support rail
zth = Z of top of driving screw hole
zbh = Z of bottom of driving screw hole
zt = Z of bottom of tongue
zg = Z of top of groove
zl = Z of bottom of leaf
ztip = Z of bottom of tip

Note that the Z position of the top of the leaf is always\
ZMIN.

Widths and Z positions must be set up so that the dashed grid lines\
shown in the figure do not change order.  They can, however, overlap.  This\
means, for example that wbs - wg <= wts <= wbs and that\
zg <= zl <= ztip.

You must also ensure that TARGET and FULL leaves can fit together by\
ensuring that zt <= Z of bottom of groove of FULL leaves and that TARGET and\
ISOCENTER leaves fit together by ensuring that zg >= Z of top of tongue\
of ISOCENTER leaves and zt <= Z of bottom of groove of ISOCENTER leaves.
}

set help_dynvmlc_iso_text {
Figure showing a cross-section through a ISOCENTER leaf and the dimensions that\
you must input:

widths (cm):
wl = leaf width (excl. tongue)
wt = tongue width
wg = groove width
wtip = tip width
wts = upper support rail
wbs = lower support rail

Z positions (cm):
ztip = Z of top of tip
zl = Z of top of leaf
zt = Z of top of tongue
zg = Z of bottom of groove
zth = Z of top of driving screw hole
zbh = Z of bottom of driving screw hole
zts = Z of top of support rail
zbs = Z of bottom of support rail

Note that the Z position of the bottom of the leaf is always\
ZMIN+ZTHICK.

Widths and Z positions must be set up so that the dashed grid lines\
shown in the figure do not change order.  They can, however, overlap.  This\
means, for example that wts - wt <= wbs <= wts and that\
zt <= zg <= zth.

You must also ensure that ISOCENTER and FULL leaves can fit together by\
ensuring that zg >= Z of bottom of tongue of FULL leaves and that TARGET and\
ISOCENTER leaves fit together by ensuring that zg >= Z of bottom of tongue\
of TARGET leaves and zt >= Z of top of groove of TARGET leaves.
}

set help_dynjaws_text {
The geometry of DYNJAWS is the same as JAWS (see figure), however, instead\
of fixed jaw settings, the user has the option of specifying "dynamic" or\
"step-and-shoot" mode for which the jaw settings are specified in a file.\

The format of the file containing jaw settings is:

TITLE -- title up to 80 chars
NFIELDS -- the no. of fields
FOR I=1,NFIELDS[
INDEX(I) -- the index of field I
FOR J=1,no. of jaws[
ZMIN(J,I),ZMAX(J,I),XFP(J,I),XBP(J,I),XFN(J,I),XBN(J,I)
]]

where:
ZMIN(J,I)=Z of front of jaw J for field I
ZMAX(J,I)=Z of back of jaw J for field I
XFP(J,I)=X/Y of front of +ve bar of jaw J for field I
XBP(J,I)=X/Y of back of +ve bar jof jaw J for field I
XFN(J,I)=X/Y of front of -ve bar of jaw J for field I
XBP(J,I)=X/Y of back of -ve bar jof jaw J for field I

INDEX(I) is a number in the range 0<=INDEX(I)<=1 and with\
INDEX(I)>INDEX(I-1) that is used to determine which field settings\
are used.  At the beginning of each primary\
history, a random number, RND, is compared to INDEX(I),I=1,NFIELDS.  The lowest value of I\
for which the RND<=INDEX(I) determines the field settings used.  If the user has\
selected "step-and-shoot" mode, the jaw settings for field I are used.  If the\
user has selected "dynamic" mode, then the jaw settings are a linear interpolation\
between fields I-1 and I based on the value of RND.  For example, the value of XFP(J),\
the front X or Y coordinate of the +ve bar of jaw J, is given by:

XFP(J)=XFP(J,I-1)+[XFP(J,I)-XFP(J,I-1)]/[INDEX(I)-INDEX(I-1)]*[RND-INDEX(I-1)]

Thus, the "dynamic" setting simulates jaw motion while the beam is on.

Note that in "step-and-shoot" and "dynamic" modes, the user must still specify\
the orientation, media, ECUT, PCUT, dose zone and bit region of each set of\
jaws in a separate input window.  If the user selects "static" mode, then\
function reverts to being identical to JAWS.

DYNJAWS is particularly useful for simulating dynamic wedges.
}

set help_syncjaws_text {
The SYNCJAWS component module is similar to the DYNJAWS component module, but\
it allows synchronization between the jaw settings (defining the field) and\
the incident angle, SSD and source isocentre of DOSXYZnrc sources 20 and 21\
(synchronized phase space and BEAMnrc simulation sources).  SYNCJAWS\
is synchronized with any other synchronized dynamic CMs (eg SYNCVMLC, SYNCHDMLC)\
in an accelerator.

The geometry of SYNCJAWS is the same as JAWS (see figure), however, instead\
of fixed jaw settings, the user has the option of specifying "dynamic" or\
"step-and-shoot" mode for which the jaw settings are specified in a file.\

The format of the file containing jaw settings is:

TITLE -- a title (up to 80 characters)
NFIELDS -- the no. of fields (different jaw settings)
FOR I=1,NFIELDS[
MUINDEX(I) -- the monitor unit index of field I
FOR J=1,no. of jaws[
ZMIN(J,I),ZMAX(J,I),XFP(J,I),XBP(J,I),XFN(J,I),XBN(J,I)
]]

where:
ZMIN(J,I)=Z of front of jaw J for field I
ZMAX(J,I)=Z of back of jaw J for field I
XFP(J,I)=X/Y of front of +ve bar of jaw J for field I
XBP(J,I)=X/Y of back of +ve bar jof jaw J for field I
XFN(J,I)=X/Y of front of -ve bar of jaw J for field I
XBP(J,I)=X/Y of back of -ve bar jof jaw J for field I

MUINDEX(I) is a number in the range 0<=MUINDEX(I)<=1 and with\
MUINDEX(I)>MUINDEX(I-1) that effectively defines the range of monitor\
units over which the jaw settings corresponding to field I \
are used.  At the beginning of each primary\
history, a random number, MU_RND, is compared to MINDEX(I),I=1,NFIELDS.  The lowest value of I\
for which the MU_RND<=MUINDEX(I) determines the settings used.  If the user has\
selected "step-and-shoot" mode, the jaw settings for field I are used.  If the\
user has selected "dynamic" mode, then the jaw settings are a linear interpolation\
between fields I-1 and I based on the value of MU_RND.  For example, the value of XFP(J),\
the front X or Y coordinate of the +ve bar of jaw J, is given by:

XFP(J)=XFP(J,I-1)+[XFP(J,I)-XFP(J,I-1)]/[MUINDEX(I)-MUINDEX(I-1)]*[MU_RND-MUINDEX(I-1)]

Thus, the "dynamic" setting simulates jaw motion while the beam is on.

Note that for a given primary history the value of MU_RND is common to all synchronized CMs in the accelerator,\
allowing synchronized motion.  Also, if the accelerator is compiled as a shared library\
for use with DOSXYZnrc source 20 or 21, MU_RND is passed to/from DOSXYZnrc, allowing\
synchronization of source motion with CM opening coordinates.\
See the BEAMnrc and DOSXYZnrc Users Manuals for more info.\

Note that in "step-and-shoot" and "dynamic" modes, the user must still specify\
the orientation, media, ECUT, PCUT, dose zone and bit region of each set of\
jaws in a separate input window.  If the user selects "static" mode, then\
function reverts to being identical to JAWS.
}

set help_syncmlce_text {
SYNCMLCE is based on the MLCE component module used to model an Elekta MLC\
        with the additional option to model changing leaf opening coordinates\
        (in dynamic or step-and-shoot mode) over the course of an accelerator similation.\
        Furthermore, leaf opening coordinates\
        (fields) can be synchronized with the incident angle, SSD, and isocentre of source 20\
        or 21 in DOSXYZnrc.  SYNCMLCE is synchronized with any other synchronized CMs\
        (e.g. SYNCJAWS) in the accelerator.

Leaf geometry is identical to that in MLCE.  A "central leaf" is used to define the\
cross-section to be used for all leaves using the points (X3,ZMIN) and (X4,ZMAX), \
the Z-positions of the steps, ZSTEPL and ZSTEPR, and the step width, TGW. The NUM_LEAF\
total leaves are duplications of the central leaf, rotated about the\
Y or X axis (depending on leaf orientation) at Z=ZMIN and translated into the
appropriate X or Y position.  Rotation angles and translation distances are determined\
by the distance between leaf centres (SPACE) projected to the SSD.  Note that leaf sides\
all focus at Z=0.  The entire leaf bank can then be rotated by a user input angle, LBROT.
Leaf ends can by cylindrical or straight focused (to ZFOCUS).

For synchronized dynamic and\
step-and-shoot fields, the leaf opening coordinates are specified in a separate\
file with format:

TITLE -- a title (up to 80 characters)
NFIELDS -- the no. of fields
FOR I=1,NFIELDS[
MUINDEX(I) -- the monitor unit index of field I
NEG, POS, NUM -- the negative and positive coordinates of the leaf openings and
                 the number of adjacent leaves with those coordinates--repeat
                 this line until opening coordinates for all leaves are defined
]

MUINDEX(I) is a number in the range 0<=MUINDEX(I)<=1 and with\
MUINDEX(I)>MUINDEX(I-1) that defines the range of monitor\
units over which field I is used.  At the beginning of each primary\
history, a random number, MU_RND, is compared to MUINDEX(I),I=1,NFIELDS.  The lowest value of I\
for which the MU_RND<=MUINDEX(I) determines the field.  If the user has\
selected "step-and-shoot" mode, the opening coordinates for field I are used.  If the\
user has selected "dynamic" mode, then the coordinates are a linear interpolation\
between fields I-1 and I based on the value of MU_RND.  For example,\
the negative leaf opening coordinate for leaf J, NEG(J), is calculated using the negative\
opening coordinates of the leaf in field I, NEG(J,I), and field I-1, NEG(J,I-1), using:

NEG(J)=NEG(J,I-1)+[NEG(J,I)-NEG(J,I-1)]/[MUINDEX(I)-MUINDEX(I-1)]*[MU_RND-MUINDEX(I-1)]

Thus, the "dynamic" setting simulates leaf motion while the beam is on.

Note that for straight leaf ends, leaf opening coordinates are specifed at Z=ZMIN, while for\
cylindrical leaf ends, opening coordinates are specified by the X or Y positions of\
the origins of the cylinders defining the leaf ends (see Y cross-section, ENDTYPE=0).

Note that for a given primary history the value of MU_RND is common to all synchronized CMs in the accelerator,\
allowing synchronized motion.  Also, if the accelerator is compiled as a shared library\
for use with DOSXYZnrc source 20 or 21, MU_RND is passed to/from DOSXYZnrc, allowing\
synchronization of source motion with CM opening coordinates.\
See the BEAMnrc and DOSXYZnrc Users Manuals for more info.\
}

set help_syncvmlc_text {
SYNCVMLC is designed to model the Varian Millenium MLC.  It is based on\
DYNVMLC but allows synchronization between the leaf opening coordinates (field)\
and the incident direction, SSD and isocentre of DOSXYZnrc sources 20\
(synchronized phase space source) and 21 (synchronized BEAMnrc shared library\
source).  SYNCVMLC is synchronized with any other synchronized CMs\
(eg SYNCJAWS) in the accelerator.

Similar to DYNVMLC, the user specifies\
cross-sections for the 3 different types of leaves, FULL, TARGET and\
ISOCENTER found in the MLC (see Figure).  These cross-sections\
can be seen in more detail by using the "?" buttons in the\
"Define leaf cross-sections" window of the GUI.\
When assigning types to leaves in the leaf\
bank, TARGET/ISOCENTER leaves must always occur in pairs (this means that\
you really only have two choices when assigning leaf type).\
Note that all leaf widths, along with START and LEAFGAP, are specified at\
Z=ZMIN, but will not be constant throughout the Z thickness of the MLC due\
to focusing of the leaf sides.\

The Y cross-sections show the\
two possible types of leaf ends: cylindrical (ENDTYPE=0) and straight\
(ENDTYPE=1).\
For rounded ends you must specify a radius (LEAFRADIUS), and for straight ends\
you must specify a focal point on the Z axis (ZFOCUS(2)).  The origin of the\
radius for cylindrical ends is always at Z=ZMIN+ZTHICK/2.

The dimensions\
of the opening (NEG,POS) are specified at Z=ZMIN+ZTHICK/2 for a leaf with\
cylindrical ends and at Z=ZMIN for a leaf with straight ends.  For dynamic and\
step-and-shoot fields, the leaf opening coordinates are specified in a separate\
file with format:

TITLE -- a title (up to 80 characters)
NFIELDS -- the no. of fields
FOR I=1,NFIELDS[
MUINDEX(I) -- the monitor unit index of field I
NEG, POS, NUM -- the negative and positive coordinates of the leaf openings and
                 the number of adjacent leaves with those coordinates--repeat
                 this line until opening coordinates for all leaves are defined
]

MUINDEX(I) is a number in the range 0<=MUINDEX(I)<=1 and with\
MUINDEX(I)>MUINDEX(I-1) that defines the range of monitor\
units over which field I is used.  At the beginning of each primary\
history, a random number, RND, is compared to MUINDEX(I),I=1,NFIELDS.  The lowest value of I\
for which the MU_RND<=MUINDEX(I) determines the field.  If the user has\
selected "step-and-shoot" mode, the opening coordinates for field I are used.  If the\
user has selected "dynamic" mode, then the coordinates are a linear interpolation\
between fields I-1 and I based on the value of MU_RND.  For example,\
the negative leaf opening coordinate for leaf J, NEG(J), is calculated using the negative\
opening coordinates of the leaf in field I, NEG(J,I), and field I-1, NEG(J,I-1), using:

NEG(J)=NEG(J,I-1)+[NEG(J,I)-NEG(J,I-1)]/[MUINDEX(I)-MUINDEX(I-1)]*[MU_RND-MUINDEX(I-1)]

Thus, the "dynamic" setting simulates leaf motion while the beam is on.

Note that for a given primary history the value of MU_RND is common to all synchronized CMs in the accelerator,\
allowing synchronized motion.  Also, if the accelerator is compiled as a shared library\
for use with DOSXYZnrc source 20 or 21, MU_RND is passed to/from DOSXYZnrc, allowing\
synchronization of source motion with CM opening coordinates.\
See the BEAMnrc and DOSXYZnrc Users Manuals for more info.\
}

set help_synchdmlc_text {
SYNCHDMLC is optimized for simulating\
the high-definition micro MLC (HD120) available on TrueBeam and Novalis\
linacs.  The original code was contributed by Borges et al, while the current\
version is from Lobo & Popescu.  SYNCHDMLC is based on SYNCVMLC.

The user must specify cross-sections for five leaf types:\
FULL, HALF TARGET, HALF ISOCENTER, QUARTER TARGET, QUARTER ISOCENTER.  In general QUARTER TARGET/ISOCENTER\
leaves are thinner in the X-dimension\
 (ORIENT=0) or Y-dimension (ORIENT=1). HALF TARGET/ISOCENTER leaves \
and QUARTER TARGET/ISOCENTER leaves must be specified in pairs.

The figure at left shows a SYNCHDMLC with 12 leaves opening in the Y-direction (ORIENT=0).\
Leaves 1,2,11,12 are FULL; leaves 3,4,9,10 are HALF TARGET/ISOCENTER pairs; leaves 5-9 are\
QUARTER TARGET/ISOCENTER pairs.  Note that, in general, the narrower QUARTER TARGET/ISOCENTER\
pairs occur closer to the centre of the MLC to allow more high-resolution field definition.\
Although cross-sections must be specified for all leaf types, not all types need be used in\
an MLC.

SYNCHDMLC has the option to specify cylindrical (ENDTYPE=0) or straight, focused (ENDTYPE=1)\
leaf ends.  The origins of the cylinders defining leaf ends are at Z=ZMIN+ZTHICK/2\
(i.e. half way through the leaves in the Z-dimension).  For straight, focused leaf ends,\
opening coordinates (NEG, POS) are specified at Z=ZMIN, while for cylindrical leaf ends,\
NEG and POS are specified at Z=ZMIN+ZTHICK/2.

When used in ``dynamic'' or ``step-and-shoot'' mode, the leaf opening coordinates must\
be supplied in a separate file with the format:

TITLE -- a title (up to 80 characters)
NFIELDS -- the no. of fields
FOR I=1,NFIELDS[
MUINDEX(I) -- fractional monitor unit up to and including field I
NEG, POS, NUM -- the negative and positive coordinates of the leaf openings and
                 the number of adjacent leaves with those coordinates--repeat
                 this line until opening coordinates for all leaves are defined
]

MUINDEX(I) is a number in the range 0<=MUINDEX(I)<=1 and with\
MUINDEX(I)>MUINDEX(I-1) and is equal to the fractional monitor\
units up to and including field I.  A particle is incident through field I\
if MUINDEX(I-1)<MU_RND<=MUINDEX(I), where MU_RND is a random number on [0,1]\
chosen at the beginning of each primary\
history.\
If the user has\
selected "step-and-shoot" mode, the opening coordinates for field I are used.  If the\
user has selected "dynamic" mode, then the coordinates are a linear interpolation\
between fields I-1 and I based on the value of MU_RND.  For example,\
the negative leaf opening coordinate for leaf J, NEG(J), is calculated using the negative\
opening coordinates of the leaf in field I, NEG(J,I), and field I-1, NEG(J,I-1), using:

NEG(J)=NEG(J,I-1)+[NEG(J,I)-NEG(J,I-1)]/[MUINDEX(I)-MUINDEX(I-1)]*[MU_RND-MUINDEX(I-1)]

Thus, the "dynamic" setting simulates leaf motion while the beam is on.

Note that for a given primary history the value of MU_RND is common to all synchronized CMs in the accelerator,\
allowing synchronized motion.  Also, if the accelerator is compiled as a shared library\
for use with DOSXYZnrc source 20 or 21, MU_RND is passed to/from DOSXYZnrc, allowing\
synchronization of source motion with CM opening coordinates.\
See the BEAMnrc and DOSXYZnrc Users Manuals for more info.
}

set help_synchdmlc_full_text {
Figure showing a cross-section through a FULL leaf and the dimensions that\
you must input:

widths (cm):
wl = leaf width (excl. tongue)
wt = tongue width
wg = groove width
wtip = tip width
wts = upper support rail
wbs = lower support rail

Z positions (cm):
ztip = Z of top of tip
zl = Z of top of leaf
zt = Z of bottom of tongue
zg = Z of bottom of groove
zth = Z of top of driving screw hole
zbh = Z of bottom of driving screw hole
zts = Z of top of support rail
zbs = Z of bottom of support rail

Note that the Z position of the bottom of the leaf is always\
ZMIN+ZTHICK.

Widths and Z positions must be set up so that the dashed grid lines\
shown in the figure do not change order.  They can, however, overlap.  This\
means, for example that wt <= wtip <= wts-wbs and that\
ztip <= zl <= zt.

You must also ensure that FULL and HALF TARGET leaves can fit together by\
ensuring that zg >= Z of bottom of tongue of HALF TARGET leaves and that FULL and\
HALF ISOCENTER leaves fit together by ensuring that zt <= Z of bottom of groove\
of HALF ISOCENTER leaves.
}

set help_synchdmlc_tar_text {
Figure showing a cross-section through a HALF TARGET leaf and the dimensions that\
you must input:

widths (cm):
wl = leaf width (excl. tongue)
wt = tongue width
wg = groove width
wtip = tip width
wts = upper support rail
wbs = lower support rail

Z positions (cm):
zts = Z of top of support rail
zbs = Z of bottom of support rail
zth = Z of top of driving screw hole
zbh = Z of bottom of driving screw hole
zt = Z of bottom of tongue
zg = Z of top of groove
zl = Z of bottom of leaf
ztip = Z of bottom of tip

Note that the Z position of the top of the leaf is always\
ZMIN.

Widths and Z positions must be set up so that the dashed grid lines\
shown in the figure do not change order.  They can, however, overlap.  This\
means, for example that wbs - wg <= wts <= wbs and that\
zg <= zl <= ztip.

You must also ensure that:
1. HALF TARGET and FULL leaves can fit together by\
ensuring that zt <= Z of bottom of groove of FULL leaves
2. HALF TARGET and HALF ISOCENTER leaves fit together by ensuring that zt <= Z of bottom of groove\
of HALF ISOCENTER leaves and zg <= Z of top of tongue of HALF ISOCENTER leaves.
3. HALF TARGET and QUARTER ISOCENTER leaves fit together by ensuring that zt <=\
Z of bottom of groove of QUARTER ISOCENTER leaves.
}

set help_synchdmlc_iso_text {
Figure showing a cross-section through a HALF ISOCENTER leaf and the dimensions that\
you must input:

widths (cm):
wl = leaf width (excl. tongue)
wt = tongue width
wg = groove width
wtip = tip width
wts = upper support rail
wbs = lower support rail

Z positions (cm):
ztip = Z of top of tip
zl = Z of top of leaf
zt = Z of top of tongue
zg = Z of bottom of groove
zth = Z of top of driving screw hole
zbh = Z of bottom of driving screw hole
zts = Z of top of support rail
zbs = Z of bottom of support rail

Note that the Z position of the bottom of the leaf is always\
ZMIN+ZTHICK.

Widths and Z positions must be set up so that the dashed grid lines\
shown in the figure do not change order.  They can, however, overlap.  This\
means, for example that wts - wt <= wbs <= wts and that\
zt <= zg <= zth.

You must also ensure that:
1. HALF ISOCENTER and FULL leaves can fit together by\
ensuring that zg >= Z of bottom of tongue of FULL leaves
2. HALF ISOCENTER and HALF TARGET leaves fit together by ensuring that zg >= Z of bottom of tongue\
of HALF TARGET leaves and zt >= Z of top of groove of HALF TARGET leaves.
3. HALF ISOCENTER and QUARTER TARGET leaves fit together by ensuring that zg >= Z of bottom of tongue\
of QUARTER TARGET leaves.
}

set help_synchdmlc_qtar_text {
Figure showing a cross-section through a QUARTER TARGET leaf and the dimensions that\
you must input:

widths (cm):
wl = leaf width (excl. tongue)
wt = tongue width
wg = groove width
wtip = tip width
wts = upper support rail
wbs = lower support rail

Z positions (cm):
zts = Z of top of support rail
zbs = Z of bottom of support rail
zth = Z of top of driving screw hole
zbh = Z of bottom of driving screw hole
zt = Z of bottom of tongue
zg = Z of top of groove
zl = Z of bottom of leaf
ztip = Z of bottom of tip

Note that the Z position of the top of the leaf is always\
ZMIN.

Widths and Z positions must be set up so that the dashed grid lines\
shown in the figure do not change order.  They can, however, overlap.  This\
means, for example that wbs - wg <= wts <= wbs and that\
zg <= zl <= ztip.

You must also ensure that:
1. QUARTER TARGET and HALF ISOCENTER leaves can fit together by\
ensuring that zt <= Z of bottom of groove of HALF ISOCENTER leaves
2. QUARTER TARGET and QUARTER ISOCENTER leaves fit together by ensuring that zt <= Z of bottom of groove\
of QUARTER ISOCENTER leaves and zg <= Z of top of tongue of QUARTER ISOCENTER leaves.
}

set help_synchdmlc_qiso_text {
Figure showing a cross-section through a QUARTER ISOCENTER leaf and the dimensions that\
you must input:

widths (cm):
wl = leaf width (excl. tongue)
wt = tongue width
wg = groove width
wtip = tip width
wts = upper support rail
wbs = lower support rail

Z positions (cm):
ztip = Z of top of tip
zl = Z of top of leaf
zt = Z of top of tongue
zg = Z of bottom of groove
zth = Z of top of driving screw hole
zbh = Z of bottom of driving screw hole
zts = Z of top of support rail
zbs = Z of bottom of support rail

Note that the Z position of the bottom of the leaf is always\
ZMIN+ZTHICK.

Widths and Z positions must be set up so that the dashed grid lines\
shown in the figure do not change order.  They can, however, overlap.  This\
means, for example that wts - wt <= wbs <= wts and that\
zt <= zg <= zth.

You must also ensure that:
1. QUARTER ISOCENTER and HALF TARGET leaves can fit together by\
ensuring that zg >= Z of bottom of tongue of HALF TARGET leaves
2. QUARTER ISOCENTER and QUARTER TARGET leaves fit together by ensuring that zg >= Z of bottom of tongue\
of QUARTER TARGET leaves and zt >= Z of top of groove of QUARTER TARGET leaves.
}

# HELP TEXT FOR SOURCES:

set source_text(0) {
ISOURC=0: Parallel Circular Beam
Four parameters are used for this source: the beam radius and\
	the (x,y,z) direction cosines.  \
	The parallel circular beam is always assumed to be\
	incident on the center\
	of the front of the first CM (i.e.  at Z_min_CM(1)).

The radius of the beam, in cm, defaults to the maximum radius or\
	root 2 * half-width\
	of the first CM if set greater than this value.  \
	The X- and Y-axis direction cosines default to 0 and the Z-axis\
	direction cosine defaults to 1 (parallel to the Z-axis).  \
	The directional cosines are automatically normalized by the square\
	root of the sum of their squares.  Note that it is possible to have\
	a pencil beam with radius 0. The figure to the left shows\
	the parallel circular beam and its input parameters.
}
set source_text(1) {
ISOURC=1: Isotropic Point Source on Z-axis

The isotropic point source is placed on the Z-axis. It is assumed\
	to be incident on the first CM, but can be placed any\
	distance above Z_min_CM(1). The field of radiation considered\
	can be a circle centred on the Z axis or a rectangle anywhere\
        on the surface of CM 1 (figure shows a circular field).

The distance of the source above Z_min_CM(1) in cm defaults to 100 cm if left\
	unspecified.  If you have chosen a circular beam, the beam radius\
	defaults to the maximum radius or root 2 * half-width of\
	the first CM if left\
	unspecified, set too large or if both the radius and \
        half-angle wrt the Z axis (GAMMA) are set to 0.  The only constraint\
        on rectangular beams is that the field be completely contained\
        within the boundaries of CM 1.  It is possible to have a \
        pencil beam when a rectangular field is chosen.

Circular fields can also be specified by their half-angle relative to\
	the Z axis (GAMMA) if the radius is set to 0.\

The space between the source and Z_min_CM(1) is assumed to be a vacuum. \
	The figure illustrates the point source and its input variables.
}

set source_text(2) {
ISOURC=3: Interior Isotropic Cylindrical Source

The uniform radiating cylindrical isotropic source can be a\
	ring centered on the Z-axis or a cylinder with its central\
	axis parallel to the X-axis. The source is\
	contained within the geometry of one or more component modules\
	(i.e.  it is able to span adjacent CMs). Currently, this\
	source can only be used inside CONESTAK, SIDETUBE or FLATFILT\
	CMs because of the need to identify the initial regions.

The first parameter is the inner radius of the vertical ring OR the\
	Z position of the centre of the horizontal cylinder.  The second\
	is the outer radius of the vertical ring OR the radius of the\
	horizontal cylinder.  For the vertical ring, the maximum value\
	is the maximum radius of the largest CM containing the source; \
	for the horizontal cylinder, the maximum value keeps\
	the radius entirely contained between Z_min_CM(1) and the bottom\
	of the geometry.  Defaults to maximum if the value entered\
	is larger than the maximum.

The third anf fourth parameters are Z of top and bottom of the vertical ring\
	OR minimum and maximum X of the horizontal cylinder.  \
	For the vertical ring, the minimum value is Z_min_CM(1) and the maximum\
	is the bottom of the geometry; for the horizontal cylinder, the\
	    minimum and maximum X values are the positive and negative of\
	    the maximum radius of the largest CM containing\
	    the source.  Any values input outside of these regions results in\
	    defaulting to the extrema.
}
set source_text(3) {
ISOURC=5: NRC Swept BEAM

The NRC swept beam is a parallel circular beam swept around the\
	outside of an imaginary cone. The beam is assumed\
	incident on the front of the first CM. The apex of the imaginary\
	cone is always at (X=0,Y=0,Z=Z_min_CM(1)), and the cone\
	angle is variable.

The half-angle of the cone in degrees is restricted to\
	between 0 and 90 degrees.  \
	The radius of the beam in cm has a maximum value equal to the maximum\
	radius or root 2 * half-width of the first CM.  \
	It defaults to this values if\
	the value entered exceeds this limit.

The figure shows the swept beam source and its input parameters.
}

set source_text(4) {
ISOURC=6: Parallel Rectangular Beam

The parallel rectangular beam is the rectangular equivalent of the\
	parallel circular beam (ISOURC=0) with the\
	exception that the beam cannot be oblique; it is always\
	perpendicular to the front surface of the first CM and this beam can\
	be offset from the Z-axis.

The (X,Y) position of beam centre (XBEAM0,YBEAM0) must be entered in cm\
	(defaults to 0 if XBEAM0^2  + YBEAM0^2  > RMAX_CM(1)^2  when the\
	first CM is circular or if |XBEAM0| or |YBEAM0| > RMAX_CM(1)\
	when the first CM is a square)

The beam half-widths in the X- and Y-directions (XBEAM,YBEAM) must also\
	be entered in cm (defaults to keep |XBEAM0| + |XBEAM|\
	less than or equal to the radius or half-width of the first CM, \
	and likewise for the Y-direction).
}

set source_text(5) {
ISOURC=7: Scanning Sawtooth Beam

In the scanning sawtooth beam source, a circular parallel beam, \
	incident on the front of the first CM, is scanned in a zig-zag\
	pattern on the X-Y plane. The angle for the scan is selected\
	as the particles leave the Z_min_CM(1) plane. This is an\
	approximation of the real case where the beam is bent\
	below this point usually. The source randomly selects points in the\
	scan. The input parameters are:


The scanning field size at SSD=100cm (FD_AT100), is defined in cm, \
	resulting in a field size of FD_AT100 x FD_AT100.  \
	This is often much greater than the actual field size.

The number of Y scans per X scan, IRATIO_YXF, defaults to 6.5\
	if set negative; also, 2xIRATIO_YXF is always rounded\
	up to the nearest odd integer.

The radius of the incident beam, in cm, defaults to 0.01 cm if set\
	negative and defaults to the maximum radius (circular first CM)\
	or root 2 * half-width (square first CM)

Note that the field is always scanned twice in the X direction; \
	thus, the number of Y scans is 2xIRATIO_YXF. It is also\
	important to note that FD_AT100 defines the field size\
	covered by the beam central axis; a beam with finite beam radius will\
	actually go outside FD_AT100.  The figure shows the\
	scanning beam and its various input parameters.
}

set source_text(6) {
ISOURC=8: Scanned Point Source for MM50-Uniform Field

This source is designed to simulate the source used in\
	the MM50 linear accelerator for electron beams. It simulates a\
	point source scanned in the X-Y plane to produce\
	uniform particle fluence at a user-specified\
	source-to-surface distance (SSD) over a field of user-specified\
	radius.  The point source is located at Z=0 but particles are\
        incident at Z=Z_min_CM(1).  Thus Z_min_CM(1) must be >=0.
        Incident X and Y positions and X, Y and Z direction cosines\
        are chosen such that, if there were no particle interactions,\
        particle fluence would be uniform at any horizontal plane intersecting the\
        cone (apex at Z=0, radius = user-specified radius at SSD)\
        defining the incident beam. There is also an option\
        to input a beam spot radius, RBEAM0.

DISTZ, the source-to-surface distance (SSD) at which uniform\
	particle distribution is desired in cm, defaults to 100 cm if it\
	is set to zero or negative.

RBEAM, the radius of the field at DISTZ in cm, defaults to a\
	maximum value which depends on how far Z_min_CM(1) is away\
	from the point source at Z=0. Basically, the cone joining\
	the circle of radius RBEAM and the source at Z=0 must be\
	within the outer boundaries of the first CM as the cone\
	crosses Z_min_CM(1) (i.e.  <=RMAX_CM(1) if the first CM\
	is circular or root 2 * RMAX_CM(1) if the first CM is square).

RBEAM0, the beam spot radius in cm.  Defaults to 0 if set <=0.\
        Note that the incident X and Y positions of particles are based\
        on uniform fluence for RBEAM0=0.  Thus, if you input a finite\
        value of RBEAM0, fluence will fall off at the outer, circular edge\
        of the beam.
}

set source_text(7) {
ISOURC=9: Scanned Point Source for MM50-Discrete Field

This source is designed to simulate the source used in the MM50\
	linear accelerator to produce photon beams. It behaves like\
	a point source emanating pencil beams to a finite number of\
	dwell points, providing discrete field coverage. The user\
	specifies by the X,Y coordinates of the dwell points on a\
	plane perpendicular to the Z-axis at a user-specified\
	source-to-surface distance (SSD) and the probability of\
	a particle being initially directed towards each point. The other\
	conditions are much as in source 8, in particular the source\
	is always considered to be at Z=0.0.

DISTZ, the source-to-surface distance (SSD) in cm, defaults\
	to 100 cm if it is set to 0 or negative.
NPTS_SRC9, the number of points used to cover the field, \
	defaults to 1 if set to 0 or negative.
X_SRC9(I) (I=1,...,NPTS_SRC9) defines the X points on the plane\
	perpendicular to Z at DISTZ, in cm.
Y_SRC9(I) (I=1,...,NPTS_SRC9) defines the Y points on the plane\
	perpendicular to Z at DISTZ, in cm .
PROB_SRC9(I) (I=1,...,NPTS_SRC9) defines the probability of\
	a particle being incident on point I.  Probabilities\
	are automatically normalized.
}

set source_text(8)  {
ISOURC=10: Parallel Circular Beam Incident from the Side

This parallel circular beam enters the first CM from the side\
	and must to be used as the source for an XTUBE.

RBEAM, the radius of the beam in cm, defaults to half the thickness\
	of the first CM if it is greater than this and to 0 if it is set\
	negative.
UINC, VINC, WINC define the incident (X,Y,Z)-axis direction cosines.  \
	UINC should be negative so the beam faces the Z-axis; \
	it is reset to -UINC if it is positive.  \
	(UINC,VINC,WINC) defaults to (-1,0,0) if UINC^2 +VINC^2 +WINC^2 = 0.  \
	The direction cosines UINC, VINC and WINC are each automatically\
	normalized by (UINC^2 +VINC^2 +WINC^2 ).
}

set source_text(9) {
ISOURC=13: Parallel Rectangular Beam Incident from Side

This beam is the rectangular equivalent of ISOURC=10.

YBEAM, the half-width of the beam in cm, defaults to 0.2 cm\
	if set to 0 or negative and defaults to half the thickness\
	of the first CM if set > than this.

ZBEAM, the half-height of the beam in cm, defaults to 0.2 cm\
	if set to 0 or neagtive and defaults to half the thickness\
	of the first CM if set > than this.

UINC, VINC are the incident X- and Y-axis direction cosines.  UINC\
	should be negative so that the beam faces the Z-axis and is\
	reset to -UINC if it is set positive.  \
	(UINC,VINC) defaults to (-1,0) if UINC^2 +VINC^2 = 0.

UINC and VINC are automatically normalized by (UINC^2 +VINC^2).  \
	Note that the Z direction cosine, WINC, is always\
	assumed to be 0 for this beam.
}

set source_text(10) {
ISOURCE=15: NRC Swept Beam with Beam Divergence and Radial Distribution


}

set source_text(11) {
ISOURCE=19: Elliptical Beam with Gaussian Distributions in X and Y, Parallel or with\
            Angular Spread

The ellipse is specified by the standard deviations (sigma) or full-width half maxima (FWHM's)\
        of the intensity distributions in X and Y (in cm).\
	If FWHM's are\
	entered, the code converts these values to sigmas.  Note that\
	the sigmas of the X and Y distributions are automatically limited to\
	be within the bounds of the first CM.  Note that if FWHM/sigma in the X-direction=0\
        then the beam collapses to a pencil beam, while if FWHM/sigma in the Y-direction=0\
        then the beam will be a circular beam with a gaussian radial intensity distribution=\
        the FWHM/sigma specified in the X-direction.

A mean angular spread, sigma_src19, about the Z-axis can be specified.\
        In this case, (UINC,VINC,WINC), the X, Y and Z direction cosines,\
        take their default values of (0,0,1) (i.e. beam directed along Z-axis).\
        If there is no spread (theta not set or set <=0), then\
        UINC, VINC, WINC can be used to specify an incident beam direction.
}

set source_text(12) {
ISOURC=21: Phase Space Source

This source routine allows a phase space file generated at any scoring plane\
to be used as a source.  Note that the LATCH values are passed on when using\
this source routine so it is necessary to number LATCH bits consistently\
between the simulation generating the input file and the current simulation.\
Also, dose and fluence are normalized by the number of initial particles in\
the original, primary source and not by the number of particles incident from\
the phase space source.

Input parameters for a phase space source are:

INIT_ICM, the component module on which the phase space source is incident\
(defaults to 1 if it is set to < 1 or > the number of CMs in the model)

SPCNAM, the filename (with extension) of the phase space source.

NRCYCL, the number of times each particle is recycled before moving on to the\
next one.  Thus, each particle is used NRCYCL+1 times before moving on.\
Note that, even using NRCYCL, the total number of histories is still\
determined by the user-input value of NCASE.\
NRCYCL is necessary to avoid restarting phase space source if phase space data\
is sparse, since restarting may cause underestimates of the uncertainty.\
Set <=0 and BEAMnrc will automatically calculate a value of NRCYCL.  The\
source may restart even with automatically calculated value of NRCYCL.\
This is not a problem if only a small fraction of the source is re-used on\
the second pass.  Otherwise, we recommend re-running the simulation with\
NRCYCL re-calculated using:

NRCYCL=NCASE/[NNPHSP-NNPHSP*(NPASS_ph_sp+NFAT_ph_sp)/(NTOT_ph_sp+\
NPASS_ph_sp+NFAT_ph_sp)]-1

where NCASE=no. of histories, NNPHSP=total no. of particles in phase space\
source, NTOT_ph_sp=total no. of particles used from source in previous run\
(not including recycling), NPASS_ph_sp= total no. of particles rejected\
from previous run as multiple passers (not including recycling), and\
NFAT_ph_sp is the no. of fat photons (not including recycling) rejected from\
the source (see below for more details).  NNPHSP, NTOT_ph_sp, NPASS_ph_sp and\
NFAT_ph_sp are available in the .egslst file from the previous run.  Always\
round your calculated values of NRCYCL up.\

IPARALLEL, only used if simulation is being divided into parallel jobs.  Set\
equal to no. of parallel jobs.  This is used with PARNUM (below) to split a\
phase space source into partitions.  If IPARALLEL<=1, phase space source is\
not split.

PARNUM, only used if simulation is divided into IPARALLEL jobs.  Each\
parallel job should have a different integer setting of PARNUM to cover the\
range 1<=PARNUM<=IPARALLEL.  Each job will use a different partition of the\
phase space source given by:

(PARNUM-1)*(NNPHSP/IPARALLEL)<INPHSP<=(PARNUM)*(NNPHSP/IPARALLEL)

where NNPHSP=total no. of particles in the phase space source and\
INPHSP=particle no. being used.  Splitting the phase space source in this\
way ensures even sampling of the source over all parallel jobs.  If\
PARNUM<=0, then the source is not split.  Note that if you are submitting\
parallel jobs from the GUI (which uses the pprocess script) IPARALLEL, and\
PARNUM are set automatically.

If you have used directional bremsstrahlung splitting (DBS--see Section 6.3.4\
of the BEAM Manual for more details) in the BEAM simulation generating\
this phase space source, then it is recommended that you reject fat\
(high-weight) photons from the source because they may compromise statistics.\
To do this, select "DBS used to generate source" (this sets ISRC_DBS=1), set\
the DBS splitting field radius (RSRC_DBS) and the SSD of the splitting field\
(SSDSRC_DBS) equal to their values in the BEAM simulation that generated this\
source and, finally, input the Z value where this source was scored in that\
simulation (ZSRC_DBS), which will be at the back of the CM in which scoring was\
done.  Note the restriction that SSDSRC_DBS>=ZSRC_DBS.  Photons whose\
trajectory from ZSRC_DBS to SSDSRC_DBS takes them outside RSRC_DBS will be\
rejected from the current simulation since they will not have been split\
in the simulation that generated the source and will, therefore, be fat.

For more information on phase space sources, see Section 4.13 of the\
BEAMnrc users manual.

}

set source_text(13) {ISOURC=23: BEAM Simulation Source

This source routine allows a BEAM accelerator that has been compiled\
as a shared library to be used as a source in second-stage BEAM simulation.\
It is also known as a BEAM shared library source.\
 This source is similar to a phase space source except that no intermediate\
phase space file needs to be stored.  Phase space data from the shared library\
source is stored in a source container, and the second-stage simulation only\
runs a new primary history in the shared library source if the source\
container contains no more data.

Inputs for the BEAM simulation source are:

The component module number on which the source is incident (INIT_ICM).\
The simulation source is always incident on the top of this component\
module.

The angle of rotation of the source plane about the X-axis (ALPHA24)\
in degrees.  Positive rotation is in the clockwise sense.  Restriction\
is -90 < ALPHA24 < 90.

The angle of rotation of the source plane about the Y-axis (BETA24)\
in degrees.  Positive rotation is in the counter-clockwise sense.\
Restriction is -90 < BETA24 < 90.

Distance of the point of rotation above INIT_ICM (DIST24).

Note that if ALPHA24~=0 or BETA24~=0, then INIT_ICM must be >1 to allow\
for source particles incident within INIT_ICM-1.  Also, note that the\
CMs:
APPLICAT,ARCCHM,CHAMBER,CIRCAPP,CONESTAK,DYNJAWS,JAWS,MESH,PYRAMIDS,\
SIDETUBE
are currently not coded to handle forward-directed particles incident\
within them.

The name of the BEAM simulation which will be run as the source \
(the_beam_code).  The "BEAM_" prefix must also be specified (i.e.\
BEAM_accelname).  This accelerator must have been compiled as a shared library\
(by typing "make library" in the accelerator directory) for the machine that\
the second-stage simulation is running on.  The BEAM GUI allows you to\
browse your EGS_HOME/bin/my_machine directory for those accelerators that\
have been compiled as shared libraries.

The input file for the BEAM simulation source (the_input_file).  The\
.egsinp extension is not required.  This input file must exist in the\
directory (EGS_HOME/BEAM_accelname) of the accelerator being\
used as the simulation source and MUST specify that phase space data\
be scored at a single scoring plane.  Instead of writing this phase space\
data to a file, it will be stored in the source container for use in the\
second-stage simulation.  The BEAM GUI allows you to browse the .egsinp\
files present in the accelerator directory of the simulation source.\
Note that the full directory path to the input file is not specified.

The pegs data for the BEAM simulation source (the_pegs_file).  The .pegs4inp\
extension is not required.  This data must exist in the main pegs data directory\
(HEN_HOUSE/pegs4/data) or in the user's pegs data directory\
(EGS_HOME/pegs4/data).  The GUI allows browsing of both these directories.

A flag (checkbox in the BEAM GUI) indicating whether fat photons from\
directional bremsstrahlung splitting (DBS) used in the\
simulation source are to be rejected (ISRC_DBS).  If the simulation source\
uses DBS, then rejection of fat photons is recommended to avoid\
compromising statistics.

Note that when a BEAM simulation source is used, scored quantities are\
normalized by the number of primary histories in the simulation source.

For more information about BEAM simulation sources, see the BEAM Users
Manual.

}

set source_text(14) {ISOURC=24: Phase space source from user-specified direction

This source is similar to ISOURC=21 (phase space source) with additional inputs\
specifying the angle of the source plane.  Additional inputs are:

The angle of rotation of the source plane about the X-axis (ALPHA24)\
in degrees.  Positive rotation is in the clockwise sense.  Restriction\
is -90 < ALPHA24 < 90.

The angle of rotation of the source plane about the Y-axis (BETA24)\
in degrees.  Positive rotation is in the counter-clockwise sense.\
Restriction is -90 < BETA24 < 90.

Distance of the point of rotation above INIT_ICM (DIST24).

Note that if ALPHA24~=0 or BETA24~=0, then INIT_ICM must be >1 to allow\
for source particles incident within INIT_ICM-1.  Also, note that the\
CMs:
APPLICAT,ARCCHM,CHAMBER,CIRCAPP,CONESTAK,DYNJAWS,JAWS,MESH,PYRAMIDS,\
SIDETUBE
are currently not coded to handle forward-directed particles incident\
within them.

The initial concept and large portions of the coding for source 24\
are courtesy of Patrick Downes at Cardiff University, Wales.

}

set source_text(15) {ISOURC=31: Phase Space Reconstructed Using Beam Models

This source routine reconstructs the phase space parameters\
	using the beam data derived from an existing phase-space file.  \
	The reconstructed phase space sources can be incident on any CM.  \
	Use of beam characterization models can save CPU time\
	for the accelerator head simulation and also result in significant\
	reduction in disk space requirement for phase-space\
	storage. For more information related to beam reconstruction\
	using beam characterization models, see NRCC report\
	``Beam Characterization: a Multiple-Source Model'' by Ma\
	and Rogers (1995).


The component module on which the phase space source is incident, CMSOU, \
	defaults to 1 if it is set to < 1 or > the number of CMs\
	in the model.
SPCNAM is the filename (with extension) of the beam model parameters.
}

set help_text(chm_option) {
The user can think of the inputs of the CHAMBER CM\
	as defining an ion chamber or\
	the central axis region of a phantom.  This choice only affects the\
	struture of the next window presented, but does not affect\
	the actual parameters input.  This parameter is not passed in the\
	input file and so must be reselected by the user each time\
	the GUI is used.
}

