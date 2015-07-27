
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: SLABS
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


proc init_SLABS { id } {
    global cmval

    for {set i 0} {$i<4} {incr i} {
	set cmval($id,$i) {}
    }
    # 30 is what is in SLABS_macros.mortran at this time.
    for {set j 0} {$j<=30} {incr j} {
	for {set k 0} {$k<=6} {incr k} {
	    set cmval($id,4,$k,$j) {}
	}
    }
}

proc read_SLABS { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read SLABS $cm_ident($id).  The inputs don't begin where\
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

    # read nslabs
    gets $fileid data
    set data [get_val $data cmval $id,2]

    #read z from front to first reference plane
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # For each slab, read 5 values, then the medium
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	gets $fileid data
	for {set j 0} {$j<=5} {incr j} {
	    set data [get_val $data cmval $id,4,$j,$i]
	}
	gets $fileid data
	set data [get_str $data cmval $id,4,6,$i]
    }
}

proc edit_SLABS { id zmax } {
    global cmval GUI_DIR helvfont help_slabs_text

    catch { destroy .slabs$id }
    toplevel .slabs$id
    wm title .slabs$id "Edit SLABS, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NSLABS.
    frame .slabs$id.frm -bd 4
    set top .slabs$id.frm

    label $top.mainlabel -text "Slabs" -font $helvfont
    pack $top.mainlabel -pady 10

    # SLABS_macros.mortran:REPLACE {$MAX_N_$SLABS} WITH {30}
    # "maximum number of slabs"
    get_1_default SLABS $top "maximum number of slabs"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp3 $id,3

    frame $top.inp2 -bd 4
    label $top.inp2.label -text {Number of slabs}
    entry $top.inp2.inp -textvariable cmval($id,2) -width 8
    button $top.inp2.nextproc -text " Define layers >> "\
	    -command "define_nslabs $id"
    pack $top.inp2.label -anchor w -side left
    pack $top.inp2.inp -side left -padx 10
    pack $top.inp2.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp2 -side top -fill x


    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .slabs$id"\
	    -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command\
	    "help_gif .slabs$id.help {$help_slabs_text} help_slabs"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command\
	    "show_SLABS $id" -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -side left -padx 10
    pack $top.buts
    pack $top
}

proc define_nslabs { id } {
    global cmval helvfont values

    # allow a maximum of 3/window, the proceed to the next window.
    set nslab $cmval($id,2)
    if $nslab==0 {
	tk_dialog .slabs$id.no "No slabs" "You've selected 0 layers."\
		error 0 OK
	return
    }
    if [string compare $nslab {}]==0 {
	tk_dialog .slabs$id.no "Undefined" "Please define the number of layers."\
		error 0 OK
	return
    }
    set nwindow [expr $nslab/3]
    if [expr $nslab%3]>0 {
	incr nwindow
    }
    for {set i 1} {$i<=$nwindow} {incr i} {
	catch { destroy .slabs$id.baby$i }
	toplevel .slabs$id.baby$i
	set top .slabs$id.baby$i
	wm title $top "Define slabs, window $i"
	for {set ii 1} {$ii<=3} {incr ii} {
	    frame $top.f$i-$ii
	    set index [expr 3*($i-1)+$ii]
	    if $index>$nslab {break}

	    # define the 6 values that go with each slab

	    label $top.f$i-$ii.lab -text "Slab $index" -font $helvfont
	    pack $top.f$i-$ii.lab -side top

	    frame $top.f$i-$ii.f0
	    set w $top.f$i-$ii.f0
	    label $w.lab -text "Slab thickness (cm)"
	    pack $w.lab -anchor w -side left
	    entry $w.ent -textvariable cmval($id,4,0,$index) -width 10
	    pack $w.ent -anchor e -side right -fill x -expand true
	    pack $top.f$i-$ii.f0 -side top -fill x

	    if [string compare $cmval($id,4,5,$index) ""]==0 {
		set cmval($id,4,5,$index) $values(25)
	    }
	    add_ecut $top.f$i-$ii.f1 $id,4,1,$index
	    add_pcut $top.f$i-$ii.f2 $id,4,2,$index
	    add_dose $top.f$i-$ii.f3 $id,4,3,$index
	    add_latch $top.f$i-$ii.f4 $id,4,4,$index

	    frame $top.f$i-$ii.f5
	    set w $top.f$i-$ii.f5
	    button $w.b -text "?" -command "help_esave $w"
	    label $w.lab -text "ESAVE for this region"
	    pack $w.b -anchor w -side left
	    pack $w.lab -padx 5 -side left
	    entry $w.ent -textvariable cmval($id,4,5,$index) -width 10
	    pack $w.ent -anchor e -side right -fill x -expand true
	    pack $top.f$i-$ii.f5 -side top -fill x

	    add_material $top.f$i-$ii.f6 $id,4,6,$index

	    frame $top.f$i-$ii.sep -bd 4 -width 100 -height 2 -relief groove
	    pack $top.f$i-$ii.sep -pady 10 -fill x

	    pack $top.f$i-$ii -side top
	}
	button $top.okb -text "OK" -command "destroy .slabs$id.baby$i"\
		-relief groove -bd 8
	pack $top.okb -pady 10
	if [expr 3*($i-1)+$ii]>$nslab {break}
    }
}

proc write_SLABS {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), NSLABS"
    puts $fileid "$cmval($id,3), ZMIN"

    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	set str {}
	set str "$str$cmval($id,4,0,$i), "
	set str "$str$cmval($id,4,1,$i), "
	set str "$str$cmval($id,4,2,$i), "
	set str "$str$cmval($id,4,3,$i), "
	set str "$str$cmval($id,4,4,$i), "
	set str "$str$cmval($id,4,5,$i)"
	puts $fileid $str
	puts $fileid $cmval($id,4,6,$i)
    }
}

proc show_SLABS { id } {
    global cmval helvfont medium nmed xrange zrange cm_ticks

    catch { destroy .slabs$id.show }
    toplevel .slabs$id.show
    wm title .slabs$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange,zrange:
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_SLABS $id }]==1 {
	destroy .slabs$id.show
	tk_dialog .slabs$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .slabs$id.show.buts
    button .slabs$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .slabs$id.show xz" -relief groove -bd 8
    button .slabs$id.show.buts.print -text "Print..." -command\
	    "print_canvas .slabs$id.show.can 600 600" -relief groove -bd 8
    button .slabs$id.show.buts.done -text "Done" -command\
	    "destroy .slabs$id.show" -relief groove -bd 8
    pack .slabs$id.show.buts.range .slabs$id.show.buts.print\
	    .slabs$id.show.buts.done -side left -padx 10
    pack .slabs$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_SLABS { id } {
    global helvfont zrange xrange xscale zscale l m cm_ticks
    global nmed colorlist medium values cmval

    catch { destroy .slabs$id.show.can }

    # put the canvas up
    canvas .slabs$id.show.can -width 600 -height 600

    # now add on the stuff that's there, scaled to fit into 500x500 (allow
    # a x-border of 100 and z-border of 50 to show the scale)

    set width 450.0
    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    set l 100.0
    set m 50.0

    add_air $id .slabs$id.show.can $xrange(0) $zrange(0) $xscale $zscale $l $m

    add_SLABS $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .slabs$id.show.can

    coverup $l $m $width .slabs$id.show.can

    set curx 0
    set cury 0
    label .slabs$id.show.can.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .slabs$id.show.can create window [expr $l/2+$width] [expr 2*$m+$width]\
	    -window .slabs$id.show.can.xy
    bind .slabs$id.show.can <Motion> {
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
	    $xscale $zscale .slabs$id.show.can

    pack .slabs$id.show.can -side top -anchor n
}

proc add_SLABS { id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist nmed medium meds_used colornum

    # assign numbers to the media
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	# assign a default colour to all media, in case a medium is undefined.
	set med($i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,4,6,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }
    set x1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set x2 [expr ($cmval($id,0)-$xmin)*$xscale+$l]

    set tot [expr $cmval($id,3)-$zmin]
    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
	set y1 [expr $tot*$zscale+$m]
	set y2 [expr ($tot+$cmval($id,4,0,$i))*$zscale+$m]
	set color [lindex $colorlist $med($i)]
	$parent_w create rectangle $x1 $y1 $x2 $y2 -fill $color -outline {}
	set tot [expr $tot+$cmval($id,4,0,$i)]
    }

    pack $parent_w -side top -anchor n
}

proc add_air { id canvas xmin zmin xscale zscale l m } {
    global values nmed medium colorlist cmval colornum

    # assign a number to the "air" medium
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $values(2) $medium($j)]==0 {
	    set med(air) [min_nrc $j $colornum]
	    break
	}
    }
    set color [lindex $colorlist $med(air)]
    # First put a rectangle on that goes from top to bottom
    set x1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set x2 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set tot [expr [get_zmax [expr $id-1]] - $zmin]
    set y1 [expr $tot*$zscale+$m]
    set y2 [expr ([get_zmax $id]-$zmin)*$zscale+$m]
    $canvas create rectangle $x1 $y1 $x2 $y2 -fill $color -outline {}

}

proc coverup { l m width canvas } {
	# draw on four rectangles of grey to cover up any stray image that goes
	# over the axes
	$canvas create rectangle 0 0 $l [expr $m+$width] -fill white -outline {}
	$canvas create rectangle $l 0 [expr $l+$width+100] $m -fill white\
		-outline {}
	$canvas create rectangle [expr $l+$width] $m [expr $l+$width+100]\
		[expr $m+$width+150] -fill white -outline {}
	$canvas create rectangle 0 [expr $m+$width] [expr $l+$width]\
		[expr $m+$width+150] -fill white -outline {}
}


proc add_axes {pair n1 n2 l m width a b c d scale1 scale2 canvas} {
    global helvfont inp_base GUI_DIR

    # $xrange(0) $xrange(1) $zrange(0) $zrange(1) $xscale $zscale
    # Puts axes on the canvas.

    set n1 [expr $n1-1]
    set n2 [expr $n2-1]

    if [string compare $pair "xz"]==0 {
	# put on x markers on x-axis
	set dr [expr double(abs($b-$a))/double($n1)]
	for {set i 0} {$i<=$n1} {incr i} {
	    set r [expr ($i*$dr)*$scale1+$l]
	    $canvas create line $r [expr $m-5] $r [expr $m+5]
	    $canvas create text $r [expr $m-15] \
		    -text [format "%3.3f" [expr $a+$i*$dr]] -font $helvfont
	}
	# put on z markers on z-axis
	set dr [expr double(abs($d-$c))/double($n2)]
	for {set i 0} {$i<=$n2} {incr i} {
	    set r [expr ($i*$dr)*$scale2+$m]
	    $canvas create line [expr $l-5] $r [expr $l+5] $r
	    $canvas create text [expr $l-35] $r \
		    -text [format "%3.3f" [expr $c+$i*$dr]] -font $helvfont
	}
	# put on x/z axes
	# z down the left:
	$canvas create line $l $m $l [expr $m+$width+25] -arrow last
	$canvas create text $l [expr $m+$width+40] -text "z"
	# x from left to right at top:
	$canvas create line $l $m [expr $l+$width+25] $m -arrow last
	$canvas create text [expr $l+$width+40] $m -text "x"
    } elseif [string compare $pair "yz"]==0 {
	# put on y markers on y-axis
	set dr [expr abs($b-$a)/double($n1)]
	for {set i 0} {$i<=$n1} {incr i} {
	    set r [expr ($i*$dr)*$scale1+$l]
	    $canvas create line $r [expr $m-5] $r [expr $m+5]
	    $canvas create text $r [expr $m-15] \
		    -text [format "%3.3f" [expr $a+$i*$dr]] -font $helvfont
	}
	# put on z markers on z-axis
	set dr [expr abs($d-$c)/double($n2)]
	for {set i 0} {$i<=$n2} {incr i} {
	    set r [expr ($i*$dr)*$scale2+$m]
	    $canvas create line [expr $l-5] $r [expr $l+5] $r
	    $canvas create text [expr $l-35] $r \
		    -text [format "%3.3f" [expr $c+$i*$dr]] -font $helvfont
	}
	# put on y/z axes
	# z down the left:
	$canvas create line $l $m $l [expr $m+$width+25] -arrow last
	$canvas create text $l [expr $m+$width+40] -text "z"
	# x from left to right at top:
	$canvas create line $l $m [expr $l+$width+25] $m -arrow last
	$canvas create text [expr $l+$width+40] $m -text "y"
    } elseif [string compare $pair "xy"]==0 {
	# put on y markers on y-axis
	set dy [expr abs($d-$c)/double($n2)]
	for {set i 0} {$i<=$n2} {incr i} {
	    set r [expr ($i*$dy)*$scale2+$m]
	    $canvas create line [expr $l-5] $r [expr $l+5] $r
	    $canvas create text [expr $l-40] $r -text\
		    [format "%3.3f" [expr $c+$i*$dy]] -font $helvfont
	}
	# put on x markers on x-axis
	set dx [expr abs($b-$a)/double($n1)]
	for {set i 0} {$i<=$n1} {incr i} {
	    # add a marker on the x-axis:
	    set x2 [expr ($i*$dx)*$scale1+$l]
	    $canvas create line $x2 [expr $m-5] $x2 [expr $m+5]
	    $canvas create text $x2 [expr $m-15] -text\
		    [format "%3.3f" [expr $a+$i*$dx]] -font $helvfont
	}

	# put on x/y axes
	# x from left to the right at the bottom
	$canvas create line $l $m [expr $width+$l+25] $m -arrow last
	$canvas create text [expr $width+$l+35] $m -text "x"
	# y from bottom to (0,0):
	$canvas create line $l $m $l [expr $width+$m+25] -arrow last
	$canvas create text $l [expr $width+$m+30] -text "y"
    }

    # Add on the nrc logo, the date, and the input filename
    $canvas create bitmap 40 [expr $width+$m+80]\
	    -bitmap @$GUI_DIR/graphics/nrc-cnrc.xbm -anchor w
    set date [clock format [clock seconds] -format %D]
    #set date [exec date +%D]
    #set date {}

    # COMMENT OUT THE NEXT LINE TO REMOVE DATE/FILENAME FROM THE CANVAS
    catch { $canvas create text [expr $width+$l+30] [expr $width+$m+80] \
	    -text "$inp_base.egs4inp  $date" -font $helvfont -anchor e }
}

proc change_cm_range { id parent xyz } {

    global helvfont xrange yrange zrange cm_ticks cm_names cm_type

    set w $parent.range
    catch {destroy $w}
    toplevel $w

    if [string compare $xyz "xyz"]==0 {
	label $w.lab -text "Set the ticks and the minimum and\
		maximum x, y and z values" -font $helvfont
    } elseif [string compare $xyz "xz"]==0 {
	label $w.lab -text "Set the ticks and the minimum and maximum x\
		and z values" -font $helvfont
    } elseif [string compare $xyz "yz"]==0 {
	label $w.lab -text "Set the ticks and the minimum and maximum y\
		and z values" -font $helvfont
    } elseif [string compare $xyz "xy"]==0 {
	label $w.lab -text "Set the ticks and the minimum and maximum x\
		and y values" -font $helvfont
    }
    pack $w.lab -pady 10

    set icol 0
    frame $w.grid
    if [string first "x" $xyz]>=0 {
	label $w.grid.lx0 -text "Minimum x"
	label $w.grid.lx1 -text "Maximum x"
	label $w.grid.lx2 -text "# of x ticks"
	for {set i 0} {$i<3} {incr i} {
	    grid config $w.grid.lx$i -row $i -column $icol -sticky e
	}
	incr icol
	entry $w.grid.ex0 -textvariable xrange(0) -width 8
	entry $w.grid.ex1 -textvariable xrange(1) -width 8
	entry $w.grid.ex2 -textvariable cm_ticks($id,x) -width 8
	for {set i 0} {$i<3} {incr i} {
	    grid config $w.grid.ex$i -row $i -column $icol
	}
	incr icol
    }
    if [string first "y" $xyz]>=0 {
	label $w.grid.ly0 -text "Minimum y"
	label $w.grid.ly1 -text "Maximum y"
	label $w.grid.ly2 -text "# of y ticks"
	for {set i 0} {$i<3} {incr i} {
	    grid config $w.grid.ly$i -row $i -column $icol -sticky e
	}
	incr icol
	entry $w.grid.ey0 -textvariable yrange(0) -width 8
	entry $w.grid.ey1 -textvariable yrange(1) -width 8
	entry $w.grid.ey2 -textvariable cm_ticks($id,y) -width 8
	for {set i 0} {$i<3} {incr i} {
	    grid config $w.grid.ey$i -row $i -column $icol
	}
	incr icol
    }
    if [string first "z" $xyz]>=0 {
	label $w.grid.lz0 -text "Minimum z"
	label $w.grid.lz1 -text "Maximum z"
	label $w.grid.lz2 -text "# of z ticks"
	for {set i 0} {$i<3} {incr i} {
	    grid config $w.grid.lz$i -row $i -column $icol -sticky e
	}
	incr icol
	entry $w.grid.ez0 -textvariable zrange(0) -width 8
	entry $w.grid.ez1 -textvariable zrange(1) -width 8
	entry $w.grid.ez2 -textvariable cm_ticks($id,z) -width 8
	for {set i 0} {$i<3} {incr i} {
	    grid config $w.grid.ez$i -row $i -column $icol
	}
	incr icol
    }
    pack $w.grid -pady 10 -padx 5
    frame $w.buts
    button $w.buts.okb -text "OK" -command "destroy $w;\
	    draw_$cm_names($cm_type($id)) $id" -relief groove -bd 8
    button $w.buts.cancelb -text "Cancel" -command "destroy $w"\
	    -relief groove -bd 8
    pack $w.buts.okb $w.buts.cancelb -side left -padx 10
    pack $w.buts -pady 10
}

