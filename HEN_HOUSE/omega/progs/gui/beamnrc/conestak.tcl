
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: CONESTAK
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


proc init_CONESTAK { id } {
    global cmval owall_conestak

    for {set i 0} {$i<2} {incr i} {
	set cmval($id,$i) {}
    }
    set cmval($id,2,0) {}
    set cmval($id,2,1) {}
    set cmval($id,3) {}

    for {set j 0} {$j<=20} {incr j} {
	for {set k 0} {$k<=2} {incr k} {
	    set cmval($id,4,$k,$j) {}
	}
    }

    for {set j 0} {$j<=3} {incr j} {
	set cmval($id,5,$j) {}
    }
    set cmval($id,6) {}
    for {set j 1} {$j<=20} {incr j} {
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,7,$j,$k) {}
	    set cmval($id,9,$j,$k) {}
	}
	set cmval($id,8,$j) {}
	set cmval($id,10,$j) {}
    }
    set owall_conestak($id) 0
}

proc read_CONESTAK { fileid id } {
    global cmval GUI_DIR owall_conestak cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read CONESTAK $cm_ident($id).  The inputs don't begin where\
		they should.  There is either an error in the CM preceding\
		this, or the accelerator module file is not the correct one\
		for use with this input file." error 0 OK
	return -code break
	# breaks out of the loops this procedure was called from.
    }

    # read rmax 0
    gets $fileid data
    set data [get_val $data cmval $id,0]

    # read title
    gets $fileid data
    set cmval($id,1) $data

    # read zmin, rbn
    gets $fileid data
    set data [get_val $data cmval $id,2,0]
    set data [get_val $data cmval $id,2,1]

    if [expr $cmval($id,2,1)]>0 {
	set owall_conestak($id) 1
    } else {
	set owall_conestak($id) 0
    }

    #read ncone (iscm_max)
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # For each cone, read 3 values, zthick,rmin,rmax
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	gets $fileid data
	for {set j 0} {$j<=2} {incr j} {
	    set data [get_val $data cmval $id,4,$j,$i]
	}
    }

    # If there's an outer wall (rbn!=0) read it's material props.
    if $cmval($id,2,1)>0 {
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,5,$j]
	}
	gets $fileid data
	set data [get_str $data cmval $id,6]
    }

    # For each conical layer, read material props for inner and outer.
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	# inner
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,7,$i,$j]
	}
	gets $fileid data
	set data [get_str $data cmval $id,8,$i]
	# outer
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,9,$i,$j]
	}
	gets $fileid data
	set data [get_str $data cmval $id,10,$i]
    }
}


proc edit_CONESTAK { id zmax } {
    global cmval GUI_DIR owall_conestak values omega helvfont help_conestak_text

    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NCONESTAK.
    catch { destroy .cones$id }
    toplevel .cones$id
    wm title .cones$id "Edit CONESTAK, CM\#$id"
    frame .cones$id.frm -bd 4
    set top .cones$id.frm

    label $top.mainlabel -text "Conestak" -font $helvfont
    pack $top.mainlabel -pady 10

    #CONESTAK_macros.mortran:REPLACE {$MAX_N_$CONESTAK} WITH {10}
    get_1_default CONESTAK $top "maximum number of layers"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    if [string compare $cmval($id,2,1) ""]==0 {
	set owall_conestak($id) 1
    } else {
	if [expr $cmval($id,2,1)]>0 {
	    set owall_conestak($id) 1
	} else {
	    set owall_conestak($id) 0
	}
    }

    add_rmax_rad $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp3 $id,2,0

    frame $top.inp2 -bd 4
    label $top.inp2.label -text {Number of conical layers}
    entry $top.inp2.inp -textvariable cmval($id,3) -width 8
    frame $top.inp2.procbuttons
    button $top.inp2.procbuttons.nextproc1 -text " Define properties >> "\
	    -command "define_conestak_props $id"
    button $top.inp2.procbuttons.nextproc2 -text " Define dimensions >> "\
	    -command "define_conestak_dim $id"
    pack $top.inp2.procbuttons.nextproc2 $top.inp2.procbuttons.nextproc1 \
	    -side top -pady 4 -fill x
    pack $top.inp2.label -anchor w -side left
    pack $top.inp2.inp -side left -padx 10
    pack $top.inp2.procbuttons -anchor e -side right -fill x -expand true
    pack $top.inp2 -side top -fill x

    frame $top.wall
    frame $top.wall.radio
    set w $top.wall.radio
    checkbutton $w.r1 -text "Outer wall" \
            -variable owall_conestak($id) -onvalue 1 -offvalue 0\
	    -command "disable_if_off $id" -anchor w
    pack $w.r1 -side top

    frame $top.wall.define
    set w $top.wall.define

    frame $w.f0
    set w2 $w.f0
    label $w2.lab -text "Inner radius of outer wall (cm)"
    pack $w2.lab -anchor w -side left
    entry $w2.ent -textvariable cmval($id,2,1) -width 8
    pack $w2.ent -anchor e -side right -fill x -expand true
    pack $w.f0 -side top -fill x

    add_ecut $w.f1 $id,5,0
    add_pcut $w.f2 $id,5,1
    add_dose $w.f3 $id,5,2
    add_latch $w.f4 $id,5,3
    add_material $w.f5 $id,6

    disable_if_off $id

    pack $w -side top
    pack $top.wall.radio $top.wall.define -side right -padx 10
    pack $top.wall -side top -pady 10

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .cones$id"\
	    -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command \
	    "help_gif .cones$id.help {$help_conestak_text} help_conestak"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_CONESTAK $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -side left -padx 10
    pack $top.buts
    pack $top
}

proc disable_if_off { id } {
    global owall_conestak

    if $owall_conestak($id)==0 {
	for {set j 0} {$j<6} {incr j} {
	    .cones$id.frm.wall.define.f$j.ent configure -state disabled
	    .cones$id.frm.wall.define.f$j.lab configure -foreground grey
	}
    } else {
	for {set j 0} {$j<6} {incr j} {
	    .cones$id.frm.wall.define.f$j.ent configure -state normal
	    .cones$id.frm.wall.define.f$j.lab configure -foreground black
	}
    }
}


proc define_conestak_props { id } {
    global cmval helvfont values

    # allow a maximum of 4/window, the proceed to the next window.
    set ncone $cmval($id,3)
    set nwindow [expr $ncone/4]
    if [expr $ncone%4]>0 {
	incr nwindow
    }

    for {set i 1} {$i<=$nwindow} {incr i} {
	catch { destroy .cones$id.baby$i }
	toplevel .cones$id.baby$i
	set top .cones$id.baby$i
	wm title $top "Define conestak, window $i"
	for {set ii 1} {$ii<=4} {incr ii} {
	    set index [expr 4*($i-1)+$ii]
	    if $index>$ncone {break}

	    frame $top.f$i-$ii -bd 4
	    set w $top.f$i-$ii

	    frame $w.inside
	    set w $w.inside
	    label $w.lab -text "Inside cone $index" -font $helvfont
	    pack $w.lab -side top

	    add_ecut $w.f0 $id,7,$index,0
	    add_pcut $w.f1 $id,7,$index,1
	    add_dose $w.f2 $id,7,$index,2
	    add_latch $w.f3 $id,7,$index,3
	    add_material $w.f4 $id,8,$index

	    pack $w -side left -padx 5

	    set w $top.f$i-$ii

	    frame $w.outside

	    set w $w.outside
	    label $w.lab -text "Outside cone $index" -font $helvfont
	    pack $w.lab -side top

	    add_ecut $w.f0 $id,9,$index,0
	    add_pcut $w.f1 $id,9,$index,1
	    add_dose $w.f2 $id,9,$index,2
	    add_latch $w.f3 $id,9,$index,3
	    add_material $w.f4 $id,10,$index

	    pack $w -side right -padx 5

	    pack $top.f$i-$ii -side top

	    frame $top.sep$i-$ii -bd 4 -width 100 -height 2 -relief groove
	    pack $top.sep$i-$ii -pady 10 -fill x
	}
	button $top.okb -text "OK" -command "destroy .cones$id.baby$i"\
	    -relief groove -bd 8
	pack $top.okb -pady 10
	if [expr 4*($i-1)+$ii]>$ncone {break}
    }
}

proc define_conestak_dim { id } {
    global cmval helvfont
    set top .cones$id.dimensions
    catch { destroy $top }
    toplevel $top
    wm title $top "CONESTAK dimensions"

    set ncone $cmval($id,3)
    frame $top.f -bd 4
    label $top.f.layer -text "Layer"
    label $top.f.thick -text "Thickness"
    label $top.f.rmin -text "Front radius"
    label $top.f.rmax -text "Back radius"
    grid $top.f.layer -column 0 -row 0
    grid $top.f.thick -column 1 -row 0
    grid $top.f.rmin -column 2 -row 0
    grid $top.f.rmax -column 3 -row 0
    for {set i 1} {$i<=$ncone} {incr i} {
	label $top.f.num$i -text $i
	entry $top.f.enthick$i -textvariable cmval($id,4,0,$i)
	entry $top.f.enrmin$i -textvariable cmval($id,4,1,$i)
	entry $top.f.enrmax$i -textvariable cmval($id,4,2,$i)
	grid $top.f.num$i -column 0 -row $i
	grid $top.f.enthick$i -column 1 -row $i
	grid $top.f.enrmin$i -column 2 -row $i
	grid $top.f.enrmax$i -column 3 -row $i
    }
    button $top.okb -text "OK" -command "destroy .cones$id.dimensions"\
	    -relief groove -bd 8
    pack $top.f $top.okb -pady 10
}

proc write_CONESTAK {fileid id} {
    global cmval cm_names cm_ident cm_type owall_conestak

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2,0), $cmval($id,2,1), ZMIN, RBN"
    puts $fileid "$cmval($id,3), NUMBER OF LAYERS"

    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set str {}
	set str "$str$cmval($id,4,0,$i), "
	set str "$str$cmval($id,4,1,$i), "
	set str "$str$cmval($id,4,2,$i), "
	puts $fileid $str
    }
    if { $owall_conestak($id)==1 } {
	set str {}
	set str "$str$cmval($id,5,0), "
	set str "$str$cmval($id,5,1), "
	set str "$str$cmval($id,5,2), "
	set str "$str$cmval($id,5,3), OUTER WALL"
	puts $fileid $str
	puts $fileid $cmval($id,6)
    }
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	# inner
	set str {}
	set str "$str$cmval($id,7,$i,0), "
	set str "$str$cmval($id,7,$i,1), "
	set str "$str$cmval($id,7,$i,2), "
	set str "$str$cmval($id,7,$i,3), "
	puts $fileid $str
	puts $fileid $cmval($id,8,$i)
	# outer
	set str {}
	set str "$str$cmval($id,9,$i,0), "
	set str "$str$cmval($id,9,$i,1), "
	set str "$str$cmval($id,9,$i,2), "
	set str "$str$cmval($id,9,$i,3), "
	puts $fileid $str
	puts $fileid $cmval($id,10,$i)
    }
}

proc show_CONESTAK { id } {
    global cmval medium nmed owall_conestak xrange zrange med cm_ticks

    catch { destroy .cones$id.show }
    toplevel .cones$id.show
    wm title .cones$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,z) 10

    # find initial zrange,xrange
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_CONESTAK $id }]==1 {
	destroy .cones$id.show
	tk_dialog .cones$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .cones$id.show.buts
    button .cones$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .cones$id.show xz" -relief groove -bd 8
    button .cones$id.show.buts.print -text "Print..." -command\
	    "print_canvas .cones$id.show.can 600 600" -relief groove -bd 8
    button .cones$id.show.buts.done -text "Done" -command\
	    "destroy .cones$id.show" -relief groove -bd 8
    pack .cones$id.show.buts.range .cones$id.show.buts.print\
	    .cones$id.show.buts.done -side left -padx 10
    pack .cones$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_CONESTAK { id } {
    global xrange zrange helvfont xscale zscale l m cm_ticks

    catch {destroy .cones$id.show.can}

    # put the canvas up
    canvas .cones$id.show.can -width 600 -height 600

    # now add on the stuff that's there, scaled to fit into 500x500 (allow
    # a x-border of 100 and z-border of 50 to show the scale)

    set width 450.0
    set l 100.0
    set m 50.0

    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    add_air $id .cones$id.show.can $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_CONESTAK $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .cones$id.show.can

    coverup $l $m $width .cones$id.show.can

    set curx 0
    set cury 0
    label .cones$id.show.can.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .cones$id.show.can create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .cones$id.show.can.xy
    bind .cones$id.show.can <Motion> {
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
	    $xscale $zscale .cones$id.show.can

    pack .cones$id.show.can -side top -anchor n
}

proc add_CONESTAK {id xscale zscale xmin zmin l m parent_w} {
    global colorlist medium nmed cmval owall_conestak meds_used colornum

    # assign numbers to the media
    # for each layer, get inner and outer media.
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set med(in,$i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    # inside
	    if [string compare $cmval($id,8,$i) $medium($j)]==0 {
		set med(in,$i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
	set med(out,$i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    # outside
	    if [string compare $cmval($id,10,$i) $medium($j)]==0 {
		set med(out,$i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    # now get outside cone medium if required
    if $owall_conestak($id)==1 {
	set med(wall) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,6) $medium($j)]==0 {
		set med(wall) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }

    set tot [expr $cmval($id,2,0)-$zmin]
    # for each layer, put in 3 polygon regions (4 points), inside and 2 outside
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	# inside cone
	set rmin $cmval($id,4,1,$i)
	set rmax $cmval($id,4,2,$i)
	set thick $cmval($id,4,0,$i)

	set y(1) [expr $tot*$zscale+$m]
	set y(2) [expr ($tot+$thick)*$zscale+$m]

	set x(1) [expr ($rmin-$xmin)*$xscale+$l]
	set x(2) [expr ($rmax-$xmin)*$xscale+$l]
	set x(3) [expr (-$rmax-$xmin)*$xscale+$l]
	set x(4) [expr (-$rmin-$xmin)*$xscale+$l]

	set color [lindex $colorlist $med(in,$i)]
	$parent_w create polygon $x(1) $y(1) $x(2) $y(2) $x(3) $y(2)\
		$x(4) $y(1) -fill $color -outline {}

	# outside cone, right
	set x(1) [expr ($rmin-$xmin)*$xscale+$l]
	set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	set x(4) [expr ($rmax-$xmin)*$xscale+$l]

	set color [lindex $colorlist $med(out,$i)]
	$parent_w create polygon $x(1) $y(1) $x(2) $y(1) $x(2) $y(2)\
		$x(4) $y(2) -fill $color -outline {}

	# outside cone, left
	set x(1) [expr (-$rmin-$xmin)*$xscale+$l]
	set x(2) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	set x(4) [expr (-$rmax-$xmin)*$xscale+$l]

	set color [lindex $colorlist $med(out,$i)]
	$parent_w create polygon $x(1) $y(1) $x(2) $y(1) $x(2) $y(2)\
		$x(4) $y(2) -fill $color -outline {}

	set tot [expr $tot+$thick]
    }

    # add on the outer wall if required
    if $owall_conestak($id)==1 {
	set rbn $cmval($id,2,1)
	set x(1) [expr ($rbn-$xmin)*$xscale+$l]
	set y(1) [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
	set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	set y(2) [expr $tot*$zscale+$m]

	set color [lindex $colorlist $med(wall)]
	$parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color\
		-outline {}

	set x(1) [expr (-$rbn-$xmin)*$xscale+$l]
	set x(2) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med(wall)]
	$parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color\
		-outline {}
    }
}

