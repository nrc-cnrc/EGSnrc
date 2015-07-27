
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: CIRCAPP
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


proc init_CIRCAPP { id } {
    global cmval

    for {set i 0} {$i<4} {incr i} {
	set cmval($id,$i) {}
    }
    for {set i 1} {$i<=20} {incr i} {
	for {set j 0} {$j<7} {incr j} {
	    set cmval($id,4,$j,$i) {}
	}
    }
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,5,$i) {}
    }
    for {set i 1} {$i<=20} {incr i} {
	set cmval($id,6,$i) {}
    }
}

proc read_CIRCAPP { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read CIRCAPP $cm_ident($id).  The inputs don't begin where\
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

    # read zback 2
    gets $fileid data
    set data [get_val $data cmval $id,2]

    # read number of scrapers 3
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # read stuff for each scraper
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	gets $fileid data
	for {set j 0} {$j<7} {incr j} {
	    set data [get_val $data cmval $id,4,$j,$i]
	}
    }

    # read ecut, pcut etc for surrounding (air) region
    gets $fileid data
    for {set j 0} {$j<4} {incr j} {
	set data [get_val $data cmval $id,5,$j]
    }

    # read medium of each scraper
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	gets $fileid data
	set data [get_str $data cmval $id,6,$i]
    }
}

proc edit_CIRCAPP { id zmax } {
    global cmval GUI_DIR values helvfont help_circapp_text

    catch { destroy .circ$id }
    toplevel .circ$id
    wm title .circ$id "Edit CIRCAPP, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NCIRCAPP.
    frame .circ$id.frm -bd 4
    set top .circ$id.frm

    label $top.mainlabel -text "Circapp" -font $helvfont
    pack $top.mainlabel -pady 10

    #CIRCAPP_macros.mortran:REPLACE {$MAX_N_$CIRCAPP} WITH {10}
    #CIRCAPP_macros.mortran:REPLACE {$GAP_F_B_AIR_MIN} WITH {0.01}
    get_2_defaults CIRCAPP $top MAX_N "maximum number of applicators"\
	    GAP_F_B_AIR_MIN "minimum air gap"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1

    frame $top.inp2 -bd 4
    set text {This is distance from the reference plane (Z=0.0) to\
	    the back of this CM including a mandatory air gap (default\
	    set to 0.01cm) between the back of the last scraper and the\
	    back of the CM.}
    button $top.inp2.help -bitmap @$GUI_DIR/help_icon.xbm \
	    -command "help_dialog $top.help Help {$text} info 0 OK"
    label $top.inp2.label -text {Z of back of CM (including mandatory\
	    0.01 cm air gap)}
    entry $top.inp2.inp -textvariable cmval($id,2) -width 8
    pack $top.inp2.help -anchor w -side left
    pack $top.inp2.label -side left -padx 5
    pack $top.inp2.inp -anchor e -side right -fill x -expand true
    pack $top.inp2 -side top -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label -text {Number of scrapers}
    entry $top.inp4.inp -textvariable cmval($id,3) -width 8
    button $top.inp4.nextproc -text " Define scrapers >> "\
	    -command "define_circapp $id"
    pack $top.inp4.label -anchor w -side left
    pack $top.inp4.inp -side left -padx 5
    pack $top.inp4.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    label $top.airlabel -text "Surrounding (air) region:" -font $helvfont
    pack $top.airlabel -anchor w -pady 10

    add_ecut $top.inp5 $id,5,0
    add_pcut $top.inp6 $id,5,1
    add_dose $top.inp7 $id,5,2
    add_latch $top.inp8 $id,5,3

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "check_circ_for_gaps $id" \
	    -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command \
	    "help_gif .circ$id.help {$help_circapp_text} help_circapp"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_CIRCAPP $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -side left -padx 10
    pack $top.buts -pady 10
    pack $top
}


proc check_circ_for_gaps { id } {
    global cmval

    # cmval($id,3) is the index of the last scraper.
    set diff 0.0
    catch {
	set last [expr int($cmval($id,3))]
	set ztot [expr $cmval($id,4,0,$last)+$cmval($id,4,1,$last)]
	set diff [expr $cmval($id,2)-$ztot]
    }
    set result 1
    if $diff<0.01 {
	set result [tk_dialog .circ$id.gap "Back air gap" "The air gap\
		at the back of the CM is less than the mandatory value of\
		0.01cm." warning 0 Change Continue]
    }
    if $result==1 {
	destroy .circ$id
    }
}

proc define_circapp { id } {
    global cmval helvfont nmed medium help_circapp_text med_per_column

    # Have to allow a maximum of 4 columns per window, otherwise will go off
    # the screen for some users.
    if [catch {expr $cmval($id,3)}]==1 {
        tk_dialog .circ$id.no "Undefined" "Please define the number of\
                scrapers." error 0 OK
        return
    }
    set nwindow [expr $cmval($id,3)/4]
    if [expr $cmval($id,3)%4]>0 {
        incr nwindow
    }

    set count 0
    for {set iwindow 1} {$iwindow<=$nwindow} {incr iwindow} {

	catch { destroy .circ$id.child$iwindow }
	toplevel .circ$id.child$iwindow
	set top .circ$id.child$iwindow
	wm title $top "Define circapp scrapers"

	frame $top.f -bd 10
	set w $top.f

	set nrow 9
	label $w.lo -text "Scraper"
	label $w.l0 -text "Distance to scraper from reference plane (cm)"
	label $w.l1 -text "Scraper thickness (cm)"
	label $w.l2 -text "Radius of inner opening (cm)"
	label $w.l3 -text "X halfwidth of outer edge (cm)"
	label $w.l4 -text "Y halfwidth of outer edge (cm)"
	label $w.l5 -text "Dose scoring zone"
	label $w.l6 -text "Associate with LATCH bit"
	label $w.l7 -text "Material"
	grid configure $w.lo -row 0 -column 0 -sticky w
	for {set i 0} {$i<8} {incr i} {
	    grid configure $w.l$i -row [expr $i+1] -column 0 -sticky w
	}

	for {set i 1} {$i<=4} {incr i} {
	    incr count
	    label $w.lab$i -text $count
	    grid configure $w.lab$i -row 0 -column $i
	    for {set j 0} {$j<7} {incr j} {
		entry $w.e$i-$j -textvariable cmval($id,4,$j,$count)
		grid configure $w.e$i-$j -row [expr $j+1] -column $i
	    }
	    # material option menu
	    menubutton $w.e$i-8 -text $cmval($id,6,$count) -menu \
		    $w.e$i-8.m -bd 1 -relief raised -indicatoron 0 -width 20
	    menu $w.e$i-8.m -tearoff no
	    for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
		$w.e$i-8.m add command -label $medium($iopt) \
			-command "set_grid_material $w.e$i-8 $iopt $id,6,$count" -columnbreak [expr $iopt % $med_per_column==0]
	    }
	    grid configure $w.e$i-8 -row 8 -column $i
	    if $count>=$cmval($id,3) { break }
	}

	pack $w
	frame $top.b
	button $top.b.okb -text "OK" -command "destroy .circ$id.child$iwindow"\
		-relief groove -bd 8
	button $top.b.helpb -text "Help" -command "help_gif \
		.circ$id.child$iwindow.help {$help_circapp_text} help_circapp"\
		-relief groove -bd 8
	pack $top.b.helpb $top.b.okb -side left -padx 10
	pack $top.b -pady 10
    }
}


proc write_CIRCAPP {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), ZBACK"
    puts $fileid "$cmval($id,3), #SCRAPERS"

    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set str {}
	for {set j 0} {$j<7} {incr j} {
	    set str "$str$cmval($id,4,$j,$i), "
	}
	puts $fileid $str
    }

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,5,$i), "
    }
    puts $fileid $str

    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	puts $fileid $cmval($id,6,$i)
    }
}


proc show_CIRCAPP { id } {
    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks

    catch { destroy .circ$id.show }
    toplevel .circ$id.show
    wm title .circ$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,y) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange, zrange:
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set yrange(0) -$cmval($id,0)
	set yrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    # this canvas needs a scrollbar, depending on the screen size...
    # make a frame to hold the canvas with its scrollbars.
    frame .circ$id.show.frm

    if [catch { draw_CIRCAPP $id }]==1 {
	destroy .circ$id.show
	tk_dialog .circ$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .circ$id.show.buts
    button .circ$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .circ$id.show xyz" -relief groove -bd 8
    button .circ$id.show.buts.print3 -text "Print xy..." -command\
	    "print_canvas .circ$id.show.frm.can.three 450 450" -relief groove -bd 8
    button .circ$id.show.buts.print1 -text "Print xz..." -command\
	    "print_canvas .circ$id.show.frm.can.one 450 450" -relief groove -bd 8
    button .circ$id.show.buts.print2 -text "Print yz..." -command\
	    "print_canvas .circ$id.show.frm.can.two 450 450" -relief groove -bd 8
    button .circ$id.show.buts.done -text "Done" -command\
	    "destroy .circ$id.show" -relief groove -bd 8
    pack .circ$id.show.buts.range .circ$id.show.buts.print3 \
	    .circ$id.show.buts.print1\
	    .circ$id.show.buts.print2 .circ$id.show.buts.done -side left -padx 10

    pack .circ$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_CIRCAPP { id } {
    global cmval zrange yrange xrange helvfont xscale yscale zscale
    global l m cm_ticks

    catch { destroy .circ$id.show.frm.can }

    # put the canvas up
    set ncan 3
    set width 300.0
    set canwidth [expr $width+150.0]
    set scrlheight [expr 2*$canwidth]
    set scrlwidth [expr 2*$canwidth]
    set winwidth [expr [winfo screenwidth .]*4.0/5.0]
    set winheight [expr [winfo screenheight .]*4.0/5.0]

    if $scrlwidth>$winwidth {
	catch { destroy .circ$id.show.frm.scrlz }
	catch { destroy .circ$id.show.frm.scrlz }
	scrollbar .circ$id.show.frm.scrlz -command\
		".circ$id.show.frm.can yview"
	scrollbar .circ$id.show.frm.scrlx -command\
		".circ$id.show.frm.can xview" -orient horizontal
	pack .circ$id.show.frm.scrlz -side right -fill y
	pack .circ$id.show.frm.scrlx -side bottom -fill x
	canvas .circ$id.show.frm.can -width $winwidth -height $winheight\
		-yscrollcommand ".circ$id.show.frm.scrlz set"\
		-xscrollcommand ".circ$id.show.frm.scrlx set"\
		-scrollregion "0 0 $scrlwidth $scrlheight"
    } else {
	canvas .circ$id.show.frm.can -width $scrlwidth -height $scrlheight
    }

    # Put some text in the upper left corner, just to fill the gap.
    .circ$id.show.frm.can create text 225 225 -text "CIRCAPP preview"\
	    -font $helvfont -width 300

    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    set l 100.0
    set m 50.0

    # top right canvas, xy view
    canvas .circ$id.show.frm.can.three -width $canwidth -height $canwidth

    add_CIRCAPP_xy $id $xscale $yscale $xrange(0) $yrange(0)\
	    $l $m .circ$id.show.frm.can.three

    coverup $l $m $width .circ$id.show.frm.can.three
    set curx 0
    set cury 0
    label .circ$id.show.frm.can.three.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .circ$id.show.frm.can.three create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .circ$id.show.frm.can.three.xy
    bind .circ$id.show.frm.can.three <Motion> {
	global l m xscale yscale xrange yrange
	set curx %x
	set cury %y
	set curx [expr ($curx-$l)/$xscale+$xrange(0)]
	set cury [expr $yrange(1)-($cury-$m)/$yscale]
        if { $curx<=$xrange(1) & $curx>=$xrange(0) & $cury<=$yrange(1) & \
                $cury>=$yrange(0) } {
	    set text [format "(%%6.3f, %%6.3f)" $curx $cury]
	    %W.xy configure -text $text
	}
    }

    add_axes xy $cm_ticks($id,x) $cm_ticks($id,y) $l $m $width $xrange(0)\
	    $xrange(1) $yrange(0) $yrange(1)\
	    $xscale $yscale .circ$id.show.frm.can.three

    .circ$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .circ$id.show.frm.can.three

    # left canvas, x/z cross-section

    canvas .circ$id.show.frm.can.one -width $canwidth -height $canwidth

    add_air $id .circ$id.show.frm.can.one $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_CIRCAPP $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .circ$id.show.frm.can.one

    coverup $l $m $width .circ$id.show.frm.can.one

    label .circ$id.show.frm.can.one.xy -text\
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .circ$id.show.frm.can.one create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .circ$id.show.frm.can.one.xy
    bind .circ$id.show.frm.can.one <Motion> {
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
	    $xscale $zscale .circ$id.show.frm.can.one

    .circ$id.show.frm.can create window [expr $canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .circ$id.show.frm.can.one

    # right canvas, YZ cross-section

    canvas .circ$id.show.frm.can.two -width $canwidth -height $canwidth

    add_air $id .circ$id.show.frm.can.two $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    add_CIRCAPP_yz $id $yscale $zscale $yrange(0) $zrange(0)\
	    $l $m .circ$id.show.frm.can.two

    coverup $l $m $width .circ$id.show.frm.can.two

    label .circ$id.show.frm.can.two.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .circ$id.show.frm.can.two create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .circ$id.show.frm.can.two.xy
    bind .circ$id.show.frm.can.two <Motion> {
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
	    $yscale $zscale .circ$id.show.frm.can.two

    .circ$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .circ$id.show.frm.can.two

    pack .circ$id.show.frm.can -side top -anchor n
    pack .circ$id.show.frm -side top -anchor n
}

proc add_CIRCAPP_xy { id xscale yscale xmin ymin l m parent_w} {

    global cmval colorlist nmed medium meds_used values colornum

    # assign numbers to the media, and air
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,6,$i) $medium($j)]==0 {
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
	    break
	}
    }

    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set y(1) [expr ($cmval($id,0)-$ymin)*$yscale+$m]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set y(2) [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $x(1) $y(1) $x(2) $y(2) -fill $color -outline {}

    # for each layer, draw a polygon that goes around the shape of the
    # applicator, starting from the bottom layer.

    # make a look-up table for sines, cosines every 10 degrees
    set pi 3.1415926535897932384626433832795
    for {set i 0} {$i<=360} {incr i 5} {
	set rad [expr double($pi*double($i)/180.0)]
	set costh($i) [expr cos($rad)]
	set sinth($i) [expr sin($rad)]
    }

    for {set i $cmval($id,3)} {$i>=1} {incr i -1} {
	set zero [expr -$xmin*$xscale+$l]
	set xout [expr ($cmval($id,4,3,$i)-$xmin)*$xscale+$l]
	set mxout [expr (-$cmval($id,4,3,$i)-$xmin)*$xscale+$l]
	set yout [expr ($cmval($id,4,4,$i)-$ymin)*$yscale+$m]
	set myout [expr (-$cmval($id,4,4,$i)-$ymin)*$yscale+$m]

	set points "$zero $yout $mxout $yout $mxout $myout\
		$xout $myout $xout $yout $zero $yout"

	for {set j 90} {$j<360} {incr j 10} {
	    set xr [expr ($cmval($id,4,2,$i)*$costh($j)-$xmin)*$xscale+$l]
	    set yr [expr ($cmval($id,4,2,$i)*$sinth($j)-$ymin)*$yscale+$m]
	    set points "$points $xr $yr"
	}
	for {set j 0} {$j<=90} {incr j 10} {
	    set xr [expr ($cmval($id,4,2,$i)*$costh($j)-$xmin)*$xscale+$l]
	    set yr [expr ($cmval($id,4,2,$i)*$sinth($j)-$ymin)*$yscale+$m]
	    set points "$points $xr $yr"
	}
	set color [lindex $colorlist $med($i)]
	eval "$parent_w create poly $points -fill $color -outline $color"
    }
}

proc add_CIRCAPP {id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist nmed medium meds_used values colornum

    # assign numbers to the media, and air
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,6,$i) $medium($j)]==0 {
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
    # rectangle for air
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z(1) [expr ($cmval($id,4,0,1)-$zmin)*$zscale+$m]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z(2) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color\
	    -outline $color

    # for each layer, put on 2 rectangles to show the apps at y=0

    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set z(1) [expr ($cmval($id,4,0,$i)-$zmin)*$zscale+$m]
	set z(2) [expr ($cmval($id,4,0,$i)+$cmval($id,4,1,$i)-$zmin)*$zscale+$m]

	# always 2 rectangles per layer
	set x(1) [expr (-$cmval($id,4,3,$i)-$xmin)*$xscale+$l]
	set x(2) [expr (-$cmval($id,4,2,$i)-$xmin)*$xscale+$l]
	set x(3) [expr ($cmval($id,4,3,$i)-$xmin)*$xscale+$l]
	set x(4) [expr ($cmval($id,4,2,$i)-$xmin)*$xscale+$l]

	set color [lindex $colorlist $med($i)]
	$parent_w create rectangle $x(1) $z(1) $x(2) $z(2)\
		-fill $color -outline {}
	$parent_w create rectangle $x(3) $z(1) $x(4) $z(2)\
		-fill $color -outline {}
    }
}

proc add_CIRCAPP_yz {id yscale zscale ymin zmin l m parent_w} {
    global cmval colorlist nmed medium values colornum

    # right canvas, YZ cross-section

    # assign numbers to the media, and air
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,6,$i) $medium($j)]==0 {
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

    # rectangle for air
    set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set z(1) [expr ($cmval($id,4,0,1)-$zmin)*$zscale+$m]
    set y(2) [expr ($cmval($id,0)-$ymin)*$yscale+$l]
    set z(2) [expr ($cmval($id,2)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color\
	    -outline $color

    # for each layer, put on 2 rectangles to show the apps at x=0

    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set z(1) [expr ($cmval($id,4,0,$i)-$zmin)*$zscale+$m]
	set z(2) [expr ($cmval($id,4,0,$i)+$cmval($id,4,1,$i)-$zmin)*$zscale+$m]

	# always 2 rectangles per layer
	set y(1) [expr (-$cmval($id,4,4,$i)-$ymin)*$yscale+$l]
	set y(2) [expr (-$cmval($id,4,2,$i)-$ymin)*$yscale+$l]
	set y(3) [expr ($cmval($id,4,4,$i)-$ymin)*$yscale+$l]
	set y(4) [expr ($cmval($id,4,2,$i)-$ymin)*$yscale+$l]

	set color [lindex $colorlist $med($i)]
	$parent_w create rectangle $y(1) $z(1) $y(2) $z(2)\
		-fill $color -outline $color
	$parent_w create rectangle $y(3) $z(1) $y(4) $z(2)\
		-fill $color -outline $color
    }
}

