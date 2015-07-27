
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: set source
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
#                   Frederic Tessier
#
###############################################################################
#
#  Portions of this code were derived from the Quality Assurance tool written
#  by Blake Walters
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


proc set_src_options { isource } {

    # set the options for each source - 2*array (5,8) will
    # hold names, values for each option.

    global srcoptnames numsrcopts srcopts helvfont values inp_file_dir env
    global monoen spec_file enflag names options dsurround
    global dflag dflagopt pegs numopts names options iphspout phspoutopt
    global numthphi angfixed ang1 ang2 nang pang ivary thphidef
    global numsets iso1 iso2 iso3 ang1 ang2 ang3 dsource muI
    global i_dbs r_dbs ssd_dbs z_dbs tcl_platform imuphspout
    global the_beam_code the_input_file the_pegs_file esplit
    global i_MLC the_vcu_code the_phsp_file the_vcu_input_file

    toplevel .main_w.srcopt
    set top .main_w.srcopt
    wm title $top "Set source options"

    #following enables or disables or disables e_split input for
    #sources 2,8,9, or 10 in the case when the source input window is
    #open at the same time as the main inputs window and keyboard
    #focus is switched over to the source input window
    bind .main_w.srcopt <FocusIn> {config_esplit $values(18)}

    if {$isource==2 || $isource==8 || $isource==9 || $isource==10 || $isource==20 || $isource==21} {
	# enflag must be 2 or 3
	if { $enflag == "" || $enflag<2 || $enflag==4 } { set enflag 2 }
    } elseif $isource==4 {
	# source 4, enflag must be 4
	set enflag 4
    } else {
	# source 0, 1, 3, 6 or 7, enflag must be 0 or 1
	if { $enflag == "" || $enflag>1 } { set enflag 0 }
    }

    label $top.srclab -text "Source $values(4)" -font $helvfont
    pack $top.srclab -pady 10

    frame $top.optfrm
    set w $top.optfrm.mainopts
    frame $w -bd 5
    for {set i 0} {$i < $numsrcopts($isource)} {incr i} {
	frame $w.inp$i -width 400
	label $w.inp$i.label -text $srcoptnames($isource,$i)
	entry $w.inp$i.text  -width 10 \
		-relief sunken -textvariable srcopts($i)
	pack $w.inp$i.label -anchor w -side left -padx 5
	pack $w.inp$i.text -anchor e -side right -fill x -expand true
	pack $w.inp$i -side top -fill x
    }
    if {$isource==7 || $isource==8 || $isource==10} {
        #get theta-phi pairs:
        #radiobutton to choose between input of pairs and input of groups
        #of pairs

        frame $w.thphiprorgrp
        radiobutton $w.thphiprorgrp.radio1 -text "theta-phi pairs"\
           -variable thphidef -value 0\
           -command "$w.thphidef.but configure -text {define theta-phi pairs}\
                     -command {define_theta_phi} -state normal"
        radiobutton $w.thphiprorgrp.radio2 -text "theta-phi groups"\
           -variable thphidef -value 1\
           -command "$w.thphidef.but configure -text {define theta-phi groups}\
                     -command {define_theta_phi} -state normal"
        pack $w.thphiprorgrp.radio1 -side top -anchor w
        pack $w.thphiprorgrp.radio2 -side top -anchor w
        pack $w.thphiprorgrp -side top -anchor w

        frame $w.thphidef -bd 4
        if {$thphidef==0} {
          button $w.thphidef.but -text "define theta-phi pairs"\
                                 -command {define_theta_phi}
        } elseif {$thphidef==1} {
          button $w.thphidef.but -text "define theta-phi groups"\
                                 -command {define_theta_phi}
        } else {
          button $w.thphidef.but -text "select theta-phi pairs or groups above"\
                                 -state disabled
        }
        pack $w.thphidef.but -side top -anchor w
        pack $w.thphidef -side top -anchor w
    }
    if {$isource==20 || $isource==21} {
	#get settings
	frame $w.setdef -bd 4
	button $w.setdef.but -text "define settings"\
                                 -command {define_setting}
	pack $w.setdef.but -side top -anchor w
        pack $w.setdef -side top -anchor w
    }
    if {$isource==2 || $isource==8 ||$isource==20} {
        #put in option to exclude fat photons from source if DBS was
        #used to generate it

        frame $w.dbs -bd 4

        frame $w.dbs.idbs -width 700
        checkbutton $w.dbs.idbs.but -text "DBS used to generate source" \
                -variable i_dbs -command "config_dbs $w.dbs"
        pack $w.dbs.idbs.but -side top -anchor w
        pack $w.dbs.idbs -side top -anchor w

        frame $w.dbs.rdbs
        label $w.dbs.rdbs.label -text "DBS splitting field radius (cm)"
        entry $w.dbs.rdbs.text -width 10 -relief sunken -textvariable r_dbs
        pack $w.dbs.rdbs.label -anchor w -side left
        pack $w.dbs.rdbs.text -anchor e -side right -fill x -expand true
        pack $w.dbs.rdbs -side top -fill x

        frame $w.dbs.ssddbs -width 700
        label $w.dbs.ssddbs.label -text "SSD of splitting field (cm)"
        entry $w.dbs.ssddbs.text -width 10 -relief sunken -textvariable ssd_dbs
        pack $w.dbs.ssddbs.label -anchor w -side left
        pack $w.dbs.ssddbs.text -anchor e -side right -fill x -expand true
        pack $w.dbs.ssddbs -side top -fill x

        frame $w.dbs.zdbs -width 700
        label $w.dbs.zdbs.label -text "Z where source scored (cm)"
        entry $w.dbs.zdbs.text -width 10 -relief sunken -textvariable z_dbs
        pack $w.dbs.zdbs.label -anchor w -side left
        pack $w.dbs.zdbs.text -anchor e -side right -fill x -expand true
        pack $w.dbs.zdbs -side top -fill x

        if {$i_dbs != 1} {
           set i_dbs 0
           $w.dbs.rdbs.label configure -fg grey
           $w.dbs.rdbs.text configure -state disabled
           $w.dbs.ssddbs.label configure -fg grey
           $w.dbs.ssddbs.text configure -state disabled
           $w.dbs.zdbs.label configure -fg grey
           $w.dbs.zdbs.text configure -state disabled
        }

        button $w.dbshelp -text "?" -command "help dbs"

        pack $w.dbshelp -anchor w -side left
        pack $w.dbs -side right -pady 10 -anchor e -fill x -expand true
    }

    pack $w -side left -anchor n

    if {$isource!=2 && $isource!=4 && $isource<8} {
  # Not source 2, 8, 9, 10 so enflag can be 0 or 1, monoenergetic or spectrum.
  # Put in radiobuttons, disabling one or the other option.

	frame $top.optfrm.not2 -bd 4
	label $top.optfrm.not2.label -text \
		"Specify source beam energy or energy spectrum filename:"\
		-font $helvfont
	pack $top.optfrm.not2.label  -anchor w

	# left side is for monoenergetic:
	frame $top.optfrm.not2.left
	set w $top.optfrm.not2.left
	radiobutton $w.radio1 -text "monoenergetic" \
		-variable enflag -value 0\
		-command { enable 0 }
	pack $w.radio1 -anchor w -pady 5
	label $w.label -text "Kinetic energy of beam (MeV)"
	entry $w.text -width 10 -relief sunken \
		-textvariable Ein
	pack $w.label -anchor w -side left -padx 5
	pack $w.text -anchor e -side left -fill x -expand true

	pack $w -pady 10 -anchor w

	label $top.optfrm.not2.orlabel -text "             OR" -font $helvfont
	pack $top.optfrm.not2.orlabel -side top -pady 10 -anchor w

	# bottom is for spectrum
	frame $top.optfrm.not2.right
	set w $top.optfrm.not2.right
	radiobutton $w.radio2 -text "spectrum" \
		-variable enflag -value 1\
		-command { enable 1 }
	pack $w.radio2 -anchor w -pady 5

	frame $w.f
	frame $w.f.text
	label $w.f.text.label -text "Spectrum filename (complete):"
	entry $w.f.text.text -width 40 -relief sunken \
		-textvariable spec_file
	pack $w.f.text.label $w.f.text.text -anchor w
	frame $w.f.buts
	button $w.f.buts.b1 -text "Browse current directory" \
		-command "browse set_spec_file $inp_file_dir"
	button $w.f.buts.b2 -text "Browse HEN_HOUSE spectra" \
		-command "browse set_spec_file $env(HEN_HOUSE)/spectra"
	pack $w.f.buts.b1 $w.f.buts.b2 -side top -fill x -anchor w
	pack $w.f.text $w.f.buts -side left -padx 5

	pack $w.f -pady 5

	pack $w -pady 10
	pack $top.optfrm.not2 -side right

    } else {
	# it's source 2, 4, 8, 9, 10, 20 21 and we need to get one of 2 filenames
        # (3 in the case of source 9, 10 and 5 in the case of 21).
	# Allow the user to choose
	# here whether (s)he wants to score a dose component (LATCH filter)
	# or multiple sources (?) and also put on the file format menu.

	set ww $top.optfrm.src2
	frame $ww

	if {$isource==2 || $isource==8} {
	    # if it's source 2 or 8, we need radiobuttons to select between
	    # enflag options (2 or 3).
	    set w $ww.psfile
	    frame $w
	    radiobutton $w.radio -text "Phase space beam input\
		    (with no LATCH filter)"\
		    -variable enflag -value 2 -command {enable 2}
	    pack $w.radio -side top -anchor w
	    pack $w  -anchor w

	    set w $ww.psdosefile
	    frame $w
	    radiobutton $w.radio -text \
		    "Phase space beam input + dose component (with LATCH filter)"\
		    -variable enflag -value 3 -command {enable 3}
	    pack $w.radio -side top -anchor w
	    pack $w -anchor w

	    label $w.flab -text "File containing phase space data:"
	    pack $w.flab -side top -anchor w -pady 5
	} elseif {$isource==9 || $isource==10 || $isource==21} {
            # radiobuttons for selecting enflag=2 or 3
            set w $ww.beamsim
            frame $w
            radiobutton $w.radio -text "BEAM simulation input\
                    (with no LATCH filter)"\
                    -variable enflag -value 2 -command {enable 2}
            pack $w.radio -side top -anchor w
            pack $w  -anchor w

            set w $ww.beamsimfilt
            frame $w
            radiobutton $w.radio -text \
                    "BEAM simulation + dose component (with LATCH filter)"\
                    -variable enflag -value 3 -command {enable 3}
            pack $w.radio -side top -anchor w
            pack $w -anchor w
        } elseif {$isource==20} {
            # radiobuttons for selecting enflag=2 or 3
            set w $ww.vcusim
            frame $w
            radiobutton $w.radio -text "Phase space beam input\
                   (with no LATCH filter)"\
                    -variable enflag -value 2 -command {enable 2}
            pack $w.radio -side top -anchor w
            pack $w  -anchor w

            set w $ww.vcusimfilt
            frame $w
            radiobutton $w.radio -text \
                    "Phase space beam input + dose component (with LATCH filter)"\
                    -variable enflag -value 3 -command {enable 3}
            pack $w.radio -side top -anchor w
            pack $w -anchor w

	    frame $ww.mode
	    button $ww.mode.help -text "?" -command "help 20"
	    label $ww.mode.label -text "Phase space file format"
	    menubutton $ww.mode.inp -text $values(20) -width 20\
		-menu $ww.mode.inp.m -bd 1 -relief raised -indicatoron 1
	    # Now make the menu, and add the lines one at a time
	    menu $ww.mode.inp.m -tearoff no
	    $ww.mode.inp.m add command -label $options(20,0) -command\
		"set values(20) {$options(20,0)}; $ww.mode.inp configure\
		-text {$options(20,0)}"
	    $ww.mode.inp.m add command -label $options(20,2) -command\
		"set values(20) {$options(20,2)}; $ww.mode.inp configure\
		-text {$options(20,2)}"
	    pack $ww.mode.help -side left -anchor w
	    pack $ww.mode.label -side left -padx 10
	    pack $ww.mode.inp -padx 5 -pady 5  -fill x -expand true
	    pack $ww.mode -anchor e
        } else {
	    label $ww.flab -text "File containing source parameters from BEAMDP:"
	    pack $ww.flab -side top -anchor w -pady 5
	}

        if {$isource!=9 && $isource!=10 && $isource!=20 && $isource!=21} {
	    frame $ww.f
	    entry $ww.f.text -width 40 -relief sunken -textvariable spec_file
	    frame $ww.f.buts
	    button $ww.f.buts.b1 -text "Browse current directory" \
		-command "browse set_spec_file $inp_file_dir"
	    pack $ww.f.buts.b1 -side top -fill x -anchor w
	    pack $ww.f.text -side left -anchor w -expand true -fill x
	    pack $ww.f.buts -side right -padx 5 -anchor e
	    pack $ww.f -fill x

	    frame $ww.ismooth
	    button $ww.ismooth.help -text "?" -command "help 11"
	    label $ww.ismooth.label -text $names(11)
	    menubutton $ww.ismooth.inp -text $values(11) -width 20\
		-menu $ww.ismooth.inp.m -bd 1 -relief raised -indicatoron 1
	    # Now make the menu, and add the lines one at a time
	    menu $ww.ismooth.inp.m -tearoff no
	    $ww.ismooth.inp.m add command -label $options(11,0) -command\
		"set values(11) {$options(11,0)}; $ww.ismooth.inp configure\
		-text {$options(11,0)}"
	    $ww.ismooth.inp.m add command -label $options(11,1) -command\
		"set values(11) {$options(11,1)}; $ww.ismooth.inp configure\
		-text {$options(11,1)}"
	    pack $ww.ismooth.help -side left -anchor w
	    pack $ww.ismooth.label -side left -padx 10
	    pack $ww.ismooth.inp -padx 5 -pady 5  -fill x -expand true
	    pack $ww.ismooth -anchor e

	    frame $ww.mode
	    button $ww.mode.help -text "?" -command "help 20"
	    label $ww.mode.label -text "Phase space file format"
	    menubutton $ww.mode.inp -text $values(20) -width 20\
		-menu $ww.mode.inp.m -bd 1 -relief raised -indicatoron 1
	    # Now make the menu, and add the lines one at a time
	    menu $ww.mode.inp.m -tearoff no
	    $ww.mode.inp.m add command -label $options(20,0) -command\
		"set values(20) {$options(20,0)}; $ww.mode.inp configure\
		-text {$options(20,0)}"
	    $ww.mode.inp.m add command -label $options(20,2) -command\
		"set values(20) {$options(20,2)}; $ww.mode.inp configure\
		-text {$options(20,2)}"
	    pack $ww.mode.help -side left -anchor w
	    pack $ww.mode.label -side left -padx 10
	    pack $ww.mode.inp -padx 5 -pady 5  -fill x -expand true
	    pack $ww.mode -anchor e
        }

	pack $ww -side left -pady 5 -anchor n

	if {$isource==2 || $isource==8 || $isource==9 || $isource==10 || $isource==20 || $isource==21} {
	    # for source 2, 8, 9, or 10 put on latch filter option.
	    frame $ww.latch
	    button $ww.latch.help -text "?" -command "help 21"
	    label $ww.latch.lab -text $names(21)
	    menubutton $ww.latch.inp -text $values(21) -width 20\
		    -menu $ww.latch.inp.m -bd 1 -relief raised -indicatoron 1
	    # Now make the menu, and add the lines one at a time
	    menu $ww.latch.inp.m -tearoff no
	    set command(0) "set_inc_or_exc .main_w 0"
	    set command(1) "set_inc_or_exc .main_w 1"
	    set command(2) "set_inc_or_exc .main_w 2"
	    set command(3) "set_inc_or_exc .main_w 3"
	    for {set j 0} {$j<4} {incr j} {
		$ww.latch.inp.m add command -label $options(21,$j)\
			-command "set values(21) {$options(21,$j)};\
			$ww.latch.inp configure -text {$options(21,$j)};\
			$command($j)"
	    }
	    pack $ww.latch.help -side left -anchor w -padx 5
	    pack $ww.latch.lab -side left
	    pack $ww.latch.inp -side right -padx 5 -fill x -expand true
            if {$isource!=9} {
	       pack $ww.latch -anchor se -pady 5
            } else {
               pack $ww.latch -anchor sw -pady 5
            }
	}

        if {$isource==2 || $isource==8} {
           #put in option to split electrons if n_split>1
           frame $ww.esplit
           button $ww.esplit.help -text "?" -command "help esplit"
           label $ww.esplit.lab -text "no. of times to split e+/e-"
           entry $ww.esplit.inp -width 25 -relief sunken -textvariable esplit
           pack $ww.esplit.help -side left -anchor w
           pack $ww.esplit.lab -side left -padx 10
           pack $ww.esplit.inp -padx 5 -fill x -expand true
           pack $ww.esplit -pady 5 -anchor e
           if {$values(18)<=1} {
               $ww.esplit.inp configure -state disabled
               $ww.esplit.lab configure -fg grey
           }
        }
        if {$isource==9 || $isource==10} {
            #put in option to exclude fat photons from source if DBS was
            #used to generate it
            #we don't need DBS radius, SSD or Z of source

            frame $ww.dbs
            checkbutton $ww.dbs.idbs -text "Exclude fat photons from DBS" \
                -variable i_dbs
            button $ww.dbs.help -text "?" -command "help dbs_forsrc9"
            pack $ww.dbs.help -side left
            pack $ww.dbs.idbs -side left
            pack $ww.dbs -side top -anchor e -fill x -expand true -padx 5

            #put in option to split electrons if n_split>1
            frame $ww.esplit
            button $ww.esplit.help -text "?" -command "help esplit"
            label $ww.esplit.lab -text "no. of times to split e+/e-"
            entry $ww.esplit.inp -width 15 -relief sunken -textvariable esplit
            pack $ww.esplit.help -side left -anchor w
            pack $ww.esplit.lab -side left -padx 10
            pack $ww.esplit.inp -padx 5 -fill x -expand true
            pack $ww.esplit -anchor e -padx 5 -pady 4
            if {$values(18)<=1} {
               $ww.esplit.inp configure -state disabled
               $ww.esplit.lab configure -fg grey
            }

            #now, make another frame for entering all required files

            set w $top.files
            frame $w -bd 10

            set my_machine [get_config_value "my_machine"]
            set egs_home $env(EGS_HOME)
            set loc_bindir [file join $egs_home bin $my_machine]
            if { "$tcl_platform(platform)"=="windows" } {
                  set libext dll
            } else {
                  set libext so
            }
            label $w.beamsimlab -text "BEAM simulation:"
            entry $w.beamsiment -width 40 -relief sunken -textvariable the_beam_code
            button $w.beamsimbrowse -text "Browse EGS_HOME/bin/$my_machine" \
                -command "query_filename set_beam_code $loc_bindir $libext"

            label $w.inpfilelab -text "input file (no ext.):"
            entry $w.inpfileent -width 40 -relief sunken -textvariable the_input_file
            if {"$the_beam_code"==""} {
               button $w.inpfilebrowse -text "Select BEAM simulation first" \
                -command "query_filename set_input_file $egs_home egsinp" \
                -state disabled
               set beamdir {}
            } else {
               set beamdir [file join $egs_home $the_beam_code]
               button $w.inpfilebrowse -text "Browse EGS_HOME/$the_beam_code" \
                  -command "query_filename set_input_file $beamdir egsinp"
            }

            set hen_house $env(HEN_HOUSE)
            set pegs_area [file join $hen_house pegs4 data]
            set user_pegs_area [file join $egs_home pegs4 data]
            label $w.pegsfilelab -text "pegs data (no ext.):"
            entry $w.pegsfileent -width 40 -relief sunken -textvariable the_pegs_file
            frame $w.pegsfilebrowse
            button $w.pegsfilebrowse.b1 -text "Browse HEN_HOUSE pegs4 area" \
                -command "query_filename set_pegs_file $pegs_area pegs4dat"
            pack $w.pegsfilebrowse.b1 -side top -fill x -anchor w
            button $w.pegsfilebrowse.b2 -text "Browse users pegs4 area" \
                -command "query_filename set_pegs_file $user_pegs_area pegs4dat"
            pack $w.pegsfilebrowse.b2 -side top -fill x -anchor w

            grid config $w.beamsimlab -column 0 -row 0 -sticky e
            grid config $w.inpfilelab -column 0 -row 1 -sticky e
            grid config $w.pegsfilelab -column 0 -row 2 -sticky e
            grid config $w.beamsiment -column 1 -row 0 -sticky w
            grid config $w.inpfileent -column 1 -row 1 -sticky w
            grid config $w.pegsfileent -column 1 -row 2 -sticky w
            grid config $w.beamsimbrowse -column 2 -row 0 -sticky w -pady 5
            grid config $w.inpfilebrowse -column 2 -row 1 -sticky w -pady 5
            grid config $w.pegsfilebrowse -column 2 -row 2 -sticky w -pady 5

        }
	if {$isource==20} {

#            frame $ww.dbs
#            checkbutton $ww.dbs.idbs -text "Exclude fat photons from DBS" \
#                -variable i_dbs
#           button $ww.dbs.help -text "?" -command "help dbs"
#           pack $ww.dbs.help -side left
#           pack $ww.dbs.idbs -side left
#           pack $ww.dbs -side top -anchor e -fill x -expand true -padx 5

            #put in option to split electrons if n_split>1
            frame $ww.esplit
            button $ww.esplit.help -text "?" -command "help esplit"
            label $ww.esplit.lab -text "no. of times to split e+/e-"
            entry $ww.esplit.inp -width 15 -relief sunken -textvariable esplit
            pack $ww.esplit.help -side left -anchor w
            pack $ww.esplit.lab -side left -padx 10
            pack $ww.esplit.inp -padx 5 -fill x -expand true
            pack $ww.esplit -anchor e -padx 5 -pady 4
            if {$values(18)<=1} {
               $ww.esplit.inp configure -state disabled
               $ww.esplit.lab configure -fg grey
            }

            #put in option to include fractional MU in phsp output (if any)
            frame $ww.muphspout
            button $ww.muphspout.help -text "?" -command "help muphspout"
            checkbutton $ww.muphspout.but -text "Include fractional MU in any phase space output" \
                -variable imuphspout
            pack $ww.muphspout.help $ww.muphspout.but -side left
            pack $ww.muphspout -anchor e -padx 5 -pady 4

            #now, make another frame for entering all required files

            set w $top.files
            frame $w -bd 10

            set my_machine [get_config_value "my_machine"]
            set egs_home $env(EGS_HOME)
            set loc_bindir [file join $egs_home bin $my_machine]
            if { "$tcl_platform(platform)"=="windows" } {
                  set libext dll
            } else {
                  set libext so
            }

	    label $w.phsplab -text "phase space file:"
            entry $w.phspent -width 40 -relief sunken -textvariable the_phsp_file
	    button $w.phspbrowse -text "Browse current directory" \
                -command "browse set_phsp_file $inp_file_dir"

        if { "$the_vcu_code"=="0" || "$the_vcu_code"=="" } {
            label $w.vcusimlab -text "BEAMnrc or external library:"
        } elseif { "$the_vcu_code"=="particleDmlc" } {
            label $w.vcusimlab -text "External library:"
        } else {
            label $w.vcusimlab -text "BEAMnrc library:"
        }
            entry $w.vcusiment -width 40 -relief sunken -textvariable the_vcu_code
            button $w.vcusimbrowse -text "Browse EGS_HOME/bin/$my_machine" \
		-command "query_filename set_vcu_code $loc_bindir $libext"

	    checkbutton $w.mlcbut -text "using BEAMnrc or external library" \
                -variable i_MLC -command "config_MLC $w"

	    if {"$the_vcu_code"=="0" || "$the_vcu_code"==""} {
		label $w.inpfilevculab -text "BEAMnrc or external library input file:"
		entry $w.inpfilevcuent -width 40 -relief sunken -textvariable the_vcu_input_file
		button $w.inpfilevcubrowse -text "Select shared library simulation first" \
		    -command "query_filename set_beam_input_file $egs_home egsinp" \
		    -state disabled
		set beamdir {}
            } elseif { "$the_vcu_code"=="particleDmlc" } {
		label $w.inpfilevculab -text "External library input file (full path):"
		entry $w.inpfilevcuent -width 40 -relief sunken -textvariable the_vcu_input_file
		button $w.inpfilevcubrowse -text "Browse EGS_HOME" \
		    -command "browse set_vcu_input_file $egs_home"
	    } else {
		set beamdir [file join $egs_home $the_vcu_code]
		label $w.inpfilevculab -text "BEAMnrc library input file:"
		entry $w.inpfilevcuent -width 40 -relief sunken -textvariable the_vcu_input_file
		button $w.inpfilevcubrowse -text "Browse EGS_HOME/$the_vcu_code" \
		    -command "query_filename set_beam_input_file $beamdir egsinp"
	    }

	    if {$i_MLC != 1} {

#		set the_vcu_code 0
#		set the_vcu_input_file 0

		$w.vcusimlab configure -fg grey
		$w.inpfilevculab configure -fg grey

		$w.vcusiment configure -state disabled
		$w.inpfilevcuent configure -state disabled

		$w.vcusimbrowse configure -state disabled
		$w.inpfilevcubrowse configure -state disabled
	    }

            set hen_house $env(HEN_HOUSE)
            set pegs_area [file join $hen_house pegs4 data]
            set user_pegs_area [file join $egs_home pegs4 data]

	    grid config $w.phsplab -column 0 -row 0 -sticky e
            grid config $w.vcusimlab -column 0 -row 2 -sticky e
            grid config $w.inpfilevculab -column 0 -row 3 -sticky e
            grid config $w.phspent -column 1 -row 0 -sticky w
	    grid config $w.mlcbut -column 1 -row 1 -sticky w -pady 5
            grid config $w.vcusiment -column 1 -row 2 -sticky w
	    grid config $w.inpfilevcuent -column 1 -row 3 -sticky w
            grid config $w.phspbrowse -column 2 -row 0 -sticky w -pady 5
            grid config $w.vcusimbrowse -column 2 -row 2 -sticky w -pady 5
	    grid config $w.inpfilevcubrowse -column 2 -row 3 -sticky w -pady 5
        }
	if {$isource==21} {
	    #put in option to exclude fat photons from source if DBS was
            #used to generate it
            #we don't need DBS radius, SSD or Z of source

            frame $ww.dbs
            checkbutton $ww.dbs.idbs -text "Exclude fat photons from DBS" \
                -variable i_dbs
            button $ww.dbs.help -text "?" -command "help dbs_forsrc9"
            pack $ww.dbs.help -side right
            pack $ww.dbs.idbs -side right
            pack $ww.dbs -side top -anchor e -fill x -expand true -padx 5

            #put in option to split electrons if n_split>1
            frame $ww.esplit
            button $ww.esplit.help -text "?" -command "help esplit"
            label $ww.esplit.lab -text "no. of times to split e+/e-"
            entry $ww.esplit.inp -width 15 -relief sunken -textvariable esplit
            pack $ww.esplit.help -side left -anchor w
            pack $ww.esplit.lab -side left -padx 10
            pack $ww.esplit.inp -padx 5 -fill x -expand true
            pack $ww.esplit -anchor e -padx 5 -pady 4
            if {$values(18)<=1} {
               $ww.esplit.inp configure -state disabled
               $ww.esplit.lab configure -fg grey
            }

            #put in option to include fractional MU in phsp output (if any)
            frame $ww.muphspout
            button $ww.muphspout.help -text "?" -command "help muphspout"
            checkbutton $ww.muphspout.but -text "Include fractional MU in any phase space output" \
                -variable imuphspout
            pack $ww.muphspout.help $ww.muphspout.but -side left
            pack $ww.muphspout -anchor e -padx 5 -pady 4

            #now, make another frame for entering all required files

            set w $top.files
            frame $w -bd 10

            set my_machine [get_config_value "my_machine"]
            set egs_home $env(EGS_HOME)
            set loc_bindir [file join $egs_home bin $my_machine]
            if { "$tcl_platform(platform)"=="windows" } {
                  set libext dll
            } else {
                  set libext so
            }
            label $w.beamsimlab -text "BEAM simulation:"
            entry $w.beamsiment -width 40 -relief sunken -textvariable the_beam_code
            button $w.beamsimbrowse -text "Browse EGS_HOME/bin/$my_machine" \
                -command "query_filename set_beam_code $loc_bindir $libext"

            label $w.inpfilelab -text "input file (no ext.):"
            entry $w.inpfileent -width 40 -relief sunken -textvariable the_input_file
            if {"$the_beam_code"==""} {
               button $w.inpfilebrowse -text "Select BEAM simulation first" \
                -command "query_filename set_input_file $egs_home egsinp" \
                -state disabled
               set beamdir {}
            } else {
               set beamdir [file join $egs_home $the_beam_code]
               button $w.inpfilebrowse -text "Browse EGS_HOME/$the_beam_code" \
                  -command "query_filename set_input_file $beamdir egsinp"
            }

	    checkbutton $w.mlcbut -text "using external MLC library" \
                -variable i_MLC -command "config_MLC $w"

	    label $w.vcusimlab -text "MLC library:"
            entry $w.vcusiment -width 40 -relief sunken -textvariable the_vcu_code
            button $w.vcusimbrowse -text "Browse EGS_HOME/bin/$my_machine" \
                -command "query_filename set_vcu_code $loc_bindir $libext"

	    label $w.inpfilevculab -text "full path to MLC input file:"
            entry $w.inpfilevcuent -width 40 -relief sunken -textvariable the_vcu_input_file
	    button $w.inpfilevcubrowse -text "Browse EGS_HOME" \
		-command "browse set_vcu_input_file $egs_home"

	    if {$i_MLC != 1} {
#		set the_vcu_code 0
#		set the_vcu_input_file 0

		$w.vcusimlab configure -fg grey
		$w.inpfilevculab configure -fg grey

		$w.vcusiment configure -state disabled
		$w.inpfilevcuent configure -state disabled

		$w.vcusimbrowse configure -state disabled
		$w.inpfilevcubrowse configure -state disabled
	    }

            set hen_house $env(HEN_HOUSE)
            set pegs_area [file join $hen_house pegs4 data]
            set user_pegs_area [file join $egs_home pegs4 data]
            label $w.pegsfilelab -text "pegs data (no ext.):"
            entry $w.pegsfileent -width 40 -relief sunken -textvariable the_pegs_file
            frame $w.pegsfilebrowse
            button $w.pegsfilebrowse.b1 -text "Browse HEN_HOUSE pegs4 area" \
                -command "query_filename set_pegs_file $pegs_area pegs4dat"
            pack $w.pegsfilebrowse.b1 -side top -fill x -anchor w
            button $w.pegsfilebrowse.b2 -text "Browse users pegs4 area" \
                -command "query_filename set_pegs_file $user_pegs_area pegs4dat"
            pack $w.pegsfilebrowse.b2 -side top -fill x -anchor w

            grid config $w.beamsimlab        -column 0 -row 0 -sticky e
            grid config $w.inpfilelab        -column 0 -row 1 -sticky e
            grid config $w.pegsfilelab       -column 0 -row 2 -sticky e
	    grid config $w.vcusimlab         -column 0 -row 4 -sticky e
            grid config $w.inpfilevculab     -column 0 -row 5 -sticky e
            grid config $w.beamsiment        -column 1 -row 0 -sticky w
            grid config $w.inpfileent        -column 1 -row 1 -sticky w
            grid config $w.pegsfileent       -column 1 -row 2 -sticky w
	    grid config $w.mlcbut            -column 1 -row 3 -sticky w -pady 5
	    grid config $w.vcusiment         -column 1 -row 4 -sticky w
            grid config $w.inpfilevcuent     -column 1 -row 5 -sticky w
            grid config $w.beamsimbrowse     -column 2 -row 0 -sticky w -pady 5
            grid config $w.inpfilebrowse     -column 2 -row 1 -sticky w -pady 5
            grid config $w.pegsfilebrowse    -column 2 -row 2 -sticky w -pady 5
	    grid config $w.vcusimbrowse      -column 2 -row 4 -sticky w -pady 5
            grid config $w.inpfilevcubrowse  -column 2 -row 5 -sticky w -pady 5

	}
    }

    pack $top.optfrm -side top -pady 5
    if {$isource==9 || $isource==10 ||$isource==20 || $isource==21 } { pack $top.files -side top -anchor w }

    enable $enflag

    #put on an ok button to save the values
    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "check_src_inputs $isource"\
	    -relief groove -bd 8
    button $top.buts.helpb -relief groove -bd 8 -text "Help" \
	    -command "help_srcopts .main_w.srcopt $isource"
    pack $top.buts.okb $top.buts.helpb -side left -padx 10
    pack $top.buts

    #output some warnings for source 9 ,10 and 21
    if {$isource==9 || $isource==10 || $isource==21} {
      if {![file exists $loc_bindir]} {
          tk_dialog .bindirexist "Warning" "Directory\
               EGS_HOME/bin/$my_machine does not exist.  All executables\
               and BEAM\
               libraries (for use as sources) are kept here.  Browsing\
               will start at EGS_HOME/bin." error 0 OK
               set loc_bindir [file join $egs_home bin]
               $w.beamsimbrowse configure -text "Browse EGS_HOME/bin" \
                -command "query_filename set_beam_code $loc_bindir $libext"
      }
      if {"$beamdir" != "" && ![file exists $beamdir]} {
          tk_dialog .acceldirexist "Warning" "Directory\
               EGS_HOME/$the_beam_code does not exist.  BEAM Input files\
               must be in this directory.  Will browse from\
               EGS_HOME." error 0 OK
               $w.inpfilebrowse configure -text "Browse EGS_HOME" \
               -command "query_filename set_input_file $egs_home egsinp"
      }
    }
}

proc config_dbs { w } {

#enable or disable inputs for r_dbs, ssd_dbs, z_dbs based on i_dbs

    global i_dbs

    if {$i_dbs == 0} {
           $w.rdbs.label configure -fg grey
           $w.rdbs.text configure -state disabled
           $w.ssddbs.label configure -fg grey
           $w.ssddbs.text configure -state disabled
           $w.zdbs.label configure -fg grey
           $w.zdbs.text configure -state disabled
    } elseif {$i_dbs == 1} {
           $w.rdbs.label configure -fg black
           $w.rdbs.text configure -state normal
           $w.ssddbs.label configure -fg black
           $w.ssddbs.text configure -state normal
           $w.zdbs.label configure -fg black
           $w.zdbs.text configure -state normal
    }
}

proc config_MLC { w } {

#enable or disable inputs for vcu code based on flag

    global i_MLC

    if {$i_MLC == 0} {

#	   set the_vcu_code 0
#	   set the_vcu_input_file 0

           $w.vcusimlab configure -fg grey
           $w.inpfilevculab configure -fg grey

           $w.vcusiment configure -state disabled
	   $w.inpfilevcuent configure -state disabled

           $w.vcusimbrowse configure -state disabled
	   $w.inpfilevcubrowse configure -state disabled
    } elseif {$i_MLC == 1} {
           $w.vcusimlab configure -fg black
           $w.inpfilevculab configure -fg black

           $w.vcusiment configure -state normal
	   $w.inpfilevcuent configure -state normal

           $w.vcusimbrowse configure -state normal
	   $w.inpfilevcubrowse configure -state normal

    }
}
proc check_src_inputs { srcnum } {

#do checking on source inputs, for now, only checks angles in source 0

    global srcopts

    if {$srcnum==0} {
        set u [expr $srcopts(4)*3.1415927/180]
        set u [expr cos($u)]
        set v [expr $srcopts(5)*3.1415927/180]
        set v [expr cos($v)]
        set w [expr $srcopts(6)*3.1415927/180]
        set w [expr cos($w)]
        if {[expr $u*$u+$v*$v+$w*$w]>1.001 || [expr $u*$u+$v*$v+$w*$w]<0.999} {
            tk_dialog .error "Error" "Error in Source 0:  \
                    Sum of squares of cosines of incident angles\
                    must be 1." error 0 OK
        } else {
            destroy .main_w.srcopt
        }
    } else {
       destroy .main_w.srcopt
    }
}


proc enable { flag } {
    # enable/disable as needed (first time, radiobuttons do the rest)
    if $flag==0 {
	.main_w.srcopt.optfrm.not2.right.f.buts.b1 configure -state disabled
	.main_w.srcopt.optfrm.not2.right.f.buts.b2 configure -state disabled
	.main_w.srcopt.optfrm.not2.right.f.text.text configure -state disabled
	.main_w.srcopt.optfrm.not2.left.text configure -state normal
    } elseif $flag==1 {
	.main_w.srcopt.optfrm.not2.right.f.buts.b1 configure -state normal
	.main_w.srcopt.optfrm.not2.right.f.buts.b2 configure -state normal
	.main_w.srcopt.optfrm.not2.right.f.text.text configure -state normal
	.main_w.srcopt.optfrm.not2.left.text configure -state disabled
   } elseif $flag==2 {
	.main_w.srcopt.optfrm.src2.latch.inp configure\
		-state disabled
	.main_w.srcopt.optfrm.src2.latch.lab configure\
		-fg grey
    } elseif $flag==3 {
	.main_w.srcopt.optfrm.src2.latch.inp configure\
		-state normal
	.main_w.srcopt.optfrm.src2.latch.lab configure\
		-fg black
    }
}


proc set_inc_or_exc { parent index } {

    # window to create the inclusive and/or exclusive filter.

    global helvfont inc exc

    toplevel $parent.inc_or_exc
    wm title $parent.inc_or_exc "LATCH bit filter"

    if { $index==0 } {
	set text "For any given particle, if any of the inclusive bits\
	    are set and none of the exclusive bits are set, the particle is\
	    used."
	set len1 23
	set len2 23
    } elseif { $index==1 } {
	set text "For any given particle, if any of the bits selected are set, \
		the particle is not used."
	set len1 28
	set len2 0
    } elseif {$index==2 } {
	set text "Use only secondary particles originating in regions\
		with the following IREGION_TO_BIT values."
	set len1 23
	set len2 0
    } elseif {$index==3} {
	set text "Exclude secondary particles originating in regions\
	    with the following IREGION_TO_BIT values."
	set len1 0
	set len2 23
    }

    set top $parent.inc_or_exc.frm
    frame $top -bd 4
    label $top.lab -text $text -font $helvfont
    pack $top.lab -side top -pady 5

    # frame to hold a grid
    frame $top.frm -bd 4

    if $len1>0 {
	if $index==1 {
	    set type exc
	} else {
	    set type inc
	}
	label $top.frm.inclab -text "${type}lusive bits:" -font $helvfont
	grid configure $top.frm.inclab -row 0 -column 0 -padx 10

	frame $top.frm.bits1 -relief groove -bd 4
	for {set j 1} {$j<=$len1} {incr j} {
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
	for {set j 1} {$j<=$len2} {incr j} {
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

    button $top.b5 -text "Done" -command "close_latbit $index"\
	    -relief groove -bd 8
    pack $top.b5 -side top -pady 10
    pack $top
}

proc close_latbit { index } {
    global inc exc latbit nbit1 nbit2

    set nbit1 0
    set nbit2 0
    if $index==0 {
	for {set i 1} {$i<29} {incr i} {
	    if $inc($i)==1 { incr nbit1 }
	    if $exc($i)==1 { incr nbit2 }
	}
	if [expr $nbit1+$nbit2]>29 {
	    tk_dialog .error "Error" "Too many bits have been selected.  \
		    You'll have to not use so many." error 0 OK
	    return
	} else {
	    set j1 0
	    set j2 0
	    for {set i 1} {$i<29} {incr i} {
		if $inc($i)==1 { incr j1; set latbit($j1) $i }
		if $exc($i)==1 { incr j2; set id [expr $nbit1+$j2];\
			set latbit($id) $i }
	    }
	}
    } elseif $index==1 {
	for {set i 1} {$i<29} {incr i} {
	    if $exc($i)==1 { incr nbit1 }
	}
	set j1 0
	for {set i 1} {$i<29} {incr i} {
	    if $exc($i)==1 { incr j1; set latbit($j1) $i }
	}
    } elseif $index==2 {
	for {set i 1} {$i<24} {incr i} {
	    if $inc($i)==1 { incr nbit1 }
	}
	set j1 0
	for {set i 1} {$i<24} {incr i} {
	    if $inc($i)==1 { incr j1; set latbit($j1) $i }
	}
    } elseif $index==3 {
	for {set i 1} {$i<24} {incr i} {
	    if $exc($i)==1 { incr nbit1 }
	}
	set j1 0
	for {set i 1} {$i<29} {incr i} {
	    if $exc($i)==1 { incr j1; set latbit($j1) $i }
	}
    }
    destroy .main_w.inc_or_exc
}

proc set_dsurround { iopt } {
    global dsurround helvfont
    set top .main_w.dsurr
    toplevel $top
    wm title $top "Region surrounding phantom"

    if $iopt==0 {
	# uniform surrounding thickness
	set text "Enter the thickness of the surrounding material, in cm."
	label $top.lab -text $text -font $helvfont
	pack $top.lab -pady 5
	frame $top.frm
	label $top.frm.lab -text "Thickness"
	entry $top.frm.inp -textvariable dsurround(1) -bg white
	pack $top.frm.lab -side left -anchor w -padx 5
	pack $top.frm.inp -side right -anchor e -expand true -fill x
	pack $top.frm -pady 5
    } else {
	# non-uniform surrounding thickness
	set text "Enter the thickness of the region surrounding the phantom\
		in the x, y and z dimensions, in cm."
	label $top.lab -text $text -font $helvfont
	pack $top.lab -pady 5
	set dtext(1) "x thickness"
	set dtext(2) "y thickness"
	set dtext(3) "bottom z thickness"
	set dtext(4) "top z thickness"
	for {set i 1} {$i<=4} {incr i} {
	    frame $top.frm$i
	    label $top.frm$i.lab -text $dtext($i)
	    entry $top.frm$i.inp -textvariable dsurround($i) -bg white
	    pack $top.frm$i.lab -side left -anchor w -padx 5
	    pack $top.frm$i.inp -side right -anchor e -expand true -fill x
	    pack $top.frm$i -fill x -pady 5 -padx 100
	}
    }
    #put on an ok button
    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy $top"\
	    -relief groove -bd 8
    pack $top.buts.okb
    pack $top.buts -pady 5
}

proc define_theta_phi { } {
    # procedure to define theta-phi pairs for source 7 and 8
    global thphidef angfixed ang1 ang2 pang ivary numthphi nang

    toplevel .main_w.srcopt.thphi
    set top .main_w.srcopt.thphi

    frame $top.grid -bd 4

    if {$thphidef==0} {
      #do it on a pair-by-pair basis
      wm title $top "Define theta-phi pairs"
      label $top.grid.thlab -text "theta (degrees)"
      label $top.grid.philab -text "phi (degrees)"
      label $top.grid.problab -text "probability"
      grid configure $top.grid.thlab -row 0 -column 1
      grid configure $top.grid.philab -row 0 -column 2
      grid configure $top.grid.problab -row 0 -column 3
      for {set i 1} {$i<=$numthphi} {incr i} {
         label $top.grid.l$i -text "Pair $i"
         grid configure $top.grid.l$i -row $i -column 0 -padx 5
         entry $top.grid.e1$i -textvariable ang1($i) -width 8
         grid configure $top.grid.e1$i -row $i -column 1
         entry $top.grid.e2$i -textvariable ang2($i) -width 8
         grid configure $top.grid.e2$i -row $i -column 2
         entry $top.grid.e3$i -textvariable pang($i) -width 8
         grid configure $top.grid.e3$i -row $i -column 3
      }
    } elseif {$thphidef==1} {
      #do it group-by-group
      wm title $top "Define theta-phi groups"
      frame $top.label
      label $top.label.text -text "All angles are in degrees!"
      pack $top.label.text
      pack $top.label -side top -anchor n
      for {set i 1} {$i<=$numthphi} {incr i} {
        #put in a separator
        frame $top.grid.sep$i
        frame $top.grid.sep$i.l -bd 4 -width 200 -height 2 -relief groove
        pack $top.grid.sep$i.l -side top -fill x -pady 10
        grid configure $top.grid.sep$i -row [expr 3*$i-3] -column 2 -columnspan 10
        #now output the group number and the radiobuttons giving the user
        #the option of varying theta or phi
        label $top.grid.l$i -text "Group $i"
        grid configure $top.grid.l$i -row [expr 3*$i-2] -column 0 -rowspan 2
        frame $top.grid.rdo$i
        radiobutton $top.grid.rdo$i.radio1 -text "vary phi"\
           -variable ivary($i) -value 0 -command "enable_group_grid $i"
        radiobutton $top.grid.rdo$i.radio2 -text "vary theta"\
           -variable ivary($i) -value 1 -command "enable_group_grid $i"
        pack $top.grid.rdo$i.radio1 -side top -anchor w
        pack $top.grid.rdo$i.radio2 -side top -anchor w
        grid configure $top.grid.rdo$i -row [expr 3*$i-2] -column 1 -rowspan 2

        #labels depend on value of ivary
        #first 2 rows are given to labels
        if {$ivary($i)==0} {
            label $top.grid.angfixedlab$i -text "fixed theta:"
            grid configure $top.grid.angfixedlab$i -row [expr 3*$i-2] -column 2\
                       -rowspan 2
            label $top.grid.ang1lab$i -text "min. phi:"
            grid configure $top.grid.ang1lab$i -row [expr 3*$i-2] -column 4\
                       -rowspan 2
            label $top.grid.ang2lab$i -text "max. phi:"
            grid configure $top.grid.ang2lab$i -row [expr 3*$i-2] -column 6\
                       -rowspan 2
            label $top.grid.nanglab$i -text "no. of phi in group:"
            grid configure $top.grid.nanglab$i -row [expr 3*$i-2] -column 8
            label $top.grid.nanginfo$i -text "(including min. and max.)"
            grid configure $top.grid.nanginfo$i -row [expr 3*$i-1] -column 8
            label $top.grid.panglab$i -text "prob. for group:"
            grid configure $top.grid.panglab$i -row [expr 3*$i-2] -column 10\
                       -rowspan 2
        } elseif {$ivary($i)==1} {
            label $top.grid.angfixedlab$i -text "fixed phi:"
            grid configure $top.grid.angfixedlab$i -row [expr 3*$i-2] -column 2\
                       -rowspan 2
            label $top.grid.ang1lab$i -text "min. theta:"
            grid configure $top.grid.ang1lab$i -row [expr 3*$i-2] -column 4\
                       -rowspan 2
            label $top.grid.ang2lab$i -text "max. theta:"
            grid configure $top.grid.ang2lab$i -row [expr 3*$i-2] -column 6\
                       -rowspan 2
            label $top.grid.nanglab$i -text "no. of theta in group:"
            grid configure $top.grid.nanglab$i -row [expr 3*$i-2] -column 8
            label $top.grid.nanginfo$i -text "(including min. and max.)"
            grid configure $top.grid.nanginfo$i -row [expr 3*$i-1] -column 8
            label $top.grid.panglab$i -text "prob. for group:"
            grid configure $top.grid.panglab$i -row [expr 3*$i-2] -column 10\
                       -rowspan 2
        } else {
            #just output a label saying the user must choose ivary
            label $top.grid.warnlab$i -text "Select varying phi or theta"
            grid configure $top.grid.warnlab$i -row [expr 3*$i-2] -column 2 \
                       -rowspan 2 -columnspan 10
        }

        #now for the entry boxes--only output them if ivary is set
        if {$ivary($i)==0 || $ivary($i)==1} {
          entry $top.grid.e1$i -textvariable angfixed($i) -width 10
          grid configure $top.grid.e1$i -row [expr 3*$i-2] -column 3 -rowspan 2
          entry $top.grid.e2$i -textvariable ang1($i) -width 10
          grid configure $top.grid.e2$i -row [expr 3*$i-2] -column 5 -rowspan 2
          entry $top.grid.e3$i -textvariable ang2($i) -width 10
          grid configure $top.grid.e3$i -row [expr 3*$i-2] -column 7 -rowspan 2
          entry $top.grid.e4$i -textvariable nang($i) -width 10
          grid configure $top.grid.e4$i -row [expr 3*$i-2] -column 9 -rowspan 2
          entry $top.grid.e5$i -textvariable pang($i) -width 10
          grid configure $top.grid.e5$i -row [expr 3*$i-2] -column 11 -rowspan 2
        }
      }
    }
    pack $top.grid -side top

    frame $top.b

    if {$thphidef==0} {
       button $top.b.addb -text "Add pair" -command "add_theta_phi"\
            -relief groove -bd 8
       button $top.b.delb -text "Delete last pair"\
            -command "del_theta_phi" -relief groove -bd 8
    } elseif {$thphidef==1} {
       button $top.b.addb -text "Add group" -command "add_theta_phi"\
            -relief groove -bd 8
       button $top.b.delb -text "Delete last group"\
            -command "del_theta_phi" -relief groove -bd 8
    }
    button $top.b.okb -text "OK"\
            -command "destroy $top" -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb -side left -padx 10
    pack $top.b -pady 10

    if {$numthphi==0} {
          $top.b.delb configure -state disabled
    }

}

proc add_theta_phi { } {
# procedure for adding a theta-phi pair or group
    global thphidef numthphi angfixed ang1 ang2 pang ivary

    set w .main_w.srcopt.thphi

    set i [incr numthphi]

    if {$thphidef==0} {
         label $w.grid.l$i -text "Pair $i"
         grid configure $w.grid.l$i -row $i -column 0 -padx 5
         entry $w.grid.e1$i -textvariable ang1($i) -width 8
         grid configure $w.grid.e1$i -row $i -column 1
         entry $w.grid.e2$i -textvariable ang2($i) -width 8
         grid configure $w.grid.e2$i -row $i -column 2
         entry $w.grid.e3$i -textvariable pang($i) -width 8
         grid configure $w.grid.e3$i -row $i -column 3
    } elseif {$thphidef==1} {

        #put in a separator
        frame $w.grid.sep$i
        frame $w.grid.sep$i.l -bd 4 -width 200 -height 2 -relief groove
        pack $w.grid.sep$i.l -side top -fill x -pady 10
        grid configure $w.grid.sep$i -row [expr 3*$i-3] -column 2 -columnspan 10

        #just output a label saying the user must choose ivary
        label $w.grid.warnlab$i -text "Select varying phi or theta"
        grid configure $w.grid.warnlab$i -row [expr 3*$i-2] -column 2\
                   -columnspan 10 -rowspan 2

        #now output the group number and the radiobuttons giving the user
        #the option of varying theta or phi
        label $w.grid.l$i -text "Group $i"
        grid configure $w.grid.l$i -row [expr 3*$i-2] -column 0 -rowspan 2
        frame $w.grid.rdo$i
        radiobutton $w.grid.rdo$i.radio1 -text "vary phi"\
           -variable ivary($i) -value 0 -command "enable_group_grid $i"
        radiobutton $w.grid.rdo$i.radio2 -text "vary theta"\
           -variable ivary($i) -value 1 -command "enable_group_grid $i"
        pack $w.grid.rdo$i.radio1 -side top -anchor w
        pack $w.grid.rdo$i.radio2 -side top -anchor w
        grid configure $w.grid.rdo$i -row [expr 3*$i-2] -column 1 -rowspan 2

        set ivary($i) {}
    }

    if {$numthphi==1} {
         #re-enable the delete last pair/group button
         $w.b.delb configure -state normal -command "del_theta_phi"
    }

}

proc del_theta_phi { } {
#procedure for deleting the last theta-phi pair or group
    global thphidef numthphi ivary

    set w .main_w.srcopt.thphi

    if {$thphidef==0} {
        destroy $w.grid.l$numthphi
        for {set i 1} {$i<=3} {incr i} {
           destroy $w.grid.e$i$numthphi
        }
    } elseif {$thphidef==1} {
        #destroy everything we know is there
        destroy $w.grid.l$numthphi
        destroy $w.grid.rdo$numthphi
        destroy $w.grid.sep$numthphi
        if {$ivary($numthphi)==0 | $ivary($numthphi)==1} {
          #destroy all the labels
          destroy $w.grid.angfixedlab$numthphi
          destroy $w.grid.ang1lab$numthphi
          destroy $w.grid.ang2lab$numthphi
          destroy $w.grid.nanglab$numthphi
          destroy $w.grid.panglab$numthphi
          destroy $w.grid.deglab$numthphi
          destroy $w.grid.nanginfo$numthphi
          for {set j 1} {$j<=5} {incr j} {
              destroy $w.grid.e$j$numthphi
          }
        } else {
          #just destroy the warning label
          destroy $w.grid.warnlab$numthphi
        }
    }

    incr numthphi -1

    if {$numthphi==0} {
          $w.b.delb configure -state disabled
    }
}

proc enable_group_grid { i } {
#if this is the first time ivary has been selected, then
#outputs labels and entry boxes for group theta-phi input
#otherwise, it just reconfigures the labels

    global ivary

    set w .main_w.srcopt.thphi

    #destroy the old warning label
    destroy $w.grid.warnlab$i

    #do the labels
    if {$ivary($i)==0} {
       if {[winfo exists $w.grid.angfixedlab$i]==1} {
            #the text exists, just reconfigure relevant text
            $w.grid.angfixedlab$i configure -text "fixed theta:"
            $w.grid.ang1lab$i configure -text "min. phi:"
            $w.grid.ang2lab$i configure -text "max. phi:"
            $w.grid.nanglab$i configure -text "no. of phi in group:"
       } else {
            #start from scratch
            label $w.grid.angfixedlab$i -text "fixed theta:"
            grid configure $w.grid.angfixedlab$i -row [expr 3*$i-2] -column 2\
                     -rowspan 2
            label $w.grid.ang1lab$i -text "min. phi:"
            grid configure $w.grid.ang1lab$i -row [expr 3*$i-2] -column 4\
                     -rowspan 2
            label $w.grid.ang2lab$i -text "max. phi:"
            grid configure $w.grid.ang2lab$i -row [expr 3*$i-2] -column 6\
                     -rowspan 2
            label $w.grid.nanglab$i -text "no. of phi in group:"
            grid configure $w.grid.nanglab$i -row [expr 3*$i-2] -column 8
            label $w.grid.nanginfo$i -text "(including min. and max.)"
            grid configure $w.grid.nanginfo$i -row [expr 3*$i-1] -column 8
            label $w.grid.panglab$i -text "prob. for group:"
            grid configure $w.grid.panglab$i -row [expr 3*$i-2] -column 10\
                     -rowspan 2
       }
    } elseif {$ivary($i)==1} {
       if { [winfo exists $w.grid.angfixedlab$i] == 1 } {
            #the text exists, just reconfigure relevant text
            $w.grid.angfixedlab$i configure -text "fixed phi:"
            $w.grid.ang1lab$i configure -text "min. theta:"
            $w.grid.ang2lab$i configure -text "max. theta:"
            $w.grid.nanglab$i configure -text "no. of theta in group:"
       } else {
            #start from scratch
            label $w.grid.angfixedlab$i -text "fixed phi:"
            grid configure $w.grid.angfixedlab$i -row [expr 3*$i-2] -column 2\
                     -rowspan 2
            label $w.grid.ang1lab$i -text "min. theta:"
            grid configure $w.grid.ang1lab$i -row [expr 3*$i-2] -column 4\
                     -rowspan 2
            label $w.grid.ang2lab$i -text "max. theta:"
            grid configure $w.grid.ang2lab$i -row [expr 3*$i-2] -column 6\
                     -rowspan 2
            label $w.grid.nanglab$i -text "no. of theta in group:"
            grid configure $w.grid.nanglab$i -row [expr 3*$i-2] -column 8
            label $w.grid.nanginfo$i -text "(including min. and max.)"
            grid configure $w.grid.nanginfo$i -row [expr 3*$i-1] -column 8
            label $w.grid.panglab$i -text "prob. for group:"
            grid configure $w.grid.panglab$i -row [expr 3*$i-2] -column 10\
                     -rowspan 2
       }
    }

    #now for the entry boxes
    if  { [winfo exists $w.grid.e1$i] == 0 } {
       entry $w.grid.e1$i -textvariable angfixed($i) -width 10
       grid configure $w.grid.e1$i -row [expr 3*$i-2] -column 3 -rowspan 2
       entry $w.grid.e2$i -textvariable ang1($i) -width 10
       grid configure $w.grid.e2$i -row [expr 3*$i-2] -column 5 -rowspan 2
       entry $w.grid.e3$i -textvariable ang2($i) -width 10
       grid configure $w.grid.e3$i -row [expr 3*$i-2] -column 7 -rowspan 2
       entry $w.grid.e4$i -textvariable nang($i) -width 10
       grid configure $w.grid.e4$i -row [expr 3*$i-2] -column 9 -rowspan 2
       entry $w.grid.e5$i -textvariable pang($i) -width 10
       grid configure $w.grid.e5$i -row [expr 3*$i-2] -column 11 -rowspan 2
    }
}
proc define_setting { } {
    # procedure to define a combination setting in source 10, 11, and 12
    global numsets iso1 iso2 iso3 ang1 ang2 ang3 dsource muI

    toplevel .main_w.srcopt.sett
    set top .main_w.srcopt.sett

    frame $top.grid -bd 4
    #do each setting
      wm title $top "Define settings"
      label $top.grid.xisolab -text "xiso (cm)"
      label $top.grid.yisolab -text "yiso (cm)"
      label $top.grid.zisolab -text "ziso (cm)"
      label $top.grid.thlab -text "theta (degrees)"
      label $top.grid.philab -text "phi (degrees)"
      label $top.grid.collab -text "phicol (degrees)"
      label $top.grid.dsourcelab -text "dsource (cm)"
      label $top.grid.muilab -text "MU Index (0-1)"
      grid configure $top.grid.xisolab -row 0 -column 1
      grid configure $top.grid.yisolab -row 0 -column 2
      grid configure $top.grid.zisolab -row 0 -column 3
      grid configure $top.grid.thlab -row 0 -column 4
      grid configure $top.grid.philab -row 0 -column 5
      grid configure $top.grid.collab -row 0 -column 6
      grid configure $top.grid.dsourcelab -row 0 -column 7
      grid configure $top.grid.muilab -row 0 -column 8
      for {set i 1} {$i<=$numsets} {incr i} {
         label $top.grid.l$i -text "Setting $i"
         grid configure $top.grid.l$i -row $i -column 0 -padx 5
         entry $top.grid.e1$i -textvariable iso1($i) -width 8
         grid configure $top.grid.e1$i -row $i -column 1
         entry $top.grid.e2$i -textvariable iso2($i) -width 8
         grid configure $top.grid.e2$i -row $i -column 2
         entry $top.grid.e3$i -textvariable iso3($i) -width 8
         grid configure $top.grid.e3$i -row $i -column 3
         entry $top.grid.e4$i -textvariable ang1($i) -width 8
         grid configure $top.grid.e4$i -row $i -column 4
         entry $top.grid.e5$i -textvariable ang2($i) -width 8
         grid configure $top.grid.e5$i -row $i -column 5
         entry $top.grid.e6$i -textvariable ang3($i) -width 8
         grid configure $top.grid.e6$i -row $i -column 6
	 entry $top.grid.e7$i -textvariable dsource($i) -width 8
         grid configure $top.grid.e7$i -row $i -column 7
	 entry $top.grid.e8$i -textvariable muI($i) -width 8
         grid configure $top.grid.e8$i -row $i -column 8
      }

    pack $top.grid -side top

    frame $top.b

    button $top.b.addb -text "Add setting" -command "add_setting"\
	-relief groove -bd 8
    button $top.b.delb -text "Delete last setting"\
	-command "del_setting" -relief groove -bd 8

    button $top.b.okb -text "OK"\
	-command "destroy $top" -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb -side left -padx 10
    pack $top.b -pady 10

    if {$numsets==0} {
          $top.b.delb configure -state disabled
    }

}
proc add_setting { } {
# procedure for adding a setting (source 10, 11, and 12)
    global  numsets iso1 iso2 iso3 ang1 ang2 ang3 dsource muI

    set w .main_w.srcopt.sett

    set i [incr numsets]

    label $w.grid.l$i -text "Setting $i"
    grid configure $w.grid.l$i -row $i -column 0 -padx 5
    entry $w.grid.e1$i -textvariable iso1($i) -width 8
    grid configure $w.grid.e1$i -row $i -column 1
    entry $w.grid.e2$i -textvariable iso2($i) -width 8
    grid configure $w.grid.e2$i -row $i -column 2
    entry $w.grid.e3$i -textvariable iso3($i) -width 8
    grid configure $w.grid.e3$i -row $i -column 3
    entry $w.grid.e4$i -textvariable ang1($i) -width 8
    grid configure $w.grid.e4$i -row $i -column 4
    entry $w.grid.e5$i -textvariable ang2($i) -width 8
    grid configure $w.grid.e5$i -row $i -column 5
    entry $w.grid.e6$i -textvariable ang3($i) -width 8
    grid configure $w.grid.e6$i -row $i -column 6
    entry $w.grid.e7$i -textvariable dsource($i) -width 8
    grid configure $w.grid.e7$i -row $i -column 7
    entry $w.grid.e8$i -textvariable muI($i) -width 8
    grid configure $w.grid.e8$i -row $i -column 8

    if {$numsets==1} {
         #re-enable the delete last pair/group button
         $w.b.delb configure -state normal -command "del_setting"
    }

}
proc del_setting { } {
#procedure for deleting the last theta-phi pair or group
    global numsets

    set w .main_w.srcopt.sett

    destroy $w.grid.l$numsets
    for {set i 1} {$i<=8} {incr i} {
	destroy $w.grid.e$i$numsets
    }
    incr numsets -1

    if {$numsets==0} {
          $w.b.delb configure -state disabled
    }
}
