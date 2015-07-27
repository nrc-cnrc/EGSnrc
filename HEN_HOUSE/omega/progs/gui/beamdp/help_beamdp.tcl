
###############################################################################
#
#  EGSnrc BEAMnrc beamdp graphical user interface: help messages
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
#  Author:          Joanne Treurniet, 1999
#
#  Contributors:    Blake Walters
#                   Iwan Kawrakow
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


set fluence_help_text {
    Each sub-source has its own particle planar fluence distribution.  These\
            distributions can be scored in a circular field with annular bins,\
            a square field with square rings, or a rectangular field with\
            rectangular regions.  Note that, \
            once chosen,  all the sub-sources will use the same field type\
            for their planar fluence distributions.

    CIRCULAR FIELD

    The circular planar fluence scoring field is centred on the z-axis.  \
	    You must specify the radius of the treatment field, the radius\
            of the scoring field and the number of annular bins (of equal\
            area) inside the treatment field and outside the treatment field\
            (but inside the scoring field).\
	    Circular fields are good for beams confined by\
	    circular linac components such as those scattered by scattering\
	    foils, monitoring chamber, mirror, and confined by ring- or\
	    cone-collimators. Fields formed by rectangular linac components\
	    such as jaws and applicators are not suitable for this field\
	    type.

    SQUARE FIELD

    The square planar fluence scoring field is also centred on the z-axis.  \
            The user must specify the half-width of the treatment field, the\
            half-width of the scoring field and the number of square ring bins\
            (of equal area) inside the treatment field and outside the\
            treatment field (but inside the scoring field).

    RECTANGULAR FIELD

    This option allows the user to set-up asymmetric and/or off-axis\
	    fields. The rectangular field is divided\
	    into Nbin x Nbin equal rectangular areas to record the\
	    planar fluence.  The resolution is the same in the treatment\
            field as outside the treatment field.  Note that with this\
            field type you do not have the option to separate the fields\
            in which the energy spectra are determined from those in which\
            fluence is determined.  Two energy spectra will be determined,\
            one inside the rectangular treatment field defined here, and one\
            outside the treatment field (but inside the scoring field).

    Note that if you are analyzing ph-sp data using sub-source specifiers from\
            an existing source model then none of these field parameters can\
            be modified.
}


set junk {

\subsection{Angular Distribution}
To correct for the effect of the electron multiple-scattering in air, the
angular distribution of the electron beam going through 100 cm air is
required by the re-construction procedure in order to produce an angular
perturbation around the already chosen electron incident direction. BEAMDP
analyzes the simulated beam phase-space data and scores the angular
spread  of the `direct' electrons within a circle of 1 cm radius (\ie,
the angle between the particle incident direction and the z-axis).  \
	This angular spread is considered to be a good approximation of
that for a pencil beam of electrons of the same energies going through an
air slab of thickness equal to the SSDdirect of the `direct'
electrons. This angular distribution is also stored in the source
parameter file for beam re-construction.  No user input is required while
running BEAMDP.
}

set subsource_help_text {The source model consists\
	of a number of sub-sources and each has its own energy spectrum\
	and field distribution at the phantom surface.

The idea behind the multiple source model is that particles from different\
	parts of an accelerator may be treated as if they are from different\
	sub-sources.  This is supported by the fact that particles from\
	different components of an accelerator have different energy, angular\
	and spatial distributions.  Particles from the same component, however, \
	have very similar characteristics, in terms of energy range and incident\
	directions, which are almost independent of their positions on the\
	scoring plane.

The component modules in a linear accelerator are classified \
	as aperture applicators, tubular applicators, collimators, \
	rings, cones and point sources, rectangular plane sources and \
	circular plane sources.

APERTURE APPLICATORS

Aperture applicators are modelled as surfaces on the (x,y) plane with zero\
	thickness. This is a good approximation of more recent applicator\
	designs.

The dimensions of the applicator opening (i.e. the aperture) should be exactly\
	the same as that of the applicator being modelled. The particles are\
	considered to be non-uniform on the surface, with more coming from the\
	edges of the opening.  It is not necessary, however, that the applicator\
	model have the same outer dimensions as those of the applicator.  The\
	dimensions of a charged particle sub-source can be considered to be\
	equivalent to inner opening dimensions + a 0.5 - 2.0 cm margin.  \
	However, for the lowest applicator (closest to the patient) the actual\
	applicator dimensions should be used as electrons created by\
	bremsstrahlung photons can also reach the phantom surface. For\
	bremsstrahlung photons, the outer dimensions of the sub-sources should\
	correspond to those of an area actually `exposed' to the electron beam; \
	most of the electrons are stopped by the applicator but the x-rays\
	created by them can reach the scoring plane (contaminant photons). In\
	most cases, the actual outer dimensions can be used for the photon\
	sources. The distance from the sub-source to the phantom surface can be\
	calculated from the mid-point of the applicator thickness to the phantom\
	surface.

TUBULAR APPLICATOR

Tubular applicators are modelled as tubular surfaces expanded in the\
	z-direction. This is an approximation of the `old' design such as\
	that used in Philips SL75-20 accelerator. A tubular applicator can also\
	be simulated using a series of stacked aperture applicators.

The source dimensions are similar to those for aperture applicators except that\
	both the distances from the bottom and the top of the sub-source to the\
	phantom surface are required.

COLLIMATOR

Collimator jaws are modelled as parallel-bars with zero height. The orientation\
	of the collimator bars can be either along the x- or y-axis. The\
	particles are considered to be from the surface non-uniformly, with\
	more coming from the edges of the opening. The distance from the\
	sub-source to the phantom surface can be calculated from the mid-point\
	of the collimator thickness to the phantom surface. The dimensions of\
	the sub-source are the same as those of the actual collimator.

RING, CONE OR POINT SOURCE

Primary collimators are usually ring- or cone-shaped; they are modelled as a\
	ring with zero height. The dimensions of the sub-source are the same as\
	that of the actual ring or cone and the distance from the sub-source to\
	the phantom surface can be calculated from the mid-point of the\
	ring/cone thickness to the phantom surface. The particles are considered\
	to be from the surface non-uniformly, with more coming from the edges of\
	the opening.  When the radius of this sub-source is set to zero the\
	sub-source becomes a point source. For a point source the user is asked\
	to input the source surface distance, Zmin, which can\
	be a dummy or default value (say, 100 cm) as Zmin will be\
	re-evaluated by the BEAMDP program anyway.

Note that for a virtual point source the source to surface distance, \
	SSDvir (= Zmin input by the user) will be re-evaluated using\
	a method similar to the `pin-hole' method.  \
	In order to derive the SSD for a (virtual) point source, the\
	particles falling into this ring (R1) will be transported\
	in vacuum. The radius of the projected image of this ring (R2)\
	at a distance, d, from the scoring plane will be calculated.  \
	The virtual SSD is then calculated by

                              SSD = d R1/(R2 - R1)

For more information, see 'Beam Characterization: a multiple-source model'\
	by C-M Ma and D.W.O. Rogers, NRC Report PIRS-0509(C).  \
	The radius (in cm) of the ring region on the scoring plane\
	should be supplied here (R > 0.0 cm and within the field).  \
	Note that if you have selected a point source, the radius must be\
	0.0 cm, and the SSD will be re-evaluated based on the phase space data.

PLANAR SOURCES

Scattering foils, mirrors and monitoring ionization chambers are modelled\
	as either rectangular or circular planar sources. The dimensions of the\
	sub-source are the same as that of an area actually `exposed' to the\
	electron beam but with zero thickness. Particles are sampled uniformly\
	on the source surface. The distance from the sub-source to the phantom\
	surface can be calculated from the mid-point of the component thickness\
	to the phantom surface.

Planar sub-sources are mainly used for bremsstrahlung photons as they are\
	created directly in these components and their origins are\
	well-defined.  For charged particles, however, planar sub-sources can\
	generally be replaced by a virtual point source.


CHARGE

The particles from the source may be photons, electrons or positrons.


LATCH

The LATCH bit number should be consistent with the phase-space\
	data (see BEAM manuals).
Note:
LATCH ranges between 1 and 23 as set in BEAM input.

Note that if you are analyzing ph-sp data using sub-source specifiers from\
an existing source model then the sub-source specifiers cannot be modified.
}
