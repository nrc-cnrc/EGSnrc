
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: define phantom
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


proc define_phantom { } {
    global CTdataflag values names numopts options grouping imax start_dir env
    global helvfont pegs4filename izscan izopts izvox iphant is_pegsless

    if [string compare $CTdataflag ""]==0 {
	# CTdataflag has not been defined.
	tk_dialog .error "Error" "You haven't chosen how you\
		would like to define the phantom\
		(voxel by voxel or phase-space file).  Please set it and try\
		again." error 0 OK
	return
    }

    if $CTdataflag==0 {
	# if voxel-by-voxel, require a pegs4 data file for media.
	if {[string compare $pegs4filename ""]==0 && $is_pegsless==0}  {
	    # get a pegs4file, read the media into pegs
	    get_pegs4file
	    # wait for the user to finish loading a pegs4file before continuing
	    tkwait window .pegs4
	}
    }

    toplevel .main_w.phantom -bd 4
    wm title .main_w.phantom "Define Phantom"
    set top .main_w.phantom

    set w .main_w.phantom
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	    - [winfo vrootx [winfo parent $w]] -400]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	    - [winfo vrooty [winfo parent $w]] -300]
    wm geom .main_w.phantom +$x+$y

    if $CTdataflag==0 {

	frame $top.ct
	set top1 $top.ct

	# This is for non-CT data input.

	label $top1.dimlab -text "Step 1: Voxel Dimensions" -font $helvfont
	pack $top1.dimlab -pady 5

	if { [catch {expr $grouping(0)}]==1 } { set grouping(0) 1 }
	if { [catch {expr $grouping(1)}]==1 } { set grouping(1) 1 }
	if { [catch {expr $grouping(2)}]==1 } { set grouping(2) 1 }

	frame $top1.xyz
	set dir(0) x; set dir(1) y; set dir(2) z
	# grouping(0/1/2) is the flag for groups/indiv.
	# 3 columns for x,y,z (grid)
	for {set j 0} {$j<3} {incr j} {
	    frame $top1.xyz.f$j

	    # radiobuttons for define voxels as ind. or grp.
	    # make a frame for the 2 radiobutton options:

	    set w $top1.xyz.f$j.rbs
	    frame $w
	    label $w.lab -text "Define $dir($j) voxels:"
	    frame $w.rr
	    radiobutton $w.rr.r1 -text "individually"\
		    -variable grouping($j) -value 0
	    radiobutton $w.rr.r2 -text "as groups"\
		    -variable grouping($j) -value 1
	    pack $w.rr.r1 $w.rr.r2 -side top -anchor w
	    pack $w.lab $w.rr -side left
	    pack $w -pady 5

	    set w $top1.xyz.f$j.nvox
	    frame $w
	    label $w.lab -text "Number of $dir($j) voxels or groups of voxels"
	    entry $w.ent -textvariable imax($j) -width 5
	    pack $w.lab $w.ent -side left
	    pack $w

	    button $top1.xyz.f$j.defb -text "Define $dir($j) voxels..."\
		    -command "define_voxels $dir($j)"
	    pack $top1.xyz.f$j.defb -pady 5

	    pack $top1.xyz.f$j -side left

	    if $j!=2 {
		frame $top1.xyz.sep$j -bd 4 -width 2 -height 100 -relief groove
		pack $top1.xyz.sep$j -side left -fill y -padx 10
	    }
	}

	pack $top1.xyz -pady 10

	# Add a separator
	frame $top1.sep1 -bd 4 -width 100 -height 2 -relief groove
	pack $top1.sep1 -side top -fill x -pady 10

	label $top1.medlab -text "Step 2: Define Media (include medium for \
                                  region surrounding phantom)" -font $helvfont
	pack $top1.medlab -pady 5

	frame $top1.inp2 -bd 4
	label $top1.inp2.label -text {Number of media}
	entry $top1.inp2.inp -textvariable values(2) -width 8
	button $top1.inp2.b -text "Define media..." -command "define_nmed"
	pack $top1.inp2.label -anchor w -side left
	pack $top1.inp2.inp -side left
	pack $top1.inp2.b -anchor e -side right -padx 5
	pack $top1.inp2 -side top

	# Add a separator
	frame $top1.sep0 -bd 4 -width 100 -height 2 -relief groove
	pack $top1.sep0 -side top -fill x -pady 10

	label $top1.outlab -text "Step 3: Output" -font $helvfont
	pack $top1.outlab -pady 5

        frame $top1.out

        checkbutton $top1.out.iphant -text "Output .egsphant file"\
                  -variable iphant

	button $top1.out.zscanbutton -text "Select the voxels for\
                which to list the dose..." -command define_izscan
        pack $top1.out.iphant $top1.out.zscanbutton -side left -padx 15
        pack $top1.out -side top -pady 5

    } else {
	frame $top.ct
	set top2 $top.ct

	# This is for CT data input

	label $top2.phlabel -text {Phantom filename:}
	pack $top2.phlabel -side top -anchor w
	frame $top2.inp1 -bd 4
	entry $top2.inp1.inp -textvariable PhantFileName -width 40
	button $top2.inp1.b1 -text "Browse local area..." \
		-command {query_filename update_phantfile "$start_dir" egsphant}
	pack $top2.inp1.inp -side left -expand true -fill x -anchor w
	pack $top2.inp1.b1 -side right -anchor e -fill x
	pack $top2.inp1 -anchor w -side left -fill x -expand true
	pack $top2.inp1 -side top -fill x

	# zeroairdose, doseprint
	for {set i 25} {$i <= 26} {incr i} {
	    set w $top2.inp$i
	    frame $w -bd 4
	    button $w.help -text "?" -command "help $i"
	    label $w.label -text $names($i)  -anchor w
	    set width [string length $values($i)]
	    if $width<20 { set width 20 }
	    menubutton $w.inp -text $values($i)\
		    -menu $w.inp.m -bd 1 \
		    -relief raised -indicatoron 1 -width $width
	    menu $w.inp.m
	    for {set iopt 0} {$iopt < $numopts($i)} {incr iopt} {
		$w.inp.m add command -label $options($i,$iopt) \
			-command "set_value $i $iopt $w.inp"
	    }
	    pack $w.help -anchor w -side left
	    pack $w.label -side left -padx 10
	    pack $w.inp -anchor e -side right -fill x -expand true
	    pack $w -side top -fill x
	}
    }

    pack $top.ct -side top -padx 5

    # Add a separator
    frame $top.sep4 -bd 4 -width 100 -height 2 -relief groove
    pack $top.sep4 -fill x -pady 10

    #put in the "Done" button:
    frame $top.button
    #update the medsur input down in the main input window
    button $top.button.b -text "Done" -command "remake_medsur_menu;\
            destroy .main_w.phantom" -relief groove -bd 8
    pack $top.button.b -pady 5
    pack $top.button -side top -pady 5
}

proc remake_medsur_menu {} {
    global values medium medsur
    #destroy the menu first

    set medflag 0
    set w .main_w.main.sim.right.dflag.med.inp
    destroy $w.m
    menu $w.m -tearoff no
    for {set j 0} {$j<=$values(2)} {incr j} {
         $w.m add command -label $medium($j)\
                  -command "set medsur {$medium($j)};\
                    $w configure -text {$medium($j)}"
         if { [string compare $medsur $medium($j)]==0 } {
              set medflag 1
         }
    }
    if {$medflag==0} {
         set j 0
         set medsur $medium($j)
         $w configure -text $medium($j)
    }
}

proc define_voxels { dir } {
    global imax ivox gvox grouping helvfont

    if [string compare $dir x]==0 { set num $imax(0); set index 0 }
    if [string compare $dir y]==0 { set num $imax(1); set index 1 }
    if [string compare $dir z]==0 { set num $imax(2); set index 2 }

    if [string compare $num ""]==0 {
	# The number of voxels or groups has not been defined.
	tk_dialog .error "Error" "You haven't set the number of voxels in\
		this direction.  Please set it and try again." error 0 OK
	return
    }

    toplevel .main_w.voxel$dir
    set top .main_w.voxel$dir
    wm title $top "Define $dir voxels"

    if $grouping($index)==1 {
	# do it in groups

	message $top.mess -text "Enter the minimum $dir-value, then for each\
		group enter the width (in cm)\
		of the voxels in this group and the number of voxels which\
		have this $dir-width." -width 4i -font $helvfont
	pack $top.mess -pady 10

	frame $top.min
	label $top.min.lab -text "Minimum $dir-boundary (cm)"
	entry $top.min.inp -textvariable gvox($index,0) -width 8
	pack $top.min.lab -anchor w -side left -padx 5
	pack $top.min.inp -side right -anchor e -expand true -fill x -padx 5
	pack $top.min -pady 5

	frame $top.grid -bd 4
	label $top.grid.wlab -text "$dir-width"
	label $top.grid.nlab -text "Number in group"
	grid config $top.grid.wlab -row 0 -column 1
	grid config $top.grid.nlab -row 0 -column 2
	for {set j 1} {$j<=$num} {incr j} {
	    label $top.grid.l$j -text "Group $j"
	    grid configure $top.grid.l$j -row $j -column 0 -padx 5
	    entry $top.grid.e1$j -textvariable gvox($index,$j,1) -width 10
	    grid configure $top.grid.e1$j -row $j -column 1
	    entry $top.grid.e2$j -textvariable gvox($index,$j,2) -width 10
	    grid configure $top.grid.e2$j -row $j -column 2
	}
	pack $top.grid -side top

	button $top.maxb -text "Show maximum $dir-value" -command\
		"group_max_bdd $dir $num"
	pack $top.maxb -pady 5
    } else {
	# define voxels individually
	message $top.mess -text "Enter the $dir-voxel spacing in cm; \
		${dir}(i-1) is the $dir-coordinate of the start of voxel i."\
		-width 5i -font $helvfont
	pack $top.mess -pady 10 -expand true -fill x

	# allow 15 points per line, then start a new one.

	frame $top.grid -bd 4
	set col 0
	set row 0
	for {set j 0} {$j<=$num} {incr j} {
	    label $top.grid.l$j -text "$dir$j"
	    grid configure $top.grid.l$j -row $row -column $col -padx 5
	    entry $top.grid.e$j -textvariable ivox($index,$j) -width 5
	    grid configure $top.grid.e$j -row [expr $row+1] -column $col
	    incr col
	    if $col>14 {
		set col 0
		incr row 2
	    }
	}
	pack $top.grid -side top
    }
    frame $top.b
    button $top.b.okb -text "OK" -command "destroy $top"\
	    -relief groove -bd 8
    pack $top.b.okb -side left -padx 10
    pack $top.b -pady 10
}

proc define_nmed {} {
    global values medium helvfont nmed pegs

    # values(2) is the number of media

    if [string compare $values(2) ""]==0 {
	# Number of media has not been defined.
	tk_dialog .error "Error" "You haven't set the number of media\
		that you intend to use.  Please set it and try\
		again." error 0 OK
	return
    }

    toplevel .main_w.phantom.media
    set top .main_w.phantom.media
    wm title $top "Media"

    message $top.mess -text "Select every medium you would like to use in this\
	    simulation (including that surrounding the phantom, for phase space beam input) \
	    and enter the ESTEPE value that you wish\
	    to use for each medium.  Any voxel for which the medium\
	    is not explicitly\
	    declared will be assumed to be the first medium selected here, with\
	    its default density." -width 500 -font $helvfont
    pack $top.mess -pady 10

    frame $top.grid -bd 4
    label $top.grid.medlab -text "Medium"
    grid config $top.grid.medlab -row 0 -column 1
    for {set j 1} {$j<=$values(2)} {incr j} {
	label $top.grid.l$j -text "$j"
	grid configure $top.grid.l$j -row $j -column 0
	# Medium: an option menu with pegs($i)
	set w $top.grid.med$j
	frame $w
	set width 24
	menubutton $w.ent -text $medium($j) -menu \
		$w.ent.m -bd 1 -relief raised -indicatoron 1\
		-width $width
	menu $w.ent.m -tearoff no
	for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
            $w.ent.m add command -label $pegs($iopt) -columnbreak [expr $iopt % 40==0] \
                    -command "set_material $w $iopt $j"
	}
	pack $w.ent -anchor e -side right -fill x -expand true
	grid configure $top.grid.med$j -row $j -column 1 -padx 5
    }
    pack $top.grid -side top

    button $top.nextstep -text "Set media of voxels..." -command voxel_med
    pack $top.nextstep -side top -pady 5

    frame $top.b
    button $top.b.okb -text "OK" -command "destroy $top"\
	    -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
	    {help medium}  -relief groove -bd 8
    pack $top.b.okb $top.b.helpb -side left -padx 10
    pack $top.b -pady 10
}

proc voxel_med {} {
    # define the voxel media

    global mvox helvfont nrow medium values

    # values(2) is the number of media

    toplevel .main_w.phantom.media.vmed
    set w .main_w.phantom.media.vmed
    wm title $w "Media of voxels"

    if [string compare $nrow ""]==0 { set nrow 1 }

    for {set i 1} {$i<=$nrow} {incr i} {
	if [catch {string compare $mvox($i,0) ""}]==1 {
	    set mvox($i,0) {}
	}
    }

    if [string compare $medium(1) ""]==0 {
	set curmed1 "undefined"
    } else {
	set curmed1 $medium(1)
    }
    message $w.message -text "Define a group of voxels by entering\
	    the voxel indices, then set the medium and medium density for\
	    the group.  Any voxel for which the medium is not explicitly\
	    declared will be assumed to be the first medium selected in\
	    the Define Media window (currently $curmed1), with\
	    its default density.  The default density of a selected material\
	    will be used if it is not overridden in the space provided.  \
	    Click 'Add a group' to define a new group, 'Remove last group'\
	    to remove the last group." \
	    -width 600 -font $helvfont
    pack $w.message -side top -anchor n -fill x -expand true

    frame $w.grid -bd 4
    label $w.grid.l0 -text "from x"
    label $w.grid.l1 -text "to x"
    label $w.grid.l2 -text "from y"
    label $w.grid.l3 -text "to y"
    label $w.grid.l4 -text "from z"
    label $w.grid.l5 -text "to z"
    label $w.grid.l6 -text "medium"
    label $w.grid.l7 -text "density"
    grid configure $w.grid.l0 -row 0 -column 0
    grid configure $w.grid.l1 -row 0 -column 1
    grid configure $w.grid.l2 -row 0 -column 2
    grid configure $w.grid.l3 -row 0 -column 3
    grid configure $w.grid.l4 -row 0 -column 4
    grid configure $w.grid.l5 -row 0 -column 5
    grid configure $w.grid.l6 -row 0 -column 6
    grid configure $w.grid.l7 -row 0 -column 7

    for {set row 1} {$row<=$nrow} {incr row} {
	entry $w.grid.e0$row -textvariable mvox($row,0,0) -width 6
	entry $w.grid.e1$row -textvariable mvox($row,0,1) -width 6
	entry $w.grid.e2$row -textvariable mvox($row,1,0) -width 6
	entry $w.grid.e3$row -textvariable mvox($row,1,1) -width 6
	entry $w.grid.e4$row -textvariable mvox($row,2,0) -width 6
	entry $w.grid.e5$row -textvariable mvox($row,2,1) -width 6
	set w2 $w.grid.e6$row
	frame $w2
	set width 24
	menubutton $w2.ent -text $mvox($row,0) -menu \
		$w2.ent.m -bd 1 -relief raised -indicatoron 1\
		-width $width
	menu $w2.ent.m -tearoff no
	for {set iopt 1} {$iopt <= $values(2)} {incr iopt} {
	    $w2.ent.m add command -label $medium($iopt) \
		    -command "set_material2 $w2 $iopt $row"
	}
	pack $w2.ent -anchor e -side right -fill x -expand true
	entry $w.grid.e7$row -textvariable mvox($row,1) -width 8
	for {set i 0} {$i<=7} {incr i} {
	    grid configure $w.grid.e$i$row -row $row -column $i
	}
    }
    pack $w.grid -side top

    frame $w.b
    button $w.b.addb -text "Add a group" -command "add_group $w"\
	    -relief groove -bd 8
    button $w.b.remb -text "Remove last group" -command "remove_group $w"\
	    -relief groove -bd 8
    button $w.b.okb -text "OK" -command "check_mvox $w"\
	    -relief groove -bd 8
    pack $w.b.remb $w.b.addb $w.b.okb -side left -padx 10
    pack $w.b -pady 10
}

proc check_mvox { w } {
    global mvox nrow

    set flag 0
    for {set i 1} {$i<=$nrow} {incr i} {
	for {set j 0} {$j<=2} {incr j} {
	    for {set k 0} {$k<=1} {incr k} {
		# mvox($i,$j,$k) must be an integer
		set rem [expr $mvox($i,$j,$k) - int($mvox($i,$j,$k))]
		if $rem!=0 {
		    set flag 1; break
		}
		# Could put a check in here to make sure indices are viable
		# with flag=2
	    }
	}
    }
    if $flag==0 {
	destroy $w
    } elseif $flag==1 {
	tk_dialog .error "Error" "The values in the text boxes must\
		be integers representing the voxel indices, not the\
		actual coordinates." error 0 OK
    }
}

proc add_group { w } {
    global mvox nrow values medium

    incr nrow

    entry $w.grid.e0$nrow -textvariable mvox($nrow,0,0) -width 6
    entry $w.grid.e1$nrow -textvariable mvox($nrow,0,1) -width 6
    entry $w.grid.e2$nrow -textvariable mvox($nrow,1,0) -width 6
    entry $w.grid.e3$nrow -textvariable mvox($nrow,1,1) -width 6
    entry $w.grid.e4$nrow -textvariable mvox($nrow,2,0) -width 6
    entry $w.grid.e5$nrow -textvariable mvox($nrow,2,1) -width 6
    set w2 $w.grid.e6$nrow
    frame $w2
    set width 24
    menubutton $w2.ent -text {} -menu $w2.ent.m -bd 1 -relief raised\
	    -indicatoron 1 -width $width
    menu $w2.ent.m -tearoff no
    for {set iopt 1} {$iopt <= $values(2)} {incr iopt} {
	$w2.ent.m add command -label $medium($iopt) \
		-command "set_material2 $w2 $iopt $nrow"
    }
    pack $w2.ent -anchor e -side right -fill x -expand true
    entry $w.grid.e7$nrow -textvariable mvox($nrow,1) -width 8
    for {set i 0} {$i<=7} {incr i} {
	grid configure $w.grid.e$i$nrow -row $nrow -column $i
    }
}
proc remove_group { w } {
    global nrow
    for {set i 0} {$i<8} {incr i} {
	destroy $w.grid.e$i$nrow
    }
    incr nrow -1
}

proc set_material { w iopt j } {
    global pegs medium
    set medium($j) $pegs($iopt)
    $w.ent configure -text $pegs($iopt)
}

proc set_material2 { w iopt j } {
    global medium mvox
    set mvox($j,0) $medium($iopt)
    $w.ent configure -text $medium($iopt)
}

proc group_max_bdd { dir ngroups } {
    global gvox

    # get the maximum $dir boundary for groups

    if [string compare $dir x]==0 {set dn 0}
    if [string compare $dir y]==0 {set dn 1}
    if [string compare $dir z]==0 {set dn 2}

    set value 0
    catch {set value $gvox($dn,0)}
    for {set i 1} {$i<=$ngroups} {incr i} {
	catch {set value [expr $value + $gvox($dn,$i,1)*$gvox($dn,$i,2)]}
    }
    tk_dialog .main_w.max_bdd "Maximum $dir boundary" "The maximum $dir\
	    boundary is $value cm, based on these inputs." info 0 OK
}

proc define_izscan {} {
    # define the voxel media

    global izvox helvfont izrow izopts

    toplevel .main_w.phantom.iz
    set w .main_w.phantom.iz
    wm title $w "IZSCAN of voxels"

    if [string compare $izrow ""]==0 { set izrow 1 }

    message $w.message -text "Define a group of voxels by entering\
	    the voxel indices, then set the direction of the the scan\
	    per page.  Unless declared here, the default is no output.  \
	    Click 'Add a group' to define a new group." \
	    -width 500 -font $helvfont
    pack $w.message -side top -anchor n -fill x -expand true

    frame $w.grid -bd 4
    label $w.grid.l0 -text "from x"
    label $w.grid.l1 -text "to x"
    label $w.grid.l2 -text "from y"
    label $w.grid.l3 -text "to y"
    label $w.grid.l4 -text "from z"
    label $w.grid.l5 -text "to z"
    label $w.grid.l6 -text "scan"
    grid configure $w.grid.l0 -row 0 -column 0
    grid configure $w.grid.l1 -row 0 -column 1
    grid configure $w.grid.l2 -row 0 -column 2
    grid configure $w.grid.l3 -row 0 -column 3
    grid configure $w.grid.l4 -row 0 -column 4
    grid configure $w.grid.l5 -row 0 -column 5
    grid configure $w.grid.l6 -row 0 -column 6

    for {set row 1} {$row<=$izrow} {incr row} {
	entry $w.grid.e0$row -textvariable izvox($row,0,0) -width 6
	entry $w.grid.e1$row -textvariable izvox($row,0,1) -width 6
	entry $w.grid.e2$row -textvariable izvox($row,1,0) -width 6
	entry $w.grid.e3$row -textvariable izvox($row,1,1) -width 6
	entry $w.grid.e4$row -textvariable izvox($row,2,0) -width 6
	entry $w.grid.e5$row -textvariable izvox($row,2,1) -width 6
	menubutton $w.grid.e6$row -text $izvox($row) -menu $w.grid.e6$row.m\
		-bd 1 -relief raised -width 15
	menu $w.grid.e6$row.m
	$w.grid.e6$row.m add command -label $izopts(0) -command\
		"set izvox($row) {$izopts(0)}; $w.grid.e6$row configure\
		-text {$izopts(0)}"
	$w.grid.e6$row.m add command -label $izopts(1) -command\
		"set izvox($row) {$izopts(1)}; $w.grid.e6$row configure\
		-text {$izopts(1)}"
        $w.grid.e6$row.m add command -label $izopts(2) -command\
                "set izvox($row) {$izopts(2)}; $w.grid.e6$row configure\
                -text {$izopts(2)}"
	for {set i 0} {$i<=6} {incr i} {
	    grid configure $w.grid.e$i$row -row $row -column $i
	}
    }
    pack $w.grid -side top

    frame $w.b
    button $w.b.addb -text "Add a group" -command "add_scan_group $w"\
	    -relief groove -bd 8
    button $w.b.remb -text "Remove last group" -command "remove_scan_group $w"\
	    -relief groove -bd 8
    button $w.b.okb -text "OK" -command "destroy $w"\
	    -relief groove -bd 8
    pack $w.b.remb $w.b.addb $w.b.okb -side left -padx 10
    pack $w.b -pady 10
}

proc add_scan_group { w } {
    global izvox izrow izopts

    incr izrow

    # give an initial value of zero to all boxes.
    for {set i 0} {$i<=2} {incr i} {
	set izvox($izrow,$i,0) 0
	set izvox($izrow,$i,1) 0
        set izvox($izrow,$i,2) 0
    }

    entry $w.grid.e0$izrow -textvariable izvox($izrow,0,0) -width 6
    entry $w.grid.e1$izrow -textvariable izvox($izrow,0,1) -width 6
    entry $w.grid.e2$izrow -textvariable izvox($izrow,1,0) -width 6
    entry $w.grid.e3$izrow -textvariable izvox($izrow,1,1) -width 6
    entry $w.grid.e4$izrow -textvariable izvox($izrow,2,0) -width 6
    entry $w.grid.e5$izrow -textvariable izvox($izrow,2,1) -width 6
    set izvox($izrow) {}
    menubutton $w.grid.e6$izrow -text $izvox($izrow) -menu $w.grid.e6$izrow.m\
	    -bd 1 -relief raised -width 15
    menu $w.grid.e6$izrow.m
    $w.grid.e6$izrow.m add command -label $izopts(0) -command\
	    "set izvox($izrow) {$izopts(0)}; $w.grid.e6$izrow configure\
	    -text {$izopts(0)}"
    $w.grid.e6$izrow.m add command -label $izopts(1) -command\
	    "set izvox($izrow) {$izopts(1)}; $w.grid.e6$izrow configure\
	    -text {$izopts(1)}"
    $w.grid.e6$izrow.m add command -label $izopts(2) -command\
            "set izvox($izrow) {$izopts(2)}; $w.grid.e6$izrow configure\
            -text {$izopts(2)}"
    for {set i 0} {$i<=6} {incr i} {
	grid configure $w.grid.e$i$izrow -row $izrow -column $i
    }
}
proc remove_scan_group { w } {
    global izrow
    for {set i 0 } {$i<7} {incr i} {
	destroy $w.grid.e$i$izrow
    }
    incr izrow -1
}

