
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: set main input parameters
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
#                   Marc-Andre Renaud
#                   Reid Townson
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


proc edit_parameters {} {

    global values names options numopts grouping helvfont medsur medium
    global CTdataflag pegs4filename dflag dflagopt ypad iphspout phspoutopt is_pegsless

    # First get a PEGS4 file if there's none loaded.
    if [string compare $pegs4filename ""]==0 {
	# get a pegs4file, read the media into pegs
	get_pegs4file
	# wait for the user to finish loading a pegs4file before continuing
	tkwait window .pegs4
    }

    #also, get the source number, this is useful later on
    set isource {}
    foreach j {0 1 2 3 4 6 7 8 9 10 20 21} {
        if [string compare $values(4) $options(4,$j)]==0 {
            set isource $j
        }
    }

    # main procedure for the set main parameters window.  Puts the text boxes/
    # option menus up on the screen.

    toplevel .main_w

    wm title .main_w "Inputs"
    wm geometry .main_w +100+100

    # WINDOW FOR SETTING THE MAIN OPTIONS -- FOR EACH ADD A HELP BUTTON,
    # A LABEL, THEN A TEXT BOX OR OPTION MENU (WHICHEVER IS APPROPRIATE).
    # NOTE THAT OPTION MENUS SPAWN CHILD WINDOWS FOR SOME PARAMETERS.

    set top .main_w.main
    frame $top -bd 5

    # The title spans the window.
    frame $top.inp1 -bd 4
    button $top.inp1.help -text "?" -command "help 1"
    label $top.inp1.label -text $names(1) -anchor w
    entry $top.inp1.inp -textvariable values(1) -width 80
    pack $top.inp1.help -anchor w -side left -padx 2
    pack $top.inp1.label -side left -padx 10
    pack $top.inp1.inp -anchor e -side right -fill x -expand true
    pack $top.inp1 -side top -fill x -padx 5

    # Add a separator
    frame $top.topsep -bd 4 -width 100 -height 2 -relief groove
    pack $top.topsep -fill x -pady [expr 2*$ypad]

    label $top.phantomlabel -text "Phantom definition" -font $helvfont
    pack $top.phantomlabel -pady [expr 2*$ypad]
    # frame to hold phantom definition stuff:

    set w $top.pdef
    frame $w

    # phantom definition parameters on $w with subframes $w.left, $w.right
    set left $top.pdef.left
    set right $top.pdef.right
    frame $right

    # ecut,pcut and max20
    foreach j {ecut pcut 27} {
        frame $right.f$j -bd 4
        button $right.f$j.help -text "?" -command "help $j"
        label $right.f$j.label -text $names($j) -anchor w
        if $numopts($j)==0 {
            entry $right.f$j.inp -relief sunken -textvariable values($j)\
                    -width 10
        } else {
            # find maximum width required on menubar
            set width 0
            for {set k 0} {$k<$numopts($j)} {incr k} {
                set len [string length $options($j,$k)]
                if $len>$width {set width $len}
            }
            incr width -9
            menubutton $right.f$j.inp -text $values($j)\
                    -menu $right.f$j.inp.m -bd 1 \
                    -relief raised -indicatoron 1 -width $width
            menu $right.f$j.inp.m
            for {set iopt 0} {$iopt < $numopts($j)} {incr iopt} {
                $right.f$j.inp.m add command -label $options($j,$iopt) \
                        -command "set_value $j $iopt $right.f$j.inp"
            }
        }
        pack $right.f$j.help -anchor w -side left
        pack $right.f$j.label -side left -padx 10
        pack $right.f$j.inp -anchor e -side right -fill x -expand true
        pack $right.f$j -side top -fill x -padx 5
    }

    frame $left

    message $left.warn -text "If you are using source 2 or 4, you must\
	    define the materials in the phantom here first before \
	    defining the source." -width 450

    frame $left.frm
    button $left.frm.help -text "?" -command "help CTdataflag"
    frame $left.frm.phdef
    # frame for the radiobuttons to sit in, on top of one another
    frame $left.frm.phdef.ct
    radiobutton $left.frm.phdef.ct.radio1 -text "non-CT data input"\
	    -variable CTdataflag -value 0
    radiobutton $left.frm.phdef.ct.radio2 -text "phantom created from CT data"\
	    -variable CTdataflag -value 1
    pack $left.frm.phdef.ct.radio1 -anchor w
    pack $left.frm.phdef.ct.radio2 -anchor w
    button $left.frm.phdef.phb -text "Define phantom using ..."\
	    -command define_phantom
    pack $left.frm.help -side left
    pack $left.frm.phdef.phb $left.frm.phdef.ct -side left -padx 5
    pack $left.frm.phdef -pady $ypad

    pack $left.warn $left.frm -side top -pady $ypad

    pack $left -side left -anchor w -fill x -padx 5
    pack $right -side right -fill x -padx 5 -expand true

    pack $w -side top -fill x -padx 5

    # give CTdataflag a default if unset
    if [string compare $CTdataflag ""]==0 { set CTdataflag 0 }

    # Add a separator
    frame $top.sep0 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep0 -fill x -pady [expr 2*$ypad]

    label $top.srclabel -text "Source parameters" -font $helvfont
    pack $top.srclabel -pady $ypad

    # Incident particle
    frame $top.inp3 -bd 4
    button $top.inp3.help -text "?" -command "help 3"
    label $top.inp3.label -text $names(3) -anchor w
    set width [string length $values(3)]
    if $width<10 { set width 10 }
    menubutton $top.inp3.inp -text $values(3)\
	    -menu $top.inp3.inp.m -bd 1 \
	    -relief raised -indicatoron 1 -width $width
    menu $top.inp3.inp.m
    for {set iopt -1} {$iopt < 2} {incr iopt} {
	$top.inp3.inp.m add command -label $options(3,$iopt) \
		-command "set_value 3 $iopt $top.inp3.inp"
    }
    # only enable "all" particles for source 2,4,8,9,10,20,21
    set iopt 2
    if { $isource==2 || $isource==4 || $isource>=8} {
       $top.inp3.inp.m add command -label $options(3,$iopt) \
                -command "set_value 3 $iopt $top.inp3.inp"
    } else {
       $top.inp3.inp.m add command -label $options(3,$iopt) -state disabled\
                -command "set_value 3 $iopt $top.inp3.inp"
    }

    pack $top.inp3.help -anchor w -side left
    pack $top.inp3.label -side left -padx 10
    pack $top.inp3.inp -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x -padx 5

    # isourc
    frame $top.inp4 -bd 4
    button $top.inp4.help -text "?" -command "help 4"
    label $top.inp4.label -text $names(4) -anchor w
    set width [string length $options(4,4)]
    menubutton $top.inp4.inp -text $values(4)\
	    -menu $top.inp4.inp.m -bd 1 \
	    -relief raised -indicatoron 1 -width $width
    menu $top.inp4.inp.m
    foreach iopt {0 1 2 3 4 6 7 8 9 10 20 21} {
	$top.inp4.inp.m add command -label $options(4,$iopt) \
		-command "enable_dflag $iopt;set_value 4 $iopt $top.inp4.inp"
    }
    pack $top.inp4.help -anchor w -side left
    pack $top.inp4.label -side left -padx 10
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x -padx 5

    # Add a separator
    frame $top.sep1 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep1 -fill x -pady [expr 2*$ypad]

    label $top.simlabel -text "Simulation parameters" -font $helvfont
    pack $top.simlabel -pady $ypad
    # main simulation parameters on a frame $w with subframes $w.left, $w.right
    set w $top.sim
    set w1 $top.sim.left
    set w2 $top.sim.right
    frame $w
    frame $w1
    frame $w2
    foreach j { 5 6 7 8 9 10 12 ihowfarless ibindos } {
	frame $w1.inp$j -bd 4
	button $w1.inp$j.help -text "?"	-command "help $j"
	label $w1.inp$j.label -text $names($j) -anchor w
	if $numopts($j)==0 {
	    entry $w1.inp$j.inp -relief sunken -textvariable values($j)\
		    -width 10
	} else {
	    # find maximum width required on menubar
	    set width 0
	    if { "$j"=="6" } {
		foreach jj {0 1 2 4} {
		    set len [string length $options($j,$jj)]
		    if $len>$width {set width $len}
		}
	    } else {
		for {set jj 0} {$jj<$numopts($j)} {incr jj} {
		    set len [string length $options($j,$jj)]
		    if $len>$width {set width $len}
		}
	    }
	    incr width -9
	    menubutton $w1.inp$j.inp -text $values($j)\
		    -menu $w1.inp$j.inp.m -bd 1 \
		    -relief raised -indicatoron 1 -width $width
	    menu $w1.inp$j.inp.m
	    if { "$j"!="6" } {
		for {set iopt 0} {$iopt < $numopts($j)} {incr iopt} {
		    $w1.inp$j.inp.m add command -label $options($j,$iopt) \
			    -command "set_value $j $iopt $w1.inp$j.inp"
		}
	    } else {
		foreach iopt {0 1 2 4} {
		    $w1.inp$j.inp.m add command -label $options($j,$iopt) \
			    -command "set_value $j $iopt $w1.inp$j.inp"
		}
	    }
	}
	pack $w1.inp$j.help -anchor w -side left
	pack $w1.inp$j.label -side left -padx 10
	pack $w1.inp$j.inp -anchor e -side right -fill x -expand true
	pack $w1.inp$j -side top -fill x
    }

    # put in dflag option, only activate of enflag>1 (isource=2,4,8,9)
    frame $w2.dflag
    frame $w2.dflag.dsur
    button $w2.dflag.dsur.help -text "?" -command "help dflag"
    label $w2.dflag.dsur.lab -text "Thickness of region outside phantom is"
    menubutton $w2.dflag.dsur.inp -text $dflag -width 20\
	    -menu $w2.dflag.dsur.inp.m -bd 1 -relief raised -indicatoron 1
    # Now make the menu, and add the lines one at a time
    menu $w2.dflag.dsur.inp.m -tearoff no
    for {set j 0} {$j<2} {incr j} {
	$w2.dflag.dsur.inp.m add command -label $dflagopt($j)\
		-command "set dflag {$dflagopt($j)};\
		$w2.dflag.dsur.inp configure -text {$dflagopt($j)};\
		set_dsurround $j"
    }
    pack $w2.dflag.dsur.help -side left -anchor w -padx 5
    pack $w2.dflag.dsur.lab -side left
    pack $w2.dflag.dsur.inp -side right -padx 5 -fill x -expand true -anchor e

    # also input the medium of the surrounding region if isource=2,4,8,9
    frame $w2.dflag.med
    button $w2.dflag.med.help -text "?" -command "help dflag"
    label $w2.dflag.med.lab -text "Medium of region outside phantom"
    menubutton $w2.dflag.med.inp -text $medsur -width 20\
                -menu $w2.dflag.med.inp.m -bd 1 -relief raised -indicatoron 1
    # Now make the menu, and add the lines one at a time
    menu $w2.dflag.med.inp.m -tearoff no
    for {set j 0} {$j<=$values(2)} {incr j} {
         $w2.dflag.med.inp.m add command -label $medium($j)\
                    -command "set medsur {$medium($j)};\
                    $w2.dflag.med.inp configure -text {$medium($j)}"
    }
    pack $w2.dflag.med.help -side left -anchor w -padx 5
    pack $w2.dflag.med.lab -side left
    pack $w2.dflag.med.inp -side right -padx 5 -fill x -expand true -anchor e

    # option to score IAEA phase space data if isource=2,4,8,9,10,20,21
    frame $w2.dflag.phsp
    button $w2.dflag.phsp.help -text "?" -command "help iphspout"
    label $w2.dflag.phsp.lab -text "Phase space output on exiting phantom"
    menubutton $w2.dflag.phsp.inp -text $phspoutopt($iphspout) -width 20\
                -menu $w2.dflag.phsp.inp.m -bd 1 -relief raised -indicatoron 1
    # Now make the menu, and add the lines one at a time
    menu $w2.dflag.phsp.inp.m -tearoff no
    for {set j 0} {$j<3} {incr j} {
         $w2.dflag.phsp.inp.m add command -label $phspoutopt($j)\
                    -command "set iphspout $j;\
                    $w2.dflag.phsp.inp configure -text {$phspoutopt($j)}"
    }
    pack $w2.dflag.phsp.help -side left -anchor w -padx 5
    pack $w2.dflag.phsp.lab -side left
    pack $w2.dflag.phsp.inp -side right -padx 5 -fill x -expand true -anchor e
    pack $w2.dflag.dsur $w2.dflag.med $w2.dflag.phsp -side top -pady $ypad -fill x
    pack $w2.dflag -pady $ypad -fill x

    # Now enable it or disable it according to the source selected.
    enable_dflag $isource

    # now 13 to 18 on right
    foreach j { 13 14 15 18 16 } {
	frame $w2.inp$j -bd 4
	button $w2.inp$j.help -text "?"	-command "help $j"
	label $w2.inp$j.label -text $names($j) -anchor w
	set width [string length $values($j)]
	if $width<10 { set width 10 }
	if $numopts($j)==0 {
	    entry $w2.inp$j.inp -relief sunken -textvariable values($j)\
		    -width $width
            if {$j==18} {
               bind $w2.inp$j.inp <Return> {config_esplit $values(18)}
               bind $w2.inp$j.inp <Tab> {config_esplit $values(18)}
            }
	} else {
	    # find maximum width required on menubar
	    set width 0
	    for {set jj 0} {$jj<$numopts($j)} {incr jj} {
		set len [string length $options($j,$jj)]
		if $len>$width {set width $len}
	    }
	    incr width -9
	    menubutton $w2.inp$j.inp -text $values($j)\
		    -menu $w2.inp$j.inp.m -bd 1 \
		    -relief raised -indicatoron 1 -width $width
	    menu $w2.inp$j.inp.m
	    for {set iopt 0} {$iopt < $numopts($j)} {incr iopt} {
		$w2.inp$j.inp.m add command -label $options($j,$iopt) \
			-command "set_value $j $iopt $w2.inp$j.inp"
	    }
	}
	pack $w2.inp$j.help -anchor w -side left
	pack $w2.inp$j.label -side left -padx 10
	pack $w2.inp$j.inp -anchor e -side right -fill x -expand true
	pack $w2.inp$j -side top -fill x
    }

    # enable/disable esave box for first pop-up
    for {set j 0} {$j<=1} {incr j} {
	if [string compare $values(14) $options(14,$j)]==0 {
	    set_value 14 $j $w2.inp14.inp
	}
    }

    #disable nrcycle input if source is not phase space (2 or 8)
    if { $isource != 2 && $isource != 8 && $isource != 20} {
      .main_w.main.sim.right.inp16.inp configure -state disabled
      .main_w.main.sim.right.inp16.label configure -fg grey
    }

    pack $w1 $w2 -side left -padx 5
    pack $w -side top -pady $ypad

    # Add a separator
    frame $top.sep2 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep2 -fill x -pady [expr 2*$ypad]

    #put in the "Done" button and an option to edit the EGSnrc parameters
    frame .main_w.button
    button .main_w.button.b1 -text "Define Media" -command "define_pegsless_media" \
              -relief groove -bd 8
    button .main_w.button.b2 -text "Edit EGSnrc Parameters" -command\
                          "set_egsnrc_parameters" -relief groove -bd 8
    button .main_w.button.b3 -text "Close" -command "destroy .main_w"\
                  -relief groove -bd 8
    pack .main_w.button.b1 -side left -padx 10
    pack .main_w.button.b2 -side left -padx 10
    pack .main_w.button.b3 -side left -padx 10
    if {$is_pegsless==0} {
          .main_w.button.b1 configure -state disabled
    }
    pack $top .main_w.button -side top -pady $ypad
}

proc set_value {io iopt w} {

    # sets the values of the parameters and calls offshoot procedures if
    # required.

    global values options

    if { $io==4 } {
	# SOURCE MENU - CALL set_src_options $iopt

	set values(4) $options(4,$iopt)
	set_src_options $iopt
        $w configure -text $options(4,$iopt)

	# DESTROY THE OLD 'Incident particle' MENU AND RE-CREATE A NEW
	# ONE WITH THE APPROPRIATE BUTTONS (SOURCES 2,4 SPECIAL CASE).
	destroy .main_w.main.inp3.inp.m
	set top2 .main_w.main.inp3
	menu $top2.inp.m -tearoff no
	if { $iopt==2 || $iopt==4 || $iopt==8 || $iopt==9 || $iopt==20 || $iopt==21} {
	    # SOURCE 2, 4, 8, 9 enable 'all' option
	    for {set iopt2 -1} {$iopt2 < 3} {incr iopt2} {
		$top2.inp.m add command -label $options(3,$iopt2) \
			-command "set_value 3 $iopt2 $top2.inp"
	    }
            #enable nrcycl
            .main_w.main.sim.right.inp16.inp configure -state normal
            .main_w.main.sim.right.inp16.label configure -fg black
	} else {
	    # NOT SOURCE 2, 4 or 8, 20 or 21 - if 'all' is selected,
	    # remove, then replace the first 3 and disable 'all'
	    if [string compare $values(3) "all"]==0 {
		$top2.inp configure -text $options(3,0)
	    }
	    for {set iopt2 -1} {$iopt2 <=1} {incr iopt2} {
		$top2.inp.m add command -label $options(3,$iopt2) \
			-command "set_value 3 $iopt2 $top2.inp"
	    }
	    $top2.inp.m add command -label $options(3,2) -state disabled \
		    -command "set_value 3 2 $top2.inp"
            #disable nrcycl
            .main_w.main.sim.right.inp16.inp configure -state disabled
            .main_w.main.sim.right.inp16.label configure -fg grey
	}
    } elseif {$io==14} {
	# range rejection on/off
        set values($io) $options($io,$iopt)
        $w configure -text $values($io)
	if $iopt==0 {
	    # disable esave entrybox, set esave to {}
	    .main_w.main.sim.right.inp15.inp configure -state disabled
	    .main_w.main.sim.right.inp15.label configure -fg grey
	    set values(15) {}
	} else {
	    # enable esave entry box
	    .main_w.main.sim.right.inp15.inp configure -state normal
	    .main_w.main.sim.right.inp15.label configure -fg black
	}
    } elseif {$io=="ibcmp"} {
        set values(ibcmp) $options(ibcmp,$iopt)
        if $iopt==2 {
          egsnrc_set_in_reg ibcmp ON
        } elseif $iopt==3 {
          egsnrc_set_in_reg ibcmp OFF
        }
        if $iopt==4 {
          .main_w.egsnrc.inputs.right.inpcomp_xsections.inp configure -state normal
        } else {
          .main_w.egsnrc.inputs.right.inpcomp_xsections.inp configure -state disabled
        }
        $w configure -text $values(ibcmp)
    } elseif {$io=="iphter"} {
        set values(iphter) $options(iphter,$iopt)
        if $iopt==2 {
          egsnrc_set_in_reg iphter ON
        } elseif $iopt==3 {
          egsnrc_set_in_reg iphter OFF
        }
        $w configure -text $values(iphter)
    } elseif {$io=="iraylr"} {
        set values(iraylr) $options(iraylr,$iopt)
        if $iopt==2 {
          egsnrc_set_in_reg iraylr ON
        } elseif $iopt==3 {
          egsnrc_set_in_reg iraylr OFF
        } elseif $iopt==4 {
          egsnrc_set_rayl_custom
        }
        $w configure -text $values(iraylr)
    } elseif {$io=="iedgfl"} {
        set values(iedgfl) $options(iedgfl,$iopt)
        if $iopt==2 {
          egsnrc_set_in_reg iedgfl ON
        } elseif $iopt==3 {
          egsnrc_set_in_reg iedgfl OFF
        }
        $w configure -text $values(iedgfl)
    } elseif {$io=="bca_algorithm"} {
        set bca_old $values(bca_algorithm)
        set values(bca_algorithm) $options(bca_algorithm,$iopt)
        if {$bca_old=="PRESTA-I" && $values(bca_algorithm)=="EXACT" \
            && $values(transport_algorithm)=="PRESTA-II"} {
            set values(smaxir) 1e10
        } elseif {$bca_old=="EXACT" && $values(bca_algorithm)=="PRESTA-I" \
            && $values(transport_algorithm)=="PRESTA-II"} {
            set values(smaxir) 5
        }
        $w configure -text $values(bca_algorithm)
    } elseif {$io=="transport_algorithm"} {
        set eta_old $values(transport_algorithm)
        set values(transport_algorithm) $options(transport_algorithm,$iopt)
        if {$eta_old=="PRESTA-I" && $values(transport_algorithm)=="PRESTA-II" \
            && $values(bca_algorithm)=="EXACT"} {
            set values(smaxir) 1e10
        } elseif {$eta_old=="PRESTA-II" && \
                  $values(transport_algorithm)=="PRESTA-I" \
                  && $values(bca_algorithm)=="EXACT"} {
            set values(smaxir) 5
        }
        $w configure -text $values(transport_algorithm)
    } else {
	# OTHERWISE
        set values($io) $options($io,$iopt)
        $w configure -text $values($io)
    }
}

proc enable_dflag { iopt } {
    if [string compare $iopt {}]==0 {
	set iopt 0
    }

    set w2 .main_w.main.sim.right
    if { $iopt==2 || $iopt==4 || $iopt==8 || $iopt==9 || $iopt==10 || $iopt==20 || $iopt==21} {
	# enable the dflag option
	$w2.dflag.dsur.lab configure -fg black
	$w2.dflag.dsur.inp configure -state normal
        $w2.dflag.med.lab configure -fg black
        $w2.dflag.med.inp configure -state normal
        $w2.dflag.phsp.lab configure -fg black
        $w2.dflag.phsp.inp configure -state normal
    } else {
	# disable the dflag option
	$w2.dflag.dsur.lab configure -fg grey
	$w2.dflag.dsur.inp configure -state disabled
        $w2.dflag.med.lab configure -fg grey
        $w2.dflag.med.inp configure -state disabled
        $w2.dflag.phsp.lab configure -fg grey
        $w2.dflag.phsp.inp configure -state disabled
    }
}

proc set_egsnrc_parameters {} {

    #open up a subwindow to set EGSnrc paramters

    global names values options numopts GUI_DIR ypad

    catch { destroy .main_w.egsnrc }

    toplevel .main_w.egsnrc
    wm title .main_w.egsnrc "EGSnrc Parameters"

    set top .main_w.egsnrc.inputs
    frame $top -bd 5

    frame $top.left
    set top1 $top.left

    #left side of window

    foreach i {smaxir estepe ximax bca_algorithm skindepth_for_bca \
                transport_algorithm spin_effects eii_flag \
                ibrdst ibr_nist} {
            frame $top1.inp$i -bd 4
            button $top1.inp$i.help -text "?" -command "help $i"
            label $top1.inp$i.label -text $names($i)  -anchor w
            if $numopts($i)==0 {
               entry $top1.inp$i.inp -relief sunken\
                         -textvariable values($i) -width 10
            } else {
               # find maximum width required on menubar
               set width 0
               for {set k 0} {$k<$numopts($i)} {incr k} {
                 set len [string length $options($i,$k)]
                 if $len>$width {set width $len}
               }
               menubutton $top1.inp$i.inp -text $values($i)\
                    -menu $top1.inp$i.inp.m -bd 1 \
                    -relief raised -indicatoron 1 -width $width
               menu $top1.inp$i.inp.m
               for {set iopt 0} {$iopt < $numopts($i)} {incr iopt} {
                 $top1.inp$i.inp.m add command -label $options($i,$iopt) \
                        -command "set_value $i $iopt $top1.inp$i.inp"
               }
            }
            pack $top1.inp$i.help -anchor w -side left
            pack $top1.inp$i.label -side left -padx 10
            pack $top1.inp$i.inp -anchor e -side right -fill x -expand true
            pack $top1.inp$i -side top -fill x -padx 5
    }

    #right side of window

    frame $top.right
    set top2 $top.right

    foreach i {ibcmp comp_xsections iprdst pair_nrc \
               iphter iraylr iedgfl photon_xsections xsec_out} {
            frame $top2.inp$i -bd 4
            button $top2.inp$i.help -text "?" -command "help $i"
            label $top2.inp$i.label -text $names($i)  -anchor w
            if $numopts($i)==0 {
               entry $top2.inp$i.inp -relief sunken\
                     -textvariable values($i) -width 10
            } else {
               # find maximum width required on menubar
               set width 0
               for {set k 0} {$k<$numopts($i)} {incr k} {
                 set len [string length $options($i,$k)]
                 if $len>$width {set width $len}
               }
               menubutton $top2.inp$i.inp -text $values($i)\
                    -menu $top2.inp$i.inp.m -bd 1 \
                    -relief raised -indicatoron 1 -width $width
               menu $top2.inp$i.inp.m
               for {set iopt 0} {$iopt < $numopts($i)} {incr iopt} {
                 $top2.inp$i.inp.m add command -label $options($i,$iopt) \
                        -command "set_value $i $iopt $top2.inp$i.inp"
               }
            }
            if {$i=="comp_xsections" && $values(ibcmp)!="Norej"} {
               #disable ability to select comp_xsections
               $top2.inp$i.inp configure -state disabled
            }
            pack $top2.inp$i.help -anchor w -side left
            pack $top2.inp$i.label -side left -padx 10
            pack $top2.inp$i.inp -anchor e -side right -fill x -expand true
            pack $top2.inp$i -side top -fill x
    }

    pack $top1 -side left -anchor n
    pack $top2 -side right -anchor n

    #put in the "Done" button:
    frame .main_w.egsnrc.button
    button .main_w.egsnrc.button.done -text "Done" -command {
            destroy .main_w.egsnrc } -relief groove -bd 8
    pack .main_w.egsnrc.button.done
    pack $top .main_w.egsnrc.button -side top -pady $ypad

}

proc egsnrc_set_in_reg { param onoff }  {

    #will open up a box to set/display on/off regions for Atomic relaxations
    #(iedgfl), photoelectron angular sampling (iphter), bound compton
    #scattering (ibcmp) and Rayleigh scattering (iraylr)

    global iedgfl_min iedgfl_max iphter_min iphter_max ibcmp_min
    global ibcmp_max iraylr_min iraylr_max level ypad

    catch { destroy .main_w.egsnrc.${param} }
    toplevel .main_w.egsnrc.${param}
    set top .main_w.egsnrc.${param}

    if { $param=="iedgfl" } {
      wm title $top "Atomic relaxations"
      label $top.label -text "Atomic relaxations $onoff in regions:"
    } elseif { $param=="iphter" } {
      wm title $top "Photelectron angular sampling"
      label $top.label -text "Photoelectron angular sampling $onoff in regions:"
    } elseif { $param=="ibcmp" } {
      wm title $top "Bound compton scattering"
      label $top.label -text "Bound compton scattering $onoff in regions:"
    } elseif { $param=="iraylr" } {
      wm title $top "Rayleigh scattering"
      label $top.label -text "Rayleigh scattering $onoff in regions:"
    }

    pack $top.label -pady $ypad

    frame $top.grid -bd 4

    label $top.grid.fromlabel -text "From region"
    label $top.grid.tolabel -text "To region"
    grid configure $top.grid.fromlabel -row 0 -column 0
    grid configure $top.grid.tolabel -row 0 -column 1

    for {set i 1} {$i<=$level(${param})} {incr i} {
        entry $top.grid.from$i -relief sunken -textvariable ${param}_min($i)
        grid configure $top.grid.from$i -row $i -column 0
        entry $top.grid.to$i -relief sunken -textvariable ${param}_max($i)
        grid configure $top.grid.to$i -row $i -column 1
    }
    pack $top.grid -side top

    frame $top.b
    button $top.b.addb -text "Add a group" -command\
              "add_egsnrc_group $param" -relief groove -bd 8
    button $top.b.delb -text "Delete last group" -command\
              "del_egsnrc_group $param" -relief groove -bd 8
    button $top.b.okb -text "OK" -command "destroy $top" -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb -side left -padx 10
    pack $top.b -pady [expr 2*$ypad]

    if {$level(${param})==0} {
          $top.b.delb configure -state disabled
    }

}

proc add_egsnrc_group { param } {
#procedure to add a group of regions in which to turn an EGSnrc option on/off
    global iedgfl_min iedgfl_max iphter_min iphter_max ibcmp_min
    global ibcmp_max iraylr_min iraylr_max level

    set w .main_w.egsnrc.${param}

    set i [incr level(${param})]

    entry $w.grid.from$i -relief sunken -textvariable ${param}_min($i)
    grid configure $w.grid.from$i -row $i -column 0
    entry $w.grid.to$i -relief sunken -textvariable ${param}_max($i)
    grid configure $w.grid.to$i -row $i -column 1

    if {$level(${param})==1} {
         #re-enable the delete last group button
         $w.b.delb configure -state normal -command "del_egsnrc_group $param"
    }
}

proc del_egsnrc_group { param } {
#procedure to del. a group of regions in which to turn an EGSnrc option on/off
    global level

    set w .main_w.egsnrc.${param}

    destroy $w.grid.from$level(${param})
    destroy $w.grid.to$level(${param})

    incr level(${param}) -1

    if {$level(${param})==0} {
         $w.b.delb configure -state disabled
    }
}

proc egsnrc_set_rayl_custom { }  {
#opens a window to set media for custom Rayleigh cross sections and
#the associated files for each medium
    global num_rayl_custom rayl_cust_med rayl_cust_file values
    global medium inp_file_dir icust

    catch { destroy .main_w.egsnrc.rayl_custom }
    toplevel .main_w.egsnrc.rayl_custom
    set top .main_w.egsnrc.rayl_custom
    wm title $top "Set custom Rayleigh cross-sections"

    label $top.label -text "Media and associated Rayleigh cross-section files:"
    pack $top.label -pady 5

    frame $top.grid -bd 4

    label $top.grid.medlabel -text "Medium" -width 20
    label $top.grid.filelabel -text "x-section file"
    grid configure $top.grid.medlabel -row 0 -column 0
    grid configure $top.grid.filelabel -row 0 -column 1

    for {set i 1} {$i<=$num_rayl_custom} {incr i} {
        set icust $i
        menubutton $top.grid.med$i -text $rayl_cust_med($i) \
             -menu $top.grid.med$i.m -bd 1 -relief raised -indicatoron 1 \
             -width 20
        menu $top.grid.med$i.m -tearoff no
        for {set iopt 0} {$iopt <= $values(2)} {incr iopt} {
            $top.grid.med$i.m add command -label $medium($iopt) \
                -command "$top.grid.med$i configure -text $medium($iopt);\
                          set rayl_cust_med($i) $medium($iopt)"
        }
        grid configure $top.grid.med$i -row $i -column 0
        entry $top.grid.file$i -relief sunken \
                 -textvariable rayl_cust_file($i)
        grid configure $top.grid.file$i -row $i -column 1
        button $top.grid.fileselect$i -text "Browse" \
            -command "set icust $i; browse set_rayl_cust_file $inp_file_dir"
        grid configure $top.grid.fileselect$i -row $i -column 2
    }
    pack $top.grid -side top

    frame $top.b
    button $top.b.addb -text "Add a medium" -command\
              "add_rayl_cust_med" -relief groove -bd 8
    button $top.b.delb -text "Delete last medium" -command\
              "del_rayl_cust_med" -relief groove -bd 8
    button $top.b.okb -text "OK" -command "destroy $top" -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb -side left -padx 10
    pack $top.b -pady 10

    if {$num_rayl_custom==0} {
          $top.b.delb configure -state disabled
    }
}

proc set_rayl_cust_file { } {
#procedure to set the name of rayl_cust_file($i), the file containing
#custom Rayleigh cs data for rayl_cust_med($i)
global rayl_cust_file filename inp_file_dir icust

# if nothing was entered, return
    if { [string compare $filename ""]==0 } {
        tk_dialog .query.empty "Empty string" "No filename was entered.  \
                Please try again." error 0 OK
        return
    }
    set rayl_cust_file($icust) [file join [pwd] $filename]
}

proc add_rayl_cust_med { } {
#procedure to add a medium for which to specify custom Rayleigh cs

    global rayl_cust_med rayl_cust_file num_rayl_custom values
    global medium inp_file_dir icust

    set top .main_w.egsnrc.rayl_custom

    set i [incr num_rayl_custom]
    if {$rayl_cust_med($i)==""} {
       menubutton $top.grid.med$i -text "select medium" \
             -menu $top.grid.med$i.m -bd 1 -relief raised -indicatoron 1 \
             -width 20
    } else {
       menubutton $top.grid.med$i -text $rayl_cust_med($i) \
             -menu $top.grid.med$i.m -bd 1 -relief raised -indicatoron 1 \
             -width 20
    }
    menu $top.grid.med$i.m -tearoff no
    for {set iopt 0} {$iopt <= $values(2)} {incr iopt} {
            $top.grid.med$i.m add command -label $medium($iopt) \
                -command "$top.grid.med$i configure -text $medium($iopt);\
                          set rayl_cust_med($i) $medium($iopt)"
    }
    grid configure $top.grid.med$i -row $i -column 0
    entry $top.grid.file$i -relief sunken \
                 -textvariable rayl_cust_file($i)
    grid configure $top.grid.file$i -row $i -column 1
    button $top.grid.fileselect$i -text "Browse" \
            -command "set icust $i; browse set_rayl_cust_file $inp_file_dir"
    grid configure $top.grid.fileselect$i -row $i -column 2

    if {$num_rayl_custom==1} {
         #re-enable the delete last group button
         $top.b.delb configure -state normal
    }
}

proc del_rayl_cust_med { } {
#procedure to del. a medium for which custom Rayleigh cs has been defined

    global num_rayl_custom

    set top .main_w.egsnrc.rayl_custom

    destroy $top.grid.med$num_rayl_custom
    destroy $top.grid.file$num_rayl_custom
    destroy $top.grid.fileselect$num_rayl_custom

    incr num_rayl_custom -1

    if {$num_rayl_custom==0} {
         $top.b.delb configure -state disabled
    }
}

proc config_esplit { n_split } {
#procedure to disable or enable e_split input when source 2,8, or 9 input window
#is open at the same time that the user enters a value of n_split in the main
#input window
      if { [winfo exists .main_w.srcopt.optfrm.src2.esplit]==1 } {
         if {$n_split>1} {
           .main_w.srcopt.optfrm.src2.esplit.inp configure -state normal
           .main_w.srcopt.optfrm.src2.esplit.lab configure -fg black
         } else {
           .main_w.srcopt.optfrm.src2.esplit.inp configure -state disabled
           .main_w.srcopt.optfrm.src2.esplit.lab configure -fg grey
         }
      }
}
