
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: PYRAMIDS
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


proc init_PYRAMIDS { id } {
    global cmval pyrsym espl_plane

#start off with 0 potential e-/e+ splitting planes for DBS
    set espl_plane($id) 0

    set pyrsym($id) 0
    for {set i 0} {$i<2} {incr i} {
	set cmval($id,$i) {}
    }
    for {set j 0} {$j<2} {incr j} {
	set cmval($id,2,$j) {}
    }
    for {set i 1} {$i<=20} {incr i} {
	for {set j 0} {$j<12} {incr j} {
	    set cmval($id,3,$j,$i) {}
	}
    }
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,4,$i) {}
    }
    for {set i 1} {$i<=20} {incr i} {
	for {set j 0} {$j<4} {incr j} {
	    set cmval($id,5,$j,$i) {}
	}
	set cmval($id,6,$i) {}
	for {set j 0} {$j<4} {incr j} {
	    set cmval($id,7,$j,$i) {}
	}
	set cmval($id,8,$i) {}
    }
}

proc read_PYRAMIDS { fileid id } {
    global cmval GUI_DIR cm_ident espl_plane espl_z

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read PYRAMIDS $cm_ident($id).  The inputs don't begin where\
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

    # read npyramids, ifill 2,0 2,1
    gets $fileid data
    set data [get_val $data cmval $id,2,0]
    set data [get_val $data cmval $id,2,1]
    if { $cmval($id,2,1)==1 } {
	set cmval($id,2,1) "user-specified materials"
    } else {
	set cmval($id,2,1) "air"
    }

    # read stuff for each scraper
    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	gets $fileid data
        #only consider the back of each layer as a possible electron splitting plane
        incr espl_plane($id)
	for {set j 0} {$j<12} {incr j} {
	    set data [get_val $data cmval $id,3,$j,$i]
            set espl_z($id,$espl_plane($id)) $cmval($id,3,1,$i)
	}
    }

    # read ecut, pcut etc for surrounding (air) region
    gets $fileid data
    for {set j 0} {$j<4} {incr j} {
	set data [get_val $data cmval $id,4,$j]
    }

    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	if [string compare $cmval($id,2,1) "air"]!=0 {
	    # not air, user-specified filler - define.
	    gets $fileid data
	    for {set j 0} {$j<4} {incr j} {
		set data [get_val $data cmval $id,5,$j,$i]
	    }
	    gets $fileid data
	    set data [get_str $data cmval $id,6,$i]
	}
	# read the materials for the levels
	gets $fileid data
	for {set j 0} {$j<4} {incr j} {
	    set data [get_val $data cmval $id,7,$j,$i]
	}
	gets $fileid data
	set data [get_str $data cmval $id,8,$i]
    }
}

proc edit_PYRAMIDS { id zmax } {
    global cmval GUI_DIR values helvfont help_pyramids_text pyrsym

    catch { destroy .pyram$id }
    toplevel .pyram$id
    wm title .pyram$id "Edit PYRAMIDS CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NPYRAMIDS.
    frame .pyram$id.frm -bd 4
    set top .pyram$id.frm

    label $top.mainlabel -text "Pyramids" -font $helvfont
    pack $top.mainlabel -pady 10

    #   PYRAMIDS_macros.mortran:REPLACE{$MAX_N_$PYRAMIDS} WITH {10}
    get_1_default PYRAMIDS $top "maximum number of layers"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1

    frame $top.inp3 -bd 4
    label $top.inp3.label -text {Central & outer regions contain}
    set w $top.inp3
    menubutton $w.inp -text $cmval($id,2,1) -menu $w.inp.m -bd 1 \
	    -relief raised -indicatoron 1 -width 10
    menu $w.inp.m -tearoff no
    $w.inp.m add command -label air \
	    -command "set cmval($id,2,1) air; \
	    $w.inp configure -text air"
    $w.inp.m add command -label {user-specified materials} \
	    -command "set cmval($id,2,1) {user-specified materials};\
	    $w.inp configure -text {user-specified materials}"
    pack $top.inp3.label -anchor w -side left
    pack $top.inp3.inp -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x

    frame $top.inp4 -bd 10
    frame $top.inp4.frm
    label $top.inp4.frm.label -text {Number of layers}
    checkbutton $top.inp4.frm.pyrsym -text "use symmetry" \
	    -onvalue 1 -offvalue 0 -variable pyrsym($id)
    pack $top.inp4.frm.label $top.inp4.frm.pyrsym -anchor w
    entry $top.inp4.inp -textvariable cmval($id,2,0) -width 8
    button $top.inp4.nextproc -text " Define layers >> "\
	    -command "define_pyramids $id"
    pack $top.inp4.frm -anchor w -side left
    pack $top.inp4.inp -side left -padx 5 -anchor n
    pack $top.inp4.nextproc -anchor e -side right -fill x -expand 0
    pack $top.inp4 -side top -fill x

    label $top.airlabel -text "Air regions:" -font $helvfont
    pack $top.airlabel -anchor w -pady 10

    add_ecut $top.inp5 $id,4,0
    add_pcut $top.inp6 $id,4,1
    add_dose $top.inp7 $id,4,2
    add_latch $top.inp8 $id,4,3

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .pyram$id"\
	    -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command\
	    "help_gif .pyram$id.help {$help_pyramids_text} help_pyramids"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_PYRAMIDS $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -padx 10 -side left
    pack $top.buts -pady 5
    pack $top
}

proc define_pyramids { id } {
    global cmval helvfont help_pyramids_text nmed medium pyrsym med_per_column

    if $cmval($id,2,0)<1 {
	tk_dialog .pyram$id.error "Error" "The number of layers must be \
		greater than zero." error 0 OK
	return
    }

    catch { destroy .pyram$id.child }
    toplevel .pyram$id.child
    set top .pyram$id.child
    wm title $top "Define pyramids"

    label $top.f1lab -text "Geometry:" -font $helvfont
    pack $top.f1lab -anchor w
    frame $top.f1 -bd 10
    set w $top.f1

    if $pyrsym($id)==0 {

	label $w.lo -text "Layer #"
	label $w.l0 -text "Distance from front to reference plane"
	label $w.l1 -text "Distance from back to reference plane"
	label $w.l2 -text "Positive x dimension of opening at front (cm)"
	label $w.l3 -text "Positive x dimension of opening at back (cm)"
	label $w.l4 -text "Negative x dimension of opening at front (cm)"
	label $w.l5 -text "Negative x dimension of opening at back (cm)"
	label $w.l6 -text "Positive y dimension of opening at front (cm)"
	label $w.l7 -text "Positive y dimension of opening at back (cm)"
	label $w.l8 -text "Negative y dimension of opening at front (cm)"
	label $w.l9 -text "Negative y dimension of opening at back (cm)"
	label $w.l10 -text "Outer x edge (cm)"
	label $w.l11 -text "Outer y edge (cm)"

	grid configure $w.lo -row 0 -column 0 -sticky w
	for {set i 0} {$i<12} {incr i} {
	    grid configure $w.l$i -row [expr $i+1] -column 0 -sticky w
	}

	for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	    label $w.lab$i -text $i
	    grid configure $w.lab$i -row 0 -column $i
	    for {set j 0} {$j<12} {incr j} {
		entry $w.e$i-$j -textvariable cmval($id,3,$j,$i) -width 10
		grid configure $w.e$i-$j -row [expr $j+1] -column $i
	    }
	}

    } else {
	label $w.lo -text "Layer #"
	label $w.l0 -text "Distance from front to reference plane"
	label $w.l1 -text "Distance from back to reference plane"
	label $w.l2 -text "+/- x/y dimension of opening at front (cm)"
	label $w.l3 -text "+/- x/y dimension of opening at back (cm)"
	label $w.l10 -text "Outer x edge (cm)"
	label $w.l11 -text "Outer y edge (cm)"

	grid configure $w.lo -row 0 -column 0 -sticky w
	foreach i {0 1 2 3 10 11} {
	    grid configure $w.l$i -row [expr $i+1] -column 0 -sticky w
	}

	for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	    label $w.lab$i -text $i
	    grid configure $w.lab$i -row 0 -column $i
	    foreach j {0 1 2 3 10 11} {
		entry $w.e$i-$j -textvariable cmval($id,3,$j,$i) -width 10
		grid configure $w.e$i-$j -row [expr $j+1] -column $i
	    }
	}
    }

    pack $w

    if [string compare $cmval($id,2,1) "air"]!=0 {
	label $top.f2lab -text "Properties in central & outer regions:" \
		-font $helvfont
	pack $top.f2lab -anchor w
	set w $top.f2
	frame $w -bd 10

	label $w.lo -text "Layer #"
	label $w.l0 -text "Electron cutoff energy (default ECUTIN) (MeV)"
	label $w.l1 -text "Photon cutoff energy (default PCUTIN) (MeV)"
	label $w.l2 -text "Dose zone (0 for no scoring)"
	label $w.l3 -text "Associate with LATCH bit"
	label $w.l4 -text "Material"

	grid configure $w.lo -row 0 -column 0 -sticky w
	for {set i 0} {$i<5} {incr i} {
	    grid configure $w.l$i -row [expr $i+1] -column 0 -sticky w
	}

	for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	    label $w.lab$i -text $i
	    grid configure $w.lab$i -row 0 -column $i
	    for {set j 0} {$j<4} {incr j} {
		entry $w.e$i-$j -textvariable cmval($id,5,$j,$i) -width 20
		grid configure $w.e$i-$j -row [expr $j+1] -column $i
	    }
	    # material option menu
	    menubutton $w.e$i-4 -text $cmval($id,6,$i) -menu \
		    $w.e$i-4.m -bd 1 -relief raised -indicatoron 0 -width 20
	    menu $w.e$i-4.m -tearoff no
	    for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
		$w.e$i-4.m add command -label $medium($iopt) \
			-command "set_grid_material $w.e$i-4 $iopt $id,6,$i" -columnbreak [expr $iopt % $med_per_column==0]
	    }
	    grid configure $w.e$i-4 -row 5 -column $i
	}
	pack $w
    }

    label $top.f3lab -text "Properties in surrounding region:" -font $helvfont
    pack $top.f3lab -anchor w
    set w $top.f3
    frame $w -bd 10

    label $w.lo -text "Layer #"
    label $w.l0 -text "Electron cutoff energy (MeV)"
    label $w.l1 -text "Photon cutoff energy (MeV)"
    label $w.l2 -text "Dose zone (0 for no scoring)"
    label $w.l3 -text "Associate with LATCH bit"
    label $w.l4 -text "Material"

    grid configure $w.lo -row 0 -column 0 -sticky w
    for {set i 0} {$i<5} {incr i} {
	grid configure $w.l$i -row [expr $i+1] -column 0 -sticky w
    }

    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	label $w.lab$i -text $i
	grid configure $w.lab$i -row 0 -column $i
	for {set j 0} {$j<4} {incr j} {
	    entry $w.e$i-$j -textvariable cmval($id,7,$j,$i) -width 20
	    grid configure $w.e$i-$j -row [expr $j+1] -column $i
	}
	# material option menu
	menubutton $w.e$i-4 -text $cmval($id,8,$i) -menu \
		$w.e$i-4.m -bd 1 -relief raised -indicatoron 0 -width 20
	menu $w.e$i-4.m -tearoff no
	for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
	    $w.e$i-4.m add command -label $medium($iopt) \
		    -command "set_grid_material $w.e$i-4 $iopt $id,8,$i" -columnbreak [expr $iopt % $med_per_column==0]
	}
	grid configure $w.e$i-4 -row 5 -column $i
    }
    pack $w

    frame $top.b
    button $top.b.okb -text "OK" -command "check_symmetry_set_espl_planes $id; \
	    destroy .pyram$id.child"  -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
	    "help_gif .pyram$id.child.help {$help_pyramids_text} help_pyramids"\
	    -relief groove -bd 8
    pack $top.b.helpb $top.b.okb -side left -padx 10
    pack $top.b -pady 10
}

proc check_symmetry_set_espl_planes { id } {
    global pyrsym cmval espl_z espl_plane
    if $pyrsym($id)==1 {
	for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	    set cmval($id,3,4,$i) -$cmval($id,3,2,$i)
	    set cmval($id,3,5,$i) -$cmval($id,3,3,$i)
	    set cmval($id,3,6,$i) $cmval($id,3,2,$i)
	    set cmval($id,3,7,$i) $cmval($id,3,3,$i)
	    set cmval($id,3,8,$i) -$cmval($id,3,2,$i)
	    set cmval($id,3,9,$i) -$cmval($id,3,3,$i)
	}
    }

    set espl_plane($id) $cmval($id,2,0)
    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
       set espl_z($id,$i) $cmval($id,3,1,$i)
    }
}

proc set_grid_material { w iopt index } {
    global cmval medium
    set cmval($index) $medium($iopt)
    $w configure -text $cmval($index)
}


proc write_PYRAMIDS {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)

    if [string compare $cmval($id,2,1) air]==0 {
	puts $fileid "$cmval($id,2,0), 0, #LAYERS, AIR OUTSIDE"
    } else {
	puts $fileid "$cmval($id,2,0), 1, #LAYERS, SPECIFY MATERIAL OUTSIDE"
    }

    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	set str {}
	for {set j 0} {$j<12} {incr j} {
	    set str "$str$cmval($id,3,$j,$i), "
	}
	puts $fileid $str
    }
    set str {}
    for {set j 0} {$j<4} {incr j} {
	set str "$str$cmval($id,4,$j), "
    }
    puts $fileid "$str ECUT ETC. FOR AIR"

    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	if [string compare $cmval($id,2,1) air]!=0 {
	    # user-specified, must write extra
	    set str {}
	    for {set j 0} {$j<4} {incr j} {
		set str "$str$cmval($id,5,$j,$i), "
	    }
	    puts $fileid $str
	    puts $fileid $cmval($id,6,$i)
	}
	set str {}
	for {set j 0} {$j<4} {incr j} {
	    set str "$str$cmval($id,7,$j,$i), "
	}
	puts $fileid $str
	puts $fileid $cmval($id,8,$i)
    }
}


proc show_PYRAMIDS { id } {

    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks

    catch { destroy .pyram$id.show }
    toplevel .pyram$id.show
    wm title .pyram$id.show "Preview"

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

    if [catch { draw_PYRAMIDS $id }]==1 {
	destroy .pyram$id.show
	tk_dialog .pyram$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .pyram$id.show.buts
    button .pyram$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .pyram$id.show xyz" -relief groove -bd 8
    button .pyram$id.show.buts.print1 -text "Print xz..." -command\
	    "print_canvas .pyram$id.show.can.one 500 500" -relief groove -bd 8
    button .pyram$id.show.buts.print2 -text "Print yz..." -command\
	    "print_canvas .pyram$id.show.can.two 500 500" -relief groove -bd 8
    button .pyram$id.show.buts.done -text "Done" -command\
	    "destroy .pyram$id.show" -relief groove -bd 8
    pack .pyram$id.show.buts.range .pyram$id.show.buts.print1\
	    .pyram$id.show.buts.print2\
	    .pyram$id.show.buts.done -side left -padx 10
    pack .pyram$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_PYRAMIDS { id } {
    global helvfont zrange yrange xrange xscale yscale zscale l m
    global cm_ticks

    catch { destroy .pyram$id.show.can }

    # put the canvas up
    set ncan 2
    set width 350.0
    set canwidth 500.0
    canvas .pyram$id.show.can -width [expr $ncan*($width+150.0)]\
	    -height [expr ($width+150.0)]

    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    # left canvas, x/z cross-section
    set l 100.0
    set m 50.0

    canvas .pyram$id.show.can.one -width $canwidth -height $canwidth

    add_air $id .pyram$id.show.can.one $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_PYRAMIDS $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .pyram$id.show.can.one

    coverup $l $m $width .pyram$id.show.can.one

    set curx 0
    set cury 0
    label .pyram$id.show.can.one.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .pyram$id.show.can.one create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .pyram$id.show.can.one.xy
    bind .pyram$id.show.can.one <Motion> {
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
	    $xscale $zscale .pyram$id.show.can.one

    .pyram$id.show.can create window [expr $canwidth/2.0]\
	    [expr $canwidth/2.0] -window .pyram$id.show.can.one

    # right canvas, YZ cross-section

    canvas .pyram$id.show.can.two -width $canwidth -height $canwidth

    add_air $id .pyram$id.show.can.two $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    add_PYRAMIDS_yz $id $yscale $zscale $yrange(0) $zrange(0)\
	    $l $m .pyram$id.show.can.two

    coverup $l $m $width .pyram$id.show.can.two

    label .pyram$id.show.can.two.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .pyram$id.show.can.two create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .pyram$id.show.can.two.xy
    bind .pyram$id.show.can.two <Motion> {
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
	    $yscale $zscale .pyram$id.show.can.two

    .pyram$id.show.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .pyram$id.show.can.two

    pack .pyram$id.show.can -side top -anchor n
}

proc add_PYRAMIDS {id xscale zscale xmin zmin l m parent_w} {
    global colorlist nmed medium cmval meds_used values colornum

    # assign numbers to the media, and air
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $values(2) $medium($j)]==0 {
	    set med(air) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    if [string compare $cmval($id,2,1) air]!=0 {
	# user-specified openings
	for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	    set med(open,$i) $colornum
	    for {set j 0} {$j<=$nmed} {incr j} {
		if [string compare $cmval($id,6,$i) $medium($j)]==0 {
		    set med(open,$i) [min_nrc $j $colornum]
		    set meds_used($j) 1
		    break
		}
	    }
	}
    } else {
	for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	    set med(open,$i) $med(air)
	}
    }
    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	set med(sur,$i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,8,$i) $medium($j)]==0 {
		set med(sur,$i) [min_nrc $j $colornum]
		set meds_used($j) 1
		break
	    }
	}
    }

    # left canvas, xz cross-section
    # rectangle for air regions
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z(1) [expr ($cmval($id,3,0,1)-$zmin)*$zscale+$m]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z(2) [expr ($cmval($id,3,1,$cmval($id,2,0))-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

    # polygons for each layer

    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	set z(1) [expr ($cmval($id,3,0,$i)-$zmin)*$zscale+$m]
	set z(2) [expr ($cmval($id,3,1,$i)-$zmin)*$zscale+$m]

	# 1st polygon is always a rectangle
	set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	set x(2) [expr (-$cmval($id,3,10,$i)-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med(open,$i)]
	$parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color\
		-outline {}

	# second is a poly
	set x(1) [expr (-$cmval($id,3,10,$i)-$xmin)*$xscale+$l]
	set x(2) [expr ($cmval($id,3,4,$i)-$xmin)*$xscale+$l]
	set x(3) [expr ($cmval($id,3,5,$i)-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med(sur,$i)]
	$parent_w create poly $x(1) $z(1) $x(1) $z(2) $x(3) $z(2)\
		$x(2) $z(1) -fill $color -outline {}

	# third is central chunk, poly
	set x(1) [expr ($cmval($id,3,2,$i)-$xmin)*$xscale+$l]
	set x(2) [expr ($cmval($id,3,3,$i)-$xmin)*$xscale+$l]
	set x(3) [expr ($cmval($id,3,4,$i)-$xmin)*$xscale+$l]
	set x(4) [expr ($cmval($id,3,5,$i)-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med(open,$i)]
	$parent_w create poly $x(1) $z(1) $x(2) $z(2) $x(4) $z(2)\
		$x(3) $z(1) -fill $color -outline {}

	# 4th is a poly
	set x(1) [expr ($cmval($id,3,10,$i)-$xmin)*$xscale+$l]
	set x(2) [expr ($cmval($id,3,2,$i)-$xmin)*$xscale+$l]
	set x(3) [expr ($cmval($id,3,3,$i)-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med(sur,$i)]
	$parent_w create poly $x(1) $z(1) $x(1) $z(2) $x(3) $z(2)\
		$x(2) $z(1) -fill $color -outline {}

	# last is the rectangle on the right
	set x(1) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	set x(2) [expr ($cmval($id,3,10,$i)-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med(open,$i)]
	$parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color\
		-outline {}
    }

}

proc add_PYRAMIDS_yz {id yscale zscale ymin zmin l m parent_w} {
    global cmval colorlist medium nmed values colornum

    # right canvas, YZ cross-section
    # assign numbers to the media, and air
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $values(2) $medium($j)]==0 {
	    set med(air) [min_nrc $j $colornum]
	    break
	}
    }
    if [string compare $cmval($id,2,1) air]!=0 {
	# user-specified openings
	for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	    set med(open,$i) $colornum
	    for {set j 0} {$j<=$nmed} {incr j} {
		if [string compare $cmval($id,6,$i) $medium($j)]==0 {
		    set med(open,$i) [min_nrc $j $colornum]
		    break
		}
	    }
	}
    } else {
	for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	    set med(open,$i) $med(air)
	}
    }
    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	set med(sur,$i) $colornum
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,8,$i) $medium($j)]==0 {
		set med(sur,$i) [min_nrc $j $colornum]
		break
	    }
	}
    }

    # rectangle for air regions
    set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set z(1) [expr ($cmval($id,3,0,1)-$zmin)*$zscale+$m]
    set y(2) [expr ($cmval($id,0)-$ymin)*$yscale+$l]
    set z(2) [expr ($cmval($id,3,1,$cmval($id,2,0))-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(air)]
    $parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color -outline {}

    # polygons for each layer

    for {set i 1} {$i<=$cmval($id,2,0)} {incr i} {
	set z(1) [expr ($cmval($id,3,0,$i)-$zmin)*$zscale+$m]
	set z(2) [expr ($cmval($id,3,1,$i)-$zmin)*$zscale+$m]

	# 1st polygon is always a rectangle
	set y(1) [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
	set y(2) [expr (-$cmval($id,3,11,$i)-$ymin)*$yscale+$l]
	set color [lindex $colorlist $med(open,$i)]
	$parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color\
		-outline {}

	# second is a poly
	set y(1) [expr (-$cmval($id,3,11,$i)-$ymin)*$yscale+$l]
	set y(2) [expr ($cmval($id,3,8,$i)-$ymin)*$yscale+$l]
	set y(3) [expr ($cmval($id,3,9,$i)-$ymin)*$yscale+$l]
	set color [lindex $colorlist $med(sur,$i)]
	$parent_w create poly $y(1) $z(1) $y(1) $z(2) $y(3) $z(2)\
		$y(2) $z(1) -fill $color -outline {}

	# third is central chunk, poly
	set y(1) [expr ($cmval($id,3,6,$i)-$ymin)*$yscale+$l]
	set y(2) [expr ($cmval($id,3,7,$i)-$ymin)*$yscale+$l]
	set y(3) [expr ($cmval($id,3,8,$i)-$ymin)*$yscale+$l]
	set y(4) [expr ($cmval($id,3,9,$i)-$ymin)*$yscale+$l]
	set color [lindex $colorlist $med(open,$i)]
	$parent_w create poly $y(1) $z(1) $y(2) $z(2) $y(4) $z(2)\
		$y(3) $z(1) -fill $color -outline {}

	# 4th is a poly
	set y(1) [expr ($cmval($id,3,11,$i)-$ymin)*$yscale+$l]
	set y(2) [expr ($cmval($id,3,6,$i)-$ymin)*$yscale+$l]
	set y(3) [expr ($cmval($id,3,7,$i)-$ymin)*$yscale+$l]
	set color [lindex $colorlist $med(sur,$i)]
	$parent_w create poly $y(1) $z(1) $y(1) $z(2) $y(3) $z(2)\
		$y(2) $z(1) -fill $color -outline {}

	# last is the rectangle on the right
	set y(1) [expr ($cmval($id,0)-$ymin)*$yscale+$l]
	set y(2) [expr ($cmval($id,3,11,$i)-$ymin)*$yscale+$l]
	set color [lindex $colorlist $med(open,$i)]
	$parent_w create rectangle $y(1) $z(1) $y(2) $z(2) -fill $color\
		-outline {}
    }
}


