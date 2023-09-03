
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: parameters
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
#                   Iwan Kawrakow
#                   Ernesto Mainegra-Hing
#                   Frederic Tessier
#                   Marc-Andre Renaud
#                   Reid Townson
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
#  This script is read from the main script dosxyznrc_gui.  It has no
#  procedure.
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

This GUI is meant to be used to generate DOSXYZnrc input files.

There are two options available to you from the main window:
1) Create a new input file,
2) Load a previous input file, and
3) Save input parameters as...

"Create a new input file" allows you to start from scratch in\
	defining a DOSXYZnrc input file.

If you wish to load an input file which has already been completely\
	defined, start with "Load a previous input file".  \
	At this point you can edit the simulation parameters.  \

The last step in using the GUI is "Save input parameters as...".  \
	Select a file name (with extension .egsinp) and click OK.  \
	That's it!  You're done.  Run DOSXYZnrc as usual.
}

set izopts(0) "x-scan per page"
set izopts(1) "z-scan per page"
set izopts(2) "y-scan per page"

set names(1) "Title";
set help_text(1) {
Title:

Type the title you wish to use for the simulation\
	in the space provided (maximum 80 characters).  \
	It is strongly recommended\
	to identify the name of the input file in the title.
}

set help_text(CTdataflag) {

Select the method by which you would like to define the phantom here.  \
	If you would like to create a phantom from binary CT data, \
	select the 'phantom created from CT data' radiobutton.  See the DOSXYZnrc\
	User's Manual for information on how to create this file.  If\
	you would like to create a phantom by defining the properties\
	of individual volume elements, select the 'non-CT data input'\
	radiobutton.  After making a selection, click on the 'Define'\
	button to proceed with the definition of the phantom.
}

set names(2) "Number of media"
set help_text(2) {
Number of media:

Enter the number of media used in the phantom.  The entire volume\
	is initially set to medium 1 (the first medium entered).  \
	The entry must exactly match the name used in pegs4dat.
}

# SOURCES
set names(3) "Incident particle"
set help_text(3) {
Incident particle, IQIN:

Set the charge of the incident beam by selecting an\
	incident particle, electron, photon or positron, or all three\
	(only available for source 2 and 4).
}
set numopts(3) 3
set options(3,-1) "electron"
set options(3,0) "photon"
set options(3,1) "positron"
set options(3,2) "all"

set names(4) "Source type";
set numopts(4) 11;

set options(4,0) "0 - Parallel beam from the front"
set numsrcopts(0) 7;
set srcoptnames(0,0) "Lower x-bound on source (cm)";
set srcoptnames(0,1) "Upper x-bound on source (cm)";
set srcoptnames(0,2) "Lower y-bound on source (cm)";
set srcoptnames(0,3) "Upper y-bound on source (cm)";
set srcoptnames(0,4) "Angle of the beam relative to the X-axis (degrees)";
set srcoptnames(0,5) "Angle of the beam relative to the Y-axis (degrees)";
set srcoptnames(0,6) "Angle of the beam relative to the Z-axis (degrees)";

set options(4,1) "1 - Parallel beam from any direction with rectangular collimation"
set numsrcopts(1) 8
set srcoptnames(1,0) "x-coordinate of the isocenter"
set srcoptnames(1,1) "y-coordinate of the isocenter"
set srcoptnames(1,2) "z-coordinate of the isocenter"
set srcoptnames(1,3) "Theta (degrees)"
set srcoptnames(1,4) "Phi (degrees)"
set srcoptnames(1,5) "Total beam x-width (cm)"
set srcoptnames(1,6) "Total beam y-width (cm)"
set srcoptnames(1,7) "Collimator angle (degrees)"

set options(4,2) "2 - Full phase-space source file"
set numsrcopts(2) 7;
set srcoptnames(2,0) "x-coordinate of the isocenter"
set srcoptnames(2,1) "y-coordinate of the isocenter"
set srcoptnames(2,2) "z-coordinate of the isocenter"
set srcoptnames(2,3) "Theta (degrees)"
set srcoptnames(2,4) "Phi (degrees)"
set srcoptnames(2,5) "Distance from source to isocenter (cm)"
set srcoptnames(2,6) "Collimator angle (degrees)"

set options(4,3) "3 - Point source from the front with rectangular collimation"
set numsrcopts(3) 5
set srcoptnames(3,0) "Lower x-bound on source (cm)"
set srcoptnames(3,1) "Upper x-bound on source (cm)"
set srcoptnames(3,2) "Lower y-bound on source (cm)"
set srcoptnames(3,3) "Upper y-bound on source (cm)"
set srcoptnames(3,4) "Source to surface distance (SSD) (cm)";

set options(4,4) "4 - Beam characterization model"
set numsrcopts(4) 7
set srcoptnames(4,0) "x-coordinate of the isocenter"
set srcoptnames(4,1) "y-coordinate of the isocenter"
set srcoptnames(4,2) "z-coordinate of the isocenter"
set srcoptnames(4,3) "Theta (degrees)"
set srcoptnames(4,4) "Phi (degrees)"
set srcoptnames(4,5) "Distance from source to isocenter (cm)"
set srcoptnames(4,6) "Collimator angle (degrees)"

set options(4,6) "6 - Uniform isotropically radiating parallelpiped within\
	DOSXYZnrc phantom"
set numsrcopts(6) 6
set srcoptnames(6,0) "min x of active volume (cm)"
set srcoptnames(6,1) "max x of active volume (cm)"
set srcoptnames(6,2) "min y of active volume (cm)"
set srcoptnames(6,3) "max y of active volume (cm)"
set srcoptnames(6,4) "min z of active volume (cm)"
set srcoptnames(6,5) "max z of active volume (cm)"

set options(4,7) "7 - Parallel rectangular beam from multiple directions"
set numsrcopts(7) 6
set srcoptnames(7,0) "x-coordinate of the isocenter"
set srcoptnames(7,1) "y-coordinate of the isocenter"
set srcoptnames(7,2) "z-coordinate of the isocenter"
set srcoptnames(7,3) "Total beam x-width (cm)"
set srcoptnames(7,4) "Total beam y-width (cm)"
set srcoptnames(7,5) "Collimator angle (degrees)"

set options(4,8) "8 - Phase-space source from multiple directions"
set numsrcopts(8) 5
set srcoptnames(8,0) "x-coordinate of the isocenter"
set srcoptnames(8,1) "y-coordinate of the isocenter"
set srcoptnames(8,2) "z-coordinate of the isocenter"
set srcoptnames(8,3) "Distance from source to isocenter (cm)"
set srcoptnames(8,4) "Collimator angle (degrees)"

set options(4,9) "9 - BEAM treatment head simulation"
set numsrcopts(9) 7;
set srcoptnames(9,0) "x-coordinate of the isocenter"
set srcoptnames(9,1) "y-coordinate of the isocenter"
set srcoptnames(9,2) "z-coordinate of the isocenter"
set srcoptnames(9,3) "Theta (degrees)"
set srcoptnames(9,4) "Phi (degrees)"
set srcoptnames(9,5) "Distance from source to isocenter (cm)"
set srcoptnames(9,6) "Collimator angle (degrees)"

set options(4,10) "10 - BEAM simulation source from multiple directions"
set numsrcopts(10) 5;
set srcoptnames(10,0) "x-coordinate of the isocenter"
set srcoptnames(10,1) "y-coordinate of the isocenter"
set srcoptnames(10,2) "z-coordinate of the isocenter"
set srcoptnames(10,3) "Distance from source to isocenter (cm)"
set srcoptnames(10,4) "Collimator angle (degrees)"

set options(4,20) "20 - Phase-space source through dynamic library with multiple variable geometry settings"
set numsrcopts(20) 0;

set options(4,21) "21 - Dynamic BEAM simulation source with multiple variable geometry settings"
set numsrcopts(21) 0;

set help_text(4) {
Source Number, ISOURC:

Set the source number by selecting the type of source to use\
	in the simulation.  A full description of each source\
	is given in the DOSXYZnrc User's Manual, as well as in the\
	help text for a selected source.

The following source types have been developed for DOSXYZnrc:

Parallel rectangular beam incident from the front (isource = 0)
Parallel rectangular beam incident from any direction (isource = 1)
Phase-space source, particles incident from any direction (isource = 2)
Point source incident from the front (isource = 3)
Beam characterization model, particles incident from any direction (isource = 4)
Isotropically radiating parallelepiped (isource = 6)
Parallel rectangular beam from multiple, user-selected angles (isource = 7)
Phase space source from multiple, user-selected (isource = 8)
Full BEAM treatment head simulation (isource = 9)

}
# (See the help for each source at the end of this file.)

set monoen 1

set names(5) "Number of histories"
set numopts(5) 0
set help_text(5) {
Number of histories, NCASE:

Set the number of histories to use in this space.  The minimum value is 100.  \
	The number of histories per batch is currently NCASE/10.
}

set names(6) "IWATCH Output";
set help_text(6) {
IWATCH Output:

"none" is normal output, the default.
"show interactions" outputs information for every discrete interaction.
"show each step" outputs information for every electron/photon step.
"for EGS_windows" outputs a file for graphics.
}
set numopts(6) 4
set options(6,0) "none"
set options(6,1) "show interactions"
set options(6,2) "show each step"
set options(6,4) "for EGS_windows"

set names(7) "Maximum CPU time (hours)"
set numopts(7) 0
set help_text(7) {
Maximum CPU time, TIMMAX:

This is the maximum CPU time allowed for the simulation.  The default value is\
	0.99 hours if unset or set to 0.  This is currently not activated in\
	DOSXYZnrc, except to print out a warning if this time is expected to be\
	exceeded.
}

set names(8) "RNG seed 1"
set numopts(8) 0
set help_text(8) {
RNG seed 1, INSEED1:

The random number generator used requires 2 seeds between 1 and 31328 and\
	1 and 30081 respectively.  For each different pair of seeds, an\
	independent random number sequence is generated.  Once the seeds are\
	used to establish the state of the generator, the "seeds" output\
	in log files etc. are really just pointers between 1 and 98.

To initiate the correlated sampling option, INSEED1 is negative.
}
set names(9) "RNG seed 2"
set numopts(9) 0
set help_text(9) {
RNG seed 2, INSEED2:

The random number generator used requires 2 seeds between 1 and 31328 and\
	1 and 30081 respectively.  For each different pair of seeds, an\
	independent random number sequence is generated.  Once the seeds are\
	used to establish the state of the generator, the "seeds" output\
	in log files etc. are really just pointers between 1 and 98.
}
set names(10) "Incident beam size (source 2, 4 or 8)"
set numopts(10) 0
set help_text(10) {
Incident beam size, BEAM_SIZE:

This allows the user to control the size of the incident beam for source\
	2 or 4 phase-space input.  It is the side of a square field in cm.  \
	The default value is 100 cm.  When phase-space particles are read\
	from a data file or reconstructed by a multiple-source model, DOSXYZnrc\
	will check their positions and discard those that fall outside of the\
	specified field.

Use with care.
}
set names(11) "Phase space data redistribution"
set numopts(11) 2
set options(11,0) "do not redistribute"
set options(11,1) "redistribute"
set help_text(11) {
Phase space redistribution, ISMOOTH:

When phase-space data is used, DOSXYZnrc will re-use the phase-space particles\
	if the number of histories required by the user is greater than\
	the number of particles stored in the phase-space file.

With this option, DOSXYZnrc can redistribute the phase-space particles when\
	they are used more than once.
}
set names(12) "Run option";
set help_text(12) {
Restart option, IRESTART:

"first time" means that this is the first run for this data\
        set (default).
"restart" means that this is a restart of a previous run.
"create input file and exit" does just that.
"analyze previous" means read in the raw data from a\
        previous run and do the statistical analysis on dose etc.
"combine parallel" combines the .pardose files resulting from\
        parallel runs to create a .3ddose file.  Assumes naming\
        scheme: inputfile_w1.pardose, inputfile_w2.pardose, etc.}
set numopts(12) 5;
set options(12,0) "first time";
set options(12,1) "restart";
set options(12,2) "create input file and exit"
set options(12,3) "analyze previous"
set options(12,4) "combine parallel"
set names(13) "Output restart data"
set numopts(13) 3
set options(13,0) "after every batch"
set options(13,1) "never"
set options(13,2) "at end of run only"
set help_text(13) {
Output restart data, IDAT:

For large phantoms, writing this file will take a lot of time.  For production\
	runs, output with restart data at the end of the run only.
}
set names(14) "Range rejection"
set numopts(14) 2
set options(14,0) "off";
set options(14,1) "on"
set help_text(14) {
Range rejection, IREJECT:

This is a switch for turning on charged particle range rejection.  Range\
	rejection can save simulation time by terminating particle histories\
	immediately if they cannot reach the boundary of the current voxel\
	with energy greater than the global electron cutoff energy (ECUT)\
	and their current energy is less than the global cutoff for range\
	rejection (ESAVE_GLOBAL).  The\
	energy associated with that particle is deposited in the current voxel.

It was found that this option can save 10-17% on computing time for 5\
	cubic mm voxels, but for smaller voxels it saves less time (3-4% for\
	2.5 cubic mm voxels).  For non-CT phantoms where one can arrange to\
	have at least some of the voxels quite large, the savings will be\
	correspondingly larger.
}
set names(15) "ESAVE: range rejection done only below this energy (MeV)"
set numopts(15) 0
set help_text(15) {
Global electron cutoff (for range rejection), ESAVE_GLOBAL:

This is the maximum energy in MeV for which range rejection calculations\
	will be performed (i.e. a particle cannot be rejected if it's energy\
	is larger than this value).  This option was created to prevent\
	termination of high-energy electrons which are likely to generate\
	bremsstrahlung.
}
set names(16) "\# times to recyle each particle in phase space source"
set numopts(16) 0
set help_text(16) {
NRCYCL, the number of times to recycle each particle in a phase space source.

Each particle in the phase space is used a total of NRCYCL+1 times (provided\
the particle has the correct charge and LATCH value) before going on to the\
next particle.  Even with NRCYCL, the total number of histories is still\
determined by the user-input value of NCASE.  NRCYCL is an essential input\
when phase space data is sparse (ie fewer particles in the source than required\
for the simulation).  Choose a value of NRCYCL that will sample the source\
fully but will prevent the source from being restarted (happens automatically\
after the last particle gets used).  This is because restarts may cause\
uncertainties to be underestimated.  If you are unsure of the value to use,\
set NRCYCL<=0 and DOSXYZnrc will automatically calculate a value.  The source\
may restart even with an automatically-calculated value of NRCYCL.  If there\
is only one restart and only a small fraction of the source is re-used on the\
second pass, this is unlikely to affect uncertainties.  However, if a\
significant fraction is re-used on the second pass, or if the source restarts\
more than once, we suggest rerunning with a recalculated value of NRCYCL, given\
by:

NCASE/[NPHSP - (NSMISS/NRCYCL_prev) - NOUTSIDE - NRJCT - NDBSRJCT] -1

where NCASE=no. of histories, NPHSP=total no. of particles in phase space\
source, NSMISS=no. of particles that missed the geometry in the previous\
run, NRCYCL_prev is the value NRCYCL in the previous run, NOUTSIDE=no. of\
particles rejected in previous run because they were outside the user-selected\
incident beam size, NRJCT=no. of particles rejected in previous run because\
they were multiple passers, and NDBSRJCT=no. of fat photons rejected\
(only if directional bremsstrahlung splitting (DBS) was used in the BEAM\
simulation that generated the source AND the user has opted to reject these\
photons--see DOSXYZnrc Manual for more details). NPHSP, NSMISS, NOUTSIDE,\
NRJCT and NDBSRJCT can be found in the .egslst file from the previous\
run.  Always round your calculated value of NRCYCL up to the nearest integer.

Note that NRCYCL will not be calculated automatically if you are using\
only positrons from the phase space source or if you are selecting a subset\
of particles based on LATCH.  In such cases, you may have to make an educated\
"guess" at the value of NRCYCL.  In the worst case, you may have to rerun the\
the simulation with NRCYCL recalculated using the formula above, where NRJCT\
now includes particles rejected from the previous run because they had the\
wrong charge and/or LATCH value.

Note that the total number of histories is still limited by NCASE even if\
you are recycling.
}

set names(18) "Photon splitting number"
set numopts(18) 0
set help_text(18) {
Photon splitting number, n_split:

If n_split > 1, then all photons will be split into n_split sub-photons.\
This option increases the efficiency of dose caluclations more than photon\
forcing.  A rule of thumb for good eff. is n_split >= No/(1-exp(-Lambda))\
where Lambda is approx. number of photon MFP in the geometry of interest and\
No >= 5.  Note that if you use the above, there will be on average approx.\
No primary interactions per incident photon => reduce the number of histories\
by this number.

The n_split algorithm works as follows:
* dpmfp_i = -log(1 - (eta+i)/n_split)
  where dpmfp_i is MFP to the next interaction for the i-th sub-photon, eta\
  is a random number (the same for all n_split sub-photons)
* Once at the interaction site, the i'th sub-photon produces electrons and/or\
  scattered photons. Scattered photons are killed with probability 1/n_split,\
  so that, if they survive, they have the weight of the original photon.\
  Electrons have the weight of 1/n_split of original weight.
* In any radiative events (brems, annih, annih at rest), photons are killed\
  with probability 1/n_split => they have again the weight of the photon that\
  initiated the history, if they survive.
}

set names(20) "File format for phase space data"
set numopts(20) 2
set options(20,0) "no ZLAST"
set options(20,2) "with ZLAST"
set help_text(20) {
File format for phase space data, MODE:

BEAM can create phase space files with 7 or 8 variables, depending\
	on whether ZLAST is stored or not.  DOSXYZnrc does not\
	use ZLAST, but if it is part of the phase space file, \
	DOSXYZnrc has to know that.  No ZLAST (7 variables per record)\
	is the default file format.
}

# bit filter, initialize to nothing
set names(21) "LATCH Bit filter"
set numopts(21) 4
set options(21,0) "Inclusive/exclusive"
set options(21,1) "Exclusive bits"
set options(21,2) "Inclusive for regions"
set options(21,3) "Exclusive for regions"
set nbit1 {}
set nbit2 {}
for {set i 1} {$i<29} {incr i} {
    set latbit($i) {}
}
set help_text(21) {
LATCH bit filter, I_BIT_FILTER:

The LATCH variable, associated with each particle in a BEAM simulation, \
	is a 32-bit variable used to track the particle's history. It is\
	discussed in detail in the BEAM User's Manual.\
        The ability to trace a particle's history using LATCH allows doses\
        to be broken down into their components. DOSXYZnrc is\
        able to score dose components including or excluding contributions\
        arising from particles with certain user-specified LATCH bit\
        settings (this is called ``bit filtering'').\
	Note that only one dose component can be\
        scored per DOSXYZnrc simulation.  Also, the total dose is NOT\
        scored along with the dose component.

Each bit is designated as follows with bit 0 being the lowest value bit:

bit 0: Set to 1 if a photon is created by a bremsstrahlung event or an\
	electron is created by a bremsstrahlung photon; 0 otherwise.

bits 1-23: Used to record the region where a particle has been and/or\
	has interacted. Note that the bit set for a region is\
	determined by IREGION_TO_BIT (which is defined in the BEAM\
	simulation) for that region.

bits 24-28: Stores the region number in which a secondary particle is\
	created; if these bits are all 0, the particle is a primary\
	particle.

bits 29-30: Store the charge of a particle at the time LATCH is output\
	to a phase-space file.

bit 31: Set to 1 if a particle has crossed a scoring plane more than once\
	when LATCH is output to a phase-space file.

The latch bit filter allows dose contributions to be separated\
	according to what regions particles have passed through/interacted\
	in, whether the particle is a primary or secondary, if the\
	particle is a secondary then where it was created, and any\
	combination of these.

To use this option, you must have selected source 2 (phase space) or 9\
        (full BEAM simulation) and have selected the option to score\
        a dose component.\
	There are four types of bit filters available, the first two of which\
	filter based on whether certain bits are set in a particle's\
	LATCH variable:

1) an inclusive/exclusive filter.  If a particle has any of the inclusive bits\
	set and none of the exclusive bits set in its LATCH variable, \
	it is used in the simulation.
2) an exclusive filter.  If a particle has any of these bits set in\
	its LATCH variable, it is not used.

The last two filters apply only to secondary particles, that is, \
	particles that originated somewhere in the accelerator.  \
	Recall that the IREGION_TO_BIT number for the region in\
	which a particle originated is \
	stored in bits 24-28 of its LATCH variable.

3) an inclusive filter for regions.  If a secondary particle was created\
	in a region\
	corresponding to any of the IREGION_TO_BIT values selected, it is\
	used in the simulation.
4) an exclusive filter for regions.  If a secondary particle was created\
	in a region corresponding to any of the IREGION_TO_BIT values selected, \
	it is not used in the simulation.
}

set names(25) "Zero the dose in air"
set numopts(25) 2
set options(25,0) "no"
set options(25,1) "yes"
set help_text(25) {
Zero the dose in air, zeroairdose:

Zero the dose in air (any material with density < 0.051 g/cm^3)\
	in the .3ddose file.
}

set names(26) "Dose output"
set numopts(26) 2
set options(26,0) "supressed"
set options(26,1) "all doses"
set help_text(26) {
Dose output, doseprint:

Output all doses to .egslst file, or suppress.
}

set names(27) "Print summary of highest 20 doses"
set help_text(27) {
Include summary of highest 20 doses, MAX20:

If this option is selected, a summary of the highest 20 doses\
	is included in the output, as well as the uncertainties on the doses\
	and the average of the 20 highest doses.  This is here for diagnostic\
	and research on efficiency purposes.
}
set numopts(27) 2;
set options(27,0) "no"
set options(27,1) "yes"

set Ein {}
set help_text(Ein) {
Incident particle energy, EIN:

Kinetic energy of the incident particles in MeV.
}
set spec_file {}
set medsur {}
set help_text(dflag) {
When using source 2, 4, 8, 9, 10, 20 or 21 (phase space, BEAM simulation, or beam model source) you\
must specify a region surrounding the phantom, known as the dsurround\
region.  This is because the dimensions of the region beyond the phantom\
are given by dsurround(1) (x thickness in both +ve and -ve directions),\
dsurround(2) (y thickness in both +ve and -ve directions),\
dsurround(3) (bottom z thickness) and dsurround(4) (top z thickness).

You can specify the region with a uniform thickness in which case\
dsurround(1),...,dsurround(4) will be the same and equal to the value that you\
input, or you can input the 4 values of dsurround individually by selecting\
the "non-uniform" option.

You must also specify the medium to be used in the dsurround region.\
The medium you select here can either be VACUUM or else one of the media\
included in the phantom definition (if you are using a CT phantom\
then the medium must be one of those at the top of the .egsphant\
file).

Uniform dsurround with AIR as the surrounding medium is useful for\
CT phantom calculations, where you want to simulate realistic transport\
of particles through air before striking the patient.  Non-uniform dsurround\
with the surrounding medium the same as the phantom medium (eg H2O) is\
useful for speeding up depth-dose or dose profile calculations because, as\
shown in the figure, it allows you to specify voxels only in column/plane\
that you are interested in, and then range rejection becomes very effective\
outside this column/plane.

You also have the option of scoring 3D phase space data (in IAEA format) on particle\
exit from the phantom (into the surrounding medium).  Phase space data can be\
scored in the DOSXYZnrc coordinate system or back-transformed into the BEAMnrc\
coordinate system.  If using source 20 or 21 (synchronized sources), you have the\
option of scoring the fractional monitor unit (MU) index associated with the particle\
as well.  This latter is a separate input and is part of those for sources 20 and 21.
}

set dflagopt(0) "uniform"
set dflagopt(1) "non-uniform"
set dflag $dflagopt(0)

set help_text(iphspout) {
For outputting IAEA format phase space data on exit\
from the phantom geometry (i_phsp_out). If i_phsp_out=1, then data\
is output in DOSXYZnrc coordinates. If i_phsp_out=2\
data is output in BEAMnrc coordinates. This option is\
only available for phase space or BEAMnrc simulation\
sources (which have a region surrounding the phantom).\
The default is i_phsp_out=0: no phase space\
output. If you are using source 20 or source 21\
(synchronized sources) then you also have the option of\
storing the MU index (frMU_indx) in the phase space file (see source inputs).
}

set phspoutopt(0) "none"
set phspoutopt(1) "in DOSXYZnrc coordinates"
set phspoutopt(2) "in BEAMnrc coordinates"
set iphspout 0

set PhantFileName {}
set help_text(PhantFileName) {
Phantom filename, PhantFileName:

The full name of the file containing the CT phantom as output\
	by ctcreate (should be a .egsphant file).
}

set ihowfarless {}
set names(ihowfarless) "'HOWFARLESS'"
set numopts(ihowfarless) 2
set options(ihowfarless,0) "off"
set options(ihowfarless,1) "on"
set values(ihowfarless) $options(ihowfarless,0)
set help_text(ihowfarless) {
'HOWFARLESS' option (ihowfarless):

For use only in a homogeneous phantom.

If turned on, then subroutines HOWFAR and HOWNEAR ignore voxel boundaries,\
only considering the extreme outer boundaries of the phantom when calculating\
distance along particle trajectory or perpendicular distance to the nearest\
boundary.  This allows charged particles to take longer steps, limited only\
by Smax (the maximum allowed step length) and estepe (the max. allowed energy\
loss/step), and, thus, speeds up simulation time.  Keep Smax set to the default\
value of 5 cm to take full advantage of the improvement in efficiency.

Efficiency improvement using the `HOWFARLESS' option\
depends on the incident beam and the boundary crossing algorithm (BCA) used,\
but will typically be ~30% in photon beams from accelerators simulated using\
BEAMnrc (phase space or BEAMnrc simulation source) and up to a factor of\
~4 in monoenergetic electron beams.  If the more accurate EXACT BCA is used\
then the efficiency improvement increases to a factor of 2.5-3.5 in photon\
beams from BEAMnrc-simulated accelerators and a factor of up to ~15 in\
monoenergetic electron beams.

Recommended for all homogeneous phantom calculations.
}

set ibindos {}
set names(ibindos) "Dose output"
set numopts(ibindos) 2
set options(ibindos,0) "3ddose"
set options(ibindos,1) "bindos (sparse binary format)"
set values(ibindos) $options(ibindos,0)
set help_text(ibindos) {
Set to "3ddose output" to output a voxel doses in a dense ASCII file.
Set to "sparse binary output" for a sparse binary dose file.
See the DOSXYZnrc manual section titled "Format of Dose Outputs" for a\
description of the file formats.
}
#################The following are the EGSnrc parameters#############

set ecut {}
set names(ecut) "Global electron cutoff energy - ECUT (MeV)"
set numopts(ecut) 0
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
set help_text(smaxir) {
Maximum step size, SMAX:

Global (in all regions) maximum step-size\
        restriction for electron transport (in cm).\
        No SMAX restriction is necessary if the electron\
        step algorithm is PRESTA-II and the EXACT boundary\
        crossing algorithm is used.  In this case, SMAX\
        will default to 1e10.  However, if either\
        Electron-step algorithm= PRESTA-I\
             or\
        Boundary crossing algorithm= PRESTA-I (the default),\
        then a step-size restriction is necessary, and\
        SMAX will default to 5 cm.
}

set estepe {}
set names(estepe) "Max. fractional energy loss/step"
set numopts(estepe) 0
set help_text(estepe) {
Maximum fractional energy loss/step, ESTEPE:

Note that this is a global option only, no\
        region-by-region setting is possible. If missing,\
        the defualt is 0.25 (25%).
}

set ximax {}
set names(ximax) "XImax"
set numopts(ximax) 0
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
        determined by `Skin depth for BCA' (see below).

        Default is PRESTA-I for efficiency reasons. This\
        is known not to be exactly correct, but does not\
        adversely affect most calculations.  However, when charged\
        particle equilibrium does not hold (e.g. small beam field\
        incident on voxels of roughly the same dimension as the field)\
        or when the phantom is not divided into uniform voxels throughout\
        (e.g. a column of small voxels down the central axis, large voxels\
        elsewhere) then use of the PRESTA-I BCA can result in dose\
        overprediction by up to 2.5\%.  In such cases, you should\
        switch to the EXACT BCA for accurate dose calculations.
}
set numopts(bca_algorithm) 2
set options(bca_algorithm,0) "EXACT"
set options(bca_algorithm,1) "PRESTA-I"

set skindepth_for_bca {}
set numopts(skindepth_for_bca) 0
set names(skindepth_for_bca) "Skin depth for BCA"
set help_text(skindepth_for_bca) {
Skin depth for BCA (skindepth_for_bca):

If Boundary crossing algorithm= PRESTA-I (default)\
        then this is the distance from the boundary (in\
        elastic MFP) at which lateral correlations will be\
        switched off.  The default in this case is to\
        calculate a value based on the scattering power at\
        ECUT (same as PRESTA in EGS4).  If\
        Boundary crossing algorithm= EXACT then this is\
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
between accuracy (EXACT BCA) and efficiency (PRESTA-I BCA)\
since the PRESTA-I BCA algorithm has proven to generally\
produce satisfactory results under conditions of charged particle
equilibrium.  Note that the new transport\
mechanics of EGSnrc are maintained away from boundaries and\
that one always has the option of verifying the accuracy\
by doing a long run with the EXACT BCA.
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
set values(eii_flag) $options(eii_flag,0)
#other options will be added after looking in the users $HEN_HOUSE/data
#directory
#set options(eii_flag,2) "Casnati"
#set options(eii_flag,3) "Kolbenstvedt"
#set options(eii_flag,4) "Gryzinski"


set photon_xsections {}
set names(photon_xsections) "Photon cross-sections"
set help_text(photon_xsections) {
Photon cross-sections (photon_xsections):

si, epdl, xcom (default), mcdf-xcom, mcdf-epdl, PEGS4, etc.  \
Determines which photon cross-sections are used.

By default EGSnrc uses the NIST XCOM cross-sections (xcom_*.data files)\
. Other photon cross section compilations provided with EGSnrc are the \
EPDL (epdl_*.data files) and Strom & Israel (si_*.data files) photon cross section\
compilations. For more details about these options, see \
the EGSnrc Manual (PIRS-701).

Selecting either mcdf-xcom or mcdf-epdl allows the use of renormalized\
photoelectric cross sections by Sabbatucci and Salvat with either XCOM\
or EDPL cross sections for the rest of the photon interactions.

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
photoelectric events, rayleigh scattering and triplet production,\
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
but significant in the keV energy range. If NRC is selected,\
NIST data including corrections for electron-electron\
brems will be used (typically only\
significant for low values of the atomic number Z\
and for k/T < 0.005).
}
set numopts(ibr_nist) 3
set options(ibr_nist,0) "BH"
set options(ibr_nist,1) "NIST"
set options(ibr_nist,2) "NRC"

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
Default is Norej\
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

set iraylr {}
set names(iraylr) "Rayleigh scattering"
set help_text(iraylr) {
Rayleigh scattering (IRAYLR):

Off, On, On in regions, Off in regions, custom\
If On, turn on coherent (Rayleigh) scattering.\
Default is On. Should be turned on for low energy\
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
set values(iraylr) $options(iraylr,1)

set iedgfl {}
set names(iedgfl) "Atomic relaxations"
set help_text(iedgfl) {
Atomic relaxations (IEDGFL):

Off, On, On in regions, Off in regions, EADL, simple.

The default is EADL (On). The effect of using On is twofold:

- In photo-electric absorption, incoherent scattering with\
  bound electrons, and electron impact ionization events, \
  the element (if material is mixture) and the shell the particle\
  is interacting with are sampled from the appropriate\
  cross sections
- Shell vacancies created in these events\
  are relaxed via emission of fluorescent X-Rays,\
  Auger and Koster-Cronig electrons.\
  Make sure to turn this option on for low energy\
  applications.

Selecting On or EADl is equivalent. A full atomic relaxation cascade\
is simulated using EADL transition probabilities.

The option to use the original algorithm only accounting for M and\
N shells in an average way is left for comparison and can be invoked\
by selecting the option simple.

If you use "On in regions", you will be prompted to enter\
pairs of region numbers.  Each pair defines a range of regions\
in which Atomic relaxations will be turned On.  Everywhere\
outside these regions Atomic relaxations will be turned Off.\

If you use "Off in regions", you will be prompted to enter\
ranges of regions where Atomic relaxation is to be turned Off.\
Everywhere else it will be turned on.
}
set numopts(iedgfl) 6
set options(iedgfl,0) "Off"
set options(iedgfl,1) "On"
set options(iedgfl,2) "On in regions"
set options(iedgfl,3) "Off in regions"
set options(iedgfl,4) "EADL"
set options(iedgfl,5) "simple"
set values(iedgfl) $options(iedgfl,4)

#################end of options for EGSnrc parameters####################

set imax(0) {}
set imax(1) {}
set imax(2) {}
set help_text(imax) {
Number of X,Y,Z voxels, IMAX, JMAX, ZMAX:

The number of voxels to split the phantom into in the X,Y,Z directions.
}

# HELP TEXT FOR SOURCES:

set source_text(0) {
Source 0: Parallel Rectangular Beam Incident from the Front

The uniform parallel rectangular beam is always assumed to be incident\
	parallel to the Z-axis from the front of the\
	phantom.

The figure shows the beam field, defined by xinu, xinl, yinu and\
	yinl (the lower and upper x- and y-bounds on the phantom surface),\
	and the angles thetax, thetay and thetaz.  The angles thetax and thetay\
	are relative to the positive x- and y-axes, respectively, while thetaz\
	is measured relative to the negative z-axis.  All angles are in \
        degrees.  Note that the sum of the squares of the cosines of the\
        incident angles (u^2+v^2+w^2) must be 1.
}

set source_text(1) {
Source 1: Parallel Rectangular Beam Incident from Any Direction

This uniform parallel rectangular beam may be incident from any\
	direction.

The figure shows the isocenter at (xiso,yiso,ziso), which is normally\
	inside the phantom.

The position of the beam collimator is defined using the angles theta and phi:  \

theta is the angle between the +z direction\
	and a line joining the center of the beam where it strikes\
	the phantom surface to the isocenter.  In a polar coordinate\
	system, this angle is known as the polar angle and normally\
	has a range 0-180 degrees.  Note that a centered beam incident\
	along the +z-axis has theta=180 degrees.  theta is not to be confused\
	with thetaz for isource=0, for which case thetaz=0 degrees to aim\
	the source=0 beam along the +z-axis.

phi is the angle between the +x direction and the projection on\
	the xy plane of the line joining the center of the beam on the\
	phantom surface to the isocenter on the xy plane. In a polar\
	coordinate system, this angle is known as the azimuthal\
	angle and normally has a range 0-360 degrees.

xcol and ycol define the dimensions of the rectangular collimator, or\
	the total x- and y-widths of the beam on the\
	plane perpendicular to the beam direction, defined by the center of the\
	beam and the isocenter.  phicol is the angle by which the\
	collimator is rotated in the collimator plane perpendicular\
	to the beam direction. phicol is determined for theta=phi=0.  \
	The positive sense of rotation is counterclockwise as one sights\
	down the beam direction. Note that the effect of setting phicol=90\
	degrees is the same as switching the values of xcol and ycol\
	when theta and phi are 0 or are both multiples of 90 degrees.

In the figure, theta is 90 degrees and phi is 180 degrees.  \
	Note that source 1 is always incident right on the phantom surface\
	(particles do not travel through air to get to the phantom).

}

set source_text(2) {
Source 2: Phase-Space Source Incident from Any Direction

This source uses a phase-space file generated during a BEAM simulation\
	at any flat scoring plane of a linear accelerator\
	geometry.  A user can choose any particular type of particles\
	from the phase-space file and score dose components using\
	the LATCH filter.  The field size of the incident beam can be\
	reduced using the 'Incident beam size' parameter.  There is also a\
	parameter called 'Phase space redistribution' which can be set to shift\
	the particles to their symmetrical positions with\
	respect to x- and y-axis in the phase-space file for repeated\
	use of these phase-space particles.  Without this option, no shift\
	will be made when phase-space particles are re-used.

The figure shows an example use of this source.  The source plane does not\
	necessarily have to be right against the surface of the phantom, \
	however it must be within the region surrounding the phantom.

The isocenter is shown as (xiso,yiso,ziso), normally located within the phantom.

The position of the beam collimator is defined using the angles theta and phi:  \

theta is the angle between the +z direction\
	and a line joining the center of the beam where it strikes\
	the phantom surface to the isocenter.  In a polar coordinate\
	system, this angle is known as the polar angle and normally\
	has a range 0-180 degrees.  Note that a centered beam incident\
	along the +z-axis has theta=180 degrees.  theta is not to be confused\
	with thetaz for isource=0, for which case thetaz=0 degrees to aim\
	the source=0 beam along the +z-axis.

phi is the angle between the +x direction and the projection on\
	the xy plane of the line joining the center of the beam on the\
	phantom surface to the isocenter on the xy plane. In a polar\
	coordinate system, this angle is known as the azimuthal\
	angle and normally has a range 0-360 degrees.

dsource is the absolute distance (in cm) from the isocenter to the source\
	center, which is, by definition, the origin of the phase-space plane\
	(the origin may not even be in the beam).

phicol is the angle by which the source is rotated in the source plane\
	perpendicular to the beam direction.  phicol is determined\
	for theta=0 or 180 degrees and phi=0 degrees.  The positive sense\
        of rotation is counterclockwise as one sights down the origin in the\
	phase-space plane.   phicol=180 degrees to maintain x,y values output\
	by BEAM.

theta, phi and dsource can be set to place the source anywhere\
	inside the phantom or the surrounding region; the\
	medium and the thickness of the surrounding region is input\
	by the user. Particles from the phase space which are initially\
	outside the phantom and surrounding region are terminated\
	immediately.  If the medium of the surrounding region is air, \
	for example, the phase-space particles will be transported\
	properly through air to the surface of the phantom. A particle\
	history is terminated if it is determined that the particle will\
	not make it to the phantom surface (the particle loses all its\
	energy in the surrounding region or it escapes through the outer\
	boundaries of the surrounding region).

Note that this source requires the user to input of the mode of the phase-space\
	file (7 or 8 variables per record), and to\
	input the medium and the thickness for the surrounding region.  \
	For more information related to phase-space sources, see the\
	section on full phase-space files in the BEAM User's Manual.

}

set source_text(3) {
Source 3: Point Source Rectangular Beam Incident from Front

The isotropically-radiating point source is placed on the Z-axis.  It is\
	assumed to be incident on the front surface of the\
	phantom, but can be placed any distance above the phantom.  The\
	beam field can be asymmetric.

The figure shows the parameters defining this source type.  xinl, xinu, yinl\
	and yinu denote the lower and upper x- and y-bounds of the field\
	on the phantom surface, and ssd is the distance from the point\
	source to the phantom surface in cm.

Note that between the source and the phantom surface, the medium is\
	assumed to be a vacuum for this source.
}

set source_text(4) {
Source 4: Beam Characterization Model Incident from Any Direction

This source uses the beam characterization models based on the\
	phase-space data generated by BEAM. This source\
	consists of a variety of sub-sources for different types\
	of particles coming from different components of a linear\
	accelerator. Each sub-source has its own spectral and (planar)\
	fluence distributions and the correlation between the\
	energy, position and incident angle is retained by sampling\
	the particle positions on the sub-source (the surface of the\
	component) and on the phase-place plane. A beam model input file, \
	generated by BEAMDP, is required by this source.

For more details about beam characterization models see the BEAMDP User's Manual and associated paper.

The isocenter is shown as (xiso,yiso,ziso), normally located within the phantom.

The position of the beam collimator is defined using the angles theta and phi:  \

theta is the angle between the +z direction\
	and a line joining the center of the beam where it strikes\
	the phantom surface to the isocenter.  In a polar coordinate\
	system, this angle is known as the polar angle and normally\
	has a range 0-180 degrees.  Note that a centered beam incident\
	along the +z-axis has theta=180 degrees.  theta is not to be confused\
	with thetaz for isource=0, for which case thetaz=0 degrees to aim\
	the source=0 beam along the +z-axis.

phi is the angle between the +x direction and the projection on\
	the xy plane of the line joining the center of the beam on the\
	phantom surface to the isocenter on the xy plane. In a polar\
	coordinate system, this angle is known as the azimuthal\
	angle and normally has a range 0-360 degrees.

dsource is the absolute distance (in cm) from the isocenter to the source\
	center, which is, by definition, the origin of the phase-space plane\
	(the origin may not even be in the beam).

phicol is the angle (in degrees) by which the source is rotated in the\
        source plane perpendicular to the beam direction.  phicol is determined\
	for theta=phi=0 degrees.  The positive sense of rotation is\
	counterclockwise as one sights down the origin in the\
	phase-space plane.

theta, phi and dsource can be set to place the source anywhere\
	inside the phantom or the surrounding region; the\
	medium and the thickness of the surrounding region is input\
	by the user.  Particles from the phase space which are initially\
	outside the phantom and surrounding region are terminated\
	immediately.  If the medium of the surrounding region is air, \
	for example, the phase-space particles will be transported\
	properly through air to the surface of the phantom.  A particle\
	history is terminated if it is determined that the particle will\
	not make it to the phantom surface (the particle loses all its\
	energy in the surrounding region or it escapes through the outer\
	boundaries of the surrounding region).

Note this source requires the user input the beam model input file name.  \
The field size of the incident beam can be reduced using the 'Incident beam\
size' parameter.
}

set source_text(6) {
Source 6: Uniform Isotropically Radiating Parallelpiped within DOSXYZnrc volume

This source allows the user to simulate a uniform isotropically radiating\
	rectangular volume (parallelpiped) within the DOSXYZnrc phantom.  \
	The active volume is restricted to being completely contained\
	within the DOSXYZnrc phantom.  However, it can be shrunk to a point\
	anywhere within the phantom by setting xinu=xinl, yinu=yinl, and\
	zinu=zinl.

The input parameters required for this source are those defining the position of\
	the rectangular volume (xinl, xinu, yinl, yinu, zinl, zinu).  The\
	figure to the left shows this.
}

set source_text(7) {
Source 7: Parallel rectangular beam incident from multiple angles

This is a parallel rectangular beam (similar to source 1) incident\
        from multiple, user-defined angles (theta-phi pairs).

The figure shows the source incident at one theta-phi pair.  The beam\
        is always incident right on the surface of the phantom.

xiso,yiso and ziso define the coordinates of the isocentre of the source\
        (usually somewhere within the phantom).

xcol and ycol define the dimensions of the rectangular collimator, or\
        the total x- and y-widths of the beam on the\
        plane perpendicular to the beam direction, defined by the center of the\        beam and the isocenter.  phicol is the angle by which the\
        collimator is rotated in the collimator plane perpendicular\
        to the beam direction. phicol is determined for theta(i)=phi(i)=0.  \
        The positive sense of rotation is counterclockwise as one sights\
        down the beam direction. Note that the effect of setting phicol=90\
        degrees is the same as switching the values of xcol and ycol\
        when theta(i) and phi(i) are 0 or are both multiples of 90 degrees.

theta(i)-phi(i) pairs are used to define the incident angles of the beam.\
        theta(i) is defined with respect to the +z direction\
        and a line joining the center of the beam to the isocenter.\
        In a polar coordinate system, this angle is known as the polar angle\
        and normally has a range 0-180 degrees.  Note that a centered beam\
        incident along the +z-axis has theta(i)=180 degrees.  phi(i) is defined\
        with respect to the +x direction and the projection on\
        the xy plane of the line joining the center of the beam \
        to the isocenter. In a polar coordinate system, this angle is known\
        as the azimuthal angle and normally has a range 0-360 degrees.

theta(i)-phi(i) pairs can be defined either on a pair-by-pair basis or in\
        groups.  On a pair-by-pair basis, theta(i), phi(i), and the probability\
        of a particle being incident from that angle (pang(i)) are input for\
        each pair i.  When defining by groups, the user can choose to vary\
        either phi(i) (ivary(k)=0) or theta(i) (ivary(k)=1) within group k.\
        If the user chooses to vary phi(i), then theta(i) is fixed at\
        a user selected value (angfixed(k)) within that group.  Similarly,\
        phi(i) is fixed at a user-selected angfixed(k) within a group that\
        has varying theta(i).  The user then inputs the minimum and maximum\
        values of the varying phi(i) or theta(i) (angmin(k) and angmax(k)),\
        the number of equally-spaced phi(i) or theta(i) between and including\
        angmin(k) and angmax(k) (ngang(k)) (Note that since ngang(k) includes\
        angmin(k) and angmax(k) it must be >= 2), and the probability of\
        a particle being incident within that group (pgang(k)).  Within group\
        k, all of the equally-spaced phi(i) or theta(i) have equal probability.\
        For both pair-by-pair and group definition of theta(i)-phi(i),\
        the probabilities input by the user are automatically normalized.
}

set source_text(8) {
Source 8: Phase space source incident from multiple angles

This is a full phase space source (similar to source 2) incident from multiple\
        user-defined angles (theta-phi pairs).

The figure shows the phase space source incident at one theta-phi pair.\
        The source plane does not necessarily have to be right against the\
        surface of the phantom, however it must be within the region\
        surrounding the phantom.

The isocenter is shown as (xiso,yiso,ziso), normally located within the phantom.

dsource is the absolute distance (in cm) from the isocenter to the source\
        center, which is, by definition, the origin of the phase-space plane\
        (the origin may not even be in the beam).

phicol is the angle by which the source is rotated in the source plane\
        perpendicular to the beam direction.  phicol is determined\
        for theta=0 or 180 degrees and phi=0 degrees.  The positive sense\
        of rotation is counterclockwise as one sights down the origin in the\
        phase-space plane.   phicol=180 degrees to maintain x,y values output\
        by BEAM.

theta(i)-phi(i) pairs are used to define the incident angles of the beam.\
        theta(i) is defined with respect to the +z direction\
        and a line joining the center of the source plane to the isocenter.\
        In a polar coordinate system, this angle is known as the polar angle\
        and normally has a range 0-180 degrees.  Note that a centered beam\
        incident along the +z-axis has theta(i)=180 degrees.  phi(i) is defined\
        with respect to the +x direction and the projection on\
        the xy plane of the line joining the center of the source plane\
        to the isocenter. In a polar coordinate system, this angle is known\
        as the azimuthal angle and normally has a range 0-360 degrees.

theta(i)-phi(i) pairs can be defined either on a pair-by-pair basis or in\
        groups.  On a pair-by-pair basis, theta(i), phi(i), and the probability\
        of a particle being incident from that angle (pang(i)) are input for\
        each pair i.  When defining by groups, the user can choose to vary\
        either phi(i) (ivary(k)=0) or theta(i) (ivary(k)=1) within group k.\
        If the user chooses to vary phi(i), then theta(i) is fixed at\
        a user selected value (angfixed(k)) within that group.  Similarly,\
        phi(i) is fixed at a user-selected angfixed(k) within a group that\
        has varying theta(i).  The user then inputs the minimum and maximum\
        values of the varying phi(i) or theta(i) (angmin(k) and angmax(k)),\
        the number of equally-spaced phi(i) or theta(i) between and including\
        angmin(k) and angmax(k) (ngang(k)) (Note that since ngang(k) includes\
        angmin(k) and angmax(k) it must be >= 2), and the probability of\
        a particle being incident within that group (pgang(k)).  Within group\
        k, all of the equally-spaced phi(i) or theta(i) have equal probability.\        For both pair-by-pair and group definition of theta(i)-phi(i),\
        the probabilities input by the user are automatically normalized.

Note that this source requires the user to input of the mode of the phase-space\        file (7 or 8 variables per record), and to\
        input the medium and the thickness for the surrounding region.  \
        For more information related to phase-space sources, see the\
        section on full phase-space files in the BEAM User's Manual.
}

set source_text(9) {
Source 9: Full BEAM treatment head simulation (incident from any direction):

This source uses a full BEAM simulation as a source.  The user must have\
        compiled the simulation as a library archive (ie the file\
        libBEAM_accelname.so must exist in $EGS_HOME/bin/$my_machine).\
        This is done by typing 'make library' in the $EGS_HOME/BEAM_accelname\
        directory.  The BEAM simulations runs simultaneously to the DOSXYZ\
        simulation and source particles for DOSXYZ are extracted at the plane\
        where they would have been scored in a standard BEAM simulation.  Thus,\
        this source is equivalent to source 2 (full phase space source)\
        without the need to store a phase space file.

The user can choose any type of particle\
        from the BEAM simulation and can score dose components based on\
        the LATCH values of particles.  The field size considered can be\
        adjusted using the 'Incident beam size' parameter.

The figure shows an example use of this source.  The source plane does not\
        necessarily have to be right against the surface of the phantom, \
        however it must be within the region surrounding the phantom.

The isocenter is shown as (xiso,yiso,ziso), normally located within the phantom.

The position of the beam collimator is defined using the angles theta and phi:
\

theta is the angle between the +z direction and a vector from\
        the isocenter to the source plane.\
        In a polar coordinate\
        system, this angle is known as the polar angle and normally\
        has a range 0-180 degrees.  Note that a centered beam incident\
        along the DOSXYZ +z-axis has theta=180 degrees.  theta is not\
        to be confused\
        with thetaz for isource=0, for which case thetaz=0 degrees to aim\
        the source=0 beam along the +z-axis.

phi is the angle between the +x direction and the projection of the\
        vector from the isocenter to the source plane on the xy plane.\
        In a polar coordinate system, this angle is known as the azimuthal\
        angle and normally has a range 0-360 degrees.

dsource is the absolute distance (in cm) from the isocenter to the centre\
        of the source plane (ie the length of the vector).

phicol is the angle by which the source is rotated about the origin of the\
        source plane (ie the beam central axis).  phicol is determined\
        for theta=0 or 180 degrees and phi=0 degrees.  The positive sense\
        of rotation is counterclockwise when theta=0 degrees.\
        phicol=180 degrees when theta=180 degrees to\
        preserve x,y values output by BEAM.

theta, phi and dsource can be set to place the source anywhere\
        inside the phantom or the surrounding region; the\
        medium and the thickness of the surrounding region is input\
        by the user. Particles from the BEAM simulation which are initially\
        outside the phantom and surrounding region are terminated\
        immediately.  If the medium of the surrounding region is air, \
        for example, the particles will be transported\
        properly through air to the surface of the phantom. A particle\
        history is terminated if it is determined that the particle will\
        not make it to the phantom surface (the particle loses all its\
        energy in the surrounding region or it escapes through the outer\
        boundaries of the surrounding region).

Note that this source requires the user to\
        input the medium and the thickness for the surrounding region.

Also note that this source is unavailable if you do not have a C or C++\
        compiler on your machine.
}

set source_text(10) {
Source 10: Full BEAM simulation source incident from multiple directions:

This is a BEAM simulation source (similar to source 9) incident from multiple\
        user-defined angles (theta-phi pairs).

The figure shows the source incident at one theta-phi pair.\
        The source plane (corresponding to the scoring plane in\
        the BEAM simulation) does not necessarily have to be right against the\
        surface of the phantom, however it must be within the region\
        surrounding the phantom.

The isocenter is shown as (xiso,yiso,ziso), normally located within the phantom.

dsource is the absolute distance (in cm) from the isocenter to the source\
        center, which is, by definition, the origin of the source plane\
        (the origin may not even be in the beam in some cases).

phicol is the angle by which the source is rotated in the source plane\
        perpendicular to the beam direction.  phicol is determined\
        for theta=0 or 180 degrees and phi=0 degrees.  The positive sense\
        of rotation is counterclockwise as one sights down the origin in the\
        source plane.   phicol=180 degrees to maintain x,y values output\
        by BEAM.

theta(i)-phi(i) pairs are used to define the incident angles of the beam.\
        theta(i) is defined with respect to the +z direction\
        and a line joining the center of the source plane to the isocenter.\
        In a polar coordinate system, this angle is known as the polar angle\
        and normally has a range 0-180 degrees.  Note that a centered beam\
        incident along the +z-axis has theta(i)=180 degrees.  phi(i) is defined\
        with respect to the +x direction and the projection on\
        the xy plane of the line joining the center of the source plane\
        to the isocenter. In a polar coordinate system, this angle is known\
        as the azimuthal angle and normally has a range 0-360 degrees.

theta(i)-phi(i) pairs can be defined either on a pair-by-pair basis or in\
        groups.  On a pair-by-pair basis, theta(i), phi(i), and the probability\
        of a particle being incident from that angle (pang(i)) are input for\
        each pair i.  When defining by groups, the user can choose to vary\
        either phi(i) (ivary(k)=0) or theta(i) (ivary(k)=1) within group k.\
        If the user chooses to vary phi(i), then theta(i) is fixed at\
        a user selected value (angfixed(k)) within that group.  Similarly,\
        phi(i) is fixed at a user-selected angfixed(k) within a group that\
        has varying theta(i).  The user then inputs the minimum and maximum\
        values of the varying phi(i) or theta(i) (angmin(k) and angmax(k)),\
        the number of equally-spaced phi(i) or theta(i) between and including\
        angmin(k) and angmax(k) (ngang(k)) (Note that since ngang(k) includes\
        angmin(k) and angmax(k) it must be >= 2), and the probability of\
        a particle being incident within that group (pgang(k)).  Within group\
        k, all of the equally-spaced phi(i) or theta(i) have equal probability.\
        For both pair-by-pair and group definition of theta(i)-phi(i),\
        the probabilities input by the user are automatically normalized.

Other inputs are the same as for source 9: You must input the name of\
        a BEAMnrc accelerator simulation (i.e. BEAM_accelname),\
        the name of a BEAMnrc input file (which must exist in your\
        $EGS_HOME/BEAM_accelname directory), and a PEGS4 data file for\
        the BEAMnrc simulation.  The accelerator must first have been compiled\
        as a shared library by typing "make library" in the\
        $EGS_HOME/BEAM_accelname directory.  The input file must define a\
        single scoring plane where source particles will be sampled from\
        (instead of scoring them in a phase space file).

Also note that this source is unavailable if you do not have a C or C++\
        compiler on your machine.
}

set source_text(20) {
Source 20: Synchronized phase space source:

This is a phase space source which can simulate continuous motion of\
the source plane over a range/multiple ranges of\
incident source orientations.  Source 20 also has an option to run the\
source through a geometry (usually an MLC) defined using either a BEAM accelerator code\
compiled as a shared library or a non-EGSnrc code compiled as a shared library.

The parameters defining the orientation of the phase space source plane relative to\
the phantom geometry have the same definition as those for\
sources 2, 8, 9, 10.  The user defines NSET control points with the following inputs\
for each control point i:
x|y|ziso(i): coordinates of the isocentre for point i
dsource(i): perpendicular distance from centre of source plane to isocentre for point i
theta(i): angle of dsource vector relative to +Z axis for point i
phi(i): angle of dsource vector relative to +X axis for point i
phicol(i): angle of rotation of source in its own plane for point i
muIndex(i): fraction of incident particles delivered up to and including point i

Note that muIndex(i) is interpreted as a fractional monitor unit index.  Restrictions\
on muIndex are:
muIndex(i-1)<=muIndex(i)
muIndex(1)=0.0
muIndex(NSET)=1.0

Prior to each incident particle, a random number, MU_RND, on [0,1] is chosen.  The\
value of i for which muIndex(i-1)<=MU_RND<muIndex(i) is then found, and the orientation\
of the source between point i-1 and i is calculated using:

param = param(i-1) + (MU_RND-muIndex(i-1))*(param(i)-param(i-1))/(muIndex(i)-muIndex(i-1))

where param is any one of the geometrical parameters defining the source orientation\
(x|y|ziso, dsource, theta, etc).  Thus, continuous source motion between the control\
points is simulated.

There is also an option to run the phase space source through a geometry\
defined using a BEAM accelerator or non-EGSnrc code.  This allows simulation\
of an MLC or other blocking geometry between the source and the phantom.  The code\
defining the geometry must be compiled as a shared library.  In the case of a\
BEAM accelerator, the BEAM input file must specify a phase space scoring plane at\
the bottom of the accelerator. The pegs data used for a BEAM accelerator is the\
same as that used for the DOSXYZnrc simulation.  The value of MU_RND used\
by DOSXYZnrc to choose the source orientation is passed to the code defining the geometry.\
This allows synchronization between the orientation of the source and the geometry\
being simulated.  For a BEAM code, this requires that synchronized component modules (CMs)\
such as SYNCJAWS, SYNCVMLC, SYNCMLCE and/or SYNCHDMLC be used in the accelerator.\
The motion of these CMs as a function of fractional monitor unit index is controlled\
using separate files read in by each synchronized CM and provided by the user.  See\
the BEAMnrc Users manual for more details.

The figure at left illustrates continuous motion of source 20 between two control points\
(1 and 2) where the points share a common isocentre but where all other geometrical parameters\
are varying.  The position of a shared library geometry (in this case an MLC) between the source plane\
and the phantom and potentially synchronized with the source is also shown.
}

set source_text(21) {
Source 21: Synchronized BEAM simulation source:

This is the equivalent of source 20, but using a full BEAM treatment head\
simulation as a source.  Source 21 can simulate continuous motion of\
the source over a range/multiple ranges of\
incident source orientations.  Source 21 also has an option to run the\
source through a geometry (usually an MLC) defined using a non-EGSnrc code\
compiled as a shared library.  Moreover, with source 21 it is possible to\
synchronized the motion of the source plane with the geometries (opening coordinates)\
of any synchronized component modules (CMs) in the BEAM accelerator.

The parameters defining the orientation of the phase space source plane relative to\
the phantom geometry have the same definition as those for\
sources 2, 8, 9, 10, 20.  The user defines NSET control points with the following input\
for each control point i:
x|y|ziso(i): coordinates of the isocentre for point i
dsource(i): perpendicular distance from centre of source plane to isocentre for point i
theta(i): angle of dsource vector relative to +Z axis for point i
phi(i): angle of dsource vector relative to +X axis for point i
phicol(i): angle of rotation of source in its own plane for point i
muIndex(i): fraction of incident particles delivered up to and including point i

Note that muIndex(i) is interpreted as a fractional monitor unit index.  Restrictions\
on muIndex are:
muIndex(i-1)<=muIndex(i)
muIndex(1)=0.0
muIndex(NSET)=1.0

In addition you must specify the name, input file and pegs data for the BEAM\
source.  The accelerator must have been compiled as a shared library, and the\
input file must specify a phase space scoring plane at the bottom of the\
accelerator.

Prior to each incident primary history, a random number, MU_RND, on [0,1] is chosen.  The\
value of i for which muIndex(i-1)<=MU_RND<muIndex(i) is then found, and the orientation\
of the source between point i-1 and i is calculated using:

param = param(i-1) + (MU_RND-muIndex(i-1))*(param(i)-param(i-1))/(muIndex(i)-muIndex(i-1))

where param is any one of the geometrical parameters defining the source orientation\
(x|y|ziso, dsource, theta, etc).  Thus, continuous source motion between the control\
points is simulated.

You also have the option of running the phase space source through a geometry\
defined using a non-EGSnrc code, such as particleDMLC.  This allows the simulation\
of an MLC or other blocking geometry between the source and the phantom.  The code\
defining the geometry must be compiled as a shared library and exist as a library\
in your $EGS_HOME/bin/config directory.

If there are any synchronized component modules (CMs),\
such as SYNCJAWS, SYNCVMLC, SYNCMLCE and/or SYNCHDMLC, in the treatment\
head simulation source, then the motion of the source 21 can be synchronized\
with the dynamic motion (e.g. changing leaf opening coordinates) of these\
CMs.  In this case, the value of MU_INDEX used by DOSXYZnrc to determine the\
source orientation is passed to DOSXYZnrc from BEAM after first having been\
used to calculate the geometry parameters of the synchronized CMs.  You must\
set up the synchronization between CMs and between the BEAM simulation and the\
motion of source 21 by matching the fractional monitor unit indices of the fields\
defined by the synchronized CMs with those of the control points defining the\
source motion.  See the DOSXYZnrc and BEAMnrc Users Manuals for more details.

The figure at left illustrates continuous motion of source 21 between two control points\
(1 and 2) where the points share a common isocentre but where all other geometrical parameters\
are varying.  The figure depicts the BEAM scoring plane and the final CM in the BEAM simulation\
source, a synchronized MLC (SYNCVMLC, SYNCHDMLC, or SYNCMLCE).  If the BEAM simulation is\
synchronized with source 21, then the continuous source motion between control points 1 and\
2 will be coordinated with motion of the MLC leaves between opening coordinates associated\
with muIndex(1) and those associated with muIndex(2).
}


set help_text(medium) {
In this window you may define the media you wish to use in the simulation.  \
	An index is associated with each medium for use when defining the\
	individual voxels.

The medium names must be the same as entered in the pegs4dat file (in\
	$HEN_HOUSE/pegs4/data) that you plan to run the simulation with.  To\
	obtain a listing of valid material names in the 700icru data set, \
	for example, use 'grep MEDIUM 700icru.pegs4dat'.
}

set help_text(dbs) {
If you have used directional bremsstrahlung splitting (DBS)\
        in the BEAM simulation used to generate the phase space source then\
        these inputs allow you to reject fat photons (those aimed outside\
        the DBS splitting field).  It is recommended that you do this\
        because these fat photons may sabotage your dose statistics.

To reject photons, click on the "DBS used to generate source" checkbox.\
        This will enable the other 3 inputs.  Set the DBS splitting field\
        radius and the SSD of the splitting field equal to their values\
        used in the BEAM simulation.  Set the Z where the source was scored\
        equal to the Z value of the scoring plane where this phase space\
        source was generated (this will be at the bottom of a CM--check\
        your BEAM input).

If this option is used then, before a photon is used in the DOSXYZ simulation,\
        it is projected along its trajectory from the Z where the source\
        was scored down to the SSD where the splitting radius was defined.\
        If its trajectory takes it beyond the DBS splitting radius, then\
        it is rejected from the DOSXYZ simulation.
}

set help_text(dbs_forsrc9) {
If you are using directional bremsstrahlung splitting (DBS)\
        in the BEAM simulation source then\
        checking this box will result in the rejection of fat photons (those aimed outside\
        the DBS splitting field) from the DOSXYZnrc simulation.  It is recommended that you do this\
        because these fat photons may sabotage your dose statistics.
}

set help_text(esplit) {
e-/e+ splitting no. (e_split):

Number of times to split charged particles as soon as they enter
the DOSXYZnrc phantom.  The weight of each particle is reduced by
1/e_split.  This option is only available with sources
2, 8 (phase space sources) and 9 (BEAMnrc simulation source) and
is only invoked when the photon splitting number (n_split) is set > 1.
It is used to ensure that the contaminant charged particles in a photon
beam have approximately the same weight as split photons.  Otherwise,
the contaminant dose will have an adverse effect on the dose uncertainty.
It is recommended that you set e_split = n_split for peak efficiency.
}

set help_text(muphspout) {
Include fractional monitor unit index (MU) in any phase space output.

If the user has selected the option to output 3-D particle phase space data (IAEA format)\
on exit from the phantom geometry (see main inputs--only available for phase space, BEAMnrc simulation\
and multiple source model sources), then by checking this option, the fractional MU associated\
with each particle is also output, essentially making the phase space 4-D by adding the time\
dimension.  This additional option is only available for sources 20 and 21 (synchronized sources).

Note that if there is a synchronized component module (CM) with time-varying settings\
in the BEAMnrc simulation that\
is being used as a shared library source (21) or was used to generate the phase space source (20)\
then the fractional MU associated with a particle is passed to DOSXYZnrc from BEAMnrc.
}

set help_text(calflag) {
Check if you want to omit the calibration run through a BEAM library geometry\
interposed between the source plane and the phantom.

The calibration run is performed by default at the beginning of a simulation\
using source 20 with a BEAM library geometry.  It generates the ratio of the number of particles emerging from the bottom of the geometry to the number of\
incident particles (survival ratio).\
This ratio is then used to modify the number of times to recycle source particles\
before moving on to the next one, NRCYCL, to prevent rewinding of the phase space\
source and potential underestimation of uncertainties.

Calibration runs use 1x10^6 incident histories and, therefore, may consume\
significant CPU time at the beginning of a simulation.

Omitting the calibration run is recommended only if there are sufficient\
particles in the phase space source that rewinding is not an issue.
}
