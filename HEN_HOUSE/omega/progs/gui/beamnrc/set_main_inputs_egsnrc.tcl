
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: set main input parameters
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


proc set_main_inputs {} {

    # main procedure for the set main parameters window.  Puts the text boxes/
    # option menus up on the screen.

    catch { destroy .main_w }
    toplevel .main_w
    global values names options numopts GUI_DIR num_cm nmed medium is_pegsless
    global med_per_column

    wm title .main_w "Main Inputs"

    if {$num_cm==0} {
	# POP UP AN INFO WINDOW TELLING THE USER THAT THEY NEED TO EITHER
	# LOAD AN ACCELERATOR OR SPECIFY A NEW ACCELERATOR
	tk_dialog .main_w "Try again..." "You haven't got an\
		accelerator to\
		edit!  Load an old accelerator or specify a new one\
		and try again." warning 0 OK
    } else {
	# WINDOW FOR SETTING THE MAIN OPTIONS -- FOR EACH ADD A HELP BUTTON,
	# A LABEL, THEN A TEXT BOX OR OPTION MENU (WHICHEVER IS APPROPRIATE).
	# NOTE THAT OPTION MENUS SPAWN CHILD WINDOWS FOR SOME PARAMETERS.

	set top .main_w.main
	frame $top -bd 5

	frame $top.left
	set top1 $top.left

	# This may seem disorganized, but I got tired of renumbering
	# parameters.  The rng options box has been moved to beside the
	# rng numbers and because the number of brem photons is now an
	# offshoot of brem splitting, (15) is gone.  Also (16) russian
	# roulette, is an offshoot if IBRSPL>0.

	# The title spans the window.  The rest lie on 2 frames.
	set i 1
	frame $top.inp$i -bd 4
	button $top.inp$i.help -bitmap @$GUI_DIR/help_icon.xbm \
		-command "help $i"
	label $top.inp$i.label -text $names($i)  -anchor w
	entry $top.inp$i.inp -textvariable values($i) -width 80
	pack $top.inp$i.help -anchor w -side left
	pack $top.inp$i.label -side left -padx 10
	pack $top.inp$i.inp -anchor e -side right -fill x -expand true
	pack $top.inp$i -side top -fill x

	# Medium: an option menu with medium($i)
	frame $top1.inp2 -bd 4
	button $top1.inp2.help -bitmap @$GUI_DIR/help_icon.xbm \
		-command "help 2"
	label $top1.inp2.label -text $names(2)  -anchor w
	set width 20
	menubutton $top1.inp2.inp -text $values(2) -menu \
		$top1.inp2.inp.m -bd 1 -relief raised -indicatoron 1\
		-width $width
	menu $top1.inp2.inp.m -tearoff no
    # multicolumn menu, 40 media per column
	for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
            $top1.inp2.inp.m add command -label $medium($iopt) -columnbreak [expr $iopt % $med_per_column == 0]\
                    -command "set_medium $iopt"
	}
	pack $top1.inp2.help -anchor w -side left
	pack $top1.inp2.label -side left -padx 10
	pack $top1.inp2.inp -anchor e -side right -fill x -expand true
	pack $top1.inp2 -side top -fill x


	for {set i 3} {$i < 14} {incr i} {
	    frame $top1.inp$i -bd 4
	    button $top1.inp$i.help -bitmap @$GUI_DIR/help_icon.xbm \
		    -command "help $i"
	    label $top1.inp$i.label -text $names($i)  -anchor w
	    if [expr $numopts($i) > 0] {
		make_menu_button $i $top1.inp$i
	    } else {
		make_text_box $i $top1.inp$i
	    }
	    pack $top1.inp$i.help -anchor w -side left
	    pack $top1.inp$i.label -side left -padx 10
	    pack $top1.inp$i.inp -anchor e -side right -fill x -expand true
	    pack $top1.inp$i -side top -fill x
	}

	# RIGHT SIDE OF FRAME

        frame $top.right
        set top2 $top.right

	foreach i {14 41 40 17 18 ecut pcut 19 20 21 22 23 24} {
	    frame $top2.inp$i -bd 4
	    button $top2.inp$i.help -bitmap @$GUI_DIR/help_icon.xbm\
		    -command "help $i"
	    label $top2.inp$i.label -text $names($i) -anchor w
	    if [expr $numopts($i) > 0] {
		make_menu_button $i $top2.inp$i
	    } else {
		make_text_box $i $top2.inp$i
	    }
	    pack $top2.inp$i.help -anchor w -side left
	    pack $top2.inp$i.label -side left -padx 10
	    pack $top2.inp$i.inp -anchor e -side right -fill x -expand true
	    pack $top2.inp$i -side top -fill x
	}
	# configure esave by range rejection
	for {set j 0} {$j<3} {incr j} {
	    if [string compare $values(19) $options(19,$j)]==0 {
		set_value 19 $j $top2.inp19.inp
	    }
	}

	pack $top1 -side left -anchor n
	pack $top2 -side right -anchor n

        #put in the "Done" button and an option to edit the EGSnrc parameters
	frame .main_w.button
        button .main_w.button.b1 -text "Define Media" -command "define_pegsless_media" \
              -relief groove -bd 8
        button .main_w.button.b2 -text "Edit EGSnrc Parameters" -command\
                          "set_egsnrc_parameters" -relief groove -bd 8
	button .main_w.button.b3 -text "Close" -command {
	    if { [string compare $values(19) $options(19,0)]!=0 \
		    && $values(20)==0 } {
		tk_dialog .warning "Warning" "You have selected range\
			rejection but have left the global ESAVE value at\
			zero.  This effectively turns off range rejection.  \
			Change the value of ESAVE\
			to turn range rejection back on."  warning 0 OK
	    }
	    destroy .main_w } -relief groove -bd 8
        pack .main_w.button.b1 -side left -padx 10
        pack .main_w.button.b2 -side left -padx 10
        pack .main_w.button.b3 -side left -padx 10
        if {$is_pegsless==0} {
          .main_w.button.b1 configure -state disabled
        }
	pack $top .main_w.button -side top -pady 5
    }
}
# this is the end of proc setinputs

proc set_medium { iopt } {
    global values medium
    set values(2) $medium($iopt)
    .main_w.main.left.inp2.inp configure -text $values(2)
}

proc make_menu_button {i w} {
    # creates a menu button.
    # sources 17 and 8 are special cases with numbering not (0,1,2,...)

    global values numopts options num_cm

    set width [string length $values($i)]
    if $width<20 { set width 20 }
    menubutton $w.inp -text $values($i) -menu $w.inp.m -bd 1 \
	    -relief raised -indicatoron 1 -width $width

    # MAKE THE MENU, ADD OPTIONS AND CALLBACKS
    menu $w.inp.m -tearoff no
    if $i==17 {
	# IT'S ELECTRON, PHOTON, POSITRON (-1,0,1)
	if {[string compare 21 [string range $values(18) 0 1]]==0 &&
            [string compare 24 [string range $values(18) 0 1]]==0 &&
            [string compare 23 [string range $values(18) 0 1]]==0 } {
	    # SOURCE 21, 24 or 23 - PHASE SPACE or sim source
	    for {set iopt -1} {$iopt <=1} {incr iopt} {
		$w.inp.m add command -label $options($i,$iopt) \
			-command "set_value $i $iopt $w.inp" -state disabled
	    }
	    $w.inp.m add command -label $options(17,2) \
		    -command "set_value $i $iopt $w.inp"
	} else {
	    # NOT SOURCE 21, 24 or 23
	    for {set iopt -1} {$iopt <=1} {incr iopt} {
		$w.inp.m add command -label $options($i,$iopt) \
			-command "set_value $i $iopt $w.inp"
	    }
	    $w.inp.m add command -label $options(17,2) -state disabled \
		    -command "set_value $i $iopt $w.inp"
	}
    } elseif $i==8 {
	# IT'S LATCH (1,2,3)
	for {set iopt 1} {$iopt <=3} {incr iopt} {
	    $w.inp.m add command -label $options($i,$iopt) \
		    -command "set_value $i $iopt $w.inp"
	}
    } elseif $i==5 {
	# IT'S IRESTART (0,1,3,4) (no create input file option)
	foreach iopt {0 1 3 4} {
	    $w.inp.m add command -label $options($i,$iopt) \
		    -command "set_value $i $iopt $w.inp"
	}
    } elseif $i==4 {
	# IT'S ISTORE (-1,0,1)
	for {set iopt -1} {$iopt <=1} {incr iopt} {
	    $w.inp.m add command -label $options($i,$iopt) \
		    -command "set_value $i $iopt $w.inp"
	}
    } elseif $i==3 {
	# IT'S IWATCH, OPTION 3 REMOVED
	foreach iopt {0 1 2 4 5} {
	    $w.inp.m add command -label $options($i,$iopt) \
		    -command "set_value $i $iopt $w.inp"
	}
    } elseif $i==40 {
	# IT'S ICM_SPLIT, HAVE TO LIMIT IT TO THE NUMBER OF CMS AVAILABLE.
	for {set iopt 0} {$iopt<=$num_cm} {incr iopt} {
	    $w.inp.m add command -label $options($i,$iopt) \
		    -command "set_value $i $iopt $w.inp"
	}
    } else {
	for {set iopt 0} {$iopt < $numopts($i)} {incr iopt} {
	    $w.inp.m add command -label $options($i,$iopt) \
		    -command "set_value $i $iopt $w.inp"
	}
    }
}

proc make_text_box {i w} {
    # creates a text box
    global values
    set width [string length $values($i)]
    if $width<20 { set width 20 }
    entry $w.inp -relief sunken -textvariable values($i) -width $width
}

proc set_value {io iopt w} {

    # sets the values of the parameters and calls offshoot procedures if
    # required.

    global values options special_N medium nmed inp_file_dir

    if { $io==3 } {
	if { $iopt==5 } {
	    # IWATCH (SPECIAL)
	    set values(3) $options(3,5)
	    set_special_N
	    $w configure -text $options(3,5)
	} elseif { $iopt==4 } {
	    set values(3) $options(3,4)
	    $w configure -text $values(3)
	    # make sure that zlast output is not set
	    if [string compare $values(9) $options(9,2)]==0 {
		tk_dialog .info "Info" "You may not have output for EGS_Windows\
		as well as outputting the x,y,z of the last interaction.\
			This will result in errors in EGS_Windows since they\
			use the same output file.  Score last Z reset to no."\
			info 0 OK
		set values(9) $options(9,0)
		.main_w.main.left.inp9.inp configure -text $values(9)
	    }
	} else {
	    set values($io) $options($io,$iopt)
	    $w configure -text $values($io)
	}
    } elseif { $io==6 & $iopt>4 } {
        # these output options require the user to modify the input
        # file manually...for now.  Output a warning
        tk_dialog .warning "Smile: It's only a Warning" "In order for this\
                        option to work you will have\
                        to modify the input file manually before running\
                        BEAM." warning 0 OK
        set values($io) $options($io,$iopt)
        $w configure -text $values($io)
    } elseif { $io==8 & $iopt==1 } {
	# check that contaminant scoring is not on
	if [string compare $values(23) $options(23,1)]==0 {
	    tk_dialog .error "Not allowed" "Dose contaminant scoring is\
		    incompatible with the non-inherited LATCH setting." \
		    error 0 OK
	    return
	}
	set values($io) $options($io,$iopt)
	$w configure -text $values($io)
    } elseif { $io==9 & $iopt==2 } {
	set values(9) $options(9,2)
	$w configure -text $values(9)
	# make sure that for EGS_Windows output is not set
	if [string compare $values(3) $options(3,4)]==0 {
	    tk_dialog .info "Info" "You may not have output for EGS_Windows\
		    as well as outputting the x,y,z of the last interaction.\
		    This will result in errors in EGS_Windows since they\
		    use the same output file.  IWATCH output reset to none."\
		    info 0 OK
	    set values(3) $options(3,0)
	    .main_w.main.left.inp3.inp configure -text $values(3)
	}
    } elseif { $io==14 } {
	set values(14) $options(14,$iopt)
	if { $iopt==1 | $iopt==2 } {
	    # call set_nbrem for brem splitting option
	    set_nbrem $iopt
	} else {
	    set values(16) $options(16,0)
	    set values(15) 0
	}
	$w configure -text $options(14,$iopt)
    } elseif { $io==41 } {
        set values(41) $options(41,$iopt)
        if { $iopt==1 } {
            set_bcse
        }
        $w configure -text $options(41,$iopt)
    } elseif { $io==18 } {
	# SOURCE MENU
	set values(18) $options(18,$iopt)
	set_src_options $iopt
        $w configure -text $options(18,$iopt)
	# DESTROY THE OLD (e+,photon,e-,ps) MENU AND RE-CREATE A NEW
	# ONE WITH THE APPROPRIATE BUTTONS (SOURCE 21 SPECIAL CASE).
	destroy .main_w.main.right.inp17.inp.m
	set top2 .main_w.main.right.inp17
	menu $top2.inp.m -tearoff no
	if {[string compare 21 [string range $values(18) 0 1]]==0 |
            [string compare 23 [string range $values(18) 0 1]]==0 |
            [string compare 24 [string range $values(18) 0 1]]==0} {
	    # SOURCE 21 23 or 24
	    $top2.inp configure -text $options(17,2)
	    for {set iopt2 -1} {$iopt2 <=1} {incr iopt2} {
		$top2.inp.m add command -label $options(17,$iopt2) \
			-command "set_value 17 $iopt2 $top2.inp" -state disabled
	    }
	    $top2.inp.m add command -label $options(17,2) \
		    -command "set_value 17 2 $top2.inp"
	} else {
	    # NOT SOURCE 21 - if phase space is selected, remove
	    if [string compare $values(17) $options(17,2)]==0 {
		$top2.inp configure -text $options(17,0)
	    }
	    for {set iopt2 -1} {$iopt2 <=1} {incr iopt2} {
		$top2.inp.m add command -label $options(17,$iopt2) \
			-command "set_value 17 $iopt2 $top2.inp"
	    }
	    $top2.inp.m add command -label $options(17,2) -state disabled \
		    -command "set_value 17 2 $top2.inp"
	}
    } elseif { $io==19 } {
	# range rejection on/off
        set values($io) $options($io,$iopt)
        $w configure -text $values($io)
	if { $iopt==0 } {
	    # disable esave entrybox, set esave to {}
	    .main_w.main.right.inp20.inp configure -state disabled
	    .main_w.main.right.inp20.label configure -fg grey
	    set values(20) {}
	} else {
	    # enable esave entry box
	    .main_w.main.right.inp20.inp configure -state normal
	    .main_w.main.right.inp20.label configure -fg black
	}
    } elseif { $io==21 & $iopt==1 } {
	# PHOTON FORCING ON
	set values(21) $options(21,1)
        set_force_options
        $w configure -text $options(21,$iopt)
    } elseif { $io==22 } {
	# NUMBER OF SCORING PLANES
	set values(22) $options(22,$iopt)
	set_scoring_options $values(22)
	$w configure -text $options(22,$iopt)
    } elseif { $io == 23 & $iopt==1} {
	# DOSE CALCULATIONS
	if [string compare $values(8) $options(8,1)]==0 {
	    # Using LATCH_OPTION=1 - not allowed.  give error.
	    tk_dialog .error "Not allowed" "Dose contaminant scoring is\
		    incompatible with the non-inherited LATCH setting." \
		    error 0 OK
	    return
	}
	set values(23) $options(23,1)
	set_itdose_on
	$w configure -text $options(23,$iopt)
    } elseif { $io==40 } {
	# ICM_SPLIT: IF >0 GET NSPLIT_PHOT, NSPLIT_ELEC
	set values(40) $options(40,$iopt)
	if { $iopt>0 } {
	    get_nsplit
	}
	$w configure -text $values(40)
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
    }  elseif {$io=="iedgfl"} {
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

proc get_nsplit {} {
    global values
    catch {destroy .nsplit}
    toplevel .nsplit -bd 4
    wm title .nsplit "Split photons and electrons"
    frame .nsplit.grid
    set w .nsplit.grid
    label $w.l1 -text "Split photons this many times:"
    label $w.l2 -text "Split electrons this many times:"
    entry $w.e1 -textvariable values(40,phot)
    entry $w.e2 -textvariable values(40,elec)
    grid config $w.l1 -row 0 -column 0 -sticky e
    grid config $w.l2 -row 1 -column 0 -sticky e
    grid config $w.e1 -row 0 -column 1
    grid config $w.e2 -row 1 -column 1
    pack $w -pady 8
    button .nsplit.b -text "OK" -command {
	if { [catch {expr $values(40,phot)}]==0 & \
		[catch {expr $values(40,elec)}]==0 } {
	    # numbers were input, make sure they're okay
	    if $values(40,phot)<1 {
		set values(40,phot) 1
		tk_dialog .message "Notice" "The photon splitting number\
			has been reset to 1.  This number must be 1 or greater."\
			info 0 OK
	    }
	    if $values(40,elec)<1 {
		set values(40,elec) 1
		tk_dialog .message "Notice" "The electron splitting number\
			has been reset to 1.  This number must be 1 or greater."\
			info 0 OK
	    }
	}
	destroy .nsplit
    } -relief groove -bd 8
    pack .nsplit.b -pady 10
}

proc set_nbrem { iopt } {

    # number of brem photons and russian roulette options.
    # uniform or directional bremsstrahlung splitting

    global values options sbrem dbrem numopts num_cm cm_type cm_ident cm_names
    global espl_plane espl_z i_dsb splitcm_dsb dsb_delta

    catch { destroy .main_w.nbrem }
    toplevel .main_w.nbrem
    if {$iopt==3} {
      wm title .main_w.nbrem "Directional Source Biasing Parameters"
    } else {
      wm title .main_w.nbrem "Brem photons"
    }
    set top .main_w.nbrem

    if {$iopt==2 || $iopt==3 } {
        frame $top.f2 -bd 4
        label $top.f2.lab1 -text "Splitting field radius (cm)"
        entry $top.f2.en -textvariable dbrem(1) -width 4
        pack $top.f2.lab1 -anchor w -side left
        pack $top.f2.en  -anchor e -side right -padx 5 -fill x -expand 1
        pack $top.f2 -side top -pady 1 -fill x

        frame $top.f3 -bd 4
        label $top.f3.lab1 -text "Source to surface distance (cm)"
        entry $top.f3.en -textvariable dbrem(2) -width 4
        pack $top.f3.lab1 -anchor w -side left
        pack $top.f3.en  -anchor e -side right -padx 5 -fill x -expand 1
        pack $top.f3 -side top -pady 1 -fill x

        frame $top.f1 -bd 4
        if {$iopt == 3} {
           label $top.f1.lab1 -text "Photon splitting number"
        } else {
           label $top.f1.lab1 -text "Brem splitting number"
        }
        entry $top.f1.en -textvariable values(15) -width 4
        pack $top.f1.lab1 -anchor w -side left
        pack $top.f1.en  -anchor e -side right -padx 5 -fill x -expand 1
        pack $top.f1 -side top -pady 1 -fill x

        if {$iopt == 3} {

          frame $top.f11 -bd 4
          label $top.f11.lab1 -text "CM for photon splitting/radial redistribution"
          if {$splitcm_dsb==0} {
           menubutton $top.f11.inp \
          -text "no photon splitting"\
          -menu $top.f11.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
          } else {
           menubutton $top.f11.inp \
          -text "$splitcm_dsb   ($cm_ident($splitcm_dsb))" \
          -menu $top.f11.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
          }
          menu $top.f11.inp.m -tearoff no
          for {set icm 0} {$icm<=$num_cm} {incr icm} {
            if {$icm==0 } {
             $top.f11.inp.m add command -label "no photon splitting" \
                -command "set splitcm_dsb 0; $top.f11.inp configure \
                -text {no photon splitting}; $top.f12.en configure -state disabled"
            } else {
             $top.f11.inp.m add command \
                -label "$icm   ($cm_ident($icm))" \
                -command "set splitcm_dsb $icm; $top.f11.inp configure \
                -text {$icm   ($cm_ident($icm))}; $top.f12.en configure -state normal"
            }
          }
          pack $top.f11.lab1 -anchor w -side left
          pack $top.f11.inp  -anchor e -side right -padx 5 -fill x -expand 1
          pack $top.f11 -side top -pady 1 -fill x

          frame $top.f12 -bd 4
          label $top.f12.lab1 -text "Min. linear distance between split photons (cm)"
          entry $top.f12.en -textvariable dsb_delta -width 4
          pack $top.f12.lab1 -anchor w -side left
          pack $top.f12.en  -anchor e -side right -padx 5 -fill x -expand 1
          pack $top.f12 -side top -pady 1 -fill x
          if {$splitcm_dsb==0} {
             $top.f12.en configure -state disabled
          }
        }

        frame $top.f10 -bd 4
        button $top.f10.rejplnhelp -text "?" -command {help dbs_rejpln}
        label $top.f10.lab1 -text "   Use rejection plane"
        checkbutton $top.f10.userejpln -variable dbrem(use_rejpln) -command\
              "config_zrejpln $top"
        label $top.f10.lab2 -text "Z (cm) of rejection plane"
        entry $top.f10.zrejpln -textvariable dbrem(z_rejpln)
        pack $top.f10.rejplnhelp -side left
        pack $top.f10.lab1 $top.f10.userejpln $top.f10.lab2 $top.f10.zrejpln \
            -side left
        pack $top.f10 -side top -pady 1 -fill x
        config_zrejpln $top

        frame $top.f4 -bd 4
        label $top.f4.lab1 -text "CM for e-/e+ splitting"
        if {$dbrem(3)==0} {
           menubutton $top.f4.inp \
          -text "no e-/e+ splitting"\
          -menu $top.f4.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
        } else {
          menubutton $top.f4.inp \
          -text "$dbrem(3)   ($cm_ident($dbrem(3)))" \
          -menu $top.f4.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
        }
        menu $top.f4.inp.m -tearoff no
        for {set icm 0} {$icm<=$num_cm} {incr icm} {
          if {$icm==0 } {
           $top.f4.inp.m add command -label "no e-/e+ splitting" \
                -command "set dbrem(3) 0; $top.f4.inp configure \
                -text {no e-/e+ splitting}; config_esplplane_menu $icm $top"
          } elseif {$cm_type($icm)==4 || $cm_type($icm)==9} {
           #only support splitting planes in FLATFILT and PYRAMIDS for now
           $top.f4.inp.m add command \
                -label "$icm   ($cm_ident($icm))" \
                -command "set dbrem(3) $icm; $top.f4.inp configure \
                -text {$icm   ($cm_ident($icm))}; \
                 config_esplplane_menu $icm $top"
          }
        }
        pack $top.f4.lab1 -anchor w -side left
        pack $top.f4.inp  -anchor e -side right -padx 5 -fill x -expand 1
        pack $top.f4 -side top -pady 1 -fill x

        frame $top.f5 -bd 5
        label $top.f5.lab1 -text "e-/e+ splitting plane no."
        if {$dbrem(3)==0} {
           menubutton $top.f5.inp -text "no CM selected" -state disabled \
           -menu $top.f5.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
           $top.f5.lab1 configure -fg grey
        } elseif {$espl_plane($dbrem(3))==0} {
           menubutton $top.f5.inp -text "define CM geometry" \
           -state disabled \
           -menu $top.f5.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
           $top.f5.lab1 configure -fg grey
        } elseif {$dbrem(4)==0 || $dbrem(4)>$espl_plane($dbrem(3))} {
           menubutton $top.f5.inp -text "select plane" \
           -menu $top.f5.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
        } else {
           menubutton $top.f5.inp \
           -text "$dbrem(4)   (Z=$espl_z($dbrem(3),$dbrem(4)) cm)" \
           -menu $top.f5.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
        }
        menu $top.f5.inp.m -tearoff no
        if {$dbrem(3)>0 && $espl_plane($dbrem(3))>0} {
            for {set i 1} {$i<=$espl_plane($dbrem(3))} {incr i} {
                $top.f5.inp.m add \
                command -label "$i   (Z=$espl_z($dbrem(3),$i) cm)" \
                -command "set dbrem(4) $i; $top.f5.inp configure \
                -text {$i   (Z=$espl_z($dbrem(3),$i) cm)}"
            }
        }
        pack $top.f5.lab1 -anchor w -side left
        pack $top.f5.inp  -anchor e -side right -padx 5 -fill x -expand 1
        pack $top.f5 -side top -pady 1 -fill x

        frame $top.f7 -bd 4
        label $top.f7.lab1 -text "Z of Russian Roulette plane (cm)"
        entry $top.f7.en -textvariable dbrem(6) -width 4
        if {$dbrem(3)==0 || $espl_plane($dbrem(3))==0} {
           $top.f7.en configure -state disabled
           $top.f7.lab1 configure -fg grey
        }
        pack $top.f7.lab1 -anchor w -side left
        pack $top.f7.en  -anchor e -side right -padx 5 -fill x -expand 1
        pack $top.f7 -side top -pady 1 -fill x

        frame $top.f8 -bd 4
        label $top.f8.lab1 -text "Redistribution of split e-/e+"
        frame $top.f8.rad -bd 4
        radiobutton $top.f8.rad.but1 -text "Do not redistribute" \
                -variable dbrem(5) -value 0
        radiobutton $top.f8.rad.but2 -text "Radially-symmetric redistribution" \
                -variable dbrem(5) -value 1
        if {$dbrem(3)==0 || $espl_plane($dbrem(3))==0} {
             $top.f8.rad.but1 configure -state disabled
             $top.f8.rad.but2 configure -state disabled
             $top.f8.lab1 configure -fg grey
        }
        pack $top.f8.rad.but1 $top.f8.rad.but2 -side top -anchor w
        pack $top.f8.lab1 -anchor w -side left
        pack $top.f8.rad -anchor e -side right -padx 5 -fill x -expand 1
        pack $top.f8

        frame $top.f9 -bd 4
        frame $top.f9.chbx
        button $top.f9.help -text "?" -command {help dbs_rr}
        label $top.f9.chbx.lab1 -text "Augmented range rejection"
        checkbutton $top.f9.chbx.offon -variable dbrem(7) -command \
            "puts $dbrem(7)"
        pack $top.f9.chbx.lab1 $top.f9.chbx.offon -side left -anchor w
        if {$dbrem(3)==0 || $espl_plane($dbrem(3))==0} {
          $top.f9.chbx.lab1 configure -fg grey
          $top.f9.chbx.offon configure -state disabled
        }
        pack $top.f9.help -anchor w -side left
        pack $top.f9.chbx -anchor w -side left -padx 10
        pack $top.f9 -anchor w
    } else {

        #turn off dsb
        set i_dsb 0

        if {[winfo exists .main_w.srcopt]==1 && $values(18)==$options(18,2)} {
          .main_w.srcopt.dsb.params configure -state disabled
        }

	frame $top.f1 -bd 4
	label $top.f1.lab1 -text "Splitting factor (number of brem photons)"
	entry $top.f1.en -textvariable values(15) -width 4
	pack $top.f1.lab1 -anchor w -side left
	pack $top.f1.en  -anchor e -side right -padx 5 -fill x -expand 1
	pack $top.f1 -side top -pady 5 -fill x
    }

    if {$iopt!=2 && $iopt!=3} {
      frame $top.f5 -bd 4
      label $top.f5.lab1 -text "Russian Roulette"
      menubutton $top.f5.inp -text $values(16) -menu $top.f5.inp.m -bd 1 \
	    -relief raised -indicatoron 1 -width [string length $options(16,0)]
      # MAKE THE MENU, ADD OPTIONS AND CALLBACKS
      menu $top.f5.inp.m -tearoff no
      for {set i 0} {$i<$numopts(16)} {incr i} {
	$top.f5.inp.m add command -label $options(16,$i) \
		-command "set values(16) {$options(16,$i)}; \
		$top.f5.inp configure -text {$options(16,$i)};\
                if {{$values(14)}=={$options(14,1)} && \
                    {$values(41)}=={$options(41,1)}} {
                     coordinate_rr ubs
                }"
      }
      if {$iopt==1 && $values(41)==$options(41,1)} {
           coordinate_rr bcse
      }
      pack $top.f5.lab1 -anchor w -side left
      pack $top.f5.inp  -side right -expand 1 -fill x -padx 5
      pack $top.f5 -side top -pady 5 -fill x

      frame $top.sep -bd 4 -width 100 -height 2 -relief groove
      pack $top.sep -side top -fill x -pady 10
    }

    frame $top.f6 -bd 4
    button $top.f6.b1 -text "OK" -command "destroy .main_w.nbrem" \
	    -relief groove -bd 8
    button $top.f6.b2 -text "Help" -command "help_nbrem $iopt .main_w.nbrem"\
	    -relief groove -bd 8
    pack $top.f6.b2 $top.f6.b1 -side left -padx 10

    pack $top.f6 -side top -pady 5
}

proc config_esplplane_menu { icm w } {
#procedure to reconfigure the splitting plane menu when a new CM is
#selected, also takes care of enabling/disabling inputs for RR plane,
#electron redistribution and augmented range rejection
    global dbrem espl_plane espl_z

    if {$icm==0} {
        $w.f5.inp configure -text "no CM selected" -state disabled
        $w.f7.en configure -state disabled
        $w.f8.rad.but1 configure -state disabled
        $w.f8.rad.but2 configure -state disabled
        $w.f9.chbx.offon configure -state disabled
        $w.f5.lab1 configure -fg grey
        $w.f7.lab1 configure -fg grey
        $w.f8.lab1 configure -fg grey
        $w.f9.chbx.lab1 configure -fg grey
    } elseif {$espl_plane($icm)==0} {
        $w.f5.inp configure -text "define CM geometry" -state disabled
        $w.f7.en configure -state disabled
        $w.f8.rad.but1 configure -state disabled
        $w.f8.rad.but2 configure -state disabled
        $w.f9.chbx.offon configure -state disabled
        $w.f5.lab1 configure -fg grey
        $w.f7.lab1 configure -fg grey
        $w.f8.lab1 configure -fg grey
        $w.f9.chbx.lab1 configure -fg grey
    } else {
        if {$dbrem(4)==0 || $dbrem(4)>$espl_plane($icm)} {
          $w.f5.inp configure -text "select plane" -state normal
        } else {
          $w.f5.inp configure -text "$dbrem(4)   (Z=$espl_z($icm,$dbrem(4)) cm)" \
            -state normal
        }
        $w.f7.en configure -state normal
        $w.f8.rad.but1 configure -state normal
        $w.f8.rad.but2 configure -state normal
        $w.f9.chbx.offon configure -state normal
        $w.f5.lab1 configure -fg black
        $w.f7.lab1 configure -fg black
        $w.f8.lab1 configure -fg black
        $w.f9.chbx.lab1 configure -fg black
    }
    destroy $w.f5.inp.m
    menu $w.f5.inp.m -tearoff no
    if {$icm>0 && $espl_plane($icm)>0} {
        for {set i 1} {$i<=$espl_plane($icm)} {incr i} {
                $w.f5.inp.m add \
                command -label "$i   (Z=$espl_z($icm,$i) cm)" \
                -command "set dbrem(4) $i; $w.f5.inp configure \
                -text {$i   (Z=$espl_z($icm,$i) cm)}"
        }
    }
}

proc set_bcse { } {

   #procedure to set options for BCSE
   global bcse_meds bcse_constant nmed medium num_bcse_meds values
   global options numopts
   global bcse_rr bcse_power

   if {$values(14)==$options(14,1)} {
   coordinate_rr ubs
   }

   catch { destroy .main_w.bcseopts }
   toplevel .main_w.bcseopts
   wm title .main_w.bcseopts "BCSE inputs"
   set top .main_w.bcseopts

   frame $top.inp -bd 4

   frame $top.inp.grid

   label $top.inp.grid.medlabel -text "Media:" -width 20
   grid configure $top.inp.grid.medlabel -row 0 -column 0

   for {set i 1} {$i<=$num_bcse_meds} {incr i} {
   # will eventually allow for BCSE factor to be specified for > 1 medium
        menubutton $top.inp.grid.med$i -text $bcse_meds($i) \
              -menu $top.inp.grid.med$i.m -bd 1 -relief raised -indicatoron 1 \
              -width 20
        menu $top.inp.grid.med$i.m -tearoff no
        for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
             $top.inp.grid.med$i.m add command -label $medium($iopt) \
                 -command "$top.inp.grid.med$i configure -text $medium($iopt);\
                           set bcse_meds($i) $medium($iopt)"
        }
        grid configure $top.inp.grid.med$i -row $i -column 0
   }

   frame $top.inp.constpowrr
   frame $top.inp.constpowrr.const
   label $top.inp.constpowrr.const.lab -text "enhancement constant:"
   entry $top.inp.constpowrr.const.inp -relief sunken -textvariable bcse_constant
   pack $top.inp.constpowrr.const.lab $top.inp.constpowrr.const.inp -side top
   frame $top.inp.constpowrr.pow
   label $top.inp.constpowrr.pow.lab -text "enhancement power:"
   entry $top.inp.constpowrr.pow.inp -relief sunken -textvariable bcse_power
   pack $top.inp.constpowrr.pow.lab $top.inp.constpowrr.pow.inp -side top
   frame $top.inp.constpowrr.rr
   label $top.inp.constpowrr.rr.lab -text "Russian Roulette:"
   menubutton $top.inp.constpowrr.rr.inp -text $bcse_rr\
             -menu $top.inp.constpowrr.rr.inp.m -bd 1 \
            -relief raised -indicatoron 1 -width 10
   # MAKE THE MENU, ADD OPTIONS AND CALLBACKS
   menu $top.inp.constpowrr.rr.inp.m -tearoff no
   for {set i 0} {$i<$numopts(16)} {incr i} {
        $top.inp.constpowrr.rr.inp.m add command -label $options(16,$i) \
                -command "set bcse_rr {$options(16,$i)}; \
                $top.inp.constpowrr.rr.inp configure -text {$options(16,$i)};\
                if {{$values(14)}=={$options(14,1)}} {
                     coordinate_rr bcse
                }"
   }
   if {$values(14)==$options(14,3)} {
        $top.inp.constpowrr.rr.lab configure -fg grey
        $top.inp.constpowrr.rr.inp configure -state disabled
   } else {
        $top.inp.constpowrr.rr.lab configure -fg black
        $top.inp.constpowrr.rr.inp configure -state normal
   }
   pack $top.inp.constpowrr.rr.lab $top.inp.constpowrr.rr.inp -side top
   pack $top.inp.constpowrr.const $top.inp.constpowrr.pow \
        $top.inp.constpowrr.rr -side top -pady 1

   pack $top.inp.grid $top.inp.constpowrr -side left -padx 2

   frame $top.b
   button $top.b.addb -text "Add a medium" -command\
              "bcse_med add" -relief groove -bd 8
   button $top.b.delb -text "Delete last medium" -command\
              "bcse_med del" -relief groove -bd 8
   button $top.b.okb -text "OK" -command "destroy $top" -relief groove -bd 8
   pack $top.b.addb $top.b.delb $top.b.okb -side left -padx 10
   pack $top.inp $top.b -side top -pady 10

   if {$num_bcse_meds==0} {
          $top.b.delb configure -state disabled
   }
}

proc bcse_med { todo } {
#procedure to add or delete a medium from the list for which BCSE
#is to be used
  global num_bcse_meds bcse_meds medium nmed

  set top .main_w.bcseopts

  if {"$todo"=="add"} {

    set i [incr num_bcse_meds]
    if {$bcse_meds($i)==""} {
       menubutton $top.inp.grid.med$i -text "select medium" \
             -menu $top.inp.grid.med$i.m -bd 1 -relief raised -indicatoron 1 \
             -width 20
    } else {
       menubutton $top.inp.grid.med$i -text $bcse_meds($i) \
             -menu $top.inp.grid.med$i.m -bd 1 -relief raised -indicatoron 1 \
             -width 20
    }
    menu $top.inp.grid.med$i.m -tearoff no
    for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
            $top.inp.grid.med$i.m add command -label $medium($iopt) \
                -command "$top.inp.grid.med$i configure -text $medium($iopt);\
                          set bcse_meds($i) $medium($iopt)"
    }
    grid configure $top.inp.grid.med$i -row $i -column 0

    if {$num_bcse_meds==1} {
         #re-enable the delete last group button
         $top.b.delb configure -state normal
    }
  } elseif {"$todo"=="del"} {
    destroy $top.inp.grid.med$num_bcse_meds

    incr num_bcse_meds -1

    if {$num_bcse_meds==0} {
         $top.b.delb configure -state disabled
    }
  }
}

proc coordinate_rr { from } {
#coordinates Russ Rou inputs for UBS and BCSE
global bcse_rr values options
  if {"$from"=="bcse"} {
    set values(16) $bcse_rr
    if {[winfo exists .main_w.nbrem]==1} {
      .main_w.nbrem.f5.inp configure -text $values(16)
    }
  } elseif {"$from"=="ubs"} {
    set bcse_rr $values(16)
    if {[winfo exists .main_w.bcseopts]==1} {
      .main_w.bcseopts.inp.constpowrr.rr.inp configure -text $bcse_rr
    }
  }
}

proc config_zrejpln { top } {
#configures Z input for rejection plane with DBS
global dbrem
  if {$dbrem(use_rejpln)==0} {
                  $top.f10.lab2 configure -fg grey
                  $top.f10.zrejpln configure -state disabled
  } elseif {$dbrem(use_rejpln)==1} {
                  $top.f10.lab2 configure -fg black
                  $top.f10.zrejpln configure -state normal
  }
}

proc set_special_N {} {

    # IWATCH option 'Special'
    global special_N

    catch { destroy .main_w.special }
    toplevel .main_w.special
    wm title .main_w.special "Special N"
    set top .main_w.special
    frame $top.f -bd 4
    label $top.f.lab1 -text "Output for every electron/photon step on history"
    entry $top.f.en -textvariable special_N -width 4
    label $top.f.lab2 -text ", normal output on all other histories. "
    frame $top.f2 -bd 4
    button $top.f2.b1 -text "OK" -command "destroy .main_w.special"\
	    -relief groove -bd 8
    button $top.f2.b2 -text "Help" -command "help_special_N .main_w.special"\
	    -relief groove -bd 8
    pack $top.f.lab1 $top.f.en $top.f.lab2 -side left -padx 0
    pack $top.f2.b1 $top.f2.b2 -side left -padx 10
    pack $top.f $top.f2 -side top
}

proc set_scoring_options { nplanes } {

    # SET CM NUMBERS FOR SCORING, # SCORING ZONES, ZONE TYPES, ZONE MARKS

    global values helvfont maxvals

    if $nplanes==0 { return }
    if $nplanes>$maxvals(MAX_SC_PLANES) {
	tk_dialog .toobig "Too many planes" "The maximum number of scoring\
		planes is currently set to $maxvals(MAX_SC_PLANES) in\
		beamnrc_user_macros.mortran" info 0 OK
	return
    }
    # this window is set up to get the scoring options for an arbitrary
    # number of scoring planes.  The max is now at 3 planes, 5 zones but the
    # user could increase them if they wanted.

    catch { destroy .main_w.set_scoring }
    toplevel .main_w.set_scoring
    wm title .main_w.set_scoring "Set scoring options"
    frame .main_w.set_scoring.frm -bd 4
    set top .main_w.set_scoring.frm
    set ncol 5
    set nrow 4
    for {set i 0} {$i<$ncol} {incr i} {
	set izone [expr $i*$nrow]
	if {$izone>$nplanes} {break}
	frame $top.col$i
	set w $top.col$i
	for {set j 1} {$j<=$nrow} {incr j} {
	    set izone [expr $i*$nrow+$j]
	    if {$izone>$nplanes} {break}
	    frame $w.frm$j
	    label $w.frm$j.mainlbl -text "Scoring plane $izone" -font $helvfont
	    pack $w.frm$j.mainlbl -pady 5 -anchor w

	    frame $w.frm$j.f1 -bd 4
	    label $w.frm$j.f1.lbl -text "CM number for plane $izone:"
	    entry $w.frm$j.f1.t -width 5 -textvariable values(22,1,$izone)
	    pack $w.frm$j.f1.lbl -anchor w -side left -padx 5
	    pack $w.frm$j.f1.t -anchor e -side right -fill x -expand 1
	    pack $w.frm$j.f1 -fill x

            frame $w.frm$j.f3 -bd 4
            label $w.frm$j.f3.lbl -text "Zone type:"
            set w2 $w.frm$j.f3
            menubutton $w2.inp -text $values(22,3,$izone) -menu $w2.inp.m -bd 1\
                    -relief raised -indicatoron 1 -width 8
            # MAKE THE MENU, ADD OPTIONS AND CALLBACKS
            menu $w2.inp.m -tearoff no
            $w2.inp.m add command -label "square" \
                    -command "set_22_3 $izone $w2 square;
                              $w.frm$j.f2.lbl configure -fg black;
                              $w.frm$j.f2.t configure -stat normal"
            $w2.inp.m add command -label "annular" \
                    -command "set_22_3 $izone $w2 annular;
                              $w.frm$j.f2.lbl configure -fg black;
                              $w.frm$j.f2.t configure -stat normal"
            $w2.inp.m add command -label "grid" \
                    -command "set_22_3 $izone $w2 grid;
                              $w.frm$j.f2.lbl configure -fg grey;
                              $w.frm$j.f2.t configure -stat disabled"
            pack $w.frm$j.f3.lbl -anchor w -side left -padx 5
            pack $w.frm$j.f3.inp -anchor e -side right\
                    -expand true -fill x -padx 5
            pack $w.frm$j.f3 -fill x

	    frame $w.frm$j.f2 -bd 4
	    label $w.frm$j.f2.lbl -text "Number of scoring zones\
		    (max. $maxvals(MAX_SC_ZONES):"
	    entry $w.frm$j.f2.t -width 5 -textvariable values(22,2,$izone)
	    pack $w.frm$j.f2.lbl -anchor w -side left -padx 5
	    pack $w.frm$j.f2.t -anchor e -side right -fill x -expand 1
	    pack $w.frm$j.f2 -fill x

            if {$values(22,3,$izone)=="grid"} {
                $w.frm$j.f2.lbl configure -fg grey
                $w.frm$j.f2.t configure -state disabled
            }

	    frame $w.frm$j.f4
	    button $w.frm$j.f4.b -text "Define zones" -command \
		    "get_zone_marks $w $izone"
	    pack $w.frm$j.f4.b -fill x -expand true
	    pack $w.frm$j.f4

	    frame $w.frm$j.sep -bd 4 -width 100 -height 2 -relief groove
	    pack $w.frm$j.sep -side top -fill x -pady 10
	    pack $w.frm$j
	}
	pack $w -side left
    }
    button .main_w.set_scoring.doneb -text "Done" \
	    -command "destroy .main_w.set_scoring" -relief groove -bd 8
    pack .main_w.set_scoring.frm .main_w.set_scoring.doneb -pady 5
}

proc get_zone_marks { w izone } {

    # window to enter zone marks for the scoring zones.

    global values marktype helvfont maxvals

  if {$values(22,3,$izone)=="annular" || $values(22,3,$izone)=="square"} {

    if { [catch {expr $values(22,2,$izone)}]==1 || $values(22,2,$izone)==0 } {
	# not a number
	tk_dialog $w.none "No zone marks" "BEAMnrc will automatically set the zone\
		marks when the number of scoring zones is 0." info 0 OK
	return
    } elseif $values(22,2,$izone)>$maxvals(MAX_SC_ZONES) {
	tk_dialog $w.toomuch "Too many zone marks" "The maximum\
		number of zone marks is currently set to $maxvals(MAX_SC_ZONES)\
		in beamnrc_user_macros.mortran." info 0 OK
	return
    } else {
	set nzone $values(22,2,$izone)
	catch { destroy $w.zztop }
	toplevel $w.zztop
	wm title $w.zztop "Zone Marks"
	frame $w.zztop.frm -bd 4
	label $w.zztop.lbl -font $helvfont -text \
		"Zone $marktype($values(22,3,$izone)) for plane $izone:"
	for {set i 1} {$i<=$nzone} {incr i} {
	    entry $w.zztop.frm.e$i -textvariable values(22,4,$izone,$i)\
		    -width 8
	    pack $w.zztop.frm.e$i -side left
	}
	button $w.zztop.b -text "Done" -command "destroy $w.zztop"\
		-relief groove -bd 8
	pack $w.zztop.lbl $w.zztop.frm $w.zztop.b -anchor w -side top -pady 5
    }
  } elseif {$values(22,3,$izone)=="grid"} {
    catch { destroy $w.zztop }
    toplevel $w.zztop
    wm title $w.zztop "Zone Marks"
    set top $w.zztop

    frame $top.grid -bd 4

    label $top.grid.dirlab -text "direction"
    label $top.grid.minlab -text "min."
    label $top.grid.maxlab -text "max."
    label $top.grid.numlab -text "no. of zones"
    grid configure $top.grid.dirlab -row 0 -column 0
    grid configure $top.grid.minlab -row 0 -column 1
    grid configure $top.grid.maxlab -row 0 -column 2
    grid configure $top.grid.numlab -row 0 -column 3
    label $top.grid.xlab -text "X"
    entry $top.grid.xmin -relief sunken -textvariable values(22,4,$izone,1)\
       -width 10
    entry $top.grid.xmax -relief sunken -textvariable values(22,4,$izone,2)\
       -width 10
    entry $top.grid.xnum -relief sunken -textvariable values(22,4,$izone,5)\
       -width 10
    grid configure $top.grid.xlab -row 1 -column 0
    grid configure $top.grid.xmin -row 1 -column 1
    grid configure $top.grid.xmax -row 1 -column 2
    grid configure $top.grid.xnum -row 1 -column 3
    label $top.grid.ylab -text "Y"
    entry $top.grid.ymin -relief sunken -textvariable values(22,4,$izone,3)\
       -width 10
    entry $top.grid.ymax -relief sunken -textvariable values(22,4,$izone,4)\
        -width 10
    entry $top.grid.ynum -relief sunken -textvariable values(22,4,$izone,6)\
        -width 10
    grid configure $top.grid.ylab -row 2 -column 0
    grid configure $top.grid.ymin -row 2 -column 1
    grid configure $top.grid.ymax -row 2 -column 2
    grid configure $top.grid.ynum -row 2 -column 3
    pack $top.grid -side top

   button $top.okb -text "OK" -command "grid_check_inputs $top $izone"\
             -relief groove -bd 8
   pack $top.okb -side top -padx 10
  }
}


proc set_22_3 { i w label } {
    # choose square or annular zones and put that on the option menu display.
    global values
    set values(22,3,$i) $label
    $w.inp configure -text $values(22,3,$i)
}

proc grid_check_inputs { w izone } {
    #checks that total no. of scoring zones input with grid does not exceed
    #max. no. of scoring zones
    global values maxvals
    if {[expr $values(22,4,$izone,5)*$values(22,4,$izone,6)]\
         > $maxvals(MAX_SC_ZONES)} {
      tk_dialog .warning "Too many scoring zones" "The grid has\
                  [expr $values(22,4,$izone,5)*$values(22,4,$izone,6)]\
                scoring zones, which is greater than the max. allowed\
                of $maxvals(MAX_SC_ZONES).  This will cause an error at\
         run time.  Either reduce the number of zones or Go into\
         \$OMEGA_HOME/beamnrc/beamnrc_user_macros.mortran\
         and increase the parameter \$MAX_SC_ZONES." \
                    warning 0 OK
    }
    destroy $w
}

proc set_itdose_on {} {

    # SET CM NUMBER, CONTAMINANT TYPE, LNINC, LNEXC
    global values options l_n_inc l_n_exc GUI_DIR

    catch { destroy .main_w.itdose }
    toplevel .main_w.itdose
    wm title .main_w.itdose "Dose Components"
    set top .main_w.itdose
    frame $top.inp
    frame $top.inp.b1
    label $top.inp.b1.label -anchor w \
	    -text "Define as contaminant when entering CM number"
    entry $top.inp.b1.text -relief sunken -textvariable values(23,1)
    pack $top.inp.b1.label -side left -padx 5
    pack $top.inp.b1.text -expand true -fill x  -side left -padx 5
    frame $top.inp.b2
    label $top.inp.b2.label -anchor w -text "Contaminant type"
    set w $top.inp.b2
    menubutton $w.inp -text $values(23,2) \
	    -menu $w.inp.m -bd 1 -relief raised -indicatoron 1 -width 20
    menu $w.inp.m -tearoff no
    $w.inp.m add command -label $options(23,2,0) \
	    -command "configure_menu $w 23 2 0"
    $w.inp.m add command -label $options(23,2,1) \
	    -command "configure_menu $w 23 2 1"
    pack $top.inp.b2.label -side left -padx 5
    pack $top.inp.b2.inp -side left -padx 5 -expand true -fill x
    frame $top.inp.b4
    label $top.inp.b4.label -anchor w \
	    -text "Number of dose components with exclusive bit filters"
    entry $top.inp.b4.text   \
		-relief sunken -textvariable values(23,3)
    button $top.inp.b4.b -text ">>"  -command { set_ln_exc }
    pack $top.inp.b4.label -side left -padx 5
    pack $top.inp.b4.text -side left -expand true -fill x  -padx 5
    pack $top.inp.b4.b -side left -padx 5
    frame $top.inp.b3
    label $top.inp.b3.label -anchor w \
	    -text "Number of dose components inclusive bit filters"
    entry $top.inp.b3.text   \
		-relief sunken -textvariable values(23,4)
    button $top.inp.b3.b -text ">>"  -command { set_ln_inc }
    pack $top.inp.b3.label -side left -padx 5
    pack $top.inp.b3.text -expand true -fill x  -side left -padx 5
    pack $top.inp.b3.b -side left -padx 5

    pack $top.inp.b1 $top.inp.b2 $top.inp.b4 $top.inp.b3 \
	    -fill x

    button $top.inp.b5 -text "Ok" -command "destroy $top" -relief groove -bd 8
    pack $top.inp.b5 -pady 5
    pack $top.inp

}

proc configure_menu { w i j k } {
    # configure option menu $w to display $options($i,$j,$k)
    global options values
    $w.inp configure -text "$options($i,$j,$k)"
    set values($i,$j) $options($i,$j,$k)
}

proc set_ln_exc {} {

    # window to create the exclusive filters.

    global l_n_exc values exc

    if { [catch {expr $values(23,3)}]==1 || $values(23,3)==0 } {
	# NOT A NUMBER, OR 0: RETURN WITH AN ERROR
	tk_dialog .edit "Sorry, try again..." "There are 0 filters.  Change the\
		number of filters if you want to define some." warning 0 OK
	return
    }

    catch { destroy .main_w.itdose.exc }
    toplevel .main_w.itdose.exc
    wm title .main_w.itdose.exc "Exclusive filters"
    frame .main_w.itdose.exc.frame -bd 4
    set top .main_w.itdose.exc.frame
    label $top.lab -text "Particles with any of the selected\
	    LATCH bits turned on will be excluded from dose\
	    calculations for the\
	    corresponding dose component."
    pack $top.lab -side top -pady 5
    for {set i 1} {$i<=$values(23,3)} {incr i} {
	frame $top.fr$i -bd 4 -relief groove
	for {set j 1} {$j<=31} {incr j} {
	    frame $top.fr$i.fr$j
	    label $top.fr$i.fr$j.l -text $j
	    checkbutton $top.fr$i.fr$j.c -variable exc($i,$j)
	    pack $top.fr$i.fr$j.l $top.fr$i.fr$j.c -side top
	    pack $top.fr$i.fr$j -side left
	}
	pack $top.fr$i -side top -pady 5
    }
    button $top.b5 -text "Close" -command "destroy .main_w.itdose.exc"\
	    -relief groove -bd 8
    pack $top.b5 -side top
    pack $top
}

proc set_ln_inc {} {

    # window to create the inclusive filters.

    global l_n_inc values inc

    if { [catch {expr $values(23,4)}]==1 || $values(23,4)==0 } {
	# NOT A NUMBER, RETURN WITH AN ERROR
	tk_dialog .edit "Sorry, try again..." "There are 0 filters.  Change the\
		number of filters if you want to define some." warning 0 OK
	return
    }

    catch { destroy .main_w.itdose.inc }
    toplevel .main_w.itdose.inc
    wm title .main_w.itdose.inc "Inclusive filters"
    frame .main_w.itdose.inc.f1 -bd 4
    set top .main_w.itdose.inc.f1
    label $top.lab -text "Particles with any of the selected\
	    included LATCH bits turned on and none of the selected"
    label $top.lab3 -text "excluded LATCH bits turned on will define the dose\
	    calculation for the corresponding dose component."
    label $top.lab2 -text "Note: Do not specify the same\
	    bit as both an included and excluded."
    pack $top.lab $top.lab3 $top.lab2 -side top -pady 0
    for {set i 1} {$i<=$values(23,4)} {incr i} {
	frame $top.fr$i -bd 4 -relief groove
	frame $top.fr$i.left
	label $top.fr$i.left.l0 -text "  "
	label $top.fr$i.left.l1 -text "Include if these bits:"
	label $top.fr$i.left.l2 -text "but not if these bits:"
	pack $top.fr$i.left.l0 $top.fr$i.left.l1 $top.fr$i.left.l2 -side top
	pack $top.fr$i.left -side left
	for {set j 1} {$j<=31} {incr j} {
	    frame $top.fr$i.fr$j
	    label $top.fr$i.fr$j.l -text $j
	    checkbutton $top.fr$i.fr$j.c1 -variable inc($i,1,$j)
	    checkbutton $top.fr$i.fr$j.c2 -variable inc($i,2,$j)
	    pack $top.fr$i.fr$j.l $top.fr$i.fr$j.c1 \
		    $top.fr$i.fr$j.c2 -side top
	    pack $top.fr$i.fr$j -side left
	}
	pack $top.fr$i -side top -pady 5
    }
    button $top.b5 -text "Close" -command "destroy .main_w.itdose.inc" \
	    -relief groove -bd 8
    pack $top.b5 -side top
    pack $top
}

proc set_force_options {} {

    # photon forcing options:
    global force_bdd num_cm

    catch { destroy .main_w.force }
    toplevel .main_w.force
    wm title .main_w.force "Photon Forcing"
    set top .main_w.force

    set force_bdd(1) 1

    frame $top.inp -bd 10

    frame $top.inp.b2
    if { $force_bdd(2) == ""} {set force_bdd(2) 1}
    label $top.inp.b2.label -width 50 -anchor w \
	    -text "Force photon interactions from interaction 1 to:"
    entry $top.inp.b2.text  -width 10 \
		-relief sunken -textvariable force_bdd(2)
    pack $top.inp.b2.label $top.inp.b2.text -side left -padx 5

    frame $top.inp.b3
    if { $force_bdd(3) == ""} {set force_bdd(3) 1}
    label $top.inp.b3.label -width 50 -anchor w \
	    -text "Start photon interaction forcing at CM number:"
    entry $top.inp.b3.text  -width 10 \
		-relief sunken -textvariable force_bdd(3)
    pack $top.inp.b3.label $top.inp.b3.text -side left -padx 5

    frame $top.inp.b4
    if { $force_bdd(4) == ""} {set force_bdd(4) $num_cm}
    label $top.inp.b4.label -width 50 -anchor w \
	    -text "Stop photon interaction forcing after CM number:"
    entry $top.inp.b4.text  -width 10 \
		-relief sunken -textvariable force_bdd(4)
    pack $top.inp.b4.label $top.inp.b4.text -side left -padx 5

    pack $top.inp.b2 $top.inp.b3 $top.inp.b4

    #put on an ok button to save the values
    frame $top.inp.buttons -bd 5
    button $top.inp.buttons.b5 -text "OK" -command "destroy $top"\
	    -relief groove -bd 8
    pack $top.inp.buttons.b5 -side left
    pack $top.inp.buttons -side top -pady 5
    pack $top.inp
}

proc set_src_options { iopt } {

    # set the options for each source - 2*array (12,4) will
    # hold names, values for each option.

    global srcoptnames numsrcopts srcopts s1opt s3opt monoen spcnam15 ioutsp
    global helvfont values spcnam21 spcnam31 spec_file inp_file_dir env options
    global src19input hen_house egs_home tcl_platform
    global isrc_dbs rsrc_dbs ssdsrc_dbs zsrc_dbs
    global source_text the_beam_code the_input_file the_pegs_file
    global alpha24 beta24 dist24 i_dsb

    catch { destroy .main_w.srcopt }
    toplevel .main_w.srcopt
    set top .main_w.srcopt
    wm title $top "Set source options"

    label $top.srclab -text "Source $values(18)" -font $helvfont
    pack $top.srclab -pady 10

    set w $top.optfrm
    frame $w -bd 5
    set i 1
    if $iopt==11 {
	frame $w.inp$i -width 400
        frame $w.inp$i.mode
        frame $w.inp$i.ellipse
        frame $w.inp$i.ellipse.x
        frame $w.inp$i.ellipse.y
	radiobutton $w.inp$i.mode.label1 -text "Sigma  or   " -variable src19input\
		-value sigma
	radiobutton $w.inp$i.mode.label2 -text FWHM -variable src19input -value fwhm
        pack $w.inp$i.mode.label1 $w.inp$i.mode.label2 -anchor w -side top
        label $w.inp$i.ellipse.x.lab -text "in X-direction"
	entry $w.inp$i.ellipse.x.text  -width 10 \
		-relief sunken -textvariable srcopts($i)
        label $w.inp$i.ellipse.x.lab1 -text "cm"
        pack $w.inp$i.ellipse.x.lab -anchor w -side left
        pack $w.inp$i.ellipse.x.lab1 -anchor e -side right
        pack $w.inp$i.ellipse.x.text -anchor e -side right -fill x -expand true
        label $w.inp$i.ellipse.y.lab -text "in Y-direction"
        entry $w.inp$i.ellipse.y.text  -width 10 \
                -relief sunken -textvariable srcopts(6)
        label $w.inp$i.ellipse.y.lab1 -text "cm"
        pack $w.inp$i.ellipse.y.lab -anchor w -side left
        pack $w.inp$i.ellipse.y.lab1 -anchor e -side right
        pack $w.inp$i.ellipse.y.text -anchor e -side right -fill x -expand true
        pack $w.inp$i.ellipse.x $w.inp$i.ellipse.y -anchor w -side top
        pack $w.inp$i.mode $w.inp$i.ellipse -anchor w -side left
	pack $w.inp$i -side top -fill x
    } else {
	frame $w.inp$i -width 400
	label $w.inp$i.label -text $srcoptnames($iopt,$i)
	entry $w.inp$i.text  -width 10 \
		-relief sunken -textvariable srcopts($i)
	pack $w.inp$i.label -anchor w -side left
	pack $w.inp$i.text -anchor e -side right -fill x -expand true
	pack $w.inp$i -side top -fill x
    }
    if {$iopt==1} {
    # add radiobutton to select circular or rectangular collimation
        frame $w.f -bd 4
        label $w.f.label -text "Rectangular or circular beam?" -font $helvfont
        pack $w.f.label
        radiobutton $w.f.radio1 -text "rectangular" \
                -variable s1opt -value "rectangular" \
           -command { for {set k 2} {$k<=3} {incr k} {
            .main_w.srcopt.optfrm.circ.inp$k.label configure -fg grey
            .main_w.srcopt.optfrm.circ.inp$k.text configure -state disabled
          }; for {set k 4} {$k<=7} {incr k} {
            .main_w.srcopt.optfrm.rect.inp$k.label configure -fg black
            .main_w.srcopt.optfrm.rect.inp$k.text configure -state normal
           }}
        radiobutton $w.f.radio2 -text "circular" \
                -variable s1opt -value "circular" \
          -command { for {set k 2} {$k<=3} {incr k} {
            .main_w.srcopt.optfrm.circ.inp$k.label configure -fg black
            .main_w.srcopt.optfrm.circ.inp$k.text configure -state normal
          }; for {set k 4} {$k<=7} {incr k} {
            .main_w.srcopt.optfrm.rect.inp$k.label configure -fg grey
            .main_w.srcopt.optfrm.rect.inp$k.text configure -state disabled
          }}
        pack $w.f.radio1 $w.f.radio2  -side top -anchor w
        pack $w.f
        #now, put in the other source options
        frame $w.circ
        for {set i 2} {$i <= 3} {incr i} {
          #to define circular beam
          frame $w.circ.inp$i -width 400
          label $w.circ.inp$i.label -text $srcoptnames($iopt,$i)
          entry $w.circ.inp$i.text  -width 10 \
                -relief sunken -textvariable srcopts($i)
          pack $w.circ.inp$i.label -anchor w -side left
          pack $w.circ.inp$i.text -anchor e -side right -fill x -expand true
          pack $w.circ.inp$i -side top -fill x
          if {"$s1opt"=="rectangular"} {
            $w.circ.inp$i.label configure -fg grey
            $w.circ.inp$i.text configure -state disabled
          }
        }
        frame $w.rect
        for {set i 4} {$i <= 7} {incr i} {
          #to define circular beam
          frame $w.rect.inp$i -width 400
          label $w.rect.inp$i.label -text $srcoptnames($iopt,$i)
          entry $w.rect.inp$i.text  -width 10 \
                -relief sunken -textvariable srcopts($i)
          pack $w.rect.inp$i.label -anchor w -side left
          pack $w.rect.inp$i.text -anchor e -side right -fill x -expand true
          pack $w.rect.inp$i -side top -fill x
          if {"$s1opt"=="circular"} {
            $w.rect.inp$i.label configure -fg grey
            $w.rect.inp$i.text configure -state disabled
          }
        }
        pack $w.circ -side left -padx 4 -anchor w
        pack $w.rect -side left -padx 4 -anchor w
    } elseif {$iopt==11} {
        frame $w.space
        pack $w.space -side top -fill x -pady 4
        for {set i 2} {$i <= [expr $numsrcopts($iopt)-2]} {incr i} {
          frame $w.inp$i -width 400
          label $w.inp$i.label -text $srcoptnames($iopt,$i)
          entry $w.inp$i.text  -width 10 \
                -relief sunken -textvariable srcopts($i)
          bind $w.inp$i.text <ButtonPress> "config_src19_cosines $w"
          pack $w.inp$i.label -anchor w -side left
          pack $w.inp$i.text -anchor e -side right -fill x -expand true
          pack $w.inp$i -side top -fill x
          if {$srcopts(5)!="" && $srcopts(5)>0} {
             $w.inp$i.label configure -fg grey
             $w.inp$i.text configure -state disabled
          }
        }
        label $w.or -text "or"
        pack $w.or -side top -fill x
        set i [expr $numsrcopts($iopt)-1]
        #careful!  recall srcopt(6) already has an entry box by now
        frame $w.inp$i -width 400
        label $w.inp$i.label -text $srcoptnames($iopt,$i)
        entry $w.inp$i.text  -width 10 \
                -relief sunken -textvariable srcopts($i)
        bind $w.inp$i.text <Return> "config_src19_cosines $w"
        bind $w.inp$i.text <Tab> "config_src19_cosines $w"
        pack $w.inp$i.label -anchor w -side left
        pack $w.inp$i.text -anchor e -side right -fill x -expand true
        pack $w.inp$i -side top -fill x
    } else {
        for {set i 2} {$i <= $numsrcopts($iopt)} {incr i} {
          frame $w.inp$i -width 400
	  label $w.inp$i.label -text $srcoptnames($iopt,$i)
	  entry $w.inp$i.text  -width 10 \
		-relief sunken -textvariable srcopts($i)
	  pack $w.inp$i.label -anchor w -side left
	  pack $w.inp$i.text -anchor e -side right -fill x -expand true
	  pack $w.inp$i -side top -fill x
        }
    }
    if {$iopt==12 | $iopt==14} {
    # source 21, 24 add extra inputs for DBS stuff

        if {$iopt==14} {
        # source 24, add inputs for rotations

           frame $w.spacer1
           pack $w.spacer1 -side top -fill x -pady 4

           frame $w.alpha24 -width 400
           label $w.alpha24.label -text "angle of rotation wrt X-axis (degrees)"
           entry $w.alpha24.text -width 10 -relief sunken -textvariable alpha24
           pack $w.alpha24.label -anchor w -side left
           pack $w.alpha24.text -anchor e -side right -fill x -expand true
           pack $w.alpha24 -side top -fill x

           frame $w.beta24 -width 400
           label $w.beta24.label -text "angle of rotation wrt Y-axis (degrees)"
           entry $w.beta24.text -width 10 -relief sunken -textvariable beta24
           pack $w.beta24.label -anchor w -side left
           pack $w.beta24.text -anchor e -side right -fill x -expand true
           pack $w.beta24 -side top -fill x

           frame $w.dist24 -width 400
           label $w.dist24.label -text "distance of point of rotation above starting CM (cm)"
           entry $w.dist24.text -width 10 -relief sunken -textvariable dist24
           pack $w.dist24.label -anchor w -side left
           pack $w.dist24.text -anchor e -side right -fill x -expand true
           pack $w.dist24 -side top -fill x
        }

        frame $w.spacer
        pack $w.spacer -side top -fill x -pady 4

        frame $w.isrcdbs -width 400
        checkbutton $w.isrcdbs.but -text "DBS used to generate source" \
                -variable isrc_dbs -command "config_dbs $w"
        pack $w.isrcdbs.but -side top -anchor w
        pack $w.isrcdbs -side top -fill x

        frame $w.rsrcdbs -width 400
        label $w.rsrcdbs.label -text "DBS splitting field radius (cm)"
        entry $w.rsrcdbs.text -width 10 -relief sunken -textvariable rsrc_dbs
        pack $w.rsrcdbs.label -anchor w -side left
        pack $w.rsrcdbs.text -anchor e -side right -fill x -expand true
        pack $w.rsrcdbs -side top -fill x

        frame $w.ssdsrcdbs -width 400
        label $w.ssdsrcdbs.label -text "SSD of splitting field (cm)"
        entry $w.ssdsrcdbs.text -width 10 -relief sunken -textvariable ssdsrc_dbs
        pack $w.ssdsrcdbs.label -anchor w -side left
        pack $w.ssdsrcdbs.text -anchor e -side right -fill x -expand true
        pack $w.ssdsrcdbs -side top -fill x

        frame $w.zsrcdbs -width 400
        label $w.zsrcdbs.label -text "Z where source scored (cm)"
        entry $w.zsrcdbs.text -width 10 -relief sunken -textvariable zsrc_dbs
        pack $w.zsrcdbs.label -anchor w -side left
        pack $w.zsrcdbs.text -anchor e -side right -fill x -expand true
        pack $w.zsrcdbs -side top -fill x

        if {$isrc_dbs != 1} {
           set isrc_dbs 0
           $w.rsrcdbs.label configure -fg grey
           $w.rsrcdbs.text configure -state disabled
           $w.ssdsrcdbs.label configure -fg grey
           $w.ssdsrcdbs.text configure -state disabled
           $w.zsrcdbs.label configure -fg grey
           $w.zsrcdbs.text configure -state disabled
        }
    }
    if {$iopt==13} {

        #rotational angles and distance

        frame $w.spacer1
        pack $w.spacer1 -side top -fill x -pady 4

        frame $w.alpha24 -width 400
        label $w.alpha24.label -text "angle of rotation wrt X-axis (degrees)"
        entry $w.alpha24.text -width 10 -relief sunken -textvariable alpha24
        pack $w.alpha24.label -anchor w -side left
        pack $w.alpha24.text -anchor e -side right -fill x -expand true
        pack $w.alpha24 -side top -fill x

        frame $w.beta24 -width 400
        label $w.beta24.label -text "angle of rotation wrt Y-axis (degrees)"
        entry $w.beta24.text -width 10 -relief sunken -textvariable beta24
        pack $w.beta24.label -anchor w -side left
        pack $w.beta24.text -anchor e -side right -fill x -expand true
        pack $w.beta24 -side top -fill x

        frame $w.dist24 -width 400
        label $w.dist24.label -text "distance of point of rotation above starting CM (cm)"
        entry $w.dist24.text -width 10 -relief sunken -textvariable dist24
        pack $w.dist24.label -anchor w -side left
        pack $w.dist24.text -anchor e -side right -fill x -expand true
        pack $w.dist24 -side top -fill x
    # source 23, add a checkbox to reject phat photons from DBS

        frame $w.spacer
        pack $w.spacer -side top -fill x -pady 4

        frame $w.isrcdbs -width 400
        checkbutton $w.isrcdbs.but -text "Reject fat photons from DBS" \
                -variable isrc_dbs
        pack $w.isrcdbs.but -side top -anchor w
        pack $w.isrcdbs -side top -fill x
    }
    pack $w
    # Add a separator
    frame $top.sep0 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep0 -fill x -pady 10

    set spectra_dir [file join $hen_house spectra]

    if $iopt==2 {
	# source 3
	frame $top.f -bd 4
	message $top.f.label -text "Select source 3 orientation, \
		vertical ring centered on Z-axis or horizontal cylinder\
		centered parallel to X-axis:" -font $helvfont -width 5i
	pack $top.f.label -fill x
	radiobutton $top.f.radio1 -text "horizontal" \
		-variable s3opt -value "horizontal"
	radiobutton $top.f.radio2 -text "vertical" \
		-variable s3opt -value "vertical"
	pack $top.f.radio1 $top.f.radio2 -side left
	pack $top.f
	# Add a separator
	frame $top.sep -bd 4 -width 100 -height 2 -relief groove
	pack $top.sep -fill x -pady 10
        # directional source biasing option
        frame $top.dsb
        message $top.dsb.l -text "Directional Source Biasing (DSB)" -font $helvfont -width 5i
        pack $top.dsb.l
        button $top.dsb.help -text "?" -command {help dsb}
        frame $top.dsb.onoff
        radiobutton $top.dsb.onoff.r1 -text "off" -variable i_dsb -value 0 \
           -command "$top.dsb.params configure -state disabled"
        radiobutton $top.dsb.onoff.r2 -text "on" -variable i_dsb -value 1 \
           -command "$top.dsb.params configure -state normal; set values(14) $options(14,2); \
                    .main_w.main.right.inp14.inp configure -text $values(14)"
        pack $top.dsb.onoff.r1 $top.dsb.onoff.r2 -side top
        button $top.dsb.params -text "Set DSB parameters" -command {set_nbrem 3}
        pack $top.dsb.help $top.dsb.onoff $top.dsb.params -side left -padx 6
        if {$i_dsb!=1} {$top.dsb.params configure -state disabled}
        pack $top.dsb
        # Add a separator
        frame $top.dsbsep -bd 4 -width 100 -height 2 -relief groove
        pack $top.dsbsep -fill x -pady 10
    } elseif $iopt==7 {
	# source 9
	button $top.s9b -text " Set discrete probabilities >> " \
		-command "get_s9vals $top" -relief raised
	pack $top.s9b
	# Add a separator
	frame $top.sep -bd 4 -width 100 -height 2 -relief groove
	pack $top.sep -fill x -pady 10
    } elseif $iopt==10 {
	# source 15
	frame $top.psf -bd 5
	label $top.psf.label -text "Complete filename for radial\
		distribution of beam:" -anchor w
	pack $top.psf.label -anchor w
	frame $top.psf.f
	entry $top.psf.f.text -width 60 -relief sunken \
		-textvariable spcnam15
	button $top.psf.f.b1 -text "Browse..." \
		-command {browse set_spcnam15 "$inp_file_dir"}
	pack $top.psf.f.text $top.psf.f.b1 -side left -padx 5
	pack $top.psf.f
	pack $top.psf
	# Add a separator
	frame $top.sep -bd 4 -width 100 -height 2 -relief groove
	pack $top.sep -fill x -pady 10
    } elseif {$iopt==12 | $iopt==14} {
	# source 21
	frame $top.psf -bd 5
	label $top.psf.label -text "Phase space information file (complete):"
	pack $top.psf.label -anchor w
	frame $top.psf.f
	entry $top.psf.f.text -width 50 -relief sunken \
		-textvariable spcnam21
	frame $top.psf.f.buts
	button $top.psf.f.buts.b1 -text "Browse current" \
		-command {browse set_spcnam21 "$inp_file_dir"}
	button $top.psf.f.buts.b2 -text "Browse generic" \
		-command {browse set_spcnam21 "$egs_home"}
	pack $top.psf.f.buts.b1 $top.psf.f.buts.b2 -side top -fill x -anchor w
	pack $top.psf.f.text $top.psf.f.buts -side left -padx 5
	pack $top.psf.f
	pack $top.psf
    } elseif $iopt==13 {
        # source 23
        set w $top.simspecs
        frame $w -bd 5
        set my_machine [get_config_value "my_machine"]
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

        pack $w
    } elseif $iopt==15 {
	# source 31
	frame $top.psf -bd 5
	label $top.psf.label -text "Multiple source model file:"
	pack $top.psf.label -anchor w
	frame $top.psf.f
	entry $top.psf.f.text -width 50 -relief sunken \
		-textvariable spcnam31
	frame $top.psf.f.buts
	button $top.psf.f.buts.b1 -text "Browse current" \
		-command {browse set_spcnam31 "$inp_file_dir"}
	button $top.psf.f.buts.b2 -text "Browse generic" \
		-command "browse set_spcnam31 $spectra_dir"
	pack $top.psf.f.buts.b1 $top.psf.f.buts.b2 -side top -fill x -anchor w
	pack $top.psf.f.text $top.psf.f.buts -side left -padx 5
	pack $top.psf.f
	pack $top.psf
    }

    if $iopt<12 {
	frame $top.lt21 -bd 4
	label $top.lt21.label -text \
		"Specify source beam energy or spectrum filename" -font $helvfont
	pack $top.lt21.label -pady 5

	# left side is for monoenergetic:
	frame $top.lt21.left
	set w $top.lt21.left
	radiobutton $w.radio1 -text "monoenergetic" \
		-variable monoen -value "monoenergetic"\
		-command "$top.lt21.right.f.buts.b1 configure -state disabled;\
		$top.lt21.right.f.buts.b2 configure -state disabled;\
		$top.lt21.right.f.text configure -state disabled;\
		$top.lt21.right.inp configure -state disabled;\
		$top.lt21.left.text configure -state normal"
	pack $w.radio1 -anchor w -pady 5
	label $w.label -text "Kinetic energy of beam (MeV)"
	entry $w.text -width 10 -relief sunken \
		-textvariable Ein_val
	pack $w.label -anchor w -side left -padx 5
	pack $w.text -anchor e -side left -fill x -expand true

	pack $w -anchor n -side left -padx 10

	# right side is for spectrum
	frame $top.lt21.right
	set w $top.lt21.right
	radiobutton $w.radio2 -text "spectrum" \
		-variable monoen -value "spectrum"\
		-command "$top.lt21.right.f.text configure -state normal;\
		$top.lt21.right.f.buts.b1 configure -state normal;\
		$top.lt21.right.f.buts.b2 configure -state normal;\
		$top.lt21.right.inp configure -state normal;\
		$top.lt21.left.text configure -state disabled"
	pack $w.radio2 -anchor w -pady 5

	label $w.label -text "Spectrum filename (complete):"
	pack $w.label -anchor w
	frame $w.f
	entry $w.f.text -width 40 -relief sunken \
		-textvariable spec_file

	frame $w.f.buts
	button $w.f.buts.b1 -text "Browse current" \
		-command {browse set_spec_file "$inp_file_dir"}
	button $w.f.buts.b2 -text "Browse generic" \
		-command "browse set_spec_file $spectra_dir"
	pack $w.f.buts.b1 $w.f.buts.b2 -side top -fill x -anchor w
	pack $w.f.text $w.f.buts -side left -padx 5

	pack $w.f -pady 5
	label $w.label2 -text "Output spectrum listing file?"
	pack $w.label2 -anchor w
	menubutton $w.inp -text $ioutsp -width 20\
	    -menu $w.inp.m -bd 1 -relief raised -indicatoron 1
	# Now make the menu, and add the lines one at a time
	menu $w.inp.m -tearoff no
	set lab(1) "no spectrum data in output summary"
	set lab(2) "include spectrum data in output summary"
	$w.inp.m add command \
		-label $lab(1) \
		-command {set_ioutsp "no spectrum data in output summary"}
	$w.inp.m add command \
		-label $lab(2) \
		-command {set_ioutsp "include spectrum data in output summary"}
	pack $w.inp -pady 5 -anchor w -fill x -expand true

	pack $w -anchor n -side right -padx 10
	pack $top.lt21

	# enable/disable as needed (first time, radiobuttons do the rest)
	if [string compare $monoen "monoenergetic"]==0 {
	    $top.lt21.right.f.buts.b1 configure -state disabled
	    $top.lt21.right.f.buts.b2 configure -state disabled
	    $top.lt21.right.f.text configure -state disabled
	    $top.lt21.right.inp configure -state disabled
	    $top.lt21.left.text configure -state normal
	} else {
	    $top.lt21.right.f.buts.b1 configure -state normal
	    $top.lt21.right.f.buts.b2 configure -state normal
	    $top.lt21.right.f.text configure -state normal
	    $top.lt21.right.inp configure -state normal
	    $top.lt21.left.text configure -state disabled
	}
    }
    # Add a separator
    frame $top.sep2 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep2 -fill x -pady 10

    #put on an ok button to save the values
    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .main_w.srcopt"\
	    -relief groove -bd 8
    if {$iopt==12} {
    #scrolling help for source 21
       button $top.buts.helpb -relief groove -bd 8 -text "Help" \
      -command {help_dialog .main_w.srcopt.help Help $source_text(12) info 0 OK}
    } else {
       button $top.buts.helpb -relief groove -bd 8 -text "Help" \
	    -command "help_srcopts .main_w.srcopt $iopt"
    }
    pack $top.buts.okb $top.buts.helpb -side left -padx 10
    pack $top.buts
}

proc get_s9vals { w } {

    # options for source 9:

    global srcopts maxvals s9vals

    if { $srcopts(2)<1 || $srcopts(2)>$maxvals(MAXPTS_SRC9) } {
	tk_dialog $w.toobig "Bad number" "You have set too few or too many\
		discrete points.  The maximum is $maxvals(MAXPTS_SRC9)\
		(set in beamnrc_user_macros.mortran) and the minimum is 1."\
		info 0 OK
	return
    }
    catch { destroy $w.source9 }
    toplevel $w.source9 -bd 4
    wm title $w.source9 "Source 9 Discrete Probabilities"
    label $w.source9.mess -text "All of these points are defined at the SSD."
    pack $w.source9.mess

    frame $w.source9.grid
    set ww $w.source9.grid

    label $ww.xlabel -text "x"
    grid configure $ww.xlabel -row 0 -column 1
    label $ww.ylabel -text "y"
    grid configure $ww.ylabel -row 0 -column 2
    label $ww.plabel -text "Probability"
    grid configure $ww.plabel -row 0 -column 3

    for {set i 1} {$i<=$srcopts(2)} {incr i} {
	label $ww.num$i -text $i
	grid config $ww.num$i -row $i -column 0
	entry $ww.x$i -width 8 -relief sunken -textvariable s9vals(1,$i)
	grid config $ww.x$i -row $i -column 1
	entry $ww.y$i -width 8 -relief sunken -textvariable s9vals(2,$i)
	grid config $ww.y$i -row $i -column 2
	entry $ww.p$i -width 8 -relief sunken -textvariable s9vals(3,$i)
	grid config $ww.p$i -row $i -column 3
    }
    pack $ww -side top -pady 5 -padx 10

    #put on an ok button to save the values
    frame $w.source9.buts
    button $w.source9.buts.okb -text "OK" -relief groove -bd 8\
	    -command "destroy $w.source9"
    button $w.source9.buts.helpb -text "Help" -relief groove -bd 8\
	    -command "help_source9 $w.source9"
    pack $w.source9.buts.okb $w.source9.buts.helpb -side left -padx 10
    pack $w.source9.buts -side bottom
}

proc set_ioutsp {label} {

    # configure the ioutsp option menu display.

    global ioutsp
    set ioutsp $label
    .main_w.srcopt.lt21.right.inp configure -text $label
}

proc set_egsnrc_parameters {} {

    #open up a subwindow to set EGSnrc paramters

    global names values options numopts GUI_DIR medium nmed

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
    pack $top .main_w.egsnrc.button -side top -pady 5

}

proc egsnrc_set_in_reg { param onoff }  {

    #will open up a box to set/display on/off regions for Atomic relaxations
    #(iedgfl), photoelectron angular sampling (iphter), bound compton
    #scattering (ibcmp) and Rayleigh scattering (iraylr)

    global iedgfl_min iedgfl_max iphter_min iphter_max ibcmp_min
    global ibcmp_max iraylr_min iraylr_max level

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

    pack $top.label -pady 5

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
    pack $top.b -pady 10

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
    global num_rayl_custom rayl_cust_med rayl_cust_file nmed
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
        for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
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

    global rayl_cust_med rayl_cust_file num_rayl_custom nmed
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
    for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
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


proc config_dbs { w } {

#enable or disable inputs for rsrc_dbs, ssdsrc_dbs, zsrc_dbs based on isrc_dbs

    global isrc_dbs

    if {$isrc_dbs == 0} {
           $w.rsrcdbs.label configure -fg grey
           $w.rsrcdbs.text configure -state disabled
           $w.ssdsrcdbs.label configure -fg grey
           $w.ssdsrcdbs.text configure -state disabled
           $w.zsrcdbs.label configure -fg grey
           $w.zsrcdbs.text configure -state disabled
    } elseif {$isrc_dbs == 1} {
           $w.rsrcdbs.label configure -fg black
           $w.rsrcdbs.text configure -state normal
           $w.ssdsrcdbs.label configure -fg black
           $w.ssdsrcdbs.text configure -state normal
           $w.zsrcdbs.label configure -fg black
           $w.zsrcdbs.text configure -state normal
    }
}

proc config_src19_cosines { w } {

#enables or disables direction cosine inputs for source 19 depending
#on setting of the mean angular divergence

     global srcopts numsrcopts

     for {set i 2} {$i <= [expr $numsrcopts(11)-2]} {incr i} {
          if {$srcopts(5)!="" && $srcopts(5)>0} {
             $w.inp$i.label configure -fg grey
             $w.inp$i.text configure -state disabled
          } else {
             $w.inp$i.label configure -fg black
             $w.inp$i.text configure -state normal
          }
      }

}

proc set_beam_code { tree filename } {
    # for source 9, called from query_filename
    global the_beam_code egs_home tcl_platform
    if { "$tcl_platform(platform)"=="windows" } {
                  set libext dll
    } else {
                  set libext so
    }
    set the_beam_code $filename
    set len [string length $the_beam_code]
    set BEAMindex [string first BEAM $the_beam_code]
    if $BEAMindex<0 {set BEAMindex 0}
    set extindex [expr [string last .$libext $the_beam_code]-1]
    if $extindex<0 {set extindex [expr $len-1]}
    set the_beam_code [string range $the_beam_code $BEAMindex $extindex]
    set beamdir [file join $egs_home $the_beam_code]
    if {[file exists $beamdir]} {
       .main_w.srcopt.simspecs.inpfilebrowse configure \
         -text "Browse EGS_HOME/$the_beam_code" \
         -command "query_filename set_input_file $beamdir egsinp" \
         -state normal
    } else {
       tk_dialog .acceldirexist "Warning" "Directory\
                    EGS_HOME/$the_beam_code does not exist.  Input files\
                    must be in this directory.  Will browse from\
                    EGS_HOME." error 0 OK
       .main_w.srcopt.simspecs.inpfilebrowse configure \
         -text "Browse EGS_HOME" \
         -command "query_filename set_input_file $egs_home egsinp"
    }
}

proc set_input_file { tree filename } {
    #for source 9, called from query_filename
    global the_input_file
    set the_input_file $filename
    set len [string length $the_input_file]
    set extindex [expr [string last .egsinp $the_input_file]-1]
    if $extindex<0 {set extindex [expr $len-1]}
    set the_input_file [string range $the_input_file 0 $extindex]
}

proc set_pegs_file { tree filename } {
    #for source 9, called from query_filename
    global the_pegs_file
    set the_pegs_file $filename
    set len [string length $the_pegs_file]
    set extindex [expr [string last .pegs4dat $the_pegs_file]-1]
    if $extindex<0 {set extindex [expr $len-1]}
    set the_pegs_file [string range $the_pegs_file 0 $extindex]
}

