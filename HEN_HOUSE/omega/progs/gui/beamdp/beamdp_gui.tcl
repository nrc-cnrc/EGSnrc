#!/bin/sh
###############################################################################
#
#  EGSnrc BEAMnrc beamdp graphical user interface: main program
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
#  Contributors:    Dave Rogers
#                   Blake Walters
#                   Ernesto Mainegra-Hing
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
#  Note: either wishx is on your system path of you must modify the following
#  line to put the explicit path to wishx. You can use the following command
#  to find wishx on your system:
#
#  cd /usr; find . -name "*wish*" -print;
#
# The backslash at the end of the comment line below is a continuation
# character in wish, but not in sh.
#
# The next line restarts using wishx\
exec wish "$0" ${1+"$@"}

# THIS IS THE MAIN PROGRAM FOR A GUI FOR BEAMDP
# Started in February 1999 by Joanne Treurniet

set chargetype(-1) "electrons"
set chargetype(0) "photons"
set chargetype(1) "positrons"
set chargetype(2) "all"
set chargetype(3) "positrons and electrons"
set chargetype(100) {}
set charge 100

set field_direction(0) "X"
set field_direction(1) "Y"
set field_direction(100) {}
set fpar(3) 100

set lflagtext {}
set lflag {}
set gflagtext "---- Graph type ----"
set gflag {}
set fluflagtext "---- Fluence type ----"
set fluflag {}
set angflagtext "---- angular plot type ----"
set angflag {}
set nbit1 0
set nbit2 0
for {set i 0} {$i<29} {incr i} {
    set inc($i) 0
    set exc($i) 0
}
set rect 1
for {set i 1} {$i<=100} {incr i} {
    set opt0vals(charge,$i) "---Select charge---"
    set opt0vals(type,$i) 0
}

set estype {}

set maxscatt {}

set types(0) "---Select type---"
set types(1) "aperture applicator"
set types(2) "collimator"
set types(3) "ring, cone or point source"
set types(4) "rectangular plane source"
set types(5) "circular plane source"
set types(11) "tubular applicator"

# Number of variables for each type:
set nvar(1) 7
set nvar(2) 8
set nvar(11) 8
set nvar(3) 4
set nvar(4) 5
set nvar(5) 2

set home $env(HOME)
set hen_house $env(HEN_HOUSE)
if [catch {set egs_home $env(EGS_HOME)}]==1 {
    tk_dialog .error "GUI error" "The environment variable EGS_HOME is \
        not set but it must be for the proper operation of EGSnrc and \
        BEAMnrc." error 0 OK
    exit
}

set omega [file join $hen_house omega]
set GUI_DIR [file join $omega progs gui beamdp]
if [catch {set egs_home $env(EGS_HOME)}]==1 {
    tk_dialog .error "GUI error" "The environment variable EGS_HOME is \
        not set but it must be for the proper operation of EGSnrc and \
        BEAMnrc." error 0 OK
    exit
}

set start_dir [pwd]
set egs_home_base [string trimright $egs_home]
if { "$tcl_platform(platform)"=="windows" } {
   set len [string length $egs_home_base]
   set len [expr $len-1]
   if { [string index $egs_home_base $len]=="\\" } {
      set len [expr $len-1]
      set egs_home_base [string range $egs_home_base 0 $len]
   }
   set beg [string last "\\" $egs_home_base]
   set beg [expr $beg+1]
   set egs_home_base [string range $egs_home_base $beg $len]
} else {
   set len [string length $egs_home_base]
   set len [expr $len-1]
   if { [string index $egs_home_base $len]=="/" } {
      set len [expr $len-1]
      set egs_home_base [string range $egs_home_base 0 $len]
   }
   set beg [string last "/" $egs_home_base]
   set beg [expr $beg+1]
   set egs_home_base [string range $egs_home_base $beg $len]
}
set start_index [string first $egs_home_base $start_dir]
if $start_index==-1 {
   # egs_home not found in start_dir.  Change it to $egs_home.
   set start_dir $egs_home
}

# Define application icon
wm iconname . "BEAMDP GUI"
wm iconbitmap . @$GUI_DIR/beamdp.xbm

if [file exists $home/.gui_defaults]==1 {
    option readfile $home/.gui_defaults
} elseif [file exists $GUI_DIR/.gui_defaults]==1 {
    option readfile $GUI_DIR/.gui_defaults
}

set helvfont "-*-helvetica-medium-r-normal--*-140-*"
set smhelvfont "-*-helvetica-medium-r-normal--*-120-*"
set normfont "-*-times-medium-r-normal--*-140-*"
set italfont "-*-times-medium-i-normal--*-140-*"

source $GUI_DIR/browser.tcl
source $GUI_DIR/help_beamdp.tcl
source $GUI_DIR/beamdp1.tcl

################## MAIN #########################

wm title . "BEAMDP GUI"
set width [winfo screenwidth .]
set height [winfo screenheight .]
set rootx [expr ($width/2) - 400]
set rooty [expr ($height/2) -300]
wm geometry . +$rootx+$rooty

frame .mbar -relief raised -bd 1

menubutton .mbar.file -text "File" -menu .mbar.file.menu
menu .mbar.file.menu
.mbar.file.menu add command -label "Exit" -command "exit"
pack .mbar.file -anchor w -side left

menubutton .mbar.help -text "Help" -menu .mbar.help.menu
menu .mbar.help.menu
.mbar.help.menu add command -label "Help" -command {
    set text {
BEAMDP (BEAM Data Processor) creates a source parameter file \
	for beam characterization models with information obtained \
	from the user and derived from a full phase-space data file\
	created by BEAM.

This programme can be used to derive planar fluence, spectrum,\
	mean energy and angle distribution, etc., from a phase-space\
	file created by BEAM.

You have the following operation options:
-----------------------------------------

(0) - data processing for beam characterization models \
(uses a multiple source model)

(1) - deriving fluence vs position from ph_sp data \
(number of particles vs position for required charge, region\
and latch)

(2) - deriving energy fluence vs position from ph_sp data \
(kinetic energy vs position for required charge, region and\
latch)

(3) - deriving energy spectrum from ph_sp data \
(particle spectrum for required charge, region and latch)

(4) - deriving energy fluence distribution from ph_sp data\
(energy fluence distribution for required charge, region and\
latch)

(5) - deriving mean energy information from ph_sp data \
(mean energy for required charge, region and latch)

(6) - deriving angular distribution from ph_sp data \
(angular distribution for required charge, region and latch)

(7) - deriving zlast distribution from ph_sp data \
(angular distribution for required charge, region and latch)

(8) - deriving information about particle weights from ph_sp data \
(distribution of weights for specified charge,region and latch)

(9) - deriving x-y positions of particles\
(x-y scatter plot of particles for specified charge,region and\
latch)

(10) - combining two ph-sp files into one  \
(write the contents of file1 into file2)

(11) - list parameters for a number of ph-sp particles\
(list iq,x,y,u,v,w,e,weight,latch on the screen)
}
help_dialog .help_beamdp {BEAMDP Help} $text help 0 OK
}
pack .mbar.help -anchor e -side right

pack .mbar -fill x

frame .intro
frame .intro.left -bd 4 -relief ridge
frame .intro.right -bd 4 -relief ridge

# message box for copyright notice

label .intro.left.title -text "beamdp GUI"\
	-font "-adobe-helvetica-medium-r-normal--24-*-*-*-p-*-iso8859-1"
message .intro.left.copy -text {
Ionizing Radiation Standards Group
Institute for National Measurement Standards
National Research Council Canada

Copyright 1999 National Research Council Canada
} -width 6i -font $smhelvfont

pack .intro.left.title  -anchor w -pady 5
pack .intro.left.copy -fill x -anchor w -pady 5

# Menubar of options, first group
set fopt(0) "Process data for beam characterization models"
set fopt(1) "Derive fluence vs. position from ph-sp data"
set fopt(2) "Derive energy fluence vs position from ph-sp data"
set fopt(3) "Derive spectral distribution from ph-sp data"
set fopt(4) "Derive energy fluence distribution from ph-sp data"
set fopt(5) "Derive mean energy distribution from ph-sp data"
set fopt(6) "Derive angular distribution from ph-sp data"
set fopt(7) "Derive ZLAST distribution from ph-sp data"
set fopt(8) "Derive distribution of particle weights from ph-sp data"
set fopt(9) "Derive X-Y scatter plot of particles from ph-sp data"
set fopt(10) "Combine two ph-sp files into one"
set fopt(11) "List parameters for a number of ph-sp particles"

set fields(0) "Circular field with circular ring bins"
set fields(1) "Square field with square ring bins"
set fields(2) "Rectangular field with rectangular bins"
set fields(3) "Circular field with energy spectra in each radial bin"

set fieldtype {}

set action_selected "---- Choose action ----"

label .intro.right.text -text "Select action:" -font $helvfont
menubutton .intro.right.mbut -text $action_selected -menu \
	.intro.right.mbut.m -bd 1 -relief raised -indicatoron 1 \
	-width [string length $fopt(8)]
menu .intro.right.mbut.m -tearoff no
.intro.right.mbut.m add cascade -label $fopt(0) -menu .intro.right.mbut.m.casm

menu .intro.right.mbut.m.casm -tearoff no
.intro.right.mbut.m.casm add command -label "Input new sub-source specifiers\
	(option to use existing source model as a reference) & analyze ph-sp data" \
        -command "do_action 0 0"
.intro.right.mbut.m.casm add command -label "Change sub-source specifiers in an\
	existing source model (no ph-sp analysis)" -command "do_action 0 1"
.intro.right.mbut.m.casm add command -label "Analyze ph-sp data using sub-source\
	specifiers from an existing source model" -command "do_action 0 2"
.intro.right.mbut.m.casm add command -label "Show and plot characteristics of a source\
        model" -command "do_action 0 3"
for {set iopt 1} {$iopt <= 11} {incr iopt} {
    .intro.right.mbut.m add command -label $fopt($iopt) \
	    -command "do_action $iopt 0"
}
pack .intro.right.text .intro.right.mbut -side top -padx 10 -pady 10 -anchor w

#frame .left.sep -bd 4 -width 100 -height 2 -relief groove
#pack .left.sep -fill x -pady 5

pack .intro.left -side left -anchor n
pack .intro.right -side right -fill y

pack .intro


proc do_action { iopt sub } {
    global fopt fields fieldtype lflag lflagtext rect action_selected
    global gflag gflagtext fluflag fluflagtext spcnam phspfile phspfile2
    global isub srcfilename waitvar errvar browservar chargetype charge
    global maxvals field_direction maxscatt angflag angflagtext tcl_platform
    global start_dir

    .intro.right.mbut configure -text $fopt($iopt)
    set action_selected $fopt($iopt)

    set isub $sub

    if $iopt==0 {
	#call beamdp1 with the appropriate isub
	if $isub==0 {
            set useref [tk_dialog .reffile "Reference file?" "Use the sub-source specifiers from an existing source model as a reference for your own input?  Note that if you do this, you cannot change the number of sub-sources." question 0 No Yes]
            if {$useref==1} {
                browse "$start_dir" set_srcfilename 1 {Select the reference source model:}
                tkwait window .query
                if $browservar==1 {
                  load_source_info
                  beamdp1 . $srcfilename
                }
            } else {
	        beamdp1 . "undefined"
            }
	} elseif $isub==1 {
            # beamdp1 is called from set_srcfilename:
            browse "$start_dir" set_srcfilename 1 {Select the source model to be modified:}
            tkwait window .query
            if $browservar==1 {
                load_source_info
                beamdp1 . $srcfilename
            }
        } elseif $isub==2 {
	    # beamdp1 is called from set_srcfilename:
	    set browservar 0
	    browse "$start_dir" set_srcfilename 1 {Select the source model containing specifiers to use for analyzing ph-sp data:}
	    tkwait window .query
	    if $browservar==1 {
		load_source_info
		beamdp1 . $srcfilename
	    }
	} elseif $isub==3 {
	    set browservar 0
	    browse "$start_dir" set_srcfilename 1  {Select the source model to plot:}
	    tkwait window .query
	    if $browservar==1 {
		load_source_info
		beamdp1 . $srcfilename
	    }
	}
    } else {
	catch { destroy .params }
	toplevel .params
	wm title .params "Parameters for option $iopt"

	label .params.title -text $fopt($iopt) -width [string length $fopt(8)]
	pack .params.title -side top -anchor n -pady 10

	frame .params.main
	set w .params.main

	if { $iopt==1 | $iopt==2 | $iopt==5 } {
	    frame $w.frm
            if {$fieldtype==""} {
               menubutton $w.frm.fieldm -text "select field type" -menu \
                    $w.frm.fieldm.m -bd 1 -relief raised -indicatoron 1
            } else {
	       menubutton $w.frm.fieldm -text $fields($fieldtype) -menu \
		    $w.frm.fieldm.m -bd 1 -relief raised -indicatoron 1
            }
	    menu $w.frm.fieldm.m -tearoff no
	    for {set i 0} {$i <= 2} {incr i} {
		$w.frm.fieldm.m add command -label $fields($i)\
			-command "get_field_parameters $i $w.frm"
	    }
	    pack $w.frm.fieldm -fill x -expand true -padx 10
            if {$fieldtype!=""} {
	      for {set i 0} {$i<=2} {incr i} {
		if [string compare $fields($fieldtype) $fields($i)]==0 {
		    get_field_parameters $i $w.frm
		    break
		}
	      }
            }
	    pack $w.frm
	} elseif { $iopt>0 & $iopt<10 } {
	    frame $w.sep2 -bd 4 -width 100 -height 2 -relief groove
	    pack $w.sep2 -fill x -pady 5

	    set text1(1) "Particle type"
	    frame $w.inp1
	    label $w.inp1.lab -text $text1(1)
	    menubutton $w.inp1.menu -text $chargetype($charge) -menu \
		    $w.inp1.menu.m -bd 1 -relief raised -indicatoron 1
	    menu $w.inp1.menu.m -tearoff no

	    for {set i -1} {$i <= 3} {incr i} {
		$w.inp1.menu.m add command -label $chargetype($i)\
			-command "set charge $i;\
			$w.inp1.menu configure -text {$chargetype($i)}"
	    }
	    pack $w.inp1.lab -anchor w -side left
	    pack $w.inp1.menu -fill x -expand true -padx 10
	    pack $w.inp1 -pady 15 -fill x

	    # put on the 5 entries and 3 entries with a radiobutton to choose.
	    # side by side
	    frame $w.frm
	    frame $w.frm.five
	    set w1 $w.frm.five
	    frame $w.frm.three
	    set w2 $w.frm.three

	    radiobutton $w1.rb -text "Rectangular region anywhere on the scoring\
		    plane" -variable rect -value 1 -command "enable 1"
	    pack $w1.rb
	    radiobutton $w2.rb -text "Annular region centred at the z-axis"\
		    -variable rect -value 0 -command "enable 0"
	    pack $w2.rb

	    set text1(2) "Xmin for the rectangular region (default -15)"
	    set text1(3) "Xmax for the rectangular region (default 15)"
	    set text1(4) "Ymin for the rectangular region (default -15)"
	    set text1(5) "Ymax for the rectangular region (default 15)"
	    set text2(2) "Rmin for the annular region (default 0)"
	    set text2(3) "Rmax for the annular region (default 15)"

	    for {set j 2} {$j<=5} {incr j} {
		textfield $w1.inp$j $text1($j) fpar($j)
	    }
	    label $w.frm.or -text "OR"


	    for {set j 2} {$j<=3} {incr j} {
		textfield $w2.inp$j $text2($j) fpar($j)
	    }
	    pack $w1 -side left -anchor nw
	    pack $w.frm.or -padx 5 -side left -anchor n
	    pack $w2 -side right -anchor ne
	    if $rect==1 {
		enable 1
		$w1.rb invoke
	    } else {
		enable 0
		$w2.rb invoke
	    }
	    pack $w.frm -pady 5

	    if { $iopt==3 | $iopt==4 } {
		# get number of bins and min and max energy
		set text(1) "Number of equal energy bins (default $maxvals(nb))"
		set text(2) "Minimum energy (MeV)"
		set text(3) "Maximum energy (MeV)"
		set helptext {The maximum number of bins is $maxvals(nb) and \
			the energy range given should be consistent with\
			the phase-space data.  For example, the minimum energy\
			should be the same as ECUT used in the simulation\
			with BEAM, and the maximum energy should be greater than\
			the maximum energy of the particles in the \
			phase space data.}
	    } elseif $iopt==6 {
		# get number of bins, min and max angle
		set text(1) "Number of angular bins (default $maxvals(nbinangle))"
		set text(2) "Minimum angle (degrees, default 0)"
		set text(3) "Maximum angle (degrees, default 90)"
		set helptext {The maximum number of bins is $maxvals(nbinangle)\
			and the angle should be between 0 and 90 degrees.}
	    } elseif $iopt==7 {
		# get number of bins, zmin and zmax
		set text(1) "Number of bins (default $maxvals(nbinangle))"
		set text(2) "Minimum Z (cm) (default 0)"
		set text(3) "Maximum Z (cm) (default 100)"
		set helptext {The maximum number of bins is $maxvals(nbinangle)\
			and the minimum and maximum Z define the range of ZLAST\
			(see your BEAM User Manual for more information).}
	    } elseif $iopt==8 {
		# get number of bins, min and max weight for the distn.
		set text(1) "Number of bins (default $maxvals(nb))"
		set text(2) "Minimum weight"
		set text(3) "Maximum weight"
		set helptext {The maximum number of bins is $maxvals(nb) and the\
			weight range given should be consistent with the\
			phase-space data and also consistent with the\
			fact that the weights are plotted on a logarithmic\
			scale.  For example, the minimum weight should be\
			slightly greater than 0 and the maximum weight\
			should be slightly greater than 1.}
	    } elseif $iopt==9 {
		# scatter plot, get min and max energy for the particles
		set helptext {The energy range given should be consistent with\
			the phase-space data.  For example, the minimum energy\
			should be the same as ECUT used in the simulation\
			with BEAM, and the maximum energy should be greater than\
			the maximum energy of the particles in the \
			phase space data.}
		set w3 .params.main.nbin
		frame $w3
		frame $w3.sep2 -bd 4 -width 100 -height 2 -relief groove
		pack $w3.sep2 -fill x -pady 5
		set text(1) "Minimum energy (MeV)"
		set text(2) "Maximum energy (MeV)"
		button $w3.left -text ? -command \
			"help_dialog $w3.left.help {Help} {$helptext}\
			question 0 OK"
		frame $w3.right
		for {set j 1} {$j<=2} {incr j} {
		    textfield $w3.right.inp$j $text($j) nbinvars($j)
		}
		pack $w3.left -side left -anchor w -padx 10
		pack $w3.right -side right -anchor e -fill x -expand 1
		pack $w3 -fill x
	    }
	    if { $iopt==3 | $iopt==4 | $iopt==6 | $iopt==7 | $iopt==8 } {
		set w3 .params.main.nbin
		frame $w3
		frame $w3.sep2 -bd 4 -width 100 -height 2 -relief groove
		pack $w3.sep2 -fill x -pady 5
		button $w3.left -text ? -command \
			"help_dialog $w3.left.help {Help} {$helptext}\
			question 0 OK"
		frame $w3.right
		for {set j 1} {$j<=3} {incr j} {
		    textfield $w3.right.inp$j $text($j) nbinvars($j)
		}
		pack $w3.left -side left -padx 10 -anchor w
		pack $w3.right -side right -anchor e -expand 1 -fill x
		pack $w3 -fill x
	    }
            # also input min. and max. energy for angular distn
            if { $iopt==6 } {
                # scatter plot, get min and max energy for the particles
                set helptext {The energy range for particles to be included\
                        in the angular distribution.  Set minimum energy to 0\
                        and maximum energy to Emax to include all particles.}
                set w3 .params.main.ange
                frame $w3
                frame $w3.sep2 -bd 4 -width 100 -height 2 -relief groove
                pack $w3.sep2 -fill x -pady 5
                set text(1) "Minimum energy (MeV)"
                set text(2) "Maximum energy (MeV)"
                button $w3.left -text ? -command \
                        "help_dialog $w3.left.help {Help} {$helptext}\
                        question 0 OK"
                frame $w3.right
                for {set j 1} {$j<=2} {incr j} {
                    textfield $w3.right.inp$j $text($j) angevars($j)
                }
                pack $w3.left -side left -anchor w -padx 10
                pack $w3.right -side right -anchor e -fill x -expand 1
                pack $w3 -fill x
            }

	}
	if { $iopt>0 && $iopt<10 } {
	    frame $w.sep3 -bd 4 -width 100 -height 2 -relief groove
	    pack $w.sep3 -fill x -pady 5

	    # get LATCH bit filters, 3 variables on one line
	    # 1) i_in_ex (flag for inc/exc) 2) nbit1, 3) nbit2
	    # On the next line goes the bits to inc/exc
	    # On another line go the bits to exc, if necessary.
	    set lflagopt(0) "Inclusive/Exclusive filter for bits"
	    set lflagopt(1) "Exclusive filter for bits"
	    set lflagopt(2) "Inclusive filter for regions"
	    set lflagopt(3) "Exclusive filter for regions"
	    set lflagopt(4) "None"
	    if { [string compare $lflagtext {}]==0 || \
		    [string compare $lflagtext "---- Choose LATCH ----"]==0 } {
		set lflagtext "---- Choose LATCH ----"
	    } else {
		set lflagtext $lflagopt($lflag)
	    }
	    set w4 $w.latch
	    frame $w4
	    button $w4.help -text ? -command \
		    "help_dialog $w4.help.help {Help} \
		    {LATCH bit filters:

You have 5 options for LATCH bit filters.  You may choose:

1) Inclusive/Exclusive for bits: if any of the 1st set of \
                  bits are set and none of the 2nd set of bits \
                  are set, the particle is scored.

2) Exclusive for bits: if any of the set of bits\
                  are set, the particle is not scored.

3) Inclusive for regions: score secondary particles that \
                  originated in the regions with IREGION_TO_BIT\
                  equal to any of the set of bits.

4) Exclusive for regions: do not score secondary particles\
                  that originated in regions with IREGION_TO_BIT equal\
                  to any of the set of bits.

5) None.

You can use bits 1 to 23 for these filters.  For more information, \
	refer to the section on LATCH bit filters in your BEAM User Manual.
} question 0 OK"
	    label $w4.text -text "LATCH option"
	    menubutton $w4.mbut -text $lflagtext -menu \
		    $w4.mbut.m -bd 1 -relief raised -indicatoron 1
	    menu $w4.mbut.m -tearoff no
	    for {set i 0} {$i <= 4} {incr i} {
		$w4.mbut.m add command -label $lflagopt($i)\
			-command "set lflagtext {$lflagopt($i)};\
			$w4.mbut configure -text {$lflagopt($i)}; \
			set_inc_or_exc .params $i"
	    }
	    pack $w4.help -anchor w -side left -padx 10
	    pack $w4.text -side left
	    pack $w4.mbut -fill x -expand true -padx 10
	    pack $w4 -fill x -pady 5
	} elseif $iopt==11 {
	    frame $w.sep3 -bd 4 -width 100 -height 2 -relief groove
	    pack $w.sep3 -fill x -pady 5
	    # get number of particles, iq.
	    set w5 $w.opt11
	    frame $w5
	    textfield $w5.f1 "Number of particles to be listed (default 100)" fpar(1)
	    set text1(1) "Particle type"

	    frame $w5.inp1
	    label $w5.inp1.lab -text $text1(1)
	    menubutton $w5.inp1.menu -text $chargetype($charge) -menu \
		    $w5.inp1.menu.m -bd 1 -relief raised -indicatoron 1
	    menu $w5.inp1.menu.m -tearoff no

	    for {set i -1} {$i <= 3} {incr i} {
		$w5.inp1.menu.m add command -label $chargetype($i)\
			-command "set charge $i;\
			$w5.inp1.menu configure -text {$chargetype($i)}"
	    }

	    pack $w5.inp1.lab -anchor w -side left
	    pack $w5.inp1.menu -fill x -expand true -padx 10
	    pack $w5.inp1 -pady 15 -fill x

#	    textfield $w5.f2 "Charge (0:phot,1:posit,-1:elect,2:all,3:e+e-)" fpar(2)
	    pack $w5 -fill x -pady 5
	}

	if { $iopt>0 && $iopt<10 || $iopt==11 } {
	    frame $w.sep7 -bd 4 -width 100 -height 2 -relief groove
	    pack $w.sep7 -fill x -pady 5
	    # get phase space filename (phspfile)
	    textandbutton $w.pfile "Name of phase space file:" phspfile\
		    "Browse" {browse "$start_dir" set_phspfile 0 {Select the phase space filename:}}
	}

	if { $iopt>0 && $iopt<10 } {
	    frame $w.sep4 -bd 4 -width 100 -height 2 -relief groove
	    pack $w.sep4 -fill x -pady 5

	    # get filename to save data for graph plotting (spcnam)
	    textandbutton $w.file "Name of file to save data for graph plotting:"\
		    spcnam "Browse" {browse "$start_dir" set_spcnam 1 {Set the filename to save xmgr graph:}}

	    if $iopt<9 {
		# choose from point graph or histogram
		set ww $w.gtype
		frame $ww
		set gtype(0) "point graph"
		set gtype(1) "histogram"
		menubutton $ww.mbut -text $gflagtext -menu \
			$ww.mbut.m -bd 1 -relief raised -indicatoron 1
		menu $ww.mbut.m -tearoff no
		for {set i 0} {$i <= 1} {incr i} {
		    $ww.mbut.m add command -label $gtype($i)\
			    -command "set gflag $i; set gflagtext {$gtype($i)};\
			    $ww.mbut configure -text {$gtype($i)}"
		}
		pack $ww.mbut -fill x -expand true -padx 10
		pack $ww -fill x -pady 5

		if { $iopt>=1 && $iopt<=4 } {
		    set ww $w.flu
		    frame $ww
		    # get choice of real fluence or planar fluence (0 or 1)
		    set flutype(0) "estimated real fluence"
		    set flutype(1) "planar fluence"
		    button $ww.help -text ? -command \
			    " help_dialog $ww.help.help {Help} \
			    {Plot of real fluence or planar fluence:

Estimated real fluence:

          sum (particle weight/cos(Z-angle))
          ----------------------------
                    total area

Planar fluence:

          E(particle weight)
          ----------------
              total area
} question 0 OK"
		    menubutton $ww.mbut -text $fluflagtext -menu \
			    $ww.mbut.m -bd 1 -relief raised -indicatoron 1
		    menu $ww.mbut.m -tearoff no
		    for {set i 0} {$i <= 1} {incr i} {
			$ww.mbut.m add command -label $flutype($i)\
				-command "set fluflag $i; set fluflagtext {$flutype($i)};\
				$ww.mbut configure -text {$flutype($i)}"
		    }
		    pack $ww.help -side left -anchor w -padx 10
		    pack $ww.mbut -fill x -expand true -padx 10 -anchor e
		    pack $ww -fill x -pady 5
                } elseif { $iopt==6 } {
                    set ww $w.ang
                    frame $ww
                    # get choice of particles/angular bin (0) or
                    # particles/unit solid angle (1)
                    set angtype(0) "plot particles/angular bin"
                    set angtype(1) "plot particles/unit solid angle"
                    button $ww.help -text ? -command \
                            " help_dialog $ww.help.help {Help} \
                            {Angular plot type:

Standard angular plot is particles/angular bin vs angle.  However, if you
select particles/unit solid angle, then the number of particles in each
angular bin will be normalized by the solid angle (in steradians) subtended
by that bin.  X axis of plot will still be angle.
} question 0 OK"
                    menubutton $ww.mbut -text $angflagtext -menu \
                            $ww.mbut.m -bd 1 -relief raised -indicatoron 1
                    menu $ww.mbut.m -tearoff no
                    for {set i 0} {$i <= 1} {incr i} {
                        $ww.mbut.m add command -label $angtype($i)\
                                -command "set angflag $i; set angflagtext {$angtype($i)};\
                                $ww.mbut configure -text {$angtype($i)}"
                    }
                    pack $ww.help -side left -anchor w -padx 10
                    pack $ww.mbut -fill x -expand true -padx 10 -anchor e
                    pack $ww -fill x -pady 5
                }
	    } elseif $iopt==9 {
           #let user choose max. number of particles to plot

                frame $w.sep5 -bd 4 -width 100 -height 2 -relief groove
                pack $w.sep5 -fill x -pady 5

                set ww $w.maxscat
                frame $ww

                button $ww.help -text ? -command \
                            " help_dialog $ww.help.help {Help} \
                            {Maximum number of particles to plot:

This is an upper limit on the number of particles to output to the
X-Y scatter plot.  The actual number output may be less than this, because
it is also limited by the charge, field size, energy range and LATCH bit
settings selected above.  This number will default to the total number of
particles in the file if it is set <=0 or > total number of particles in the
file.
} question 0 OK"

                 label $ww.label -text "Maximum number of particles to plot:"
                 entry $ww.inp -textvariable maxscatt
                 pack $ww.help -anchor w -side left -padx 10
                 pack $ww.label -side left
                 pack $ww.inp -anchor e -fill x -expand true -padx 10
                 pack $ww -fill x -pady 5
            }
	}

	pack $w

	frame .params.sep -bd 4 -width 100 -height 2 -relief groove
	pack .params.sep -fill x -pady 5

	frame .params.buts -relief raised -bd 1
	button .params.buts.closeb -text "Close" -command "destroy .params"\
		-relief raised -bd 2
	button .params.buts.exec -text "Execute beamdp" -relief raised -bd 2\
		-command "run_job $iopt"
	pack .params.buts.exec .params.buts.closeb -padx 10 -pady 5 -side left
	pack .params.buts -pady 10
    }

    if $iopt==10 {
	# Put on spaces for 2 phase space files; inform the user that the second
	# one will be overwritten with the combined data.

	label $w.opt10lab -text "Input the 2 file\
		names.  The combined data will overwrite the second file input."
	pack $w.opt10lab -pady 10 -padx 10

	textandbutton $w.pfile "Name of first phase space file:" phspfile\
		"Browse" {browse "$start_dir" set_phspfile 0 {Select the first phase space filename:}}
	textandbutton $w.pfile2 "Name of second phase space file:" phspfile2\
		"Browse" {browse "$start_dir" set_phspfile2 0 {Select the second phase space filename:}}
    }
}

proc run_job {iopt} {
    global spcnam GUI_DIR tcl_platform


    if { $iopt>0 & $iopt<10 } {
	set buttonopt 0
	if [file exists $spcnam]==1 {
	    set file [file tail $spcnam]
	    set buttonopt [tk_dialog .question "Overwrite" "The file $file\
		    already exists.  Do you want to overwrite it, \
		    add this plot to the existing one, or cancel and change\
		    the filename?" info 2 Overwrite Add Cancel]
	}
	if $buttonopt==2 {
	    return
	} elseif $buttonopt==1 {
	    # spcnam -> oldspcnam, temp.xmgr -> spcnam
	    set oldspcnam $spcnam
	    set spcnam beamdpguitemp.xmgr
	}
    }
    set returnval [create_script]
    if $returnval==1 {
        if { $iopt>0 & $iopt<10 } {
         if { $buttonopt<=1 } {
            if { [winfo exists .run] == 1 } {
              destroy .run
            }
         }
        }
	toplevel .run -bd 5
	wm title .run "beamdp Output"
	frame .run.frm
	scrollbar .run.frm.scroll2 -command {.run.frm.dialg yview} \
		-orient vertical
	if $iopt==11 {
	    text .run.frm.dialg -yscroll {.run.frm.scroll2 set} -bg white \
		    -width 100 -font fixed
	} else {
	    text .run.frm.dialg -yscroll {.run.frm.scroll2 set} -bg white \
		    -width 80
	}
	pack .run.frm.dialg .run.frm.scroll2 -side left -fill y
	button .run.buts -text "Close" -command "destroy .run"
	pack .run.frm .run.buts -side top -pady 10
	.run.frm.dialg insert end "\n Running beamdp... \n\n"
	set waitvar 0
	set errvar 0
	startjob
	displaydialg "temp"
	grab .run
	tkwait variable waitvar
	grab release .run
	if $errvar==1 {
	    tk_dialog .error "Error" "Beamdp has aborted for some \
		    reason.  Please examine the output to determine\
		    the cause." error 0 OK
	}
	file delete temp
	if { $iopt>0 & $iopt<10 } {
	    if $buttonopt==1 {
		# do stuff to concatenate 2 xmgr files
		exec $GUI_DIR/concatxmgr.script $oldspcnam
		set spcnam $oldspcnam
		file delete beamdpguitemp.xmgr
	    }
            if { "$tcl_platform(platform)"=="windows" } {
                # cannot run xmgr/xmgrace
                tk_dialog .info "xmgr/xmgrace" "xmgr/xmgrace not automatically  \
                    called because you are running on Windows." error 0 OK
                return
            }
	    exec xmgrace $spcnam >& /dev/null &
	}
    }
}

proc enable { i } {
    set w .params.main
    set w1 $w.frm.five
    set w2 $w.frm.three

    if $i==1 {
	for {set j 2} {$j<=5} {incr j} {
	    $w1.inp$j.label configure -fg black
	    $w1.inp$j.inp configure -state normal
	    $w1.inp$j.inp configure -fg black
	}
	for {set j 2} {$j<=3} {incr j} {
	    $w2.inp$j.label configure -fg grey
	    $w2.inp$j.inp configure -state disabled
	    $w2.inp$j.inp configure -fg white
	}
    } else {
	for {set j 2} {$j<=5} {incr j} {
	    $w1.inp$j.label configure -fg grey
	    $w1.inp$j.inp configure -state disabled
	    $w1.inp$j.inp configure -fg white
	}
	for {set j 2} {$j<=3} {incr j} {
	    $w2.inp$j.label configure -fg black
	    $w2.inp$j.inp configure -state normal
	    $w2.inp$j.inp configure -fg black
	}
    }
}
proc get_field_parameters { i par } {
    global fields fpar fieldtype charge chargetype field_direction

    $par.fieldm configure -text $fields($i)

    set fieldtype $i

    catch { destroy $par.fieldp }

    catch { destroy $par.fieldhelp }

    frame $par.fieldp -bd 4

    if $i==0 {
	set ftext(1) "Number of radial bins into which the\
		field is divided"
	set ftext(2) "Charge of the particles"
	set ftext(3) "Outer radius of the circular field"
	set pmax 3
    } elseif $i==1 {
	set ftext(1) "Number of square bins into which the\
		field is divided"
	set ftext(2) "Charge of the particles"
	set ftext(3) "Half-side of the square field"
	set pmax 3
    } elseif $i==2 {
        if {$fpar(3)!=0 && $fpar(3)!=1} {
            set ftext(1) "Number of bins in X or Y direction"
        } else {
	    set ftext(1) "Number of bins in $field_direction($fpar(3)) direction"
        }
	set ftext(2) "Charge of the particles"
	set ftext(3) "Plot orientation"
	set ftext(4) "Xmin of rectangular field"
	set ftext(5) "Xmax of rectangular field"
	set ftext(6) "Ymin of rectangular field"
	set ftext(7) "Ymax of rectangular field"
	set pmax 7
    }

    set io $i

    textfield $par.fieldp.inp1 $ftext(1) fpar(1)
    frame $par.fieldp.inp2
    set w1 $par.fieldp.inp2
    label $w1.lab -text "Particle type"
    menubutton $w1.menu -text $chargetype($charge) -menu \
	    $w1.menu.m -bd 1 -relief raised -indicatoron 1
    menu $w1.menu.m -tearoff no

    for {set i -1} {$i <= 3} {incr i} {
	$w1.menu.m add command -label $chargetype($i) -command "set charge $i;\
		$w1.menu configure -text {$chargetype($i)}"
    }

    pack $w1.lab -anchor w -side left
    pack $w1.menu -fill x -expand true
    pack $w1 -pady 10 -fill x -padx 10

    if {$io<2} {

        for {set j 3} {$j<=$pmax} {incr j} {
          textfield $par.fieldp.inp$j $ftext($j) fpar($j)
        }
        pack $par.fieldp
    } elseif {$io==2} {

        button $par.fieldhelp -text ? -command \
                        "help_gif $par.fieldhelp.help rect_fields"

        frame $par.fieldp.inp3
        set w1 $par.fieldp.inp3
        label $w1.lab -text "Plot orientation"
        if {$fpar(3)!=1 && $fpar(3)!=0} {
          menubutton $w1.menu -text "select field direction" -menu \
            $w1.menu.m -bd 1 -relief raised -indicatoron 1
        } else {
          menubutton $w1.menu -text $field_direction($fpar(3)) -menu \
            $w1.menu.m -bd 1 -relief raised -indicatoron 1
        }
        menu $w1.menu.m -tearoff no

        for {set i 0} {$i <= 1} {incr i} {
          $w1.menu.m add command -label $field_direction($i)\
                    -command "set fpar(3) $i;\
                              $w1.menu configure -text {$field_direction($i)};\
                              $par.fieldp.inp1.label configure -text\
                      {Number of bins in $field_direction($i) direction}"
        }

        pack $w1.lab -anchor w -side left
        pack $w1.menu -fill x -expand true
        pack $w1 -pady 10 -fill x -padx 10

        for {set j 4} {$j<=$pmax} {incr j} {
        	textfield $par.fieldp.inp$j $ftext($j) fpar($j)
        }

        pack $par.fieldhelp -anchor w -side left
        pack $par.fieldp -anchor e
    }
}


proc set_inc_or_exc { parent index } {

    # window to create the inclusive and/or exclusive filters.

    global helvfont inc exc nbit1 nbit2 lflag

    set lflag $index

    # If "none" return
    if $index==4 {
	set nbit1 0
	set nbit2 0
	return
    }

    toplevel $parent.inc_or_exc
    wm title $parent.inc_or_exc "LATCH bit filters"

    if { $index==0 } {
        set text "For any given particle, if any of the inclusive bits\
            are set and none of the exclusive bits are set, the particle is\
            used."
        set len1 29
        set len2 29
    } elseif { $index==1 } {
        set text "For any given particle, if any of the bits selected are set, \
                the particle is not used."
        set len1 29
        set len2 0
    } elseif {$index==2 } {
        set text "Use secondary particles that originated in the regions\
                associated with the bits specified here."
        set len1 24
        set len2 0
    } elseif {$index==3} {
        set text "Do not use secondary particles that originated\
            in the regions associated with the bits specified here."
        set len1 24
        set len2 0
    }

    set top $parent.inc_or_exc.frm
    frame $top -bd 4
    label $top.lab -text $text -font $helvfont
    pack $top.lab -side top -pady 5

    # frame to hold a grid
    frame $top.frm -bd 4

    if $len1>0 {
        if {$index==1 || $index==3} {
            set type exc
        } else {
            set type inc
        }
        label $top.frm.inclab -text "${type}lusive bits:" -font $helvfont
        grid configure $top.frm.inclab -row 0 -column 0 -padx 10

        frame $top.frm.bits1 -relief groove -bd 4
        for {set j 0} {$j<$len1} {incr j} {
            frame $top.frm.bits1.fr$j
            label $top.frm.bits1.fr$j.l -text $j
            checkbutton $top.frm.bits1.fr$j.c -variable ${type}($j) -onvalue 1\
                    -offvalue 0
            pack $top.frm.bits1.fr$j.l $top.frm.bits1.fr$j.c -side top
            pack $top.frm.bits1.fr$j -side left
        }
        grid configure $top.frm.bits1 -row 0 -column 1 -pady 5 -sticky w
    }
    if $len2>0 {
        label $top.frm.exclab -text "Exclusive bits:" -font $helvfont
        grid configure $top.frm.exclab -row 1 -column 0 -padx 10

        frame $top.frm.bits2 -relief groove -bd 4
        for {set j 0} {$j<$len2} {incr j} {
            frame $top.frm.bits2.fr$j
            label $top.frm.bits2.fr$j.l -text $j
            checkbutton $top.frm.bits2.fr$j.c -variable exc($j) -onvalue 1\
                    -offvalue 0
            pack $top.frm.bits2.fr$j.l $top.frm.bits2.fr$j.c -side top
            pack $top.frm.bits2.fr$j -side left
        }
        grid configure $top.frm.bits2 -row 1 -column 1 -pady 5 -sticky w
    }
    pack $top.frm -side top -pady 5

    button $top.b5 -text "Done" -command "close_latbit $index \
	    $parent.inc_or_exc" -relief groove -bd 8
    pack $top.b5 -side top -pady 10
    pack $top
}

proc close_latbit { index win } {
    global inc exc latbit nbit1 nbit2

    set nbit1 0
    set nbit2 0
    if $index==0 {
        for {set i 0} {$i<29} {incr i} {
            if $inc($i)==1 { incr nbit1 }
            if $exc($i)==1 { incr nbit2 }
        }
        if [expr $nbit1+$nbit2]>29 {
            tk_dialog .error "Error" "Too many bits have been selected.  \
                    You'll have to not use so many." error 0 OK
            return
        } else {
            set j1 -1
            set j2 -1
            for {set i 0} {$i<29} {incr i} {
                if $inc($i)==1 { incr j1; set latbit($j1) $i }
                if $exc($i)==1 { incr j2; set id [expr $nbit1+$j2];\
                        set latbit($id) $i }
            }
        }
    } elseif $index==1 {
        for {set i 0} {$i<29} {incr i} {
            if $exc($i)==1 { incr nbit1 }
        }
        set j1 -1
        for {set i 1} {$i<29} {incr i} {
            if $exc($i)==1 { incr j1; set latbit($j1) $i }
        }
    } elseif $index==2 {
        for {set i 0} {$i<24} {incr i} {
            if $inc($i)==1 { incr nbit1 }
        }
        set j1 -1
        for {set i 0} {$i<24} {incr i} {
            if $inc($i)==1 { incr j1; set latbit($j1) $i }
        }
    } elseif $index==3 {
        for {set i 0} {$i<24} {incr i} {
            if $exc($i)==1 { incr nbit1 }
        }
        set j1 -1
        for {set i 1} {$i<29} {incr i} {
            if $exc($i)==1 { incr j1; set latbit($j1) $i }
        }
    }
    destroy $win
}

proc create_script {} {
    global env action_selected fopt fieldtype fields fpar charge nbinvars rect
    global lflag gflag fluflag exc inc nbit1 nbit2 spcnam phspfile phspfile2
    global maxvals maxscatt angevars angflag

    set fpardefaults(0,1) $maxvals(nb)
    set fpardefaults(0,3) 15.0
    set fpardefaults(1,1) $maxvals(nb)
    set fpardefaults(1,3) 15.0
    set fpardefaults(2,1) $maxvals(nb)
    set fpardefaults(2,4) -15.0
    set fpardefaults(2,5) 15.0
    set fpardefaults(2,6) -15.0
    set fpardefaults(2,7) 15.0

    # Writes the script to run beamdp.
    set id [open beamdpguitemp.script w]

    puts $id n

    set ichoice -1
    for {set i 0} {$i<=11} {incr i} {
	if [string compare $fopt($i) $action_selected]==0 {
	    puts $id $i
	    set ichoice $i
	    break
	}
    }

    if $ichoice==-1 {
	# Not found.  Error.
	tk_dialog .error "Error" "Can't write script to run beamdp.  \
		Massive failure." error 0 OK
	return 0
    }

    if $ichoice!=10 {

	if { $ichoice==1||$ichoice==2||$ichoice==5 } {
	    # Put msmfxy, circ ring, square ring, rectangular
	    for {set i 0} {$i<=2} {incr i} {
		if [string compare $fields($fieldtype) $fields($i)]==0 {
		    puts $id $i
		    set msmfxy $i
		    break
		}
	    }
	    set str {}

	    set fpar(1) [expr int($fpar(1))]

	    if $msmfxy<2 {
		for {set i 1} {$i<=3} {incr i} {
		    if $i!=2 {
			if [catch {expr $fpar($i)}]==1 {
			    set fpar($i) $fpardefaults($msmfxy,$i)
			}
			set str "$str$fpar($i), "
		    } else {
			if $charge==100 {
			    tk_dialog .error "Error" "The charge is undefined.\
				    Can't write script to run beamdp." \
				    error 0 OK
			    return 0
			}
			set str "$str$charge, "
		    }
		}
	    } elseif $msmfxy==2 {
		foreach i {1 3 2 4 5 6 7} {
		    if $i==3 {
			if [catch {expr $fpar($i)}]==1 {
			    tk_dialog .error "Error" "The orientation is\
				    undefined.  Can't write script to run\
				    beamdp." error 0 OK
			    return 0
			}
			set str "$str$fpar($i), "
		    } elseif $i==2 {
			if $charge==100 {
			    tk_dialog .error "Error" "The charge is undefined.\
				    Can't write script to run beamdp." \
				    error 0 OK
			    return 0
			}
			set str "$str$charge, "
		    } else {
			if [catch {expr $fpar($i)}]==1 {
			    set fpar($i) $fpardefaults($msmfxy,$i)
			}
			set str "$str$fpar($i), "
		    }
		}
	    }
	    puts $id $str
	} elseif { $ichoice>0 & $ichoice!=11 } {
	    # Put iqsmfp(1),smfmnx(1),smfmxx(1),smfmny(1),smfmxy(1)
	    if $charge==100 {
		tk_dialog .error "Error" "The charge is undefined.\
			Can't write script to run beamdp." \
			error 0 OK
		return 0
	    }
	    set str $charge

	    if $rect==1 {
		if [catch {expr $fpar(2)}]==1 { set fpar(2) -15.0 }
		set str "$str, $fpar(2)"
		if [catch {expr $fpar(3)}]==1 { set fpar(3) 15.0 }
		set str "$str, $fpar(3)"
		if [catch {expr $fpar(4)}]==1 { set fpar(4) -15.0 }
		set str "$str, $fpar(4)"
		if [catch {expr $fpar(5)}]==1 { set fpar(5) 15.0 }
		set str "$str, $fpar(5)"
	    } else {
		if [catch {expr $fpar(2)}]==1 { set fpar(2) 0 }
		if $fpar(2)<0 { set fpar(2) 0 }
		set str "$str, $fpar(2)"
		if [catch {expr $fpar(3)}]==1 { set fpar(3) 15.0 }
		set str "$str, $fpar(3), 0, 0"
	    }
	    puts $id $str

	    if $ichoice==6 {
		if [catch {expr $nbinvars(1)}]==1 { set nbinvars(1) $maxvals(nbinangle) }
		if [catch {expr $nbinvars(2)}]==1 { set nbinvars(2) 0. }
		if [catch {expr $nbinvars(3)}]==1 { set nbinvars(3) 90. }
		# check the angles make sense
		if { $nbinvars(3)<$nbinvars(2) | $nbinvars(3)>180 | \
			$nbinvars(2)<0 } {
		    tk_dialog .error "Error" "The angles are not defined\
			    properly.  The minimum angle must be greater than\
			    0, the maximum less than 180 and the maximum  \
			    greater than the minimum. " error 0 OK
		    return 0
		}
                if { [catch {expr $angevars(1)}]==1 | [catch {expr\
                        $angevars(2)}]==1 | $angevars(2)<$angevars(1) | \
                        $angevars(1)<0 | $angevars(2)<0 } {
                  tk_dialog .failed "Failed" "The minimum and maximum energies\
                          are not defined properly.  Emax>Emin>0." error 0 OK
                    return 0
                }
	    } elseif $ichoice==7 {
		if [catch {expr $nbinvars(1)}]==1 { set nbinvars(1) $maxvals(nbinangle) }
		if [catch {expr $nbinvars(2)}]==1 { set nbinvars(2) 0. }
		if [catch {expr $nbinvars(3)}]==1 { set nbinvars(3) 100. }
		# check zmin, zmax make sense
		if $nbinvars(3)<$nbinvars(2) {
		    tk_dialog .error "Error" "The Z values are not defined\
			    properly.  The maximum value must be  \
			    greater than the minimum. " error 0 OK
		    return 0
		}
	    } elseif $ichoice==8 {
		if [catch {expr $nbinvars(1)}]==1 { set nbinvars(1) $maxvals(nb) }
		if { [catch {expr $nbinvars(2)}]==1 | [catch {expr\
			$nbinvars(3)}]==1 | $nbinvars(3)<$nbinvars(2) | \
			$nbinvars(2)<=0 } {
		    tk_dialog .error "Error" "The minimum weight must be\
			    greater than 0 and the maximum weight \
			    greater than the minimum. " error 0 OK
		    return 0
		}
	    } elseif $ichoice==9 {
		if { [catch {expr $nbinvars(1)}]==1 | [catch {expr\
			$nbinvars(2)}]==1 | $nbinvars(2)<$nbinvars(1) | \
			$nbinvars(1)<0 | $nbinvars(2)<0 } {
		    tk_dialog .failed "Failed" "The minimum and maximum energies\
			    are not defined properly.  Emax>Emin>0." error 0 OK
		    return 0
		}
	    } elseif { $ichoice==3 | $ichoice==4 } {
		if [catch {expr $nbinvars(1)}]==1 { set nbinvars(1) $maxvals(nb) }
		if { [catch {expr $nbinvars(2)}]==1 | [catch {expr\
			$nbinvars(3)}]==1 | $nbinvars(3)<$nbinvars(2) | \
			$nbinvars(2)<0 | $nbinvars(3)<0 } {
		    tk_dialog .failed "Failed" "The minimum and maximum energies\
			    are not defined properly.  Emax>Emin>0." error 0 OK
		    return 0
		}
	    }

	    set nbinvars(1) [expr int($nbinvars(1))]

	    if { $ichoice==9 } {
		# Put emin, emax
		puts $id "$nbinvars(1), $nbinvars(2)"
	    } else {
		if { $ichoice == 6 | $ichoice == 7 } {
		    set max $maxvals(nbinangle)
		} else {
		    set max $maxvals(nb)
		}
		if { $nbinvars(1)>$max | $nbinvars(1)<1 } {
		    tk_dialog .failed "Failed" "The number of bins must be less\
			    than $max and greater than 0." error 0 OK
		    return 0
		}
		# Put nbin, emin, emax
		puts $id "$nbinvars(1), $nbinvars(2), $nbinvars(3)"
                if { $ichoice == 6 } {
                   # put emin, emax for angular distn
                   puts $id "$angevars(1), $angevars(2)"
                }
	    }
	}
	if { $ichoice>0 && $ichoice<10 } {

	    # get latch bit filters, if specified
	    if [catch {expr $lflag}]==1 {
		tk_dialog .failed "Failed" "The latch type is unspecified. \
			Specify and try again." error 0 OK
		return 0
	    }
	    if $lflag==4 {
		puts $id "0, 0, 0"
	    } else {
		puts $id "$lflag, $nbit1, $nbit2"
		switch $lflag {
		    0 { set len1 29; set len2 29; }
		    1 { set len1 29; set len2 0; }
		    2 { set len1 24; set len2 0; }
		    3 { set len1 24; set len2 0; }
		}
		if { $lflag==1 || $lflag==3 } {
                  if { $nbit1>0 } {
		    set str {}
		    for {set i 0} {$i<$len1} {incr i} {
			if $exc($i)==1 { set str "$str$i," }
		    }
		    puts $id $str
                  }
		} elseif { $nbit1>0 } {
		    set str {}
		    for {set i 0} {$i<$len1} {incr i} {
			if $inc($i)==1 { set str "$str$i, " }
		    }
		    puts $id $str
		}
		if $nbit2>0 {
		    set str {}
		    for {set i 0} {$i<$len2} {incr i} {
			if $exc($i)==1 { set str "$str$i, " }
		    }
		    puts $id $str
		}
	    }
	}
	if $ichoice==11 {
	    # Put numberp,charge
	    if { [catch {expr $fpar(1)}]==1 } { set fpar(1) 100 }
	    if $charge==100 {
		tk_dialog .failed "Failed" "The charge is not \
			specified.  Specify and try again." error 0 OK
		return 0
	    }
	    puts $id "$fpar(1), $charge"
	}

	if { $ichoice>0 && $ichoice<10 || $ichoice==11} {
	    # Put phase-space filename
	    if [file exists $phspfile]==0 {
		tk_dialog .failure "Failure" "The file $phspfile doesn't\
			exist!" error 0 OK
		return 0
	    }
	    puts $id $phspfile
	}

	if { $ichoice>0 && $ichoice<10 } {
	    # Put name of file to save to for plotting
	    if [string compare $spcnam {}]==0 {
		tk_dialog .failure "Failure" "You haven't specified a file\
			to output the xmgr data to."  error 0 OK
		return 0
	    }
	    puts $id $spcnam
	    if $ichoice<9 {
		# Put normal or histogam
		if [catch {expr $gflag}]==1 {
		    tk_dialog .failed "Failed" "The graph type is unspecified.\
			    Specify and try again." error 0 OK
		    return 0
		}
		puts $id $gflag
		if { $ichoice>=1 && $ichoice<=4 } {
		    # put real or planar fluence ( 0 or 1 )
		    if [catch {expr $fluflag}]==1 {
			tk_dialog .failed "Failed" "The fluence type is \
				unspecified.  Specify and try again." error 0 OK
			return 0
		    }
		    puts $id $fluflag
		} elseif { $ichoice==6 } {
                    # put particles/angular bin (0) or particles/unit solid
                    # angle (1)
                    if [catch {expr $angflag}]==1 {
                        tk_dialog .failed "Failed" "The angle plot type is \
                                unspecified.  Specify and try again." error 0 OK
                        return 0
                    }
                    puts $id $angflag
                }
	    } elseif $ichoice==9 {
                #put in max number of particles to plot
                puts $id $maxscatt
            }
	}

	# Now end with 0 to quit and 0 to not plot with xvgr before exiting.
	puts $id 0
	puts $id 0
    } else {
	if [file exists $phspfile]==0 {
	    tk_dialog .failure "Failure" "The file $phspfile doesn't\
		    exist!" error 0 OK
	    return 0
	}
	puts $id $phspfile
	if [file exists $phspfile2]==0 {
	    tk_dialog .failure "Failure" "The file $phspfile2 doesn't\
		    exist!" error 0 OK
	    return 0
	}
	puts $id $phspfile2
    }
    # add a blank line at the end, just for kicks
    puts $id {}
    close $id
    return 1
}

proc get_1_default { name } {
    global omega
    # read beamdp.mortran for default values.

    set filename $omega/progs/beamdp/beamdp.mortran

    set default1 {}
    if { [file readable $filename] } {
	set fileid [open $filename "r"]
	while { [eof $fileid]!=1  & [string compare $default1 {}]==0} {
	  gets $fileid data
	  set data [string trimright $data]
          if [string first "REPLACE" $data]>=0 {
	    if [string first "\{\$$name\}" $data]>0 {
		set start [string last "\{" $data]
		incr start
		set end [string last "\}" $data]
		incr end -1
		set default1 [string range $data $start $end]
	    }
          }
	}
	close $fileid
    } else {
	tk_dialog .nope "Too bad" "Couldn't get the defaults" info 0 OK
    }
    return $default1
}

proc help_dialog {w title text bitmap default args} {

    # help dialog box to put text in a scrolling window.

    global helvfont
    toplevel $w
    wm title $w "Help"

    frame $w.msg -bd 5

    # find the height required for the text:
    # 60 chars/line, max of 30 lines, min of 5 lines
    set len [string length $text]
    set height [expr $len/60 + 5]
    if $height>30 {set height 30}
    if $height<5 {set height 5}


    # text box for help text (white background):
    text $w.msg.text -font $helvfont -width 60 -height $height -wrap word\
	-relief sunken -bd 5 -yscrollcommand "$w.msg.v_scroll set" -bg white
    $w.msg.text insert 1.0 $text
    # no editing allowed:
    $w.msg.text configure -state disabled

    # vertical scrollbar:
    scrollbar $w.msg.v_scroll -command "$w.msg.text yview"

    pack $w.msg.text -side left
    pack $w.msg.v_scroll -side right -fill y
    pack $w.msg -side top

    frame $w.buts -bd 4
    button $w.buts.okb -text "OK" -command "destroy $w" \
	    -relief groove -bd 8
    pack $w.buts.okb -side left -padx 10
    pack $w.buts -pady 10
}

proc help_gif { w iconfile } {

    # THIS DIALOG IS FOR HELP WHEN A GIF FILE IS USED AS AN ICON

    global helvfont GUI_DIR
    toplevel $w -bd 8
    wm title $w "Help"
    wm geometry $w +100+100

    frame $w.f1 -bd 0 -relief sunken -bg white
    # Create the image from a gif file:
    if {$iconfile != ""} {
        image create photo graphic -file \
                [file join $GUI_DIR graphics $iconfile.gif]
        label $w.f1.lbl -image graphic -bg white
    } else {
        label $w.f1.lbl -text "No graphic for this source"
    }

    pack $w.f1.lbl -side left -padx 10 -pady 5 -fill both
    pack $w.f1
    frame $w.sep -bd 4 -width 100 -height 2 -relief groove
    pack $w.sep -pady 10 -fill x
    button $w.okb -text "OK" -command "destroy $w" \
            -relief groove -bd 8
    pack $w.okb -pady 10
}


# THESE DEFAULTS MAY NEED TO BE INCORPORATED SOMEWHERE
#REPLACE {$NS} WITH {22}  "maximum number of sub-sources for the source model
#REPLACE {$NB} WITH {200} "maximum number of bins for a distribution
#REPLACE {$ND} WITH {2000}  "maximum number of bins for a distribution
#REPLACE {$NBINANGLE} WITH {40}; "number of bins for the angular distribution"
#REPLACE {$MAXANGLE} WITH {0.1745};
set maxvals(nb) [get_1_default NB]
set maxvals(nbinangle) [get_1_default NBINANGLE]

