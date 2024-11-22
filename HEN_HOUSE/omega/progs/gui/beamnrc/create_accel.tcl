
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: create accelerator
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
#  Procedures for creating the window of selected CMs, for selecting CMs to
#  place on it, and placing Cms on it after saving a CM set or loading a
#  previous one.
#
###############################################################################


proc specify_new_module {} {
    # check whether a module is loaded, if so ask if OK, then call create_accel.
    global num_cm GUI_DIR mod_file maxvals values
    if {$num_cm>0} {
	set result [tk_dialog .okay "Discard?" "The accelerator currently loaded\
		will be discarded.  Are you sure?" \
		question 0 Yes Cancel ]
	if $result==0 {
	    # YES, discard
            source $GUI_DIR/beamnrc_params.tcl
	    set num_cm 0
	    set mod_file {}
	    .modloaded configure -text {}
	    destroy .cm_selected
	    create_accel
	}
    } else {
	create_accel
    }
}

proc popup_cm_window {} {
    # CREATE A NEW TOPLEVEL TO HOLD THE ICONS WHICH SHOW THE CMS SELECTED.
    global rootx rooty

    catch { destroy .cm_selected }
    toplevel .cm_selected  -width 200

    wm title .cm_selected "Selected components"
    set x [expr $rootx + [winfo width .] + 50]
    set y [expr $rooty - 100]
    wm geometry .cm_selected +$x+$y

    frame .cm_selected.f1 -bd 10 -width 200
}

proc update_popup {} {

    # put the selected CMs on the .cm_selected window, or if there are
    # none, destroy the window.

    global num_cm cm_names cm_type home cm_ident normfont italfont

    button .cm_selected.f1.mainb -text "Edit main input parameters" \
	-command "set_main_inputs" -font $normfont -state disabled
    pack .cm_selected.f1.mainb -fill x -expand true

    if [expr $num_cm > 0] {
	for {set i 1} {$i <= $num_cm} {incr i} {
	    set index $cm_type($i)
	    set long .cm_selected.f1.cm($i)
	    frame $long
	    label $long.name -text "$cm_names($index)" \
		    -font $normfont -width 15 -anchor  w
	    label $long.id -text "$cm_ident($i)" \
		    -font $italfont -width 15 -anchor  w
	    button $long.editb -text "Edit..." -width 6 \
		    -command "edit_cm $i $index" \
		     -font $normfont \
		    -state disabled
	    pack $long.name $long.id $long.editb -side left -padx 5

	    pack $long -in .cm_selected.f1
	}
	pack .cm_selected.f1
    } else {
	destroy .cm_selected
    }
}

proc edit_cm {id index} {
    global cm_names cmval cm_type values

    # id is the chronological cm number and index is cm_type($id)
    # here is where the z-coord. of the previous cm is calculated.
    # that number is then passed to the next CM being edited so that the
    # user knows how big it is.
    # pid is the location of the previous CM.

    set pid [expr $id - 1]

    if [catch {set zmax [get_zmax $pid]}]==1 {
	set zmax "The position of the previous CM is unknown."
    } else {
	if [string compare $zmax  ""]==0 {
	    set zmax "The position of the previous CM is unknown."
	} else {
	    set zmax "When this window was opened, the previous CM ended\
		    at $zmax cm."
	}
    }
    edit_$cm_names($index) $id $zmax
}

proc create_accel {} {

    # a window where the user can select the CMs to add to create an accelerator
    # by selecting CMs and giving them identifiers.

    global cm_names cm_ident new_cm_name rootx rooty
    catch { destroy .select }
    toplevel .select
    set top .select.main

    wm title .select "Create accelerator"
    set x [expr $rootx +30]
    set y [expr $rooty +30]
    wm geometry .select +$x+$y

    frame $top -bd 4

    #define frames for placement
    frame $top.l
    frame $top.m
    frame $top.r

    label $top.l.label -text "Select from: "
    pack $top.l.label -anchor w

    # add listbox, scrollbar, insert all availabel CM names
    listbox $top.l.list -height 10 -yscrollcommand "$top.l.scrl set" \
	    -bg white
    for {set i 1} {$i <= [array size cm_names]} {incr i} {
	$top.l.list insert end $cm_names($i)
    }
    scrollbar $top.l.scrl -command "$top.l.list yview"
    pack $top.l.scrl -side right -fill y
    pack $top.l.list -side left -fill x -fill y

    # The ">>" button, puts a selected CM name in textbox
    label $top.m.spacefiller -text "        "
    button $top.m.b -text ">>" -command {put_cm_in_textbox}
    pack $top.m.spacefiller $top.m.b  -pady 4
    # OR double-click does the same thing:
    bind .select.main.l.list <Double-Button-1> {
	global cm_names new_cm_name new_cm_ident
	set index [.select.main.l.list curselection]
	incr index
	set new_cm_name $cm_names($index)
	set new_cm_ident {}
    }

    # textboxes for CM name, identifier
    label $top.r.namelabel -text "CM Name: " -anchor w
    entry $top.r.nameentry -textvariable new_cm_name -bg white
    label $top.r.idlabel -text "CM Identifier: " -anchor w
    entry $top.r.identry -textvariable new_cm_ident -bg white
    button $top.r.okb -text "Add" -command "add_cm"
    pack $top.r.namelabel $top.r.nameentry $top.r.idlabel \
	    $top.r.identry $top.r.okb -pady 5 -fill both

    pack $top.l $top.m $top.r -side left -fill both -padx 5 -pady 5

    frame .select.f
    button .select.f.saveb -text "Save & close" \
	-command "save_module"
    button .select.f.cancelb -text "Cancel" \
	    -command "seeifsaved"
    pack .select.f.saveb .select.f.cancelb -side left -padx 10
    pack $top .select.f -side top -pady 10
}

proc seeifsaved {} {
    # if the .cm_selected window exists, they had an accelerator
    # made.  Ask if they want to lose it.
    global GUI_DIR mod_base num_cm
    if { [winfo exists .cm_selected]==1 } {
	# THE USER WAS WORKING ON A NEW ACCELERATOR -
	# ASK IF THEY WANT TO DISCARD IT.
	set result [tk_dialog .okay "Save accelerator" \
		"Are you sure you want to discard this accelerator?" \
		question 0 Yes No  ]
	if {$result==0} {
	    destroy .cm_selected
	    set num_cm 0
	    destroy .select
	} else {
	    return
	}
    } else {
	destroy .select
    }
}

proc put_cm_in_textbox {} {
    global cm_names new_cm_name new_cm_ident
    set index [.select.main.l.list curselection]
    incr index
    set new_cm_name $cm_names($index)
    set new_cm_ident {}
}

proc add_cm {} {
    # add a CM to the accelerator.  set its type and identifier then
    # call update_popup to put it on .cm_selected.

    global num_cm cm_type cm_ident cm_names new_cm_name new_cm_ident maxvals

    if $num_cm>=$maxvals(MAX_CMs) {
	tk_dialog .toobig "Maximum reached" "You've reached the maximum number\
		of CMs allowed, as set in the file beam_user_macros.mortran." \
		info 0 OK
	return
    }

    if { [string first " " $new_cm_ident]>=0 || \
	    [string first "." $new_cm_ident]>=0 || \
	    [string length $new_cm_ident]>8 || \
	    [string compare $new_cm_ident "exit"]==0 } {
	tk_dialog .name "Bad CM identifier" "The identifying name of a\
		CM cannot contain spaces or periods, and\
		must not have more than 8 characters in length.  The word\
		'exit' cannot be used as an identifer."  error 0 OK
	return
    }

    for {set i 1} {$i <= [array size cm_names]} {incr i} {
	if [string compare $new_cm_name $cm_names($i)]==0 {
	    set index [incr i -1]
	    break
	}
    }
    if [expr $num_cm == 0] popup_cm_window
    # only destroy mainb if it exists
    catch {destroy .cm_selected.f1.mainb}
    for {set i 1} {$i <= $num_cm} {incr i} {
	destroy .cm_selected.f1.cm($i)
    }
    incr num_cm
    set cm_type($num_cm) [expr $index+1]
    set cm_ident($num_cm) $new_cm_ident
    update_popup
}



