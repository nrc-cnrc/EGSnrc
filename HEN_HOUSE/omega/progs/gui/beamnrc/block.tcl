
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: BLOCK
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


# Note:  Tcl seems to think of the toplevel ".block" as something special
# and does not give it a title bar.  I don't know why.

proc init_BLOCK { id } {
    global cmval

    for {set i 0} {$i<2} {incr i} {
	set cmval($id,$i) {}
    }
    for {set i 0} {$i<=2} {incr i} {
	set cmval($id,2,$i) {}
    }
    set cmval($id,3) {}

    for {set j 1} {$j<=20} {incr j} {
	set cmval($id,4,$j) {}
	for {set i 0} {$i<=20} {incr i} {
	    set cmval($id,4,$j,0,$i) {}
	    set cmval($id,4,$j,1,$i) {}
	}
    }
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,5,$i) {}
    }
    for {set j 0} {$j<4} {incr j} {
	set cmval($id,6,$j) {}
    }
    for {set j 0} {$j<4} {incr j} {
	set cmval($id,7,$j) {}
    }
    set cmval($id,8) {}
    for {set j 0} {$j<4} {incr j} {
	set cmval($id,9,$j) {}
    }
    set cmval($id,10) {}
}

proc read_BLOCK { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read BLOCK $cm_ident($id).  The inputs don't begin where\
		they should.  There is either an error in the CM preceding\
		this, or the accelerator module file is not the correct one\
		for use with this input file." error 0 OK
	return -code break
	# breaks out of the loops this procedure was called from.
    }

    # read rmax 0
    gets $fileid data
    set data [get_val $data cmval $id,0]

    # read title 1
    gets $fileid data
    set cmval($id,1) $data

    # read zmin, zmax, zfocus 2
    gets $fileid data
    for {set i 0} {$i<3} {incr i} {
	set data [get_val $data cmval $id,2,$i]
    }

    # read isub_max 3
    gets $fileid data
    set data [get_val $data cmval $id,3]

    for {set j 1} {$j<=$cmval($id,3)} {incr j} {
	gets $fileid data
	set data [get_val $data cmval $id,4,$j]
	for {set i 1} {$i<=$cmval($id,4,$j)} {incr i} {
	    # read xhi, yhi
	    gets $fileid data
	    set data [get_val $data cmval $id,4,$j,0,$i]
	    set data [get_val $data cmval $id,4,$j,1,$i]
	}
    }

    # read xpmax, ypmax, etc
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,5,$i]
    }

    # read ecut etc for air in gap
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,6,$i]
    }

    # read ecut etc for openings and beyond
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,7,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,8]

    # read ecut etc for block material
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,9,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,10]
}

proc edit_BLOCK { id zmax } {
    global cmval GUI_DIR helvfont help_block_text

    catch { destroy .block$id }
    toplevel .block$id
    wm title .block$id "Edit BLOCK, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NBLOCK.
    frame .block$id.frm -bd 4
    set top .block$id.frm

    label $top.mainlabel -text "Block" -font $helvfont
    pack $top.mainlabel -pady 10

    #BLOCK_macros.mortran:REPLACE {$MAX_SUB_$BLOCK} WITH {20}
    #BLOCK_macros.mortran:REPLACE {$MAX_POINT_$BLOCK} WITH {50}
    get_2_defaults BLOCK $top MAX_SUB "maximum number of subregions"\
	    MAX_POINT "maximum number of points"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    frame .block$id.frm.left
    set top .block$id.frm.left

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp2 $id,2,0

    frame $top.inp3 -bd 4
    button $top.inp3.help -bitmap @$GUI_DIR/help_icon.xbm \
	    -command "help_zback $top"
    label $top.inp3.label -text {Distance of back of CM to reference plane (cm)}
    entry $top.inp3.inp -textvariable cmval($id,2,1) -width 8
    pack $top.inp3.help -anchor w -side left
    pack $top.inp3.label -padx 5 -side left
    pack $top.inp3.inp -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label -text {Z coordinate of focal point,\
	    from reference plane (cm)}
    entry $top.inp4.inp -textvariable cmval($id,2,2) -width 8
    pack $top.inp4.label  -anchor w -side left
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    frame $top.inp5 -bd 4
    label $top.inp5.label -text {Larger X coordinate of outer edge (cm)}
    entry $top.inp5.inp -textvariable cmval($id,5,0) -width 8
    pack $top.inp5.label -anchor w -side left
    pack $top.inp5.inp -anchor e -side right -fill x -expand true
    pack $top.inp5 -side top -fill x

    frame $top.inp6 -bd 4
    label $top.inp6.label -text {Larger Y coordinate of outer block edge (cm)}
    entry $top.inp6.inp -textvariable cmval($id,5,1) -width 8
    pack $top.inp6.label -anchor w -side left
    pack $top.inp6.inp -anchor e -side right -fill x -expand true
    pack $top.inp6 -side top -fill x

    frame $top.inp7 -bd 4
    label $top.inp7.label -text {Smaller X coordinate of outer block edge (cm)}
    entry $top.inp7.inp -textvariable cmval($id,5,2) -width 8
    pack $top.inp7.label -anchor w -side left
    pack $top.inp7.inp -anchor e -side right -fill x -expand true
    pack $top.inp7 -side top -fill x

    frame $top.inp8 -bd 4
    label $top.inp8.label -text {Smaller Y coordinate of outer block edge (cm)}
    entry $top.inp8.inp -textvariable cmval($id,5,3) -width 8
    pack $top.inp8.label -anchor w -side left
    pack $top.inp8.inp -anchor e -side right -fill x -expand true
    pack $top.inp8 -side top -fill x

    frame $top.inp9 -bd 4
    label $top.inp9.label -text {Number of subregions used to define opening(s)}
    entry $top.inp9.inp -textvariable cmval($id,3) -width 8
    button $top.inp9.nextproc -text " Define geometry >> "\
	    -command "define_block $id"
    pack $top.inp9.label -anchor w -side left
    pack $top.inp9.inp -side left -padx 5
    pack $top.inp9.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp9 -side top -fill x

    pack $top -side left -padx 5

    set top .block$id.frm.right
    frame $top

    label $top.lab1 -text "Air in gap at top" -font $helvfont
    pack $top.lab1 -side top -pady 5
    add_ecut $top.f0 $id,6,0
    add_pcut $top.f1 $id,6,1
    add_dose $top.f2 $id,6,2
    add_latch $top.f3 $id,6,3

    label $top.lab2 -text "Openings and beyond edges of block" \
	    -font $helvfont
    pack $top.lab2 -side top -pady 5
    add_ecut $top.f4 $id,7,0
    add_pcut $top.f5 $id,7,1
    add_dose $top.f6 $id,7,2
    add_latch $top.f7 $id,7,3
    add_material $top.f8 $id,8

    label $top.lab3 -text "Block material" -font $helvfont
    pack $top.lab3 -side top -pady 5
    add_ecut $top.f9 $id,9,0
    add_pcut $top.f10 $id,9,1
    add_dose $top.f11 $id,9,2
    add_latch $top.f12 $id,9,3
    add_material $top.f13 $id,10

    pack $top -side right -padx 5
    pack .block$id.frm -pady 5

    frame .block$id.buts -bd 4
    button .block$id.buts.okb -text "OK" -command "destroy .block$id" \
	    -relief groove -bd 8
    button .block$id.buts.helpb -text "Help" -command \
	    "help_gif .block$id.help {$help_block_text} help_block"\
	    -relief groove -bd 8
    button .block$id.buts.prev -text "Preview" -command "show_BLOCK $id"\
	    -relief groove -bd 8
    pack .block$id.buts.helpb .block$id.buts.okb .block$id.buts.prev \
	    -side left -padx 10
    pack .block$id.buts -pady 10
}

proc define_block { id } {
    global cmval helvfont help_block_text

    catch { destroy .block$id.child }
    toplevel .block$id.child
    set top .block$id.child
    wm title $top "Define subregions"

    frame $top.f -bd 10
    set w $top.f

    for {set j 1} {$j<=$cmval($id,3)} {incr j} {
	label $w.l$j -text "Number of points defining subregion $j"
	entry $w.e$j -textvariable cmval($id,4,$j)
	button $w.b$j -text "Define points >>" -command "define_points $j $id"
	grid configure $w.l$j -row [expr $j-1] -column 0 -sticky w
	grid configure $w.e$j -row [expr $j-1] -column 1
	grid configure $w.b$j -row [expr $j-1] -column 2
    }

    pack $w
    frame $top.b
    button $top.b.okb -text "OK" -command "destroy .block$id.child"\
	    -relief groove -bd 8
    button $top.b.helpb -text "Help" -command \
	    "help_gif .block$id.child.help {$help_block_text} help_block"\
	    -relief groove -bd 8
    pack $top.b.helpb $top.b.okb -side left -padx 10
    pack $top.b -pady 10
}

proc define_points { j id } {
    global cmval helvfont

    catch { destroy .block$id.child.w$j }
    toplevel .block$id.child.w$j
    set top .block$id.child.w$j
    wm title $top "Define subregion $j"

    message $top.mess -text "Note that all internal angles must be less\
	    that 180 degrees." -width 350 -font $helvfont
    pack $top.mess -pady 5

    frame $top.f -bd 10
    set w $top.f

    label $w.label0 -text "Point"
    label $w.label1 -text "x"
    label $w.label2 -text "y"
    grid configure $w.label0 -row 0 -column 0
    grid configure $w.label1 -row 0 -column 1
    grid configure $w.label2 -row 0 -column 2
    for {set i 1} {$i<=$cmval($id,4,$j)} {incr i} {
	label $w.l$i -text $i
	entry $w.e0$i -textvariable cmval($id,4,$j,0,$i)
	entry $w.e1$i -textvariable cmval($id,4,$j,1,$i)
	grid configure $w.l$i -row $i -column 0 -sticky w
	grid configure $w.e0$i -row $i -column 1
	grid configure $w.e1$i -row $i -column 2
    }

    pack $w
    frame $top.b
    button $top.b.okb -text "OK" -command "check_block_angles $id $j; \
	    destroy .block$id.child.w$j" -relief groove -bd 8
    pack $top.b.okb
    pack $top.b -pady 10
}

proc check_block_angles { id subreg } {
    global cmval

    # determine the direction of movement of the first 3 points:
    # cross returns 0 for left and 1 for right

    set x1 $cmval($id,4,$subreg,0,1)
    set x2 $cmval($id,4,$subreg,0,2)
    set x3 $cmval($id,4,$subreg,0,3)
    set y1 $cmval($id,4,$subreg,1,1)
    set y2 $cmval($id,4,$subreg,1,2)
    set y3 $cmval($id,4,$subreg,1,3)
    set dir [cross $x1 $y1 $x2 $y2 $x3 $y3]

    # for each new set of 3 points, see if direction changes.
    for {set i 2} {$i<=$cmval($id,4,$subreg)} {incr i} {
	set idx2 [expr $i+1]
	set idx3 [expr $i+2]
	if $i==[expr $cmval($id,4,$subreg)-1] {
	    set idx3 1
	}
	if $i==$cmval($id,4,$subreg) {
	    set idx2 1
	    set idx3 2
	}
	set x1 $cmval($id,4,$subreg,0,$i)
	set x2 $cmval($id,4,$subreg,0,$idx2)
	set x3 $cmval($id,4,$subreg,0,$idx3)
	set y1 $cmval($id,4,$subreg,1,$i)
	set y2 $cmval($id,4,$subreg,1,$idx2)
	set y3 $cmval($id,4,$subreg,1,$idx3)

	set newdir [cross $x1 $y1 $x2 $y2 $x3 $y3]

	if $newdir!=$dir {
	    tk_dialog .angles "Convex Angles" "Warning! This polygon \
		    has an internal angle greater than 180 degrees!  \
		    BEAM will not accept this input.  Try using more\
		    than one polygon to define the shape." \
		    warning 0 OK
	    break
	}

    }
}

proc cross { x1 y1 x2 y2 x3 y3 } {
    set cp [expr ($x2-$x1)*($y3-$y2)-($y2-$y1)*($x3-$x2)]
    if $cp<0 {
	return 0
    } else {
	return 1
    }
}

proc write_BLOCK {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2,0), $cmval($id,2,1), $cmval($id,2,2), ZMIN, ZMAX, ZFOCUS"
    puts $fileid "$cmval($id,3), # OF SUBREGIONS"

    for {set j 1} {$j<=$cmval($id,3)} {incr j} {
	puts $fileid "$cmval($id,4,$j), NUMBER OF POINTS IN SUBREGION $j"
	set str {}
	for {set i 1} {$i<=$cmval($id,4,$j)} {incr i} {
	    puts $fileid "$cmval($id,4,$j,0,$i), $cmval($id,4,$j,1,$i), "
	}
    }

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,5,$i), "
    }
    puts $fileid $str

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,6,$i), "
    }
    puts $fileid $str

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,7,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,8)

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,9,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,10)

}


proc show_BLOCK { id } {
    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks

    catch { destroy .block$id.show }
    toplevel .block$id.show
    wm title .block$id.show "Preview"


    # have to make an initial xrange,yrange (for top view):
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set yrange(0) -$cmval($id,0)
	set yrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    set cm_ticks($id,x) 6
    set cm_ticks($id,y) 6
    set cm_ticks($id,z) 10

    set topw .block$id.show
    frame $topw.toggle
    set toggle$id 0
    radiobutton $topw.toggle.off -text "Lines off" -variable toggle$id \
	    -value 0 -command "set toggle$id 0; $topw.can.two itemconfigure \
	    block_tag -outline {}"
    radiobutton $topw.toggle.on -text "Lines on" -variable toggle$id \
	    -value 1 -command "set toggle$id 1; $topw.can.two itemconfigure \
	    block_tag -outline black"
    pack $topw.toggle.off $topw.toggle.on -anchor w
    pack $topw.toggle -anchor w
    $topw.toggle.on deselect
    $topw.toggle.off select

    if [catch { draw_BLOCK $id }]==1 {
	destroy .block$id.show
	tk_dialog .block$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .block$id.show.buts
    button .block$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .block$id.show xyz" -relief groove -bd 8
    button .block$id.show.buts.print1 -text "Print xz..." -command\
	    "print_canvas .block$id.show.can.one 500 500" -relief groove -bd 8
    button .block$id.show.buts.print2 -text "Print xy..." -command\
	    "print_canvas .block$id.show.can.two 500 500" -relief groove -bd 8
    button .block$id.show.buts.done -text "Done" -command\
	    "destroy .block$id.show" -relief groove -bd 8
    pack .block$id.show.buts.range .block$id.show.buts.print1\
	    .block$id.show.buts.print2 \
	    .block$id.show.buts.done -side left -padx 10
    pack .block$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_BLOCK { id } {
    global helvfont cmval zrange yrange xrange med colorlist
    global xscale yscale zscale l m cm_ticks

    catch { destroy .block$id.show.can }

    # put the canvas up
    set ncan 2
    set width 350.0
    set canwidth 500.0
    canvas .block$id.show.can -width [expr $ncan*($width+150.0)]\
	    -height [expr ($width+150.0)]

    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    # left canvas, x/z cross-section
    set l 100.0
    set m 50.0

    canvas .block$id.show.can.one -width $canwidth -height $canwidth

    set curx 0
    set cury 0
    label .block$id.show.can.one.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .block$id.show.can.one create window [expr $l/2+$width] [expr 2*$m+$width]\
	    -window .block$id.show.can.one.xy

    add_air $id .block$id.show.can.one $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_BLOCK $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .block$id.show.can.one

    coverup $l $m $width .block$id.show.can.one

    bind .block$id.show.can.one <Motion> {
	global curx cury l m xscale zscale xrange zrange
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
	    $xscale $zscale .block$id.show.can.one

    .block$id.show.can create window [expr $canwidth/2.0]\
	    [expr $canwidth/2.0] -window .block$id.show.can.one

    # right canvas, XY cross-section

    canvas .block$id.show.can.two -width $canwidth -height $canwidth

    label .block$id.show.can.two.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .block$id.show.can.two create window [expr $l/2+$width] [expr 2*$m+$width]\
	    -window .block$id.show.can.two.xy

    add_BLOCK_xy $id $xscale $yscale $xrange(0) $yrange(0)\
	    $l $m .block$id.show.can.two

    coverup $l $m $width .block$id.show.can.two

    bind .block$id.show.can.two <Motion> {
	global xrange yrange xscale yscale l m
	set curx %x
	set cury %y
	set curx [expr ($curx-$l)/$xscale+$xrange(0)]
	set cury [expr ($cury-$m)/$yscale+$yrange(0)]
        if { $curx<=$xrange(1) & $curx>=$xrange(0) & $cury<=$yrange(1) & \
                $cury>=$yrange(0) } {
	    set text [format "(%%6.3f, %%6.3f)" $curx $cury]
	    %W.xy configure -text $text
	}
    }

    add_axes xy $cm_ticks($id,x) $cm_ticks($id,y) $l $m $width $xrange(0)\
	    $xrange(1) $yrange(0) $yrange(1)\
	    $xscale $yscale .block$id.show.can.two

    .block$id.show.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .block$id.show.can.two

    pack .block$id.show.can -side top -anchor n
}

proc add_BLOCK {id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist medium nmed meds_used helvfont colornum

    # assign numbers to the media
    # (there are only 2 here, in openings and in block)
    set med(openings) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,8) $medium($j)]==0 {
	    set med(openings) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(block) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,10) $medium($j)]==0 {
	    set med(block) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }

    # first a rectangle of the outside medium
    set x1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z1 [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set x2 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z2 [expr ($cmval($id,2,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(openings)]
    $parent_w create rectangle $x1 $z1 $x2 $z2 -fill $color -outline {}

    # put in the block
    set x1 [expr ($cmval($id,5,0)-$xmin)*$xscale+$l]
    set z1 [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set x2 [expr ($cmval($id,5,2)-$xmin)*$xscale+$l]
    set z2 [expr ($cmval($id,2,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(block)]
    $parent_w create rectangle $x1 $z1 $x2 $z2 -fill $color -outline {}

    # for each subregion, see if the opening crosses the y=0 line.
    # if it does, get the x-points where the cross, xtop
    set color [lindex $colorlist $med(openings)]
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set counter 0
	for {set j 1} {$j<$cmval($id,4,$i)} {incr j} {
	    # compare y(j) to y(j+1)
	    set yj [expr double($cmval($id,4,$i,1,$j))]
	    set yjp [expr double($cmval($id,4,$i,1,[expr $j+1]))]
	    if $yj==0 {
		incr counter
		set xtop($counter) [expr double($cmval($id,4,$i,0,$j))]
	    } elseif {$yj<0 & $yjp>0 || $yj>0 & $yjp<0} {
		incr counter
		set xj [expr double($cmval($id,4,$i,0,$j))]
		set xjp [expr double($cmval($id,4,$i,0,[expr $j+1]))]
		set xtop($counter) [expr $xj-$yj*(($xj-$xjp)/($yj-$yjp))]
	    }
	}
	# same, connect y(1) to y(npts) and check for intersection with y=0
	set yj [expr double($cmval($id,4,$i,1,1))]
	set yjp [expr double($cmval($id,4,$i,1,$cmval($id,4,$i)))]
	if $yjp==0 {
	    incr counter
	    set xtop($counter) [expr double($cmval($id,4,$i,0,$cmval($id,4,$i)))]
	} elseif {$yj<0 & $yjp>0 || $yj>0 & $yjp<0} {
	    incr counter
	    set xj [expr double($cmval($id,4,$i,0,1))]
	    set xjp [expr double($cmval($id,4,$i,0,$cmval($id,4,$i)))]
	    set xtop($counter) [expr $xj-$yj*(($xj-$xjp)/($yj-$yjp))]
	}
	set z1 [expr double($cmval($id,2,0))]
	set z2 [expr double($cmval($id,2,1))]
	set zf [expr double($cmval($id,2,2))]
	for {set j 1} {$j<=$counter} {incr j} {
	    # calculate xbot
	    set xbot($j) [expr $xtop($j)*(($z2-$zf)/($z1-$zf))]
	}
	# There should only be one polygon per opening
	if $counter>0 {
	    set x1 [expr ($xtop(1)-$xmin)*$xscale+$l]
	    set x2 [expr ($xtop(2)-$xmin)*$xscale+$l]
	    set x3 [expr ($xbot(1)-$xmin)*$xscale+$l]
	    set x4 [expr ($xbot(2)-$xmin)*$xscale+$l]
	    set z1 [expr ($z1-$zmin)*$zscale+$m]
	    set z2 [expr ($z2-$zmin)*$zscale+$m]
	    $parent_w create poly $x1 $z1 $x2 $z1 $x4 $z2 $x3 $z2 -fill $color\
		    -outline {}
	}
    }
}

proc add_BLOCK_xy {id xscale yscale xmin ymin l m parent_w} {
    global cmval colorlist medium nmed colornum

    # assign numbers to the media
    # (there are only 2 here, in openings and in block)
    set med(openings) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,8) $medium($j)]==0 {
	    set med(openings) [min_nrc $j $colornum]
	    break
	}
    }
    set med(block) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,10) $medium($j)]==0 {
	    set med(block) [min_nrc $j $colornum]
	    break
	}
    }

    # first a rectangle of the block medium for the outside
    set x1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set y1 [expr ($cmval($id,0)-$ymin)*$yscale+$m]
    set x2 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set y2 [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
    set color [lindex $colorlist $med(openings)]
    $parent_w create rectangle $x1 $y1 $x2 $y2 -fill $color -outline {}

    # put in the block
    set x1 [expr ($cmval($id,5,0)-$xmin)*$xscale+$l]
    set y1 [expr ($cmval($id,5,1)-$ymin)*$yscale+$m]
    set x2 [expr ($cmval($id,5,2)-$xmin)*$xscale+$l]
    set y2 [expr ($cmval($id,5,3)-$ymin)*$yscale+$m]
    set color [lindex $colorlist $med(block)]
    $parent_w create rectangle $x1 $y1 $x2 $y2 -fill $color -outline {} -tag block_tag

    # for each subregion, add a polygon
    set color [lindex $colorlist $med(openings)]
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set points {}
	for {set j 1} {$j<=$cmval($id,4,$i)} {incr j} {
	    set x($j) [expr ($cmval($id,4,$i,0,$j)-$xmin)*$xscale+$l]
	    set y($j) [expr ($cmval($id,4,$i,1,$j)-$ymin)*$yscale+$m]
	    set points "$points $x($j) $y($j)"
	}
	# eval used to make sure $points is fully substituted
	eval "$parent_w create poly $points -fill $color -outline {} -tag block_tag"
    }

}

proc add_BLOCK_yz {id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist medium nmed meds_used helvfont colornum

    # This is the same as add_BLOCK, but reversing x's and y's.
    # (4,$j,0,$i)<->(4,$j,1,$i)
    # (5,0) <-> (5,1)   (5,2) <-> (5,3)

    # assign numbers to the media
    # (there are only 2 here, in openings and in block)
    set med(openings) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,8) $medium($j)]==0 {
	    set med(openings) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(block) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,10) $medium($j)]==0 {
	    set med(block) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }

    # first a rectangle of the outside medium
    set x1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z1 [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set x2 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z2 [expr ($cmval($id,2,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(openings)]
    $parent_w create rectangle $x1 $z1 $x2 $z2 -fill $color -outline {}

    # put in the block
    set x1 [expr ($cmval($id,5,1)-$xmin)*$xscale+$l]
    set z1 [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set x2 [expr ($cmval($id,5,3)-$xmin)*$xscale+$l]
    set z2 [expr ($cmval($id,2,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(block)]
    $parent_w create rectangle $x1 $z1 $x2 $z2 -fill $color -outline {}

    # for each subregion, see if the opening crosses the y=0 line.
    # if it does, get the x-points where the cross, xtop
    set color [lindex $colorlist $med(openings)]
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set counter 0
	for {set j 1} {$j<$cmval($id,4,$i)} {incr j} {
	    # compare y(j) to y(j+1)
	    set yj [expr double($cmval($id,4,$i,0,$j))]
	    set yjp [expr double($cmval($id,4,$i,0,[expr $j+1]))]
	    if $yj==0 {
		incr counter
		set xtop($counter) [expr double($cmval($id,4,$i,1,$j))]
	    } elseif {$yj<0 & $yjp>0 || $yj>0 & $yjp<0} {
		incr counter
		set xj [expr double($cmval($id,4,$i,1,$j))]
		set xjp [expr double($cmval($id,4,$i,1,[expr $j+1]))]
		set xtop($counter) [expr $xj-$yj*(($xj-$xjp)/($yj-$yjp))]
	    }
	}
	# same, connect y(1) to y(npts) and check for intersection with y=0
	set yj [expr double($cmval($id,4,$i,0,1))]
	set yjp [expr double($cmval($id,4,$i,0,$cmval($id,4,$i)))]
	if $yjp==0 {
	    incr counter
	    set xtop($counter) [expr double($cmval($id,4,$i,1,$cmval($id,4,$i)))]
	} elseif {$yj<0 & $yjp>0 || $yj>0 & $yjp<0} {
	    incr counter
	    set xj [expr double($cmval($id,4,$i,1,1))]
	    set xjp [expr double($cmval($id,4,$i,1,$cmval($id,4,$i)))]
	    set xtop($counter) [expr $xj-$yj*(($xj-$xjp)/($yj-$yjp))]
	}
	set z1 [expr double($cmval($id,2,0))]
	set z2 [expr double($cmval($id,2,1))]
	set zf [expr double($cmval($id,2,2))]
	for {set j 1} {$j<=$counter} {incr j} {
	    # calculate xbot
	    set xbot($j) [expr $xtop($j)*(($z2-$zf)/($z1-$zf))]
	}
	# There should only be one polygon per opening
	if $counter>0 {
	    set x1 [expr ($xtop(1)-$xmin)*$xscale+$l]
	    set x2 [expr ($xtop(2)-$xmin)*$xscale+$l]
	    set x3 [expr ($xbot(1)-$xmin)*$xscale+$l]
	    set x4 [expr ($xbot(2)-$xmin)*$xscale+$l]
	    set z1 [expr ($z1-$zmin)*$zscale+$m]
	    set z2 [expr ($z2-$zmin)*$zscale+$m]
	    $parent_w create poly $x1 $z1 $x2 $z1 $x4 $z2 $x3 $z2 -fill $color\
		    -outline {}
	}
    }
}

proc print_canvas { w canwidth canheight } {
    global helvfont file prcolour orient fname prname

    catch { destroy .print }
    toplevel .print
    wm title .print "Print options"
    label .print.title -text "Print Options" -font $helvfont
    pack .print.title -pady 5

    message .print.mess -text "If you are printing the entire accelerator, \
	    you may want to scale the canvas to a smaller size before\
	    printing/exporting, so that the fonts are readable in the\
	    postscript output." -font $helvfont -width 400
    pack .print.mess -pady 5

    frame .print.frm

    frame .print.frm.colour -relief groove -bd 2
    radiobutton .print.frm.colour.r1 -text "Colour" -variable prcolour\
	    -value color
    radiobutton .print.frm.colour.r2 -text "Greyscale" -variable prcolour\
	    -value gray
    pack .print.frm.colour.r1 .print.frm.colour.r2  -anchor w

    frame .print.frm.orient -relief groove -bd 2
    radiobutton .print.frm.orient.r0 -text "Portrait" -variable orient -value 0
    radiobutton .print.frm.orient.r1 -text "Landscape" -variable orient -value 1
    pack .print.frm.orient.r0 .print.frm.orient.r1 -anchor w

    pack .print.frm.colour .print.frm.orient -side left -padx 10 -pady 5
    pack .print.frm

    frame .print.f2 -relief groove -bd 2
    radiobutton .print.f2.file0 -text "To printer:" -variable file -value 0
    radiobutton .print.f2.file1 -text "To file:" -variable file -value 1
    entry .print.f2.prname -textvariable prname -width 15
    entry .print.f2.fname -textvariable fname -width 15
    grid config .print.f2.file0 -row 0 -column 0 -sticky w
    grid config .print.f2.file1 -row 1 -column 0 -sticky w
    grid config .print.f2.prname -row 0 -column 1 -sticky w
    grid config .print.f2.fname -row 1 -column 1 -sticky w
    pack .print.f2 -pady 5

    frame .print.buts
    button .print.buts.pb -text "Print" -command "print_cmd $w $canwidth\
	    $canheight; destroy .print" -relief groove -bd 8
    button .print.buts.cb -text "Cancel" -command "destroy .print"\
	    -relief groove -bd 8
    pack .print.buts.pb .print.buts.cb -side left -padx 5
    pack .print.buts -pady 10

}

proc print_cmd { w canwidth canheight } {
    global env file prcolour orient fname prname run_dir

    if {$file==1} {
	set fff $run_dir/$fname
    } else {
	set fff $env(HOME)/guibeamtemppsfile.ps
    }

    if $orient==0 {
	# portrait output
	set scalew [expr $canwidth/8.0]
	set scaleh [expr $canheight/10.5]

	if $scaleh>$scalew {
	    $w postscript -colormode $prcolour -rotate $orient\
		    -file $fff -height $canheight -width $canwidth\
		    -pageheight 10.5i
	} else {
	    $w postscript -colormode $prcolour -rotate $orient\
		    -file $fff -height $canheight -width $canwidth\
		    -pagewidth 8.i
	}
    } else {
	# landscape output
	set scalew [expr $canwidth/10.5]
	set scaleh [expr $canheight/8.0]

	if $scaleh>$scalew {
	    $w postscript -colormode $prcolour -rotate $orient\
		    -file $fff -height $canheight -width $canwidth\
		    -pageheight 8.i
	} else {
	    $w postscript -colormode $prcolour -rotate $orient\
		    -file $fff -height $canheight -width $canwidth\
		    -pagewidth 10.5i
	}
    }

    if $file==0 {
	# send to printer
	if [string compare $prname {}]==0 {
	    tk_dialog .printerr "Printer unspecified" "You haven't specified\
		    a printer.  The print job has not been sent." error 0 OK
	} else {
	    exec lpr -P$prname $env(HOME)/guibeamtemppsfile.ps
	}
	exec rm $env(HOME)/guibeamtemppsfile.ps
    }
}
