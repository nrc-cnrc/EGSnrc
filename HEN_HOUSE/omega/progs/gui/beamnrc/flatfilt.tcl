
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: FLATFILT
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


proc init_FLATFILT { id } {
    global cmval espl_plane

#start off with 0 potential e-/e+ splitting planes for DBS
    set espl_plane($id) 0

    for {set i 0} {$i<4} {incr i} {
	set cmval($id,$i) {}
    }
    for {set i 1} {$i<=100} {incr i} {
	set cmval($id,4,0,$i) {}
	set cmval($id,4,1,$i) {}
	for {set j 1} {$j<=100} {incr j} {
	    set cmval($id,5,$i,$j) {}
	    set cmval($id,6,$i,$j) {}
	}
    }
    for {set i 1} {$i<=100} {incr i} {
	for {set j 1} {$j<=100} {incr j} {
	    set cmval($id,7,0,$i,$j) {}
	    set cmval($id,8,$i,$j) {}
	}
    }
}

proc read_FLATFILT { fileid id } {
    global cmval GUI_DIR cm_ident espl_plane espl_z

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read FLATFILT $cm_ident($id).  The inputs don't begin where\
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

    # read zmin 2
    gets $fileid data
    set data [get_val $data cmval $id,2]
    set espl_plane($id) 1
    set espl_z($id,$espl_plane($id)) $cmval($id,2)

    # read nfilt 3
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # read no. cones and thickness for each layer
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	gets $fileid data
	set data [get_val $data cmval $id,4,0,$i]
	set data [get_val $data cmval $id,4,1,$i]
        set planem1 $espl_plane($id)
        incr espl_plane($id)
        set espl_z($id,$espl_plane($id)) [expr $espl_z($id,$planem1)+$cmval($id,4,1,$i)]

	# Read top radius of cone j in layer i
	gets $fileid data
	for {set j 1} {$j<=$cmval($id,4,0,$i)} {incr j} {
	    set data [get_val $data cmval $id,5,$i,$j]
	}
	# Read bottom radius of cone j in layer i
	gets $fileid data
	for {set j 1} {$j<=$cmval($id,4,0,$i)} {incr j} {
	    set data [get_val $data cmval $id,6,$i,$j]
	}
    }

    # For each cone in each filter, read 4 values, then the medium
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	for {set j 1} {$j<=[expr $cmval($id,4,0,$i)+1]} {incr j} {
	    gets $fileid data
	    for {set k 0} {$k<=3} {incr k} {
		set data [get_val $data cmval $id,7,$k,$i,$j]
	    }
	    gets $fileid data
	    set data [get_str $data cmval $id,8,$i,$j]
	}
    }
}

proc edit_FLATFILT { id zmax } {
    global cmval GUI_DIR omega helvfont help_flatfilt_text

    catch { destroy .flatf$id }
    toplevel .flatf$id
    wm title .flatf$id "Edit FLATFILT, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NFLATFILT.
    frame .flatf$id.frm -bd 4
    set top .flatf$id.frm

    label $top.mainlabel -text "Flatfilt" -font $helvfont
    pack $top.mainlabel -pady 10

    #FLATFILT_macros.mortran:REPLACE {$MAX_N_$FLATFILT} WITH {80}
    #FLATFILT_macros.mortran:REPLACE {$MAX_N_SC_$FLATFILT} WITH {80}
    get_2_defaults FLATFILT $top MAX_N_SC "maximum number of cones per layer"\
	    MAX_N "maximum number of layers"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_rad $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp2 $id,2

    frame $top.inp3 -bd 4
    label $top.inp3.label -text {Number of layers}
    entry $top.inp3.inp -textvariable cmval($id,3) -width 8
    button $top.inp3.nextproc -text " Define layers >> "\
	    -command "define_layers $id"
    pack $top.inp3.label -anchor w -side left
    pack $top.inp3.inp -side left -padx 5
    pack $top.inp3.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x

    frame $top.buts -bd 4
    button $top.buts.okb -text "OK" -command "destroy .flatf$id ; \
            set_espl_planes_flatfilt $id" -relief groove -bd 8
    button $top.buts.helpb -text "Help" -command\
	    "help_gif .flatf$id.help {$help_flatfilt_text} help_flatfilt"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_FLATFILT $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -side left -padx 10
    pack $top.buts -pady 10
    pack $top
}

proc set_espl_planes_flatfilt { id } {

#sets the no. and Z values of the potential e- splitting planes when
#flatfilt is edited

    global cmval espl_z espl_plane

    if {$cmval($id,3)>0} {
       set espl_plane($id) [expr $cmval($id,3)+1]
       for {set i 1} {$i<=$espl_plane($id)} {incr i} {
          if {$i==1} {
            set espl_z($id,$i) $cmval($id,2)
          } else {
            set espl_z($id,$i) [expr $espl_z($id,[expr $i-1])+ \
             $cmval($id,4,1,[expr $i-1])]
          }
       }
    }
}


proc define_layers { id } {
    global cmval helvfont

    set nlayer $cmval($id,3)
    set nwindow [expr $nlayer/6]
    if [expr $nlayer%6]>0 {
	incr nwindow
    }
    if { $nwindow>1 } {
	# group the layers and put buttons for editing them on a window
	catch { destroy .flatf$id.child }
	toplevel .flatf$id.child -bd 10
	set top .flatf$id.child
	wm title $top "Define layers"
	for {set i 1} {$i<=$nwindow} {incr i} {
	    set start [expr 6*($i-1) + 1]
	    if $i<$nwindow {
		set end [expr $start + 5]
	    } else {
		set end $nlayer
	    }
	    button $top.b$i -text "Edit layers $start to $end" -command \
		    "define_layer_window .flatf$id.child $id $i $start $end"
	    set_flatfilt_button_color $id $i $start $end
	    pack $top.b$i -side top -pady 5 -fill x
	}
	button $top.closeb -text "Close" -command "destroy $top"\
		-relief groove -bd 8
	pack $top.closeb -side top -pady 5

    } else {
	# put the layers on one window
	define_layer_window .flatf$id $id 1 1 $nlayer
    }
}

proc set_flatfilt_button_color { id ilayer start end } {
    global cmval
    # ilayer is the layer to be checked.
    # All of the cones contained in this group must be checked!
    # (from start to end)
    set buttoncolor($ilayer) black
    # if nlayers or thickness are not done, set button to red
    for {set k $start} {$k<=$end} {incr k} {
	if { [catch {expr $cmval($id,4,0,$k)}]==1 || \
		[catch {expr $cmval($id,4,1,$k)}]==1 } {
	    set buttoncolor($ilayer) red
	}
	# if top or bottom radii of cones are not done, red:
	for {set j 1} {$j<=$cmval($id,4,0,$k)} {incr j} {
	    if { [catch {expr $cmval($id,5,$k,$j)}]==1 || \
		    [catch {expr $cmval($id,6,$k,$j)}]==1 } {
		set buttoncolor($ilayer) red
	    }
	}
	#if material is not set, set to red
	for {set j 1} {$j<=$cmval($id,4,0,$k)} {incr j} {
	    if [string compare $cmval($id,8,$k,$j) {}]==0 {
		set buttoncolor($ilayer) red
	    }
	}
    }
    .flatf$id.child.b$ilayer configure -fg $buttoncolor($ilayer)
}

proc set_flatfilt_button_color2 { id w ilayer start end } {
    global cmval
    # icone is the cone to be checked.
    # All of the cones contained in this group must be checked!
    # (from start to end)
    set buttoncolor black
    # if ncones or thickness are not done, set button to red
    # if top or bottom radii of cones are not done, red:
    for {set k $start} {$k<=$end} {incr k} {
	if { [catch {expr $cmval($id,5,$ilayer,$k)}]==1 || \
		[catch {expr $cmval($id,6,$ilayer,$k)}]==1 } {
	    set buttoncolor red
	}
	#if material is not set, set to red
	if [string compare $cmval($id,8,$ilayer,$k) {}]==0 {
		set buttoncolor red
	}
    }
    $w configure -fg $buttoncolor
}


proc define_layer_window { parent id i start end } {
    global cmval helvfont

    # Allow 6 layers per column

    catch { destroy $parent.baby$i }
    toplevel $parent.baby$i -bd 6
    set top $parent.baby$i
    wm title $top "Define layers, window $i"
    set index [expr $start - 1]
    for {set ii $start} {$ii<=$end} {incr ii} {
	frame $top.f$i-$ii
	incr index

	label $top.f$i-$ii.lab -text "Layer $index" -font $helvfont
	pack $top.f$i-$ii.lab -side top

	frame $top.f$i-$ii.f0
	set w $top.f$i-$ii.f0
	label $w.lab -text "Layer thickness"
	entry $w.ent -textvariable cmval($id,4,1,$index) -width 8
	pack $w.lab -anchor w -side left
	pack $w.ent -anchor e -side right -fill x -expand true
	pack $w -side top -fill x

	frame $top.f$i-$ii.f1
	set w $top.f$i-$ii.f1
	label $w.lab -text "Number of conical sections in this layer"
	entry $w.ent -textvariable cmval($id,4,0,$index) -width 8
	button $w.nextproc -text " Define cones >> "\
		-command "define_cones $top $id $index"
	pack $w.lab -anchor w -side left
	pack $w.ent -side left -padx 5
	pack $w.nextproc -anchor e -side right -fill x -expand true
	pack $w -side top -fill x

	frame $top.f$i-$ii.sep -bd 4 -width 100 -height 2 -relief groove
	pack $top.f$i-$ii.sep -side top -fill x -pady 10

	pack $top.f$i-$ii -side top
    }
    if {$parent == ".flatf$id"} {
	button $top.okb -text "OK" -command "destroy $parent.baby$i" \
		-relief groove -bd 8
    } else {
	button $top.okb -text "OK" -command "destroy $parent.baby$i; \
		set_flatfilt_button_color $id $i $start $end"\
		-relief groove -bd 8
    }
    pack $top.okb -pady 10
}

proc define_cones { parent id ilayer } {
    global cmval helvfont

    set ncone $cmval($id,4,0,$ilayer)
    set nwindow [expr $ncone/3]
    if [expr $ncone%3]>0 {
	incr nwindow
    }
    if { $nwindow>1 } {
	# group the cones and put buttons for editing them on a window
	catch { destroy $parent.child }
	toplevel $parent.child -bd 10
	set top $parent.child
	wm title $top "Define cones for layer $ilayer"
	for {set i 1} {$i<=$nwindow} {incr i} {
	    set start [expr 3*($i-1) + 1]
	    if $i<$nwindow {
		set end [expr $start + 2]
	    } else {
		set end $ncone
	    }
	    button $top.b$i -text "Edit cones $start to $end" -command \
		    "define_cone_window $parent.child $id $ilayer $i $start $end"
	    set_flatfilt_button_color2 $id $parent.child.b$i $ilayer $start $end
	    pack $top.b$i -side top -pady 5 -fill x
	}
	button $top.closeb -text "Close" -command "destroy $top"\
		-relief groove -bd 8
	pack $top.closeb -side top -pady 5

    } else {
	# put the cones on one window
	define_cone_window $parent $id $ilayer 1 1 $ncone
    }
}

proc define_cone_window { parent id ilayer iwin start end } {
    global cmval values nmed medium med_per_column

    set ncone $cmval($id,4,0,$ilayer)

    catch { destroy $parent.cone }
    toplevel $parent.cone -bd 6
    set top $parent.cone
    wm title $top "Cones $start to $end, window $iwin"

    frame $top.f -bd 4

    # Make a grid of geometry then a grid of properties

    label $top.f.lab -text "Layer $ilayer, cones $start to $end"
    pack $top.f.lab -side top

    # Geometry
    set w $top.f
    label $w.geomlab -text "Define conical section geometries and material \
	    properties:"
    pack $w.geomlab -anchor w -pady 10

    frame $w.pf

    label $w.pf.ncon -text "Cone #"
    label $w.pf.l0 -text "Electron cutoff energy (default ECUTIN) (MeV)"
    label $w.pf.l1 -text "Photon cutoff energy (default PCUTIN) (MeV)"
    label $w.pf.l2 -text "Dose zone (0 for no scoring)"
    label $w.pf.l3 -text "Associate with LATCH bit"
    label $w.pf.l4 -text "Material"
    label $w.pf.l5 -text "Top radius"
    label $w.pf.l6 -text "Bottom radius"

    grid configure $w.pf.ncon -column 0 -row 0 -sticky w
    for {set j 0} {$j<=6} {incr j} {
	grid configure $w.pf.l$j -column 0 -row [expr $j+1] -sticky w
    }

    for {set j $start} {$j<=$end} {incr j} {
	label $w.pf.ncon$j -text $j
	entry $w.pf.etop$j -textvariable cmval($id,5,$ilayer,$j) -width 20
	entry $w.pf.ebot$j -textvariable cmval($id,6,$ilayer,$j) -width 20

	entry $w.pf.ee$j -textvariable cmval($id,7,0,$ilayer,$j) -width 20
	entry $w.pf.ep$j -textvariable cmval($id,7,1,$ilayer,$j) -width 20
	entry $w.pf.ed$j -textvariable cmval($id,7,2,$ilayer,$j) -width 20
	entry $w.pf.el$j -textvariable cmval($id,7,3,$ilayer,$j) -width 20
	# material option menu
	menubutton $w.pf.em$j -text $cmval($id,8,$ilayer,$j) -menu \
		$w.pf.em$j.m -bd 1 -relief raised -indicatoron 0 -width 20
	menu $w.pf.em$j.m -tearoff no
	for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
	    $w.pf.em$j.m add command -label $medium($iopt) \
		    -command "set_grid_material $w.pf.em$j $iopt $id,8,$ilayer,$j" -columnbreak [expr $iopt % $med_per_column==0]
	}
	if [string compare $cmval($id,7,0,$ilayer,$j) ""]==0 {
	    set cmval($id,7,0,$ilayer,$j) $values(ecut)
	}
	if [string compare $cmval($id,7,1,$ilayer,$j) ""]==0 {
	    set cmval($id,7,1,$ilayer,$j) $values(pcut)
	}
	grid configure $w.pf.ncon$j -column $j -row 0
	grid configure $w.pf.ee$j -column $j -row 1
	grid configure $w.pf.ep$j -column $j -row 2
	grid configure $w.pf.ed$j -column $j -row 3
	grid configure $w.pf.el$j -column $j -row 4
	grid configure $w.pf.em$j -column $j -row 5
	grid configure $w.pf.etop$j -column $j -row 6
	grid configure $w.pf.ebot$j -column $j -row 7
    }
    # For the outside, if it's the last one...
    if $end==$ncone {
	set j [expr $ncone+1]
	label $w.pf.ncon$j -text "Outer region"
	entry $w.pf.ee$j -textvariable cmval($id,7,0,$ilayer,$j)
	entry $w.pf.ep$j -textvariable cmval($id,7,1,$ilayer,$j)
	entry $w.pf.ed$j -textvariable cmval($id,7,2,$ilayer,$j)
	entry $w.pf.el$j -textvariable cmval($id,7,3,$ilayer,$j)
	label $w.pf.etop$j -text $cmval($id,0)
	label $w.pf.ebot$j -text $cmval($id,0)

	# material option menu
	menubutton $w.pf.em$j -text $cmval($id,8,$ilayer,$j) -menu \
		$w.pf.em$j.m -bd 1 -relief raised -indicatoron 0 -width 20
	menu $w.pf.em$j.m -tearoff no
	for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
	    $w.pf.em$j.m add command -label $medium($iopt) \
		    -command "set_grid_material $w.pf.em$j $iopt $id,8,$ilayer,$j" -columnbreak [expr $iopt % $med_per_column==0]
	}
	if [string compare $cmval($id,7,0,$ilayer,$j) ""]==0 {
	    set cmval($id,7,0,$ilayer,$j) $values(ecut)
	}
	if [string compare $cmval($id,7,1,$ilayer,$j) ""]==0 {
	    set cmval($id,7,1,$ilayer,$j) $values(pcut)
	}
	grid configure $w.pf.ncon$j -column $j -row 0
	grid configure $w.pf.ee$j -column $j -row 1
	grid configure $w.pf.ep$j -column $j -row 2
	grid configure $w.pf.ed$j -column $j -row 3
	grid configure $w.pf.el$j -column $j -row 4
	grid configure $w.pf.em$j -column $j -row 5
	grid configure $w.pf.etop$j -column $j -row 6 -sticky w
	grid configure $w.pf.ebot$j -column $j -row 7 -sticky w
    }

    pack $w.pf -side top -fill x
    pack $w

    if $ncone>3 {
	button $top.okb -text "OK" -command "destroy $parent.cone; \
		set_flatfilt_button_color2 $id $parent.b$iwin $ilayer\
		$start $end" -relief groove -bd 8
    } else {
	button $top.okb -text "OK" -command "destroy $parent.cone" \
		-relief groove -bd 8
    }
    pack $top.okb -pady 10
}

proc write_FLATFILT {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2), ZMIN"
    puts $fileid "$cmval($id,3), NUMBER OF LAYERS"

    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set str {}
	set str "$str$cmval($id,4,0,$i), "
	set str "$str$cmval($id,4,1,$i), # CONES, ZTHICK OF LAYER $i"
	puts $fileid $str
	set str {}
	for {set j 1} {$j<=$cmval($id,4,0,$i)} {incr j} {
	    set str "$str$cmval($id,5,$i,$j), "
	}
	puts $fileid $str
	set str {}
	for {set j 1} {$j<=$cmval($id,4,0,$i)} {incr j} {
	    set str "$str$cmval($id,6,$i,$j), "
	}
	puts $fileid $str
    }
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	for {set j 1} {$j<=[expr $cmval($id,4,0,$i)+1]} {incr j} {
	    set str {}
	    for {set k 0} {$k<4} {incr k} {
		set str "$str$cmval($id,7,$k,$i,$j), "
	    }
	    puts $fileid $str
	    puts $fileid $cmval($id,8,$i,$j)
	}
    }
}


proc show_FLATFILT { id } {
    global cmval medium nmed xrange zrange med cm_ticks

    catch { destroy .flatf$id.show }
    toplevel .flatf$id.show
    wm title .flatf$id.show "Preview"

    set cm_ticks($id,x) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange,zrange:
    catch {
	set xrange(0) -$cmval($id,0)
	set xrange(1) $cmval($id,0)
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_FLATFILT $id }]==1 {
	destroy .flatf$id.show
	tk_dialog .flatf$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .flatf$id.show.buts
    button .flatf$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .flatf$id.show xz" -relief groove -bd 8
    button .flatf$id.show.buts.print -text "Print..." -command\
	    "print_canvas .flatf$id.show.can 600 600" -relief groove -bd 8
    button .flatf$id.show.buts.done -text "Done" -command\
	    "destroy .flatf$id.show" -relief groove -bd 8
    pack .flatf$id.show.buts.range .flatf$id.show.buts.print\
	    .flatf$id.show.buts.done -side left -padx 10
    pack .flatf$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_FLATFILT {id} {

    global xrange zrange helvfont xscale zscale l m cm_ticks

    catch {destroy .flatf$id.show.can}

    # put the canvas up
    canvas .flatf$id.show.can -width 600 -height 600

    # now add on the stuff that's there, scaled to fit into 500x500 (allow
    # a x-border of 100 and z-border of 50 to show the scale)

    set width 450.0

    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]
    set l 100.0
    set m 50.0

    add_air $id .flatf$id.show.can $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    add_FLATFILT $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .flatf$id.show.can

    coverup $l $m $width .flatf$id.show.can

    set curx 0
    set cury 0
    label .flatf$id.show.can.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .flatf$id.show.can create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .flatf$id.show.can.xy
    bind .flatf$id.show.can <Motion> {
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
	    $xscale $zscale .flatf$id.show.can

    pack .flatf$id.show.can -anchor n -side top
}

proc add_FLATFILT {id xscale zscale xmin zmin l m parent_w} {
    global cmval colorlist nmed medium meds_used colornum

    # assign numbers to the media
    # for each cone in each layer, get medium.
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	# i is the layer
	for {set j 1} {$j<=[expr $cmval($id,4,0,$i)+1]} {incr j} {
	    # cone j in layer i
	    set med($i,$j) $colornum
	    for {set k 0} {$k<=$nmed} {incr k} {
		if [string compare $cmval($id,8,$i,$j) $medium($k)]==0 {
		    set med($i,$j) [min_nrc $k $colornum]
		    set meds_used($k) 1
		    break
		}
	    }
	}
    }
    set tot [expr -$zmin+$cmval($id,2)]
    set rmax $cmval($id,0)
    # for each layer, put each cone on the right then on the left with polygons
    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
	set thick $cmval($id,4,1,$i)

	set y1 [expr $tot*$zscale+$m]
	set y2 [expr ($tot+$thick)*$zscale+$m]

	set rtlast 0.0
	set rblast 0.0

	for {set j 1} {$j<=$cmval($id,4,0,$i)} {incr j} {
	    set rtop $cmval($id,5,$i,$j)
	    set rbot $cmval($id,6,$i,$j)

	    set x1 [expr ($rtlast-$xmin)*$xscale+$l]
	    set x2 [expr ($rtop-$xmin)*$xscale+$l]
	    set x3 [expr ($rbot-$xmin)*$xscale+$l]
	    set x4 [expr ($rblast-$xmin)*$xscale+$l]

	    set color [lindex $colorlist $med($i,$j)]
	    $parent_w create polygon $x1 $y1 $x2 $y1 $x3 $y2 $x4 $y2\
		    -fill $color

	    # mirror on negative x side
	    set x1 [expr (-$rtlast-$xmin)*$xscale+$l]
	    set x2 [expr (-$rtop-$xmin)*$xscale+$l]
	    set x3 [expr (-$rbot-$xmin)*$xscale+$l]
	    set x4 [expr (-$rblast-$xmin)*$xscale+$l]

	    set color [lindex $colorlist $med($i,$j)]
	    $parent_w create polygon $x1 $y1 $x2 $y1 $x3 $y2 $x4 $y2\
		    -fill $color

	    set rtlast $rtop
	    set rblast $rbot
	}
	# region from last cone to outside

	set x1 [expr ($rtlast-$xmin)*$xscale+$l]
	set x2 [expr ($rmax-$xmin)*$xscale+$l]
	set x3 $x2
	set x4 [expr ($rblast-$xmin)*$xscale+$l]

	set color [lindex $colorlist $med($i,$j)]
	$parent_w create polygon $x1 $y1 $x2 $y1 $x3 $y2 $x4 $y2\
		-fill $color

	# mirror on negative x side
	set x1 [expr (-$rtlast-$xmin)*$xscale+$l]
	set x2 [expr (-$rmax-$xmin)*$xscale+$l]
	set x3 $x2
	set x4 [expr (-$rblast-$xmin)*$xscale+$l]

	set color [lindex $colorlist $med($i,$j)]
	$parent_w create polygon $x1 $y1 $x2 $y1 $x3 $y2 $x4 $y2\
		-fill $color

	set tot [expr $tot+$thick]
    }
}

