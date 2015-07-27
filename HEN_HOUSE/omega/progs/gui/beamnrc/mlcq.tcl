
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: MLCQ
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


proc init_MLCQ { id } {
    global cmval ngroups

    for {set i 0} {$i<5} {incr i} {
	set cmval($id,$i) {}
    }
    set cmval($id,5,0) {}
    set cmval($id,5,1) {}
    set cmval($id,6) {}
    set cmval($id,7,0) {}
    set cmval($id,7,1) {}
    # for each leaf set: neg, pos, num
    for {set i 1} {$i<=100} {incr i} {
	set cmval($id,8,0,$i) {}
	set cmval($id,8,1,$i) {}
	set cmval($id,8,2,$i) {}
    }
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,9,$i) {}
    }
    set cmval($id,10) {}
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,11,$i) {}
    }
    set cmval($id,12) {}
    set ngroups($id) 1
}

proc read_MLCQ { fileid id } {
    global cmval GUI_DIR ngroups cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read MLCQ $cm_ident($id).  The inputs don't begin where\
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

    # read idmlfc 2
    gets $fileid data
    set data [get_val $data cmval $id,2]

    # read zmin 3
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # read zthick 4
    gets $fileid data
    set data [get_val $data cmval $id,4]

    # read numleaf, twidth 5
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
	set data [get_val $data cmval $id,5,$i]
    }

    # read zfocus(1) 6
    gets $fileid data
    set data [get_val $data cmval $id,6]

    # read zfocus(1) 7
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
	set data [get_val $data cmval $id,7,$i]
    }

    # read in the leaf coordinates, neg,pos,num.  When the sum over num
    # is > numleaf, stop reading them.
    set numsum 0
    set ngroups($id) 0
    while {$numsum<$cmval($id,5,0)} {
	incr ngroups($id)
	gets $fileid data
	for {set i 0} {$i<3} {incr i} {
	    set data [get_val $data cmval $id,8,$i,$ngroups($id)]
	}
	if { $cmval($id,8,2,$ngroups($id))<=0 } {
	    set cmval($id,8,2,$ngroups($id)) 1
	}
	incr numsum $cmval($id,8,2,$ngroups($id))
    }

    # read ecut etc inside collimator
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,9,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,10]

    # read ecut etc in collimator leaves
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,11,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,12]
}

proc edit_MLCQ { id zmax } {
    global cmval GUI_DIR helvfont cm_ident ngroups help_mlcq_text

    catch { destroy .mlcq$id }
    toplevel .mlcq$id
    wm title .mlcq$id "Edit MLCQ, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NMLCQ.
    frame .mlcq$id.top -bd 4
    set top .mlcq$id.top


    label $top.mainlabel -text "Multi-leaf collimator $cm_ident($id)" \
	    -font $helvfont
    pack $top.mainlabel -side top -padx 10

    # MLCQ_macros.mortran:REPLACE {$MAXLEAF} WITH {64}
    # read the macro file for CM MLCQ to get defaults and put a label on the
    # main edit window.  Has to be different from the others because there's
    # no MLCQ in the default name.

    global omega helvfont
    set filename $omega/beamnrc/CMs/MLCQ_macros.mortran
    set default1 {}
    if { [file readable $filename] } {
	set fileid [open $filename "r"]
	set i 0
	while {$i<100 & [string compare $default1 {}]==0} {
	    incr i
	    gets $fileid data
	    set data [string trimright $data]
	    if [string first "REPLACE" $data]>=0 {
		if [string first MAXLEAF $data]>0 {
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

    label $top.default1 -text "The default maximum number\
	    of leaves is $default1." -font $helvfont
    pack $top.default1 -side top

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp3 $id,3

    # leaf direction radiobuttons
    frame $top.inp2 -bd 10
    radiobutton $top.inp2.r1 -text "Leaves parallel to y"\
            -variable cmval($id,2) -value 0
    radiobutton $top.inp2.r2 -text "Leaves parallel to x"\
            -variable cmval($id,2) -value 1
    pack $top.inp2.r1 $top.inp2.r2 -side top -pady 5
    pack $top.inp2 -side top -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label -text {Thickness of the leaves (cm)}
    entry $top.inp4.inp -textvariable cmval($id,4) -width 10
    pack $top.inp4.label -anchor w -side left
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    frame $top.inp5 -bd 4
    label $top.inp5.label -text {Number of leaves}
    entry $top.inp5.inp -textvariable cmval($id,5,0) -width 10
    button $top.inp5.b -text "Define leaf sizes >>" -command \
	    "define_mlcq_leaves $id"
    pack $top.inp5.label -anchor w -side left
    pack $top.inp5.inp -side left -fill x -expand true
    pack $top.inp5.b -anchor e -side right -padx 5
    pack $top.inp5 -side top -fill x

    frame $top.inp6 -bd 4
    label $top.inp6.label -text {Total width of leaves (cm)}
    entry $top.inp6.inp -textvariable cmval($id,5,1) -width 10
    pack $top.inp6.label -anchor w -side left
    pack $top.inp6.inp -anchor e -side right -fill x -expand true
    pack $top.inp6 -side top -fill x

    frame $top.inp7 -bd 4
    label $top.inp7.label -text {Focal point on Z-axis of leaf sides}
    entry $top.inp7.inp -textvariable cmval($id,6) -width 10
    pack $top.inp7.label -anchor w -side left
    pack $top.inp7.inp -anchor e -side right -fill x -expand true
    pack $top.inp7 -side top -fill x

    frame $top.inp8 -bd 4
    label $top.inp8.label -text {Radius of circle defining leaf ends}
    entry $top.inp8.inp -textvariable cmval($id,7,0) -width 10
    pack $top.inp8.label -anchor w -side left
    pack $top.inp8.inp -anchor e -side right -fill x -expand true
    pack $top.inp8 -side top -fill x

    frame $top.inp9 -bd 4
    label $top.inp9.label -text {Z of origin of circle defining leaf ends}
    entry $top.inp9.inp -textvariable cmval($id,7,1) -width 10
    pack $top.inp9.label -anchor w -side left
    pack $top.inp9.inp -anchor e -side right -fill x -expand true
    pack $top.inp9 -side top -fill x

    pack $top -side top -pady 5

    frame .mlcq$id.bottom
    frame .mlcq$id.bottom.left
    frame .mlcq$id.bottom.right

    label .mlcq$id.bottom.left.lab -text "Inside collimator" -font $helvfont
    pack .mlcq$id.bottom.left.lab -side top -pady 5
    add_ecut .mlcq$id.bottom.left.f0 $id,9,0
    add_pcut .mlcq$id.bottom.left.f1 $id,9,1
    add_dose .mlcq$id.bottom.left.f2 $id,9,2
    add_latch .mlcq$id.bottom.left.f3 $id,9,3
    add_material .mlcq$id.bottom.left.f4 $id,10

    label .mlcq$id.bottom.right.lab -text "Collimator leaves" -font $helvfont
    pack .mlcq$id.bottom.right.lab -side top -pady 5
    add_ecut .mlcq$id.bottom.right.f0 $id,11,0
    add_pcut .mlcq$id.bottom.right.f1 $id,11,1
    add_dose .mlcq$id.bottom.right.f2 $id,11,2
    add_latch .mlcq$id.bottom.right.f3 $id,11,3
    add_material .mlcq$id.bottom.right.f4 $id,12

    pack .mlcq$id.bottom.left -side left -anchor n -padx 10
    pack .mlcq$id.bottom.right -side right -anchor n -padx 10

    pack .mlcq$id.bottom -side top -pady 5

    frame .mlcq$id.buts -bd 4
    button .mlcq$id.buts.okb -text "OK" -command "destroy .mlcq$id"\
	    -relief groove -bd 8
    button .mlcq$id.buts.helpb -text "Help" -command\
	    "help_gif .mlcq$id.help {$help_mlcq_text} help_mlcq"\
	    -relief groove -bd 8
    button .mlcq$id.buts.prev -text "Preview" -command\
	    "show_MLCQ $id" -relief groove -bd 8
    pack .mlcq$id.buts.helpb .mlcq$id.buts.okb .mlcq$id.buts.prev -side left -padx 10
    pack .mlcq$id.buts -pady 10
}

proc define_mlcq_leaves { id } {
    global cmval helvfont ngroups to from help_mlcq_text

    catch { destroy .mlcq$id.child }
    toplevel .mlcq$id.child
    set top .mlcq$id.child
    wm title $top "Define Leaves"

    frame $top.f -bd 10
    set w $top.f

    if $cmval($id,2)==0 {
	set orientation Y
    } else {
	set orientation X
    }

    if [string compare $ngroups($id) ""]==0 {
	set ngroups($id) 1
    }

    message $w.message -text "Specify the minimum and maximum $orientation\
	    value (at the top of the CM) \
	    for each leaf in the MLCQ by groups here.  Add a row to start a\
	    new group.  When a new row is added the next leaf in the order will\
	    automatically appear in the 'from leaf' box.  This must repeat\
	    until you have defined the minimum and maximum for all\
	    $cmval($id,5,0) leaves (until the last 'to leaf' value is\
	    $cmval($id,5,0))." -width 400 -font $helvfont
    pack $w.message -side top -anchor n -fill x -expand true

    frame $w.grid -bd 4
    label $w.grid.l0 -text "from leaf"
    label $w.grid.l1 -text "to leaf"
    label $w.grid.l2 -text "$orientation of leaf origin, - side"
    label $w.grid.l3 -text "$orientation of leaf origin, + side"
    grid configure $w.grid.l0 -row 0 -column 0
    grid configure $w.grid.l1 -row 0 -column 1
    grid configure $w.grid.l2 -row 0 -column 2
    grid configure $w.grid.l3 -row 0 -column 3
    set to(0) 0
    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	set from($j) [expr $to([expr $j-1])+1]
	if [catch {set to($j) [expr $to([expr $j-1])+$cmval($id,8,2,$j)]}]==1 {
	    set to($j) {}
	}
	entry $w.grid.e0$j -textvariable from($j) -width 8
	grid configure $w.grid.e0$j -row $j -column 0
	entry $w.grid.e1$j -textvariable to($j) -width 8
	grid configure $w.grid.e1$j -row $j -column 1
	entry $w.grid.e2$j -textvariable cmval($id,8,0,$j) -width 15
	grid configure $w.grid.e2$j -row $j -column 2
	entry $w.grid.e3$j -textvariable cmval($id,8,1,$j) -width 15
	grid configure $w.grid.e3$j -row $j -column 3
    }
    pack $w.grid -side top

    pack $w
    frame $top.b
    button $top.b.addb -text "Add a row" -command "add_mlcq_row $id"\
	    -relief groove -bd 8
    button $top.b.okb -text "OK" -command "destroy .mlcq$id.child; save_mlcq $id"\
	    -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
	    "help_gif .mlcq$id.child.help {$help_mlcq_text} help_mlcq"\
	    -relief groove -bd 8
    pack $top.b.addb $top.b.okb $top.b.helpb -side left -padx 10
    pack $top.b -pady 10
}

proc save_mlcq { id } {
    global cmval ngroups from to
    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	if [catch {set cmval($id,8,2,$j) [expr $to($j)-$from($j)+1]}]==1 {
	    tk_dialog .nope "No no no" "The leaf geometries for group $j\
		    have been improperly set or not set at all.\
		    Go back and fix them." warning 0 OK
	}
    }
}

proc add_mlcq_row { id } {
    global ngroups from to cmval

    set w .mlcq$id.child.f

    if $to($ngroups($id))==$cmval($id,5,0) {
	# They've been finished already.  Refuse.
	tk_dialog .refuse "Too many" "You've already completed the leaves.\
		I can't add another row unless you change the last 'to'\
		box to a values less than $cmval($id,5,0) or increase the\
		number of leaves." warning 0 OK
	return
    }

    set j [incr ngroups($id)]
    set from($j) [expr $to([expr $j-1])+1]
    if [catch {set to($j) [expr $to([expr $j-1])+$cmval($id,8,2,$j)]}]==1 {
	set to($j) {}
    }
    entry $w.grid.e0$j -textvariable from($j) -width 8
    grid configure $w.grid.e0$j -row $j -column 0
    entry $w.grid.e1$j -textvariable to($j) -width 8
    grid configure $w.grid.e1$j -row $j -column 1
    entry $w.grid.e2$j -textvariable cmval($id,8,0,$j) -width 8
    grid configure $w.grid.e2$j -row $j -column 2
    entry $w.grid.e3$j -textvariable cmval($id,8,1,$j) -width 8
    grid configure $w.grid.e3$j -row $j -column 3

}

proc write_MLCQ {fileid id} {
    global cmval cm_names cm_ident cm_type ngroups

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), IDMLFC"
    puts $fileid "$cmval($id,3), ZMIN"
    puts $fileid "$cmval($id,4), ZTHICK"
    puts $fileid "$cmval($id,5,0), $cmval($id,5,1), # LEAVES, TOTAL WIDTH"
    puts $fileid "$cmval($id,6), ZFOCUS(1)"
    puts $fileid "$cmval($id,7,0), $cmval($id,7,1), R0LEAF, Z0LEAF"

    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	puts $fileid "$cmval($id,8,0,$j), $cmval($id,8,1,$j), $cmval($id,8,2,$j)"
    }

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,9,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,10)

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,11,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,12)

}


proc show_MLCQ { id } {
    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks

    catch { destroy .mlcq$id.show }
    toplevel .mlcq$id.show
    wm title .mlcq$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,y) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange, yrange, zrange:
    set xrange(0) -$cmval($id,0)
    set xrange(1) $cmval($id,0)
    set yrange(0) -$cmval($id,0)
    set yrange(1) $cmval($id,0)
    set zrange(0) [get_zmax [expr $id-1]]
    set zrange(1) [get_zmax $id]

    # need a frame in case scrollbars are needed.
    frame .mlcq$id.show.frm

    draw_MLCQ $id

    frame .mlcq$id.show.buts
    button .mlcq$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .mlcq$id.show xyz" -relief groove -bd 8
    button .mlcq$id.show.buts.print1 -text "Print xy..." -command\
	    "print_canvas .mlcq$id.show.frm.can.one 450 450" -relief groove -bd 8
    button .mlcq$id.show.buts.print2 -text "Print xz..." -command\
	    "print_canvas .mlcq$id.show.frm.can.two 450 450" -relief groove -bd 8
    button .mlcq$id.show.buts.print3 -text "Print yz..." -command\
	    "print_canvas .mlcq$id.show.frm.can.three 450 450" -relief groove -bd 8
    button .mlcq$id.show.buts.done -text "Done" -command\
	    "destroy .mlcq$id.show" -relief groove -bd 8
    pack .mlcq$id.show.buts.range .mlcq$id.show.buts.print1\
	    .mlcq$id.show.buts.print2 .mlcq$id.show.buts.print3\
	    .mlcq$id.show.buts.done -side left -padx 10
    pack .mlcq$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_MLCQ { id } {
    global helvfont zrange yrange xrange zscale yscale xscale l m
    global cmval cm_ticks

    catch { destroy .mlcq$id.show.frm.can }

    # put the canvas up
    set ncan 3
    set width 300.0
    set canwidth [expr $width+150.0]
    set scrlheight [expr 2*$canwidth]
    set scrlwidth [expr 2*$canwidth]
    set winwidth [expr [winfo screenwidth .]*4.0/5.0]
    set winheight [expr [winfo screenheight .]*4.0/5.0]

    if $scrlwidth>$winwidth {
	catch { destroy .mlcq$id.show.frm.scrlz }
	catch { destroy .mlcq$id.show.frm.scrlx }
	scrollbar .mlcq$id.show.frm.scrlz -command\
		".mlcq$id.show.frm.can yview"
	scrollbar .mlcq$id.show.frm.scrlx -command\
		".mlcq$id.show.frm.can xview" -orient horizontal
	pack .mlcq$id.show.frm.scrlz -side right -fill y
	pack .mlcq$id.show.frm.scrlx -side bottom -fill x
	canvas .mlcq$id.show.frm.can -width $winwidth -height $winheight\
		-yscrollcommand ".mlcq$id.show.frm.scrlz set"\
		-xscrollcommand ".mlcq$id.show.frm.scrlx set"\
		-scrollregion "0 0 $scrlwidth $scrlheight"
    } else {
	canvas .mlcq$id.show.frm.can -width $scrlwidth -height $scrlheight
    }

    # Put some text in the upper left corner, just to fill the gap.
    .mlcq$id.show.frm.can create text 225 225 -text "The preview of the\
	    leaf sides\
	    shows only those leaves that pass through x (yz view) or y\
	    (xz view) equal to zero.  The preview of the leaf ends has a top\
	    width equal to the total width of the leaves, regardless of\
	    what the width is of the leaf passing through zero.  The xy view\
	    shows the plane through the maximum radius."\
	    -font $helvfont -width 300


    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    set l 100.0
    set m 50.0

    # XY, upper right

    canvas .mlcq$id.show.frm.can.one -width $canwidth -height $canwidth

    add_MLCQ_xy $id $xscale $yscale $xrange(0) $yrange(0)\
	    $l $m .mlcq$id.show.frm.can.one

    set curx 0
    set cury 0
    label .mlcq$id.show.frm.can.one.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .mlcq$id.show.frm.can.one create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .mlcq$id.show.frm.can.one.xy
    bind .mlcq$id.show.frm.can.one <Motion> {
	global l m xscale yscale xrange yrange
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
    coverup $l $m $width .mlcq$id.show.frm.can.one

    add_axes xy $cm_ticks($id,x) $cm_ticks($id,y) $l $m $width $xrange(0)\
	    $xrange(1) $yrange(0) $yrange(1)\
	    $xscale $yscale .mlcq$id.show.frm.can.one

    .mlcq$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .mlcq$id.show.frm.can.one

    # XZ, lower left: if leaves // to y, show leaf sides,
    # else if leaves // to x, show only one leaf at y=0 (leaf ends).

    canvas .mlcq$id.show.frm.can.two -width $canwidth -height $canwidth

    add_air $id .mlcq$id.show.frm.can.two $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    if $cmval($id,2)==0 {
	add_MLCQ_sides $id $xscale $zscale $xrange(0) $zrange(0) $zrange(1)\
		$l $m .mlcq$id.show.frm.can.two
    } else {
	add_MLCQ_ends $id $xscale $zscale $xrange(0) $zrange(0) $zrange(1)\
		$l $m .mlcq$id.show.frm.can.two
    }

    coverup $l $m $width .mlcq$id.show.frm.can.two

    label .mlcq$id.show.frm.can.two.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .mlcq$id.show.frm.can.two create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .mlcq$id.show.frm.can.two.xy
    bind .mlcq$id.show.frm.can.two <Motion> {
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
	    $xscale $zscale .mlcq$id.show.frm.can.two

    .mlcq$id.show.frm.can create window [expr $canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .mlcq$id.show.frm.can.two

    # YZ, lower right

    canvas .mlcq$id.show.frm.can.three -width $canwidth -height $canwidth

    add_air $id .mlcq$id.show.frm.can.three $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    if $cmval($id,2)==0 {
	add_MLCQ_ends $id $yscale $zscale $yrange(0) $zrange(0) $zrange(1)\
		$l $m .mlcq$id.show.frm.can.three
    } else {
	add_MLCQ_sides $id $yscale $zscale $yrange(0) $zrange(0) $zrange(1)\
		$l $m .mlcq$id.show.frm.can.three
    }

    coverup $l $m $width .mlcq$id.show.frm.can.three

    label .mlcq$id.show.frm.can.three.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .mlcq$id.show.frm.can.three create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .mlcq$id.show.frm.can.three.xy
    bind .mlcq$id.show.frm.can.three <Motion> {
	global l m yscale zscale yrange zrange
	set curx %x
	set cury %y
	set curx [expr ($curx-$l)/$yscale+$yrange(0)]
	set cury [expr ($cury-$m)/$zscale+$zrange(0)]
        if { $curx<=$yrange(1) & $curx>=$yrange(0) & $cury<=$zrange(1) & \
                $cury>=$zrange(0) } {
	    set text [format "(%%6.3f, %%6.3f)" $curx $cury]
	    %W.xy configure -text $text
	}
    }

    add_axes yz $cm_ticks($id,y) $cm_ticks($id,z) $l $m $width $yrange(0)\
	    $yrange(1) $zrange(0) $zrange(1)\
	    $yscale $zscale .mlcq$id.show.frm.can.three

    .mlcq$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .mlcq$id.show.frm.can.three

    pack .mlcq$id.show.frm.can -side top -anchor n
    pack .mlcq$id.show.frm -side top -anchor n
}

proc add_MLCQ_xy {id xscale yscale xmin ymin l m parent_w} {
    global cmval colorlist medium nmed colornum

    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,10) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,12) $medium($j)]==0 {
	    set med(leaves) [min_nrc $j $colornum]
	    break
	}
    }
    # XY
    # rectangle for background
    set x1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set y1 [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
    set x2 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set y2 [expr ($cmval($id,0)-$ymin)*$yscale+$m]
    set color [lindex $colorlist $med(in)]
    $parent_w create rectangle $x1 $y1 $x2 $y2 -fill $color

    # set up an array of neg and pos for each leaf
    set i 1; #group i
    set k 0; #leaf k
    while {$k < $cmval($id,5,0)} {
	for {set j 1} {$j<=$cmval($id,8,2,$i)} {incr j} {
	    incr k
	    set neg($k) $cmval($id,8,0,$i)
	    set pos($k) $cmval($id,8,1,$i)
	}
	incr i
    }

    set color [lindex $colorlist $med(leaves)]

    # if it's parallel to x, neg and pos are x-values, else neg and pos are
    # y-values.  Define a rectangle by (a,b) to (c,d).
    # the first leaf starts at -twidth/2

    # dl is the width of each leaf, r0leaf is the radius from the origin
    set dl [expr double($cmval($id,5,1))/$cmval($id,5,0)]
    set r0leaf $cmval($id,7,0)

    if $cmval($id,2)==1 {
	# leaves parallel to x
	set lstart [expr -$cmval($id,5,1)/2]
	set rmin [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	set rmax [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	for {set i 1} {$i<=$cmval($id,5,0)} {incr i} {
	    set a [expr ($neg($i)+$r0leaf-$xmin)*$xscale+$l]
	    set b [expr ($lstart-$ymin)*$yscale+$m]
	    set c [expr ($pos($i)-$r0leaf-$xmin)*$xscale+$l]
	    set d [expr ($lstart+$dl-$ymin)*$yscale+$m]
	    $parent_w create rectangle $rmin $b $a $d -fill $color
	    $parent_w create rectangle $rmax $b $c $d -fill $color
            if { $i==1 } {
            # draw rectangle out to -RMAX
               set rminy [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
               $parent_w create rectangle $rmin $rminy $rmax $b -fill $color
            } elseif { $i==$cmval($id,5,0) } {
            # draw rectangle out to RMAX
               set rmaxy [expr ($cmval($id,0)-$ymin)*$yscale+$m]
               $parent_w create rectangle $rmin $d $rmax $rmaxy -fill $color
            }
	    set lstart [expr $lstart+$dl]
	}
    } else {
	# leaves parallel to y
	set lstart [expr -$cmval($id,5,1)/2]
	set rmin [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
	set rmax [expr ($cmval($id,0)-$ymin)*$yscale+$m]
	for {set i 1} {$i<=$cmval($id,5,0)} {incr i} {
	    set a [expr ($neg($i)+$r0leaf-$ymin)*$yscale+$m]
	    set b [expr ($lstart-$xmin)*$xscale+$l]
	    set c [expr ($pos($i)-$r0leaf-$ymin)*$yscale+$m]
	    set d [expr ($lstart+$dl-$xmin)*$xscale+$l]
	    $parent_w create rectangle $b $rmax $d $c -fill $color
	    $parent_w create rectangle $b $a $d $rmin -fill $color
            if { $i==1 } {
            # draw rectangle out to -RMAX
               set rminx [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
               $parent_w create rectangle $rminx $rmin $b $rmax -fill $color
            } elseif { $i==$cmval($id,5,0) } {
            # draw rectangle out to RMAX
               set rmaxx [expr ($cmval($id,0)-$xmin)*$xscale+$l]
               $parent_w create rectangle $d $rmin $rmaxx $rmax -fill $color
            }
	    set lstart [expr $lstart+$dl]
	}
    }
}

proc add_MLCQ_ends {id xscale zscale xmin zmin zmax l m parent_w} {
    global cmval colorlist medium nmed meds_used colornum

    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,10) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,12) $medium($j)]==0 {
	    set med(leaves) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    # add a rectangle
    set x1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z1 [expr ($cmval($id,3)-$zmin)*$zscale+$m]
    set x2 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z2 [expr ($cmval($id,3)+$cmval($id,4)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(in)]
    $parent_w create rectangle $x1 $z1 $x2 $z2 -fill $color -outline {}

    set r0leaf $cmval($id,7,0)
    set z0leaf $cmval($id,7,1)
    # set up an array of neg and pos for each leaf
    set i 1; #group i
    set k 0; #leaf k
    while {$k < $cmval($id,5,0)} {
	for {set j 1} {$j<=$cmval($id,8,2,$i)} {incr j} {
	    incr k
	    set neg($k) $cmval($id,8,0,$i)
	    set pos($k) $cmval($id,8,1,$i)
	}
	incr i
    }

    # Choose the middle leaf, nleaf/2
    set ml [expr $cmval($id,5,0)/2]

    # add an arc for the negative side and an arc for the positive side
    # 1: negative bounding box
    set a [expr ($neg($ml)-$r0leaf-$xmin)*$xscale+$l]
    set b [expr ($neg($ml)+$r0leaf-$xmin)*$xscale+$l]
    set c [expr ($z0leaf-$r0leaf-$zmin)*$zscale+$m]
    set d [expr ($z0leaf+$r0leaf-$zmin)*$zscale+$m]
    # angles:
    set alp1 [expr asin(($z0leaf-$cmval($id,3))/$r0leaf)]
    set alp2 [expr asin(($z0leaf-$cmval($id,3)-$cmval($id,4))/$r0leaf)]
    set alp1d [expr $alp1*180/3.14159]
    set alp2d [expr $alp2*180/3.14159]
    set color [lindex $colorlist $med(leaves)]
    $parent_w create arc $a $c $b $d -start $alp1d -extent [expr $alp2d-$alp1d]\
	    -fill $color -outline black

    # 2: positive bounding box
    set a [expr ($pos($ml)-$r0leaf-$xmin)*$xscale+$l]
    set b [expr ($pos($ml)+$r0leaf-$xmin)*$xscale+$l]
    $parent_w create arc $a $c $b $d -start [expr 180-$alp1d]\
	    -extent [expr $alp1d-$alp2d] -fill $color -outline black

    # 3: polygons to fill in the rest
    set a [expr ($neg($ml)+$r0leaf*cos($alp2)-$xmin)*$xscale+$l+1]
    set b [expr ($neg($ml)+$r0leaf*cos($alp1)-$xmin)*$xscale+$l+1]
    set c [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set d [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set e [expr ($cmval($id,3)-$zmin)*$zscale+$m]
    set f [expr ($cmval($id,3)+$cmval($id,4)-$zmin)*$zscale+$m]
    # negative
    $parent_w create poly $a $f $b $e $c $e $c $f -fill $color -outline {}
    # positive
    set a [expr ($pos($ml)-$r0leaf*cos($alp2)-$xmin)*$xscale+$l-1]
    set b [expr ($pos($ml)-$r0leaf*cos($alp1)-$xmin)*$xscale+$l-1]
    $parent_w create poly $a $f $b $e $d $e $d $f -fill $color -outline {}

}

proc add_MLCQ_sides {id yscale zscale ymin zmin zmax l m parent_w} {
    global cmval colorlist medium nmed meds_used colornum

    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,10) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,12) $medium($j)]==0 {
	    set med(leaves) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    # add a rectangle
    set y1 [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set z1 [expr ($cmval($id,3)+$cmval($id,4)-$zmin)*$zscale+$m]
    set y2 [expr ($cmval($id,0)-$ymin)*$yscale+$l]
    set z2 [expr ($cmval($id,3)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(in)]
    $parent_w create rectangle $y1 $z1 $y2 $z2 -fill $color -outline {}

    # add polygons with the top having width dl and the bottom having width
    # which depends on zfocus(1): bw=tw*((zmax-zfocus)/(zmin-zfocus))
    # where zmin is the distance from z=0 to the from of the mlcq.

    set zmlcq [expr double($cmval($id,3))]
    set zlarge [expr $zmlcq+$cmval($id,4)]

    set zfocus $cmval($id,6)
    set tw [expr double($cmval($id,5,1))/$cmval($id,5,0)]
    #set bw [expr $tw*(($zlarge-$zfocus)/($zmlcq-$zfocus))]

    # t1 is -twidth/2
    # b1 is the bottom point that corresponds to it
    set t1 [expr -double($cmval($id,5,1))/2.0]
    set b1 [expr $t1*(($zlarge-$zfocus)/($zmlcq-$zfocus))]

    set color [lindex $colorlist $med(leaves)]
    set rmax1 [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set rmax2 [expr ($cmval($id,0)-$ymin)*$yscale+$l]

    set r0leaf $cmval($id,7,0)
    # set up an array of neg and pos for each leaf
    set i 1; #group i
    set k 0; #leaf k
    while {$k < $cmval($id,5,0)} {
	for {set j 1} {$j<=$cmval($id,8,2,$i)} {incr j} {
	    incr k
	    set neg($k) [expr $cmval($id,8,0,$i)+$r0leaf]
	    set pos($k) [expr $cmval($id,8,1,$i)-$r0leaf]
	}
	incr i
    }

    for {set i 1} {$i<=$cmval($id,5,0)} {incr i} {
	set t2 [expr $t1+$tw]
	set b2 [expr $t2*(($zlarge-$zfocus)/($zmlcq-$zfocus))]
	# see if the gap between the leaves passes thru x or y =0.
	if { $pos($i)==$neg($i) || $pos($i)<=0 || $neg($i)>=0 } {
	    if abs($b1)>$cmval($id,0) {
		if $b1<0 {set bstart -$cmval($id,0)} else {set bstart $cmval($id,0)}
		set zstart [expr $cmval($id,0)*($zmlcq-$zfocus)/abs($t1)+$zfocus]
		set zstart [expr ($zstart-$zmin)*$zscale+$m]
	    } else {
		set bstart $b1
		set zstart $z1
	    }
	    if abs($b2)>$cmval($id,0) {
		if $b2<0 {set bend -$cmval($id,0)} else {set bend $cmval($id,0)}
		set zend [expr $cmval($id,0)*($zmlcq-$zfocus)/abs($t2)+$zfocus]
		set zend [expr ($zend-$zmin)*$zscale+$m]
	    } else {
		set bend $b2
		set zend $z1
	    }
	    set a [expr ($t1-$ymin)*$yscale+$l]
	    set b [expr ($bstart-$ymin)*$yscale+$l]
	    set c [expr ($t2-$ymin)*$yscale+$l]
	    set d [expr ($bend-$ymin)*$yscale+$l]

	    if { abs($b1)>$cmval($id,0) && abs($b2)<$cmval($id,0) } {
		# need an extra point in the polygon
		$parent_w create poly $a $z2 $b $zstart $rmax1 $z1\
			$d $zend $c $z2 -fill $color -outline {}
	    } elseif { abs($b1)<$cmval($id,0) && abs($b2)>$cmval($id,0) } {
		# need an extra point in the polygon
		$parent_w create poly $a $z2 $b $zstart $rmax2 $z1\
			$d $zend $c $z2 -fill $color -outline {}
	    } else {
		$parent_w create poly $a $z2 $b $zstart $d $zend $c $z2\
			-fill $color -outline {}
	    }
            $parent_w create line $a $z2 $b $zstart
            if { $i<$cmval($id,5,0) } {
               set j [expr $i+1]
               if { $pos($j)!=$neg($j) && $pos($j)>0 && $neg($j)<0 } {
               #draw another line for the other polygon boundary
                  $parent_w create line $c $z2 $d $zend
               }
            }
	}
        if { $i==1 } {
        # put in a polygon for leaf material out to -RMAX
           if { abs($b1)>$cmval($id,0) && abs($b2)<$cmval($id,0) } {
              set a [expr ($t1-$ymin)*$yscale+$l]
              set zstart [expr $cmval($id,0)*($zmlcq-$zfocus)/abs($t1)+$zfocus]
              set zstart [expr ($zstart-$zmin)*$zscale+$m]
              $parent_w create poly $y1 $z2 $y1 $zstart $a $z2 -fill $color -outline {}
              $parent_w create line $a $z2 $y1 $zstart
           } else {
              set a [expr ($t1-$ymin)*$yscale+$l]
              set bstart [expr ($b1-$ymin)*$yscale+$l]
              $parent_w create poly $y1 $z2 $y1 $z1 $bstart $z1 $a $z2 -fill $color -outline {}
              $parent_w create line $a $z2 $bstart $z1
           }
        } elseif { $i==$cmval($id,5,0) } {
        #put in a polygon for leaf material out to RMAX
          if { abs($b2)>$cmval($id,0) && abs($b1)<$cmval($id,0) } {
              set c [expr ($t2-$ymin)*$yscale+$l]
              set zend [expr $cmval($id,0)*($zmlcq-$zfocus)/abs($t2)+$zfocus]
              set zend [expr ($zend-$zmin)*$zscale+$m]
              $parent_w create poly $c $z2 $y2 $zend $y2 $z2 -fill $color -outline {}
              $parent_w create line $c $z2 $y2 $zend
           } else {
              set c [expr ($t2-$ymin)*$yscale+$l]
              set bend [expr ($b2-$ymin)*$yscale+$l]
              $parent_w create poly $c $z2 $bend $z1 $y2 $z1 $y2 $z2 -fill $color -outline {}
              $parent_w create line $c $z2 $bend $z1
           }
        }
	set t1 $t2
	set b1 $b2
    }
}


