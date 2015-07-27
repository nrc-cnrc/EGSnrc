
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: CONS3R
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


proc init_CONS3R { id } {
    global cmval

    for {set i 0} {$i<5} {incr i} {
	set cmval($id,$i) {}
    }
    # I'm not sure 20 will do...
    for {set j 0} {$j<=20} {incr j} {
	set cmval($id,5,$j) {}
    }
    for {set i 0 } {$i<2} {incr i} {
	for {set j 0} {$j<6} {incr j} {
	    set cmval($id,6,$i,$j) {}
	}
    }
}

proc read_CONS3R { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read CONS3R $cm_ident($id).  The inputs don't begin where\
		they should.  There is either an error in the CM preceding\
		this, or the accelerator module file is not the correct one\
		for use with this input file." error 0 OK
	return -code break
	# breaks out of the loops this procedure was called from.
    }

    # read rmax (0)
    gets $fileid data
    set data [get_val $data cmval $id,0]

    # read title (1)
    gets $fileid data
    set cmval($id,1) $data

    # read zmin (2)
    gets $fileid data
    set data [get_val $data cmval $id,2]

    #read zthick (3)
    gets $fileid data
    set data [get_val $data cmval $id,3]

    #read numnode (4)
    gets $fileid data
    set data [get_val $data cmval $id,4]

    # For each node, read 2 values, (z,r) (0,1)
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	gets $fileid data
	for {set j 0} {$j<=1} {incr j} {
	    set data [get_val $data cmval $id,5,$j,$i]
	}
    }

    # For inner, read 5 values then material
    for {set i 0} {$i<=1} {incr i} {
	gets $fileid data
	for {set j 0} {$j<=4} {incr j} {
	    set data [get_val $data cmval $id,6,$i,$j]
	}
	gets $fileid data
	set data [get_str $data cmval $id,6,$i,5]
    }
}

proc edit_CONS3R { id zmax } {
    global cmval GUI_DIR values omega helvfont help_cons3r_text nmed medium med_per_column

    catch { destroy .cons3r$id }
    toplevel .cons3r$id
    wm title .cons3r$id "Edit CONS3R, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NUMNODE.
    frame .cons3r$id.frm -bd 4
    set top .cons3r$id.frm

    label $top.mainlabel -text "Cons3r" -font $helvfont
    pack $top.mainlabel -pady 10

    #CONS3R_macros.mortran:REPLACE {$MAX_N_$CONS3R} WITH {3}
    #CONS3R_macros.mortran:REPLACE {$NPOINT_$CONS3R} WITH {15}
    get_2_defaults CONS3R $top MAX_N "maximum number of layers"\
	    NPOINT "maximum number of points"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    set w $top

    add_rmax_rad $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp2 $id,2

    frame $w.inp3
    label $w.inp3.label -text {Thickness of CM (cm)}
    entry $w.inp3.inp -textvariable cmval($id,3) -width 8
    pack $w.inp3.label -anchor w -side left
    pack $w.inp3.inp -anchor e -side right -fill x -expand true
    pack $w.inp3 -side top -fill x

    frame $top.inp5
    label $top.inp5.label -text {Number of points used to define the CM}
    entry $top.inp5.inp -textvariable cmval($id,4) -width 8
    button $top.inp5.nextproc -text " Define points >> "\
	    -command "define_nnode $id"
    pack $top.inp5.label -anchor w -side left
    pack $top.inp5.inp -side left -padx 10
    pack $top.inp5.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp5 -side top -fill x

    frame $top.f2
    set w $top.f2

    label $w.l0 -text "Electron cutoff energy (default ECUTIN) (MeV)"
    label $w.l1 -text "Photon cutoff energy (default PCUTIN) (MeV)"
    label $w.l2 -text "Dose zone (0 for no scoring)"
    label $w.l3 -text "Associate with LATCH bit"
    label $w.l4 -text "Range rejection as global (0) or off (-1)"
    label $w.l5 -text "Material"

    button $w.h0 -text "?" -command "help_ecut $top"
    button $w.h1 -text "?" -command "help_pcut $top"
    button $w.h2 -text "?" -command "help_dose $top"
    button $w.h3 -text "?" -command "help_latch $top"
    button $w.h4 -text "?" -command "help_cons3r_RR $top"
    button $w.h5 -text "?" -command "help_material $top"

    for {set i 0} {$i<6} {incr i} {
	grid configure $w.h$i -column 0 -row [expr $i+1] -sticky w
	grid configure $w.l$i -column 1 -row [expr $i+1] -sticky w
    }

    label $w.inside -text "Inside"
    if [string compare $cmval($id,6,0,0) ""]==0 {
	set cmval($id,6,0,0) $values(ecut)
    }
    if [string compare $cmval($id,6,0,1) ""]==0 {
	set cmval($id,6,0,1) $values(pcut)
    }

    entry $w.e0 -textvariable cmval($id,6,0,0)
    entry $w.e1 -textvariable cmval($id,6,0,1)
    entry $w.e2 -textvariable cmval($id,6,0,2)
    entry $w.e3 -textvariable cmval($id,6,0,3)
    entry $w.e4 -textvariable cmval($id,6,0,4)
    # material option menu
    menubutton $w.e5 -text $cmval($id,6,0,5) -menu \
	    $w.e5.m -bd 1 -relief raised -indicatoron 0 -width 20
    menu $w.e5.m -tearoff no
    for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
	$w.e5.m add command -label $medium($iopt) \
		-command "set_grid_material $w.e5 $iopt $id,6,0,5" -columnbreak [expr $iopt % $med_per_column==0]
    }

    grid configure $w.inside -column 2 -row 0
    for {set i 0} {$i<6} {incr i} {
	grid configure $w.e$i -column 2 -row [expr $i+1]
    }

    label $w.outside -text "Outside"
    if [string compare $cmval($id,6,1,0) ""]==0 {
	set cmval($id,6,1,0) $values(ecut)
    }
    if [string compare $cmval($id,6,1,1) ""]==0 {
	set cmval($id,6,1,1) $values(pcut)
    }
    entry $w.ee0 -textvariable cmval($id,6,1,0)
    entry $w.ee1 -textvariable cmval($id,6,1,1)
    entry $w.ee2 -textvariable cmval($id,6,1,2)
    entry $w.ee3 -textvariable cmval($id,6,1,3)
    entry $w.ee4 -textvariable cmval($id,6,1,4)
    # material option menu
    menubutton $w.ee5 -text $cmval($id,6,1,5) -menu \
	    $w.ee5.m -bd 1 -relief raised -indicatoron 0 -width 20
    menu $w.ee5.m -tearoff no
    for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
	$w.ee5.m add command -label $medium($iopt) \
		-command "set_grid_material $w.ee5 $iopt $id,6,1,5"
    }

    grid configure $w.outside -column 3 -row 0
    for {set i 0} {$i<6} {incr i} {
	grid configure $w.ee$i -column 3 -row [expr $i+1] -padx 10
    }

    pack $top.f2 -side top -fill both -pady 5

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "check_cons3r $id; destroy \
	    .cons3r$id" -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command\
	    "help_gif .cons3r$id.help {$help_cons3r_text} help_cons3r"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "check_cons3r2 $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -padx 10 -side left
    pack $top.buts
    pack $top
}

proc define_nnode { id } {
    global cmval helvfont

    catch { destroy .cons3r$id.baby }
    toplevel .cons3r$id.baby
    set top .cons3r$id.baby
    wm title $top "Define cons3r node points"

    set nnode $cmval($id,4)

    label $top.lab -text "Define (Z,R) coordinates" -font $helvfont
    pack $top.lab -side top -padx 5 -pady 5

    frame $top.f2 -bd 4

    set w $top.f2

    for {set i 1} {$i<=$nnode} {incr i} {
	label $w.l$i -text "Point $i"
	grid config $w.l$i -column 0 -row $i
    }

    label $w.lz -text "Z"
    grid config $w.lz -column 1 -row 0
    for {set i 1} {$i<=$nnode} {incr i} {
	entry $w.e$i -textvariable cmval($id,5,0,$i) -width 8
	grid config $w.e$i -column 1 -row $i
    }

    label $w.lr -text "R"
    grid config $w.lr -column 2 -row 0
    for {set i 1} {$i<=$nnode} {incr i} {
	entry $w.ee$i -textvariable cmval($id,5,1,$i) -width 8
	grid config $w.ee$i -column 2 -row $i
    }

    pack $top.f2 -side top
    button $top.okb -text "OK" -command "check_cons3r $id; destroy \
	    .cons3r$id.baby" -relief groove -bd 8
    pack $top.okb -pady 10
}

proc check_cons3r { id } {
    global cmval

    if $cmval($id,2)!=$cmval($id,5,0,1) {
	tk_dialog .warning "Warning" "Zmin must have the same value as\
		the first of the set of (z,r) points.  Closing, but not\
		happy about it." info 0 OK
    }
}

proc check_cons3r2 { id } {
    global cmval
    if $cmval($id,2)==$cmval($id,5,0,1) {
	show_CONS3R $id
    } else {
	tk_dialog .warning "Error" "Zmin must have the same value as\
		the first of the set of (z,r) points.  Cannot show preview."\
		error 0 OK
    }
}

proc write_CONS3R {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), ZMIN"
    puts $fileid "$cmval($id,3), ZTHICK"
    puts $fileid "$cmval($id,4), NUM_NODE"

    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set str {}
	for {set j 0} {$j<=1} {incr j} {
	    set str "$str$cmval($id,5,$j,$i), "
	}
	puts $fileid $str
    }
    for {set i 0} {$i<=1} {incr i} {
	set str {}
	set str "$str$cmval($id,6,$i,0), "
	set str "$str$cmval($id,6,$i,1), "
	set str "$str$cmval($id,6,$i,2), "
	set str "$str$cmval($id,6,$i,3), "
	set str "$str$cmval($id,6,$i,4), "
	puts $fileid $str
	puts $fileid $cmval($id,6,$i,5)
    }
}


proc show_CONS3R { id } {
    global cmval xrange zrange med nmed medium cm_ticks

    catch { destroy .cons3r$id.show }
    toplevel .cons3r$id.show
    wm title .cons3r$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange,zrange:
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_CONS3R $id }]==1 {
	destroy .cons3r$id.show
	tk_dialog .cons3r$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .cons3r$id.show.buts
    button .cons3r$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .cons3r$id.show xz" -relief groove -bd 8
    button .cons3r$id.show.buts.print -text "Print..." -command\
	    "print_canvas .cons3r$id.show.can 600 600" -relief groove -bd 8
    button .cons3r$id.show.buts.done -text "Done" -command\
	    "destroy .cons3r$id.show" -relief groove -bd 8
    pack .cons3r$id.show.buts.range .cons3r$id.show.buts.print\
	    .cons3r$id.show.buts.done -side left -padx 10
    pack .cons3r$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_CONS3R { id } {
    global xrange zrange helvfont xscale zscale l m cm_ticks

    catch {destroy .cons3r$id.show.can}

    # put the canvas up
    canvas .cons3r$id.show.can -width 600 -height 600

    # now add on the stuff that's there, scaled to fit into 500x500 (allow
    # a border of 50(z)/100(x) to show the scale)

    set width 450.0
    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    set l 100.0
    set m 50.0

    add_air $id .cons3r$id.show.can $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_CONS3R $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .cons3r$id.show.can

    coverup $l $m $width .cons3r$id.show.can

    set curx 0
    set cury 0
    label .cons3r$id.show.can.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .cons3r$id.show.can create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .cons3r$id.show.can.xy
    bind .cons3r$id.show.can <Motion> {
	global l m xscale zscale xrange zrange
	set curx %x
	set cury %y
	set curx [expr ($curx-$l)/$xscale+$xrange(0)]
	set cury [expr ($cury-$m)/$zscale+$zrange(0)]
        if { $curx<=$xrange(1) & $curx>=$xrange(0) & $cury<=$zrange(1) & \
                $cury>=$zrange(0) } {
	    set text [format "(%%6.3f, %%6.3f)" $curx $cury]
	    %W.xy configure -text $text
	}
    }

    add_axes xz $cm_ticks($id,x) $cm_ticks($id,z) $l $m $width $xrange(0)\
	    $xrange(1) $zrange(0) $zrange(1)\
	    $xscale $zscale .cons3r$id.show.can

    pack .cons3r$id.show.can -anchor n -side top
}

proc add_CONS3R {id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist medium nmed meds_used colornum

    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,6,0,5) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(out) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,6,1,5) $medium($j)]==0 {
	    set med(out) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }

    # Since I can't seem to make it accept a list of an arbitrary number of
    # points, stack polygons with 4 points.  (Found a way since this was done)

    set rmaxp [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set rmaxm [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    for {set i 1} {$i<$cmval($id,4)} {incr i} {
	set x(1) [expr ($cmval($id,5,1,$i)-$xmin)*$xscale+$l]
	set x(2) [expr ($cmval($id,5,1,[expr $i+1])-$xmin)*$xscale+$l]
	set x(3) [expr (-$cmval($id,5,1,[expr $i+1])-$xmin)*$xscale+$l]
	set x(4) [expr (-$cmval($id,5,1,$i)-$xmin)*$xscale+$l]

	set y(1) [expr ($cmval($id,5,0,$i)-$zmin)*$zscale+$m]
	set y(2) [expr ($cmval($id,5,0,[expr $i+1])-$zmin)*$zscale+$m]
	set y(3) [expr ($cmval($id,5,0,[expr $i+1])-$zmin)*$zscale+$m]
	set y(4) [expr ($cmval($id,5,0,$i)-$zmin)*$zscale+$m]

	set color [lindex $colorlist $med(in)]
	$parent_w create polygon $x(1) $y(1) $x(2) $y(2) $x(3)\
		$y(3) $x(4) $y(4) -fill $color

	# fill in the outside
	$parent_w create polygon $rmaxp $y(1) $rmaxp $y(2)\
		$x(2) $y(2) $x(1) $y(1)	-fill [lindex $colorlist $med(out)]
	$parent_w create polygon $rmaxm $y(1) $rmaxm $y(2)\
		$x(3) $y(3) $x(4) $y(4)	-fill [lindex $colorlist $med(out)]
    }
}

