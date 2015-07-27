
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: JAWS
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


proc init_JAWS { id } {
    global cmval

    for {set i 0} {$i<3} {incr i} {
	set cmval($id,$i) {}
    }
    for {set i 1} {$i<=20} {incr i} {
	set cmval($id,3,$i) {}
	for {set j 0} {$j<6} {incr j} {
	    set cmval($id,4,$j,$i) {}
	}
    }
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,5,$i) {}
    }
    for {set i 1} {$i<=20} {incr i} {
	for {set j 0} {$j<4} {incr j} {
	    set cmval($id,6,$j,$i) {}
	}
	set cmval($id,7,$i) {}
    }
}

proc read_JAWS { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read JAWS $cm_ident($id).  The inputs don't begin where\
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

    # read number of paired jaws 2
    gets $fileid data
    set data [get_val $data cmval $id,2]

    # read stuff for each pair
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	gets $fileid data

	# X or Y orientation:
        set indx [string first , $data]
        if { $indx<1 } { set indx [string length $data] }
        set cmval($id,3,$i) [string range $data 0 [incr indx -1] ]
	# Make sure it's upper case:
	if [string compare $cmval($id,3,$i) x]==0 { set cmval($id,3,$i) X }
	if [string compare $cmval($id,3,$i) y]==0 { set cmval($id,3,$i) Y }

	gets $fileid data
	for {set j 0} {$j<6} {incr j} {
	    set data [get_val $data cmval $id,4,$j,$i]
	}
    }
    gets $fileid data
    for {set j 0} {$j<4} {incr j} {
	set data [get_val $data cmval $id,5,$j]
    }
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	gets $fileid data
	for {set j 0} {$j<4} {incr j} {
	    set data [get_val $data cmval $id,6,$j,$i]
	}
	gets $fileid data
	set data [get_str $data cmval $id,7,$i]
    }
}

proc edit_JAWS { id zmax } {
    global cmval GUI_DIR omega helvfont help_jaws_text values

    catch { destroy .jaws$id }
    toplevel .jaws$id
    wm title .jaws$id "Edit JAWS, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NJAWS.
    frame .jaws$id.frm -bd 4
    set top .jaws$id.frm

    label $top.mainlabel -text "Jaws" -font $helvfont
    pack $top.mainlabel -pady 10

    #JAWS_macros.mortran:REPLACE {$MAX_N_$JAWS} WITH {10}
    get_1_default JAWS $top "maximum number of paired bars or jaws"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1

    frame $top.inp3 -bd 4
    label $top.inp3.label -text {Number of paired bars}
    entry $top.inp3.inp -textvariable cmval($id,2) -width 8
    button $top.inp3.nextproc -text " >> "\
	    -command "define_jaws $id"
    pack $top.inp3.label -anchor w -side left
    pack $top.inp3.inp -side left -padx 5
    pack $top.inp3.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x

    label $top.airlabel -text "Openings:" -font $helvfont
    pack $top.airlabel -anchor w -pady 10

    add_ecut $top.inp4 $id,5,0
    add_pcut $top.inp5 $id,5,1
    add_dose $top.inp6 $id,5,2
    add_latch $top.inp7 $id,5,3

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .jaws$id"\
	    -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command \
	    "help_gif .jaws$id.help {$help_jaws_text} help_jaws"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command \
	    "show_JAWS $id" -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -padx 10 -side left
    pack $top.buts -pady 5
    pack $top
}

proc define_jaws { id } {
    global cmval helvfont values help_jaws_text nmed medium med_per_column

    catch { destroy .jaws$id.child }
    toplevel .jaws$id.child
    set top .jaws$id.child
    wm title $top "Define jaws"

    frame $top.f -bd 10
    set w $top.f

    label $w.lo -text "Orientation"
    label $w.l0 -text "Distance from front of jaw to reference plane (cm)"
    label $w.l1 -text "Distance from back of jaw to reference plane (cm)"
    label $w.l2 -text "x/y coordinate at positive front of jaw (cm)"
    label $w.l3 -text "x/y coordinate at positive back of jaw (cm)"
    label $w.l4 -text "x/y coordinate at negative front of jaw (cm)"
    label $w.l5 -text "x/y coordinate at negative back of jaw (cm)"
    label $w.l6 -text "Electron cutoff (default ECUTIN) (MeV)"
    label $w.l7 -text "Photon cutoff (default PCUTIN) (MeV)"
    label $w.l8 -text "Dose zone (0 for no scoring)"
    label $w.l9 -text "Associate with LATCH bit"
    label $w.l10 -text "Material"

    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	menubutton $w.xy$i -text $cmval($id,3,$i) -menu $w.xy$i.m -bd 1 \
		-relief raised -indicatoron 0 -width 20
	menu $w.xy$i.m -tearoff no
	$w.xy$i.m add command -label X \
		-command "set cmval($id,3,$i) X; \
		$w.xy$i configure -text X"
	$w.xy$i.m add command -label Y \
		-command "set cmval($id,3,$i) Y; \
		$w.xy$i configure -text Y"
	for {set j 0} {$j<6} {incr j} {
	    set indx [expr $j+1]
	    entry $w.e$i-$indx -textvariable cmval($id,4,$j,$i)
	}
	for {set j 0} {$j<4} {incr j} {
	    set indx [expr $j+7]
	    if $j==0 {
		if [string compare $cmval($id,6,0,$i) ""]==0 {
		    set cmval($id,6,0,$i) $values(ecut)
		}
	    } elseif $j==1 {
		if [string compare $cmval($id,6,1,$i) ""]==0 {
		    set cmval($id,6,1,$i) $values(pcut)
		}
	    }
	    entry $w.e$i-$indx -textvariable cmval($id,6,$j,$i) -width 20
	}
	set indx 11
	# material option menu
	menubutton $w.e$i-11 -text $cmval($id,7,$i) -menu \
		$w.e$i-11.m -bd 1 -relief raised -indicatoron 0 -width 20
	menu $w.e$i-11.m -tearoff no
	for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
	    $w.e$i-11.m add command -label $medium($iopt) \
		    -command "set_grid_material $w.e$i-11 $iopt $id,7,$i" -columnbreak [expr $iopt % $med_per_column==0]
	}
    }

    button $w.quikdef -text \
"Define x/y
using field
size and SSD" -command "quikdef $id" -relief groove -bd 8
    grid configure $w.quikdef -row 3 -column 0 -rowspan 4 -sticky w

    grid configure $w.lo -row 0 -column 1 -sticky w
    for {set i 0} {$i<11} {incr i} {
	grid configure $w.l$i -row [expr $i+1] -column 1 -sticky w
    }
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	grid configure $w.xy$i -row 0 -column [expr $i+1]
    }
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	for {set j 1} {$j<12} {incr j} {
	    grid configure $w.e$i-$j -row $j -column [expr $i+1]
	}
    }
    pack $w
    frame $top.b
    button $top.b.okb -text "OK" -command "destroy .jaws$id.child"\
	    -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
	    "help_gif .jaws$id.child.help {$help_jaws_text} help_jaws"\
	    -relief groove -bd 8
    pack $top.b.helpb $top.b.okb -side left -padx 10
    pack $top.b -pady 5
}


proc write_JAWS {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), # PAIRED BARS OR JAWS"

    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	puts $fileid $cmval($id,3,$i)
	set str {}
	for {set j 0} {$j<6} {incr j} {
	    set str "$str$cmval($id,4,$j,$i), "
	}
	puts $fileid $str
    }
    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,5,$i), "
    }
    puts $fileid $str
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	set str {}
	for {set k 0} {$k<4} {incr k} {
	    set str "$str$cmval($id,6,$k,$i), "
	}
	puts $fileid $str
	puts $fileid $cmval($id,7,$i)
    }
}


proc show_JAWS { id } {
    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks

    catch { destroy .jaws$id.show }
    toplevel .jaws$id.show
    wm title .jaws$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,y) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange,zrange:
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set yrange(0) -$cmval($id,0)
	set yrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_JAWS $id }]==1 {
	destroy .jaws$id.show
	tk_dialog .jaws$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .jaws$id.show.buts
    button .jaws$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .jaws$id.show xyz" -relief groove -bd 8
    button .jaws$id.show.buts.print1 -text "Print xz..." -command\
	    "print_canvas .jaws$id.show.can.one 450 450" -relief groove -bd 8
    button .jaws$id.show.buts.print2 -text "Print yz..." -command\
	    "print_canvas .jaws$id.show.can.two 450 450" -relief groove -bd 8
    button .jaws$id.show.buts.done -text "Done" -command\
	    "destroy .jaws$id.show" -relief groove -bd 8
    pack .jaws$id.show.buts.range .jaws$id.show.buts.print1\
	    .jaws$id.show.buts.print2 .jaws$id.show.buts.done\
	    -side left -padx 10
    pack .jaws$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_JAWS { id } {
    global helvfont zrange yrange xrange zscale yscale xscale l m
    global cm_ticks

    catch { destroy .jaws$id.show.can }

    # put the canvas up
    canvas .jaws$id.show.can -width 900 -height 400

    # need 2 canvases (canvi?) to show an xz and yz cross-section.
    # let each be 300x300, with 100 on the left of each:
    # |100|300|150|300|50|

    set width 300.0
    set canwidth 450.0
    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    # left canvas, x/z cross-section
    set l 100.0
    set m 50.0

    canvas .jaws$id.show.can.one -width $canwidth -height $canwidth

    add_air $id .jaws$id.show.can.one $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_JAWS $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .jaws$id.show.can.one

    coverup $l $m $width .jaws$id.show.can.one

    set curx 0
    set cury 0
    label .jaws$id.show.can.one.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .jaws$id.show.can.one create window [expr $l/2+$width] [expr 2*$m+$width-10]\
	    -window .jaws$id.show.can.one.xy
    bind .jaws$id.show.can.one <Motion> {
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
	    $xscale $zscale .jaws$id.show.can.one

    .jaws$id.show.can create window [expr $canwidth/2.0]\
	    [expr $canwidth/2.0] -window .jaws$id.show.can.one

    # right canvas, YZ cross-section

    canvas .jaws$id.show.can.two -width $canwidth -height $canwidth

    add_air $id .jaws$id.show.can.two $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    add_JAWS_yz $id $yscale $zscale $yrange(0) $zrange(0)\
	    $l $m .jaws$id.show.can.two

    coverup $l $m $width .jaws$id.show.can.two

    label .jaws$id.show.can.two.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .jaws$id.show.can.two create window [expr $l/2+$width] [expr 2*$m+$width-10]\
	    -window .jaws$id.show.can.two.xy
    bind .jaws$id.show.can.two <Motion> {
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
	    $yscale $zscale .jaws$id.show.can.two

    .jaws$id.show.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .jaws$id.show.can.two

    pack .jaws$id.show.can -side top -anchor n
}

proc add_JAWS {id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist nmed medium meds_used values colornum

    # assign numbers to the media
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,7,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $values(2) $medium($j)]==0 {
	    set med(air) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }

    # air in background
    set z(1) [expr ($cmval($id,4,0,1)-$zmin)*$zscale+$m]
    set z(2) [expr ($cmval($id,4,1,$cmval($id,2))-$zmin)*$zscale+$m]
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	if [string compare $cmval($id,3,$i) "X"]==0 {
	    set y1 [expr ($cmval($id,4,0,$i)-$zmin)*$zscale+$m]
	    set y2 [expr ($cmval($id,4,1,$i)-$zmin)*$zscale+$m]
	    set x(xmn) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	    set x(xmp) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	    set x(xfp) [expr ($cmval($id,4,2,$i)-$xmin)*$xscale+$l]
	    set x(xbp) [expr ($cmval($id,4,3,$i)-$xmin)*$xscale+$l]
	    set x(xfn) [expr ($cmval($id,4,4,$i)-$xmin)*$xscale+$l]
	    set x(xbn) [expr ($cmval($id,4,5,$i)-$xmin)*$xscale+$l]
	    set color [lindex $colorlist $med($i)]
	    $parent_w create polygon $x(xfp) $y1 $x(xmp) $y1 $x(xmp) $y2\
		    $x(xbp) $y2 -fill $color -outline $color
	    $parent_w create polygon $x(xfn) $y1 $x(xbn) $y2 $x(xmn) $y2\
		    $x(xmn) $y1 -fill $color -outline $color
	}
    }
}

proc add_JAWS_yz {id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist nmed medium values colornum

    # assign numbers to the media
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,7,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		break
	    }
	}
    }
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $values(2) $medium($j)]==0 {
	    set med(air) [min_nrc $j $colornum]
	    break
	}
    }

    # The x's here are really y's...
    # air in background, 2 rectangles
    set z(1) [expr $zmin*$zscale+$m]
    set z(2) [expr (-$cmval($id,4,1,$cmval($id,2)))*$zscale+$m]
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	if [string compare $cmval($id,3,$i) "Y"]==0 {
	    set y1 [expr ($cmval($id,4,0,$i)-$zmin)*$zscale+$m]
	    set y2 [expr ($cmval($id,4,1,$i)-$zmin)*$zscale+$m]
	    set x(xmn) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	    set x(xmp) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	    set x(xfp) [expr ($cmval($id,4,2,$i)-$xmin)*$xscale+$l]
	    set x(xbp) [expr ($cmval($id,4,3,$i)-$xmin)*$xscale+$l]
	    set x(xfn) [expr ($cmval($id,4,4,$i)-$xmin)*$xscale+$l]
	    set x(xbn) [expr ($cmval($id,4,5,$i)-$xmin)*$xscale+$l]
	    set color [lindex $colorlist $med($i)]
	    $parent_w create polygon $x(xfp) $y1 $x(xmp) $y1 $x(xmp) $y2\
		    $x(xbp) $y2 -fill $color -outline $color
	    $parent_w create polygon $x(xfn) $y1 $x(xbn) $y2 $x(xmn) $y2\
		    $x(xmn) $y1 -fill $color -outline $color
	}
    }
}

proc quikdef { id } {

    global cmval ssd fmin fmax fieldfocus

# procedure which allows the user to define the x/y coordinates of the
# jaws using field dimensions, SSD and Z from which SSD is measured

    catch { destroy .jaws$id.child.qkdef }
    toplevel .jaws$id.child.qkdef
    set top .jaws$id.child.qkdef
    wm title $top "Define openings using field limits & SSD"

    #upper part of window
    frame $top.u

    #some text explaining what's going on
    message $top.u.lab -text "Specify the field limits, SSD and\
    Z focus (from which SSD is measured) of the field and select the paired\
    jaws to apply this field to.  Click Update x/y coords. to calculate the\
     opening coordinates.  Note: only valid for photon beams." -width 10c
    pack $top.u.lab -pady 5


    #left side of the window
    frame $top.u.l -bd 5

    frame $top.u.l.fmin
    label $top.u.l.fmin.lab -text "lower edge of field (cm)"
    entry $top.u.l.fmin.inp -textvariable fmin($id)
    pack $top.u.l.fmin.inp $top.u.l.fmin.lab -side right -fill x

    frame $top.u.l.fmax
    label $top.u.l.fmax.lab -text "upper edge of field (cm)"
    entry $top.u.l.fmax.inp -textvariable fmax($id)
    pack $top.u.l.fmax.inp $top.u.l.fmax.lab -side right -fill x

    frame $top.u.l.ssd
    label $top.u.l.ssd.lab -text "SSD (cm)"
    entry $top.u.l.ssd.inp -textvariable ssd($id)
    pack $top.u.l.ssd.inp $top.u.l.ssd.lab -side right -fill x

    frame $top.u.l.ff
    label $top.u.l.ff.lab -text "Z focus of field (cm)"
    entry $top.u.l.ff.inp -textvariable fieldfocus($id)
    pack $top.u.l.ff.inp $top.u.l.ff.lab -side right -fill x

    pack $top.u.l.fmin $top.u.l.fmax $top.u.l.ssd $top.u.l.ff -side top -fill x

    pack $top.u.l -side left

    #right side of the window
    frame $top.u.r

    label $top.u.r.lab -text "apply to jaw #:"

    pack $top.u.r.lab -side top -anchor w

    frame $top.u.r.list
    listbox $top.u.r.list.num -height 5 -selectmode multiple \
            -yscrollcommand "$top.u.r.list.scrl set"
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
      $top.u.r.list.num insert end "$i"
    }
    scrollbar $top.u.r.list.scrl -command "$top.u.r.list.num yview"

    pack $top.u.r.list.scrl -side right -fill y
    pack $top.u.r.list.num -side left
    pack $top.u.r.list -side left

    pack $top.u.r -side left

    frame $top.but -bd 4
    button $top.but.update -text "Update x/y coords." -command "quikcalc $id"\
            -relief groove -bd 8
    button $top.but.quit -text "Quit" -command "destroy .jaws$id.child.qkdef"\
            -relief groove -bd 8
    pack $top.but.update $top.but.quit -side left
    pack $top.u $top.but -side top

}

proc quikcalc { id } {
    global cmval ssd halfwidth fieldfocus fmin fmax

#   procedure to calculate X/Y coordinates based on field dimensions, ssd
#   and Z from which SSD is measured
#   and updates the coordinates as displayed in the jaws definition window

    set ierror 0

    set jawlist [.jaws$id.child.qkdef.u.r.list.num curselection]

    if { $jawlist=="" } {
       tk_dialog .jaws$id.warning1 "No jaws selected" "You must select at\
       least 1 set of jaws to apply this field size & SSD to." warning 0 OK
       return
    }

    if { $ssd($id)=="" || $fmin($id)=="" || $fmin($id)=="" || $fieldfocus($id)==""} {
       tk_dialog .jaws$id.warning2 \
       "No field dimensions or SSD or field focus specified" "You must\
    specify field dimensions, an SSD and a Z value from which SSD is measured\
       in order to calculate the x/y coordinates of the openings." warning 0 OK
       return
    } elseif { $fmin($id) > $fmax($id) } {
       tk_dialog .jaws$id.warning3 \
       "Field dimension problem" "The lower edge of the field must be <=\
    the upper edge of the field in order for jaw settings to be calculated\
    properly." warning 0 OK
       return
    }

    foreach i $jawlist {
       set j [expr $i+1]
       if { $cmval($id,4,0,$j)!="" && $cmval($id,4,1,$j)!="" &&\
             $cmval($id,4,1,$j)>$cmval($id,4,0,$j) } {
           # grid automatically updates once the values are recalculated
           set cmval($id,4,2,$j) \
[expr ($cmval($id,4,0,$j)-$fieldfocus($id))*double($fmax($id))/double($ssd($id))]
           set cmval($id,4,2,$j) [format "%3.5f" $cmval($id,4,2,$j)]
           set cmval($id,4,3,$j) \
[expr ($cmval($id,4,1,$j)-$fieldfocus($id))*double($fmax($id))/double($ssd($id))]
           set cmval($id,4,3,$j) [format "%3.5f" $cmval($id,4,3,$j)]
           set cmval($id,4,4,$j) \
[expr ($cmval($id,4,0,$j)-$fieldfocus($id))*double($fmin($id))/double($ssd($id))
]
           set cmval($id,4,4,$j) [format "%3.5f" $cmval($id,4,4,$j)]
           set cmval($id,4,5,$j) \
[expr ($cmval($id,4,1,$j)-$fieldfocus($id))*double($fmin($id))/double($ssd($id))
]
           set cmval($id,4,5,$j) [format "%3.5f" $cmval($id,4,5,$j)]
        } else {
           incr ierror
        }
    }

    if { $ierror > 0 } {
       tk_dialog .jaws$id.warning4 "Front and back of jaws not defined"\
       "$ierror of the selected jaws did not\
       have front and back Z positions defined or had front Z position >\
       back Z position.  The x/y coordinates of the opening(s) for this/these\
       jaw(s) was not updated." warning 0 OK
    }

}

