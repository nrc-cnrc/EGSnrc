
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: VARMLC
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
#  Author:          Blake Walters, 2000
#
#  Contributors:    Iwan Kawrakow
#                   Frederic Tessier
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


proc init_VARMLC { id } {
    global cmval ngroups nleaf

    set cmval($id,0) {}
    set cmval($id,1) {}
    set cmval($id,2,0) 0
    set cmval($id,2,1) 1
    set cmval($id,3) {}
    set cmval($id,4) {}
    # for each leaf group, set no. and width
    for {set i 1} {$i<=100} {incr i} {
        set cmval($id,5,0,$i) {}
        set cmval($id,5,1,$i) {}
    }
    set cmval($id,6) {}
    set cmval($id,7,0) {}
    set cmval($id,7,1) {}
    for {set i 8} {$i<10} {incr i} {
       set cmval($id,$i,0) {}
       set cmval($id,$i,1) {}
       set cmval($id,$i,2) {}
    }
    for {set i 10} {$i<14} {incr i} {
       set cmval($id,$i) {}
    }
    # for each leaf set: neg, pos, num
    for {set i 1} {$i<=100} {incr i} {
	set cmval($id,14,0,$i) {}
	set cmval($id,14,1,$i) {}
	set cmval($id,14,2,$i) {}
    }
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,15,$i) {}
    }
    set cmval($id,16) {}
    for {set i 0} {$i<5} {incr i} {
	set cmval($id,17,$i) {}
    }
    set cmval($id,18) {}
    set ngroups($id) 1
    set nleaf($id) {}
}

proc read_VARMLC { fileid id } {
    global cmval GUI_DIR ngroups cm_ident nleaf

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read VARMLC $cm_ident($id).  The inputs don't begin where\
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

    # read orient, ngroup 2
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
        set data [get_val $data cmval $id,2,$i]
    }
    if $cmval($id,2,1)==0 {
        set cmval($id,2,1) 1
    }

    # read zmin 3
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # read zthick 4
    gets $fileid data
    set data [get_val $data cmval $id,4]

    # read numleaf, leafwidth for each group--5
    set nleaf($id) 0
    for {set i 1} {$i<=$cmval($id,2,1)} {incr i} {
      gets $fileid data
      for {set j 0} {$j<2} {incr j} {
	set data [get_val $data cmval $id,5,$j,$i]
      }
      incr nleaf($id) $cmval($id,5,0,$i)
    }

    # read start 6
    gets $fileid data
    set data [get_val $data cmval $id,6]

    # read wscrew, hscrew 7
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
        set data [get_val $data cmval $id,7,$i]
    }

    # read wtongue, etc 8-9
    for {set j 8} {$j<10} {incr j} {
      gets $fileid data
      for {set i 0} {$i<3} {incr i} {
	set data [get_val $data cmval $id,$j,$i]
      }
    }

    # read leafgap, etc 10-13
    for {set j 10} {$j<14} {incr j} {
      gets $fileid data
      set data [get_val $data cmval $id,$j]
    }

    # read in the leaf coordinates, neg,pos,num.  When the sum over num
    # is > numleaf, stop reading them.
    set numsum 0
    set ngroups($id) 0
    while {$numsum<$nleaf($id)} {
	incr ngroups($id)
	gets $fileid data
	for {set i 0} {$i<3} {incr i} {
	    set data [get_val $data cmval $id,14,$i,$ngroups($id)]
	}
	if { $cmval($id,14,2,$ngroups($id))<=0 } {
	    set cmval($id,14,2,$ngroups($id)) 1
	}
	incr numsum $cmval($id,14,2,$ngroups($id))
    }

    # read ecut etc in air and air gaps
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,15,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,16]

    # read ecut etc in collimator leaves
    gets $fileid data
    for {set i 0} {$i<5} {incr i} {
	set data [get_val $data cmval $id,17,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,18]
}

proc edit_VARMLC { id zmax } {
    global cmval GUI_DIR helvfont cm_ident ngroups help_varmlc_text
    global nleaf default1

    catch { destroy .varmlc$id }
    toplevel .varmlc$id
    wm title .varmlc$id "Edit VARMLC, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY VARMLC.
    frame .varmlc$id.top -bd 4
    set top .varmlc$id.top


    label $top.mainlabel -text "Multi-leaf collimator $cm_ident($id)" \
	    -font $helvfont
    pack $top.mainlabel -side top -padx 10

    # VARMLC_macros.mortran:REPLACE {$MAXLEAF} WITH {64}
    # read the macro file for CM VARMLC to get defaults and put a label on the
    # main edit window.  Has to be different from the others because there's
    # no VARMLC in the default name.

    global omega helvfont
    set filename $omega/beamnrc/CMs/VARMLC_macros.mortran
    set default1 {}
    if { [file readable $filename] } {
	set fileid [open $filename "r"]
	set i 0
	while {$i<100 & [string compare $default1 {}]==0} {
	    incr i
	    gets $fileid data
	    set data [string trimright $data]
	    if [string first "REPLACE" $data]>=0 {
		if {[string first MAXLEAF $data]>0} {
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

    # leaf direction radiobuttons + selection to ignore air gaps
    # for range rejection in leaves
    frame $top.inp2 -bd 2
    frame $top.inp2.left
    radiobutton $top.inp2.left.r1 -text "Leaves parallel to y"\
            -variable cmval($id,2,0) -value 0
    radiobutton $top.inp2.left.r2 -text "Leaves parallel to x"\
            -variable cmval($id,2,0) -value 1
    pack $top.inp2.left.r1 $top.inp2.left.r2 -side top
    frame $top.inp2.right
    checkbutton $top.inp2.right.c1 -text "Ignore air gaps for range \
                   rejection in leaves" -variable cmval($id,17,4)
    button $top.inp2.right.help -bitmap @$GUI_DIR/help_icon.xbm \
            -command "help_ignoregaps [winfo parent $top.inp2.right]"
    pack $top.inp2.right.help $top.inp2.right.c1 -side right
    pack $top.inp2.left $top.inp2.right -side left -expand true -fill x
    pack $top.inp2 -side top -expand true -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label -text {Thickness of the leaves (cm)}
    entry $top.inp4.inp -textvariable cmval($id,4) -width 10
    pack $top.inp4.label -anchor w -side left
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    frame $top.leaves -bd 4
    frame $top.leaves.nleaf -bd 4
    label $top.leaves.nleaf.label -text {Number of leaves}
    entry $top.leaves.nleaf.inp -textvariable nleaf($id) -width 10
    pack $top.leaves.nleaf.label -anchor w -side left
    pack $top.leaves.nleaf.inp -anchor e -side right -fill x -expand true
    frame $top.leaves.def -bd 4
    button $top.leaves.def.inp5 -text "Define leaf widths >>" -command \
            "define_varmlc_widths $id"
    button $top.leaves.def.inp6 -text "Define leaf openings >>" -command \
            "define_varmlc_leaves $id"
    pack $top.leaves.def.inp5 -anchor w -side left -fill x -expand true
    pack $top.leaves.def.inp6 -anchor w -side left -fill x -expand true
    pack $top.leaves.nleaf -side top -fill x
    pack $top.leaves.def -side top -fill x
    pack $top.leaves -side top -fill x

    frame $top.inp7 -bd 4
    label $top.inp7.label -text {Starting position of leaf sides}
    entry $top.inp7.inp -textvariable cmval($id,6) -width 10
    pack $top.inp7.label -anchor w -side left
    pack $top.inp7.inp -anchor e -side right -fill x -expand true
    pack $top.inp7 -side top -fill x

    frame $top.screw -bd 4

    frame $top.screw.title -bd 4
    label $top.screw.title.label -text {Driving screws:}
    pack $top.screw.title.label -anchor w -side left
    pack $top.screw.title -anchor w -side left

    frame $top.screw.inps -bd 4

    frame $top.screw.inps.inp8 -bd 4
    label $top.screw.inps.inp8.label -text {width (cm)}
    entry $top.screw.inps.inp8.inp -textvariable cmval($id,7,0) -width 10
    pack $top.screw.inps.inp8.label -anchor w -side left
    pack $top.screw.inps.inp8.inp -anchor w -side left -fill x -expand true
    pack $top.screw.inps.inp8 -anchor w -side left -fill x -expand true

    frame $top.screw.inps.inp9 -bd 4
    label $top.screw.inps.inp9.label -text {height (cm)}
    entry $top.screw.inps.inp9.inp -textvariable cmval($id,7,1) -width 10
    pack $top.screw.inps.inp9.label -anchor w -side left
    pack $top.screw.inps.inp9.inp -anchor w -side left -fill x -expand true
    pack $top.screw.inps.inp9 -anchor w -side left -fill x -expand true
    pack $top.screw.inps -anchor w -side left -fill x -expand true
    pack $top.screw -side top -fill x

    frame $top.tongue -bd 4

    frame $top.tongue.title -bd 4
    label $top.tongue.title.label -text {Tongue:}
    pack $top.tongue.title.label -anchor w -side left
    pack $top.tongue.title -anchor w -side left

    frame $top.tongue.inps -bd 4

    frame $top.tongue.inps.inp10 -bd 4
    label $top.tongue.inps.inp10.label -text {width (cm)}
    entry $top.tongue.inps.inp10.inp -textvariable cmval($id,8,0) -width 10
    pack $top.tongue.inps.inp10.label -anchor w -side left
    pack $top.tongue.inps.inp10.inp -anchor w -side left -fill x -expand true
    pack $top.tongue.inps.inp10 -anchor w -side left -fill x -expand true

    frame $top.tongue.inps.inp11 -bd 4
    label $top.tongue.inps.inp11.label -text {height (cm)}
    entry $top.tongue.inps.inp11.inp -textvariable cmval($id,8,1) -width 10
    pack $top.tongue.inps.inp11.label -anchor w -side left
    pack $top.tongue.inps.inp11.inp -anchor w -side left -fill x -expand true
    pack $top.tongue.inps.inp11 -anchor w -side left -fill x -expand true

    frame $top.tongue.inps.inp12 -bd 4
    label $top.tongue.inps.inp12.label -text {Z at top}
    entry $top.tongue.inps.inp12.inp -textvariable cmval($id,8,2) -width 10
    pack $top.tongue.inps.inp12.label -anchor w -side left
    pack $top.tongue.inps.inp12.inp -anchor w -side left -fill x -expand true
    pack $top.tongue.inps.inp12 -anchor w -side left -fill x -expand true
    pack $top.tongue.inps -anchor w -side left -fill x -expand true
    pack $top.tongue -side top -fill x

    frame $top.groove -bd 4

    frame $top.groove.title -bd 4
    label $top.groove.title.label -text {Groove:}
    pack $top.groove.title.label -anchor w -side left
    pack $top.groove.title -anchor w -side left

    frame $top.groove.inps -bd 4

    frame $top.groove.inps.inp13 -bd 4
    label $top.groove.inps.inp13.label -text {width (cm)}
    entry $top.groove.inps.inp13.inp -textvariable cmval($id,9,0) -width 10
    pack $top.groove.inps.inp13.label -anchor w -side left
    pack $top.groove.inps.inp13.inp -anchor w -side left -fill x -expand true
    pack $top.groove.inps.inp13 -anchor w -side left

    frame $top.groove.inps.inp14 -bd 4
    label $top.groove.inps.inp14.label -text {height (cm)}
    entry $top.groove.inps.inp14.inp -textvariable cmval($id,9,1) -width 10
    pack $top.groove.inps.inp14.label -anchor w -side left
    pack $top.groove.inps.inp14.inp -anchor w -side left -fill x -expand true
    pack $top.groove.inps.inp14 -anchor w -side left -fill x -expand true

    frame $top.groove.inps.inp15 -bd 4
    label $top.groove.inps.inp15.label -text {Z at top}
    entry $top.groove.inps.inp15.inp -textvariable cmval($id,9,2) -width 10
    pack $top.groove.inps.inp15.label -anchor w -side left
    pack $top.groove.inps.inp15.inp -anchor w -side left -fill x -expand true
    pack $top.groove.inps.inp15 -anchor w -side left -fill x -expand true
    pack $top.groove.inps -anchor w -side left -fill x -expand true
    pack $top.groove -side top -fill x

    frame $top.inp16 -bd 4
    label $top.inp16.label -text {Width of air gap (cm)}
    entry $top.inp16.inp -textvariable cmval($id,10) -width 10
    pack $top.inp16.label -anchor w -side left
    pack $top.inp16.inp -anchor e -side right -fill x -expand true
    pack $top.inp16 -side top -fill x

    frame $top.endparam

    frame $top.endparam.inp17 -bd 4
    label $top.endparam.inp17.label -text {Leaf end type}
    end_param_menu_varmlc $top.endparam.inp17.inp $id
    pack $top.endparam.inp17.label -anchor w -side left
    pack $top.endparam.inp17.inp -anchor e -side right -fill x -expand true

    frame $top.endparam.inp18 -bd 4
    if {$cmval($id,11)==1} {
        label $top.endparam.inp18.label -text {Zfocus of leaf ends}
    } else {
        label $top.endparam.inp18.label -text {Radius of leaf ends}
    }

    entry $top.endparam.inp18.inp -textvariable cmval($id,12) -width 10
    pack $top.endparam.inp18.label -anchor w -side left
    pack $top.endparam.inp18.inp -anchor e -side right -fill x -expand true
    pack $top.endparam.inp18 -anchor e -side right -fill x -expand true
    pack $top.endparam.inp17 -anchor w -side left
    pack $top.endparam.inp18 -anchor e -side right -fill x -expand true
    pack $top.endparam -side top -fill x

    frame $top.inp19 -bd 4
    label $top.inp19.label -text {Z focus of leaf sides}
    entry $top.inp19.inp -textvariable cmval($id,13) -width 10
    pack $top.inp19.label -anchor w -side left
    pack $top.inp19.inp -anchor e -side right -fill x -expand true
    pack $top.inp19 -side top -fill x

    pack $top -side top -pady 5

#now get ecut,pcut,materials,etc

    frame .varmlc$id.bottom
    frame .varmlc$id.bottom.left
    frame .varmlc$id.bottom.right

    label .varmlc$id.bottom.left.lab -text "Opening(s) and air gaps" \
                       -font $helvfont
    pack .varmlc$id.bottom.left.lab -side top -pady 5
    add_ecut .varmlc$id.bottom.left.f0 $id,15,0
    add_pcut .varmlc$id.bottom.left.f1 $id,15,1
    add_dose .varmlc$id.bottom.left.f2 $id,15,2
    add_latch .varmlc$id.bottom.left.f3 $id,15,3
    add_material .varmlc$id.bottom.left.f4 $id,16

    label .varmlc$id.bottom.right.lab -text "Collimator leaves" -font $helvfont
    pack .varmlc$id.bottom.right.lab -side top -pady 5
    add_ecut .varmlc$id.bottom.right.f0 $id,17,0
    add_pcut .varmlc$id.bottom.right.f1 $id,17,1
    add_dose .varmlc$id.bottom.right.f2 $id,17,2
    add_latch .varmlc$id.bottom.right.f3 $id,17,3
    add_material .varmlc$id.bottom.right.f4 $id,18

    pack .varmlc$id.bottom.left -side left -anchor n -padx 10
    pack .varmlc$id.bottom.right -side right -anchor n -padx 10

    pack .varmlc$id.bottom -side top -pady 5

    frame .varmlc$id.buts -bd 4
    button .varmlc$id.buts.okb -text "OK" -command "destroy .varmlc$id"\
	    -relief groove -bd 8
    button .varmlc$id.buts.helpb -text "Help" -command\
	    "help_gif .varmlc$id.help {$help_varmlc_text} help_varmlc"\
	    -relief groove -bd 8
#don't have a preview yet, get it working after course, will call show_VARMLC
    button .varmlc$id.buts.prev -text "Preview" -command\
                          "show_VARMLC $id" -relief groove -bd 8
#	    { tk_dialog .nopreview "No Preview" \
#             "No preview available yet." info 0 OK } -relief groove -bd 8
    pack .varmlc$id.buts.helpb .varmlc$id.buts.okb .varmlc$id.buts.prev -side left -padx 10
    pack .varmlc$id.buts -pady 10
}

proc end_param_menu_varmlc { w id } {
    global cmval

    set inw .varmlc$id.top.endparam.inp18.label

    if { $cmval($id,11)==1 } {
      menubutton $w -text "straight" -menu  $w.m -bd 1 \
                  -relief raised -indicatoron 1 -width 20
    } else {
      menubutton $w -text "rounded" -menu  $w.m -bd 1 \
                  -relief raised -indicatoron 1 -width 20
    }
    menu $w.m -tearoff no
    $w.m add command -label "rounded" \
               -command "set_endval_varmlc $id 0 $w;\
                         $inw configure -text {Radius of leaf ends}"
    $w.m add command -label "straight" \
               -command "set_endval_varmlc $id 1 $w;\
                         $inw configure -text {Zfocus of leaf ends}"
}

proc set_endval_varmlc { id val w } {
    global cmval

    set cmval($id,11) $val

    if { $val==1 } {
        $w configure -text "straight"
    } else {
        $w configure -text "rounded"
    }
}

proc define_varmlc_widths { id } {
    global cmval helvfont ngroups tow fromw help_varmlc_text
    global nleaf default1

# procedure for defining leaf widths and, in the process, the total
# number of leaves

    if {$nleaf($id)==""} {
       tk_dialog .setnleaf "Set number of leaves" "Before you can specify\
       the leaf widths, you must specify the number of leaves." warning 0 OK
       return
    }

    catch { destroy .varmlc$id.childw }
    toplevel .varmlc$id.childw
    set top .varmlc$id.childw
    wm title $top "Define leaf widths"

    frame $top.f -bd 10
    set w $top.f

    if {$cmval($id,2,0)==0} {
        set orientation X
    } else {
        set orientation Y
    }

    message $w.message -text "Specify the $orientation widths of the \
            leaves at the top of the CM.  Add a row to start a \
            new group.  When a new row is added the next leaf in the order will\
            automatically appear in the 'from leaf' box."\
            -width 400 -font $helvfont
    pack $w.message -side top -anchor n -fill x -expand true

    frame $w.grid -bd 4
    label $w.grid.l0 -text "from leaf"
    label $w.grid.l1 -text "to leaf"
    label $w.grid.l2 -text "width (cm)"

    grid configure $w.grid.l0 -row 0 -column 0
    grid configure $w.grid.l1 -row 0 -column 1
    grid configure $w.grid.l2 -row 0 -column 2

    set tow(0) 0
    for {set j 1} {$j<=$cmval($id,2,1)} {incr j} {
      set fromw($j) [expr $tow([expr $j-1])+1]
      if [catch {set tow($j) [expr $tow([expr $j-1])+$cmval($id,5,0,$j)]}]==1 {
            set tow($j) {}
      }
      entry $w.grid.e0$j -textvariable fromw($j) -width 8
      grid configure $w.grid.e0$j -row $j -column 0
      entry $w.grid.e1$j -textvariable tow($j) -width 8
      grid configure $w.grid.e1$j -row $j -column 1
      entry $w.grid.e2$j -textvariable cmval($id,5,1,$j) -width 15
      grid configure $w.grid.e2$j -row $j -column 2
    }
    pack $w.grid -side top

    pack $w
    frame $top.b
    button $top.b.addb -text "Add a row" -command "add_varmlc_row_width $id"\
            -relief groove -bd 8
    button $top.b.delb -text "Delete last row"\
            -command "del_varmlc_row_width $id" -relief groove -bd 8
    button $top.b.okb -text "OK"\
            -command "save_varmlc_width $id" -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
            "help_gif .varmlc$id.childw.help {$help_varmlc_text} help_varmlc"\
            -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb $top.b.helpb -side left -padx 10
    pack $top.b -pady 10

}

proc define_varmlc_leaves { id } {
    global cmval helvfont ngroups nleaf to from help_varmlc_text

    if {$nleaf($id)==""} {
       tk_dialog .setnleaf "Set number of leaves" "Before you can specify\
       the leaf openings, you must specify the number of leaves." warning 0 OK
       return
    }

    catch { destroy .varmlc$id.child }
    toplevel .varmlc$id.child
    set top .varmlc$id.child
    wm title $top "Define leaf openings"

    frame $top.f -bd 10
    set w $top.f

    if $cmval($id,2,0)==1 {
	set orientation X
    } else {
	set orientation Y
    }

    if [string compare $ngroups($id) ""]==0 {
	set ngroups($id) 1
    }

    message $w.message -text "Specify the minimum and maximum $orientation\
	    value (at the top of the CM) for the opening in \
	    each leaf in the VARMLC by groups here.  Add a row to start a\
	    new group.  When a new row is added the next leaf in the order will\
	    automatically appear in the 'from leaf' box.  This must repeat\
	    until you have defined the minimum and maximum for all\
	    $nleaf($id) leaves." -width 400 -font $helvfont
    pack $w.message -side top -anchor n -fill x -expand true

    frame $w.grid -bd 4
    label $w.grid.l0 -text "from leaf"
    label $w.grid.l1 -text "to leaf"
    if { $cmval($id,11)==0 } {
      label $w.grid.l2 -text "min. $orientation of opening   "
      label $w.grid.l3 -text "max. $orientation of opening"
    } else {
      label $w.grid.l2 -text "min. $orientation of opening at ZMIN    "
      label $w.grid.l3 -text "max. $orientation of opening at ZMIN"
    }
    grid configure $w.grid.l0 -row 0 -column 0
    grid configure $w.grid.l1 -row 0 -column 1
    grid configure $w.grid.l2 -row 0 -column 2
    grid configure $w.grid.l3 -row 0 -column 3
    set to(0) 0
    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	set from($j) [expr $to([expr $j-1])+1]
	if [catch {set to($j) [expr $to([expr $j-1])+$cmval($id,14,2,$j)]}]==1 {
	    set to($j) {}
	}
	entry $w.grid.e0$j -textvariable from($j) -width 8
	grid configure $w.grid.e0$j -row $j -column 0
	entry $w.grid.e1$j -textvariable to($j) -width 8
	grid configure $w.grid.e1$j -row $j -column 1
	entry $w.grid.e2$j -textvariable cmval($id,14,0,$j) -width 15
	grid configure $w.grid.e2$j -row $j -column 2
	entry $w.grid.e3$j -textvariable cmval($id,14,1,$j) -width 15
	grid configure $w.grid.e3$j -row $j -column 3
    }
    pack $w.grid -side top

    pack $w
    frame $top.b
    button $top.b.addb -text "Add a row" -command "add_varmlc_row $id"\
	    -relief groove -bd 8
    button $top.b.delb -text "Delete last row"\
            -command "del_varmlc_row $id" -relief groove -bd 8
    button $top.b.okb -text "OK" -command "save_varmlc $id" -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
	    "help_gif .varmlc$id.child.help {$help_varmlc_text} help_varmlc"\
	    -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb $top.b.helpb -side left -padx 10
    pack $top.b -pady 10
}

proc save_varmlc { id } {
    global cmval ngroups from to nleaf

    if {$to($ngroups($id))>$nleaf($id)} {
       tk_dialog .toomanyopeninggroups "Too many leaves" "You have specified too many\
       leaf openings for the number of leaves ($nleaf($id))." warning 0 OK
       return
    } elseif {$to($ngroups($id))<$nleaf($id)} {
       tk_dialog .toofewopeninggroups "Too few leaves" "You have specified too few\
       leaf openings for the number of leaves ($nleaf($id))." warning 0 OK
       return
    }

    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	if [catch {set cmval($id,14,2,$j) [expr $to($j)-$from($j)+1]}]==1 {
	    tk_dialog .nope "No no no" "The leaf geometries for group $j\
		    have been improperly set or not set at all.\
		    Go back and fix them." warning 0 OK
	}
    }

    destroy .varmlc$id.child

}

proc save_varmlc_width { id } {
    global cmval fromw tow nleaf

    if {$tow($cmval($id,2,1))>$nleaf($id)} {
       tk_dialog .toomanywidthgroups "Too many leaves" "You have specified too many\
       leaf widths for the number of leaves ($nleaf($id))." warning 0 OK
       return
    } elseif {$tow($cmval($id,2,1))<$nleaf($id)} {
       tk_dialog .toofewwidthgroups "Too few leaves" "You have specified too few\
       leaf widths for the number of leaves ($nleaf($id))." warning 0 OK
       return
    }

    for {set j 1} {$j<=$cmval($id,2,1)} {incr j} {
        if [catch {set cmval($id,5,0,$j) [expr $tow($j)-$fromw($j)+1]}]==1 {
            tk_dialog .nope "No no no" "The leaf widths for group $j\
                    have been improperly set or not set at all.\
                    Go back and fix them." warning 0 OK
        }
    }

    destroy .varmlc$id.childw

}

proc add_varmlc_row { id } {
    global ngroups from to cmval nleaf

    set w .varmlc$id.child.f

    if $to($ngroups($id))>=$nleaf($id) {
	# They've been finished already.  Refuse.
	tk_dialog .refuse "Too many" "You've already completed the leaves.\
		I can't add another row unless you change the last 'to'\
		box to a values less than $nleaf($id) or increase the\
		number of leaves." warning 0 OK
	return
    }

    set j [incr ngroups($id)]
    set from($j) [expr $to([expr $j-1])+1]
    if [catch {set to($j) [expr $to([expr $j-1])+$cmval($id,14,2,$j)]}]==1 {
	set to($j) {}
    }
    entry $w.grid.e0$j -textvariable from($j) -width 8
    grid configure $w.grid.e0$j -row $j -column 0
    entry $w.grid.e1$j -textvariable to($j) -width 8
    grid configure $w.grid.e1$j -row $j -column 1
    entry $w.grid.e2$j -textvariable cmval($id,14,0,$j) -width 15
    grid configure $w.grid.e2$j -row $j -column 2
    entry $w.grid.e3$j -textvariable cmval($id,14,1,$j) -width 15
    grid configure $w.grid.e3$j -row $j -column 3

}

proc add_varmlc_row_width { id } {
    global cmval fromw tow nleaf

    set w .varmlc$id.childw.f

    if $tow($cmval($id,2,1))>=$nleaf($id) {
        # They've been finished already.  Refuse.
        tk_dialog .refuse "Too many" "You've already completed the leaves.\
                I can't add another row unless you change the last 'to'\
                box to a values less than $nleaf($id) or increase the\
                number of leaves." warning 0 OK
        return
    }

    set j [incr cmval($id,2,1)]
    set fromw($j) [expr $tow([expr $j-1])+1]
    if [catch {set tow($j) [expr $tow([expr $j-1])+$cmval($id,5,0,$j)]}]==1 {
        set tow($j) {}
    }
    entry $w.grid.e0$j -textvariable fromw($j) -width 8
    grid configure $w.grid.e0$j -row $j -column 0
    entry $w.grid.e1$j -textvariable tow($j) -width 8
    grid configure $w.grid.e1$j -row $j -column 1
    entry $w.grid.e2$j -textvariable cmval($id,5,1,$j) -width 15
    grid configure $w.grid.e2$j -row $j -column 2

}

proc del_varmlc_row { id } {
    global ngroups

    set w .varmlc$id.child.f

    for {set i 0} {$i<4} {incr i} {
         destroy $w.grid.e$i$ngroups($id)
    }
    incr ngroups($id) -1
}

proc del_varmlc_row_width { id } {
    global cmval

    set w .varmlc$id.childw.f

    for {set i 0} {$i<3} {incr i} {
         destroy $w.grid.e$i$cmval($id,2,1)
    }
    incr cmval($id,2,1) -1
}

proc write_VARMLC {fileid id} {
    global cmval cm_names cm_ident cm_type ngroups

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2,0), $cmval($id,2,1), ORIENT, NGROUP"
    puts $fileid "$cmval($id,3), ZMIN"
    puts $fileid "$cmval($id,4), ZTHICK"
    for {set j 1} {$j<=$cmval($id,2,1)} {incr j} {
        puts $fileid "$cmval($id,5,0,$j), $cmval($id,5,1,$j)"
    }
    puts $fileid "$cmval($id,6), START"
    puts $fileid "$cmval($id,7,0), $cmval($id,7,1), WSCREW, HSCREW"
    puts $fileid "$cmval($id,8,0), $cmval($id,8,1), $cmval($id,8,2), WTONGUE, HTONGUE, ZTONGUE"
    puts $fileid "$cmval($id,9,0), $cmval($id,9,1), $cmval($id,9,2), WGROOVE, HGROOVE, ZGROOVE"
    puts $fileid "$cmval($id,10), LEAFGAP"
    puts $fileid "$cmval($id,11), ENDTYPE"
    puts $fileid "$cmval($id,12), ZFOCUS or RADIUS of leaf ends"
    puts $fileid "$cmval($id,13), ZFOCUS of leaf sides"

    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	puts $fileid "$cmval($id,14,0,$j), $cmval($id,14,1,$j), $cmval($id,14,2,$j)"
    }

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,15,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,16)

    set str {}
    for {set i 0} {$i<5} {incr i} {
	set str "$str$cmval($id,17,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,18)

}


proc show_VARMLC { id } {
    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks nleaf

    catch { destroy .varmlc$id.show }
    toplevel .varmlc$id.show
    wm title .varmlc$id.show "Preview"

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
    frame .varmlc$id.show.frm

    draw_VARMLC $id

    frame .varmlc$id.show.buts
    button .varmlc$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .varmlc$id.show xyz" -relief groove -bd 8
    button .varmlc$id.show.buts.print1 -text "Print xy..." -command\
	    "print_canvas .varmlc$id.show.frm.can.one 450 450" -relief groove -bd 8
    button .varmlc$id.show.buts.print2 -text "Print xz..." -command\
	    "print_canvas .varmlc$id.show.frm.can.two 450 450" -relief groove -bd 8
    button .varmlc$id.show.buts.print3 -text "Print yz..." -command\
	    "print_canvas .varmlc$id.show.frm.can.three 450 450" -relief groove -bd 8
    button .varmlc$id.show.buts.done -text "Done" -command\
	    "destroy .varmlc$id.show" -relief groove -bd 8
    pack .varmlc$id.show.buts.range .varmlc$id.show.buts.print1\
	    .varmlc$id.show.buts.print2 .varmlc$id.show.buts.print3\
	    .varmlc$id.show.buts.done -side left -padx 10
    pack .varmlc$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_VARMLC { id } {
    global helvfont zrange yrange xrange zscale yscale xscale l m
    global cmval cm_ticks nleaf

    catch { destroy .varmlc$id.show.frm.can }

    # put the canvas up
    set ncan 3
    set width 300.0
    set canwidth [expr $width+150.0]
    set scrlheight [expr 2*$canwidth]
    set scrlwidth [expr 2*$canwidth]
    set winwidth [expr [winfo screenwidth .]*4.0/5.0]
    set winheight [expr [winfo screenheight .]*4.0/5.0]

    if $scrlwidth>$winwidth {
	catch { destroy .varmlc$id.show.frm.scrlz }
	catch { destroy .varmlc$id.show.frm.scrlx }
	scrollbar .varmlc$id.show.frm.scrlz -command\
		".varmlc$id.show.frm.can yview"
	scrollbar .varmlc$id.show.frm.scrlx -command\
		".varmlc$id.show.frm.can xview" -orient horizontal
	pack .varmlc$id.show.frm.scrlz -side right -fill y
	pack .varmlc$id.show.frm.scrlx -side bottom -fill x
	canvas .varmlc$id.show.frm.can -width $winwidth -height $winheight\
		-yscrollcommand ".varmlc$id.show.frm.scrlz set"\
		-xscrollcommand ".varmlc$id.show.frm.scrlx set"\
		-scrollregion "0 0 $scrlwidth $scrlheight"
    } else {
	canvas .varmlc$id.show.frm.can -width $scrlwidth -height $scrlheight
    }

    # Put some text in the upper left corner, just to fill the gap.
    .varmlc$id.show.frm.can create text 225 225 -text "The preview of the\
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

    canvas .varmlc$id.show.frm.can.one -width $canwidth -height $canwidth

    add_VARMLC_xy $id $xscale $yscale $xrange(0) $yrange(0)\
	    $l $m .varmlc$id.show.frm.can.one

    set curx 0
    set cury 0
    label .varmlc$id.show.frm.can.one.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .varmlc$id.show.frm.can.one create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .varmlc$id.show.frm.can.one.xy
    bind .varmlc$id.show.frm.can.one <Motion> {
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
    coverup $l $m $width .varmlc$id.show.frm.can.one

    add_axes xy $cm_ticks($id,x) $cm_ticks($id,y) $l $m $width $xrange(0)\
	    $xrange(1) $yrange(0) $yrange(1)\
	    $xscale $yscale .varmlc$id.show.frm.can.one

    .varmlc$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .varmlc$id.show.frm.can.one

    # XZ, lower left: if leaves // to y, show leaf sides,
    # else if leaves // to x, show only one leaf at y=0 (leaf ends).

    canvas .varmlc$id.show.frm.can.two -width $canwidth -height $canwidth

    add_air $id .varmlc$id.show.frm.can.two $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    if $cmval($id,2,0)==0 {
	add_VARMLC_sides $id $xscale $zscale $xrange(0) $zrange(0) $zrange(1)\
		$l $m .varmlc$id.show.frm.can.two
    } else {
        add_VARMLC_ends $id $xscale $zscale $xrange(0) $zrange(0) $zrange(1)\
		$l $m .varmlc$id.show.frm.can.two
    }

    coverup $l $m $width .varmlc$id.show.frm.can.two

    label .varmlc$id.show.frm.can.two.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .varmlc$id.show.frm.can.two create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .varmlc$id.show.frm.can.two.xy
    bind .varmlc$id.show.frm.can.two <Motion> {
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
	    $xscale $zscale .varmlc$id.show.frm.can.two

    .varmlc$id.show.frm.can create window [expr $canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .varmlc$id.show.frm.can.two

    # YZ, lower right

    canvas .varmlc$id.show.frm.can.three -width $canwidth -height $canwidth

    add_air $id .varmlc$id.show.frm.can.three $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    if $cmval($id,2,0)==0 {
	add_VARMLC_ends $id $yscale $zscale $yrange(0) $zrange(0) $zrange(1)\
		$l $m .varmlc$id.show.frm.can.three
    } else {
	add_VARMLC_sides $id $yscale $zscale $yrange(0) $zrange(0) $zrange(1)\
		$l $m .varmlc$id.show.frm.can.three
    }

    coverup $l $m $width .varmlc$id.show.frm.can.three

    label .varmlc$id.show.frm.can.three.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .varmlc$id.show.frm.can.three create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .varmlc$id.show.frm.can.three.xy
    bind .varmlc$id.show.frm.can.three <Motion> {
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
	    $yscale $zscale .varmlc$id.show.frm.can.three

    .varmlc$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .varmlc$id.show.frm.can.three

    pack .varmlc$id.show.frm.can -side top -anchor n
    pack .varmlc$id.show.frm -side top -anchor n
}

proc add_VARMLC_xy {id xscale yscale xmin ymin l m parent_w} {
    global cmval colorlist medium nmed nleaf colornum

    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,16) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,18) $medium($j)]==0 {
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
    while {$k < $nleaf($id)} {
	for {set j 1} {$j<=$cmval($id,14,2,$i)} {incr j} {
	    incr k
	    set neg($k) $cmval($id,14,0,$i)
	    set pos($k) $cmval($id,14,1,$i)
	}
	incr i
    }

    set color [lindex $colorlist $med(leaves)]

    # if it's parallel to x, neg and pos are x-values, else neg and pos are
    # y-values.  Define a rectangle by (a,b) to (c,d).
    # the first leaf starts at START

    # dt is the width of the tongue
    # ds is the width of the screw, da is the width of the airgap
    set dt $cmval($id,8,0)
    set ds $cmval($id,7,0)
    set da $cmval($id,10)

    # ts is a value to add/subtract to pos/neg values of leaves to
    # determine where rectangle showing screws should end on the
    # inner (opening) side for rounded leaf ends
    set ts 0
    if $cmval($id,11)==0 {
        set ts [expr ($cmval($id,4)-2*$cmval($id,7,1))/2]
        set ts [expr sqrt($cmval($id,12)*$cmval($id,12)-$ts*$ts)]
        set ts [expr $cmval($id,12)-$ts]
    }

    if $cmval($id,2,0)==1 {
	# leaves parallel to x
	set lstart $cmval($id,6)
	set rmin [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	set rmax [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        set i 0; #leaf i
        set k 1; #width group k
	while {$i < $nleaf($id)} {
          for {set j 1} {$j<=$cmval($id,5,0,$k)} {incr j} {
            incr i
            #dl is the width of each leaf
            set dl $cmval($id,5,1,$k)
            if {$i==1} { #make a rect for the first tongue
               set a [expr ($neg($i)-$xmin)*$xscale+$l]
               set b [expr ($lstart-$ymin)*$yscale+$m]
               set c [expr ($pos($i)-$xmin)*$xscale+$l]
               set d [expr ($lstart+$dt-$ymin)*$yscale+$m]
               $parent_w create rectangle $rmin $b $a $d -fill $color -outline black
               $parent_w create rectangle $rmax $b $c $d -fill $color -outline black
               set lstart [expr $lstart+$dt]
            }
            # now make a rect for the each leaf
	    set a [expr ($neg($i)-$xmin)*$xscale+$l]
	    set b [expr ($lstart-$ymin)*$yscale+$m]
	    set c [expr ($pos($i)-$xmin)*$xscale+$l]
	    set d [expr ($lstart+$dl-$ymin)*$yscale+$m]
	    $parent_w create rectangle $rmin $b $a $d -fill $color -outline black
	    $parent_w create rectangle $rmax $b $c $d -fill $color -outline black
            # now make a rect for the screw on top
            set a [expr ($neg($i)-$ts-$xmin)*$xscale+$l]
            set b [expr ($lstart+($dl-$ds)/2-$ymin)*$yscale+$m]
            set c [expr ($pos($i)+$ts-$xmin)*$xscale+$l]
            set d [expr ($lstart+($dl-$ds)/2+$ds-$ymin)*$yscale+$m]
            $parent_w create rectangle $rmin $b $a $d -fill $color -outline black
            $parent_w create rectangle $rmax $b $c $d -fill $color -outline black
	    set lstart [expr $lstart+$dl]
            if $i<$nleaf($id) { #draw rectangle for leaf gap
              if $da>$dt { #airgap is > tongue width
                  #draw the air space
                  set color [lindex $colorlist $med(in)]
                  set a [expr ($neg($i)-$xmin)*$xscale+$l]
                  set b [expr ($lstart-$ymin)*$yscale+$m]
                  set c [expr ($pos($i)-$xmin)*$xscale+$l]
                  set d [expr ($lstart+($da-$dt)-$ymin)*$yscale+$m]
                  $parent_w create rectangle $rmin $b $a $d -fill $color -outline black
                  $parent_w create rectangle $rmax $b $c $d -fill $color -outline black
                  set lstart [expr $lstart+$da-$dt]
                  #draw full tongue
                  set color [lindex $colorlist $med(leaves)]
                  set a [expr ($neg($i)-$xmin)*$xscale+$l]
                  set b [expr ($lstart-$ymin)*$yscale+$m]
                  set c [expr ($pos($i)-$xmin)*$xscale+$l]
                  set d [expr ($lstart+$dt-$ymin)*$yscale+$m]
                  $parent_w create rectangle $rmin $b $a $d -fill $color -outline black
                  $parent_w create rectangle $rmax $b $c $d -fill $color -outline black
                  set lstart [expr $lstart+$dt]
              } else {
                  #draw exposed portion of tongue
                  set a [expr ($neg($i)-$xmin)*$xscale+$l]
                  set b [expr ($lstart-$ymin)*$yscale+$m]
                  set c [expr ($pos($i)-$xmin)*$xscale+$l]
                  set d [expr ($lstart+$da-$ymin)*$yscale+$m]
                  $parent_w create rectangle $rmin $b $a $d -fill $color -outline black
                  $parent_w create rectangle $rmax $b $c $d -fill $color -outline black
                  set lstart [expr $lstart+$da]
              }
            }
          }
          incr k
	}
    } else {
	# leaves parallel to y
        set lstart $cmval($id,6)
        set rmin [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
        set rmax [expr ($cmval($id,0)-$ymin)*$yscale+$m]
        set i 0; #leaf i
        set k 1; #width group k
        while {$i < $nleaf($id)} {
          for {set j 1} {$j<=$cmval($id,5,0,$k)} {incr j} {
            incr i
            #dl is the width of each leaf
            set dl $cmval($id,5,1,$k)
            if {$i==1} { #make a rect for the first tongue
               set a [expr ($neg($i)-$ymin)*$yscale+$m]
               set b [expr ($lstart-$xmin)*$xscale+$l]
               set c [expr ($pos($i)-$ymin)*$yscale+$m]
               set d [expr ($lstart+$dt-$xmin)*$xscale+$l]
               $parent_w create rectangle $b $rmax $d $c -fill $color -outline black
               $parent_w create rectangle $b $a $d $rmin -fill $color -outline black
               set lstart [expr $lstart+$dt]
            }
            # now make a rect for the each leaf
            set a [expr ($neg($i)-$ymin)*$yscale+$m]
            set b [expr ($lstart-$xmin)*$xscale+$l]
            set c [expr ($pos($i)-$ymin)*$yscale+$m]
            set d [expr ($lstart+$dl-$xmin)*$xscale+$l]
            $parent_w create rectangle $b $rmax $d $c -fill $color -outline black
            $parent_w create rectangle $b $a $d $rmin -fill $color -outline black
            # now make a rect for the screw on top
            set a [expr ($neg($i)-$ts-$ymin)*$yscale+$m]
            set b [expr ($lstart+($dl-$ds)/2-$xmin)*$xscale+$l]
            set c [expr ($pos($i)+$ts-$ymin)*$yscale+$m]
            set d [expr ($lstart+($dl-$ds)/2+$ds-$xmin)*$xscale+$l]
            $parent_w create rectangle $b $rmax $d $c -fill $color -outline black
            $parent_w create rectangle $b $a $d $rmin -fill $color -outline black
            set lstart [expr $lstart+$dl]
            if $i<$nleaf($id) { #draw rectangle for leaf gap
              if $da>$dt { #airgap is > tongue width
                  #draw the air space
                  set color [lindex $colorlist $med(in)]
                  set a [expr ($neg($i)-$ymin)*$yscale+$m]
                  set b [expr ($lstart-$xmin)*$xscale+$l]
                  set c [expr ($pos($i)-$ymin)*$yscale+$m]
                  set d [expr ($lstart+($da-$dt)-$xmin)*$xscale+$l]
                  $parent_w create rectangle $b $rmax $d $c -fill $color -outline black
                  $parent_w create rectangle $b $a $d $rmin -fill $color -outline black
                  set lstart [expr $lstart+$da-$dt]
                  #draw full tongue
                  set color [lindex $colorlist $med(leaves)]
                  set a [expr ($neg($i)-$ymin)*$yscale+$m]
                  set b [expr ($lstart-$xmin)*$xscale+$l]
                  set c [expr ($pos($i)-$ymin)*$yscale+$m]
                  set d [expr ($lstart+$dt-$xmin)*$xscale+$l]
                  $parent_w create rectangle $b $rmax $d $c -fill $color -outline black
                  $parent_w create rectangle $b $a $d $rmin -fill $color -outline black
                  set lstart [expr $lstart+$dt]
              } else {
                  #draw exposed portion of tongue
                  set a [expr ($neg($i)-$ymin)*$yscale+$m]
                  set b [expr ($lstart-$xmin)*$xscale+$l]
                  set c [expr ($pos($i)-$ymin)*$yscale+$m]
                  set d [expr ($lstart+$da-$xmin)*$xscale+$l]
                  $parent_w create rectangle $b $rmax $d $c -fill $color -outline black
                  $parent_w create rectangle $b $a $d $rmin -fill $color -outline black
                  set lstart [expr $lstart+$da]
              }
            }
          }
          incr k
        }
    }
}

proc add_VARMLC_ends {id xscale zscale xmin zmin zmax l m parent_w} {
    global cmval colorlist medium nmed meds_used nleaf colornum

    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,16) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,18) $medium($j)]==0 {
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

    # set up an array of neg and pos for each leaf
    set i 1; #opening group i
    set k 0; #leaf k
    while {$k < $nleaf($id)} {
             for {set j 1} {$j<=$cmval($id,14,2,$i)} {incr j} {
                 incr k
                 set neg($k) $cmval($id,14,0,$i)
                 set pos($k) $cmval($id,14,1,$i)
             }
             incr i
    }
    #now find out which leaves are intersected by Y=0 (if leaves || to Y)
    #or X=0 (if leaves || to X)
    #if only the tongue of a leaf intersects 0, show the entire leaf
    set ml 0
    set lstart $cmval($id,6)
    set i 1; #width group i
    set k 0; #leaf k
    while {$k<$nleaf($id) && $i<=$cmval($id,2,1)} {
      for {set j 1} {$j<=$cmval($id,5,0,$i)} {incr j} {
        incr k
        set lw [expr $cmval($id,5,1,$i)+$cmval($id,8,0)]
        if {$lstart<=0 && [expr $lstart+$lw]>=0} {
           set ml $k
           break
        }
        set lstart [expr $lstart+$lw+$cmval($id,10)-$cmval($id,8,0)]
      }
      incr i
    }

    if $ml>0 {
      set color [lindex $colorlist $med(leaves)]

      if $cmval($id,11)==0 { ;#rounded leaf ends, do similar to MLCQ
        set r0leaf $cmval($id,12)
        set z0leaf [expr $cmval($id,3)+$cmval($id,4)/2]

        # add an arc for the negative side and an arc for the positive side
        # 1: negative bounding box
        set a [expr ($neg($ml)-2*$r0leaf-$xmin)*$xscale+$l]
        set b [expr ($neg($ml)-$xmin)*$xscale+$l]
        set c [expr ($z0leaf-$r0leaf-$zmin)*$zscale+$m]
        set d [expr ($z0leaf+$r0leaf-$zmin)*$zscale+$m]
        # angles:
        set alp1 [expr asin(($z0leaf-$cmval($id,3))/$r0leaf)]
        set alp1d [expr $alp1*180/3.1415927]
        $parent_w create arc $a $c $b $d -start $alp1d -extent [expr -2*$alp1d]\
	    -fill $color -style pieslice -outline {}
        $parent_w create arc $a $c $b $d -start $alp1d -extent [expr -2*$alp1d]\
            -fill $color -style arc -outline black

        # 2: positive bounding box
        set a [expr ($pos($ml)-$xmin)*$xscale+$l]
        set b [expr ($pos($ml)+2*$r0leaf-$xmin)*$xscale+$l]
        $parent_w create arc $a $c $b $d -start [expr 180-$alp1d]\
	    -extent [expr 2*$alp1d] -fill $color -style pieslice -outline {}
        $parent_w create arc $a $c $b $d -start [expr 180-$alp1d]\
            -extent [expr 2*$alp1d] -fill $color -style arc -outline black

        # 3: rectangles to fill in the rest
        set a [expr ($neg($ml)-$r0leaf+$r0leaf*cos($alp1)-$xmin)*$xscale+$l]
        set b [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set c [expr ($cmval($id,3)-$zmin)*$zscale+$m]
        set d [expr ($cmval($id,3)+$cmval($id,4)-$zmin)*$zscale+$m]
        # negative
        $parent_w create rectangle $b $d $a $c -fill $color -outline {}
        $parent_w create line $b $c $a $c
        $parent_w create line $b $d $a $d
        # positive
        set a [expr ($pos($ml)+$r0leaf-$r0leaf*cos($alp1)-$xmin)*$xscale+$l]
        set b [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        $parent_w create rectangle $a $d $b $c -fill $color -outline {}
        $parent_w create line $a $c $b $c
        $parent_w create line $a $d $b $d

        # 4: lines to show where screws are
        #negative leavesa
        set a [expr ($cmval($id,4)-2*$cmval($id,7,1))/2]
        set a [expr sqrt($r0leaf*$r0leaf-$a*$a)]
        set a [expr ($neg($ml)-$r0leaf+$a-$xmin)*$xscale+$l]
        set b [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set c [expr ($cmval($id,3)+$cmval($id,7,1)-$zmin)*$zscale+$m]
        set d [expr ($cmval($id,3)+$cmval($id,4)-$cmval($id,7,1)-$zmin)*$zscale+$m]
        $parent_w create line $b $c $a $c
        $parent_w create line $b $d $a $d
        #positive leaves
        set a [expr ($cmval($id,4)-2*$cmval($id,7,1))/2]
        set a [expr sqrt($r0leaf*$r0leaf-$a*$a)]
        set a [expr ($pos($ml)+$r0leaf-$a-$xmin)*$xscale+$l]
        set b [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        $parent_w create line $a $c $b $c
        $parent_w create line $a $d $b $d

      } else { # straight, focused leaf ends, similar to MLC
        #use filled polygons for the leaves

        set ratio [expr $cmval($id,3)+$cmval($id,4)-$cmval($id,12)]
        set ratio [expr $ratio/($ratio-$cmval($id,4))]

        #negative side
        set y1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set z1 [expr ($cmval($id,3)-$zmin)*$zscale+$m]
        set y2 [expr ($neg($ml)-$xmin)*$xscale+$l]
        set z2 $z1
        if $ratio*$neg($ml)<-$cmval($id,0) {
          #lower edge of opening beyond -RMAX
          set y3 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
          set z3 [expr -$cmval($id,0)*($cmval($id,3)-$cmval($id,12))/$neg($ml)]
          set z3 [expr $z3-($cmval($id,3)-$cmval($id,12))]
          set z3 [expr ($z3-$zmin)*$zscale+$m]
          #draw a triangle
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 -fill $color \
                                   -outline black
        } else {
          #lower edge of opening > -RMAX
          set y3 [expr ($ratio*$neg($ml)-$xmin)*$xscale+$l]
          set z3 [expr ($cmval($id,3)+$cmval($id,4)-$zmin)*$zscale+$m]
          set y4 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
          set z4 $z3
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 $y4 $z4 -fill $color\
                                   -outline black
        }
        #draw lines to indicate screws if within RMAX
        set y1 [expr $cmval($id,3)+$cmval($id,4)-$cmval($id,7,1)-$cmval($id,12)]
        set y1 [expr $y1/($cmval($id,3)-$cmval($id,12))*$neg($ml)]
        if $y1>-$cmval($id,0) {
          set y1 [expr ($y1-$xmin)*$xscale+$l]
          set z1 [expr ($cmval($id,3)+$cmval($id,4)-$cmval($id,7,1)-$zmin)*$zscale+$m]
          set y2 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
          set z2 $z1
          $parent_w create line $y1 $z1 $y2 $z2
        }
        #assume upper screws within RMAX
        set y1 [expr $cmval($id,3)+$cmval($id,7,1)-$cmval($id,12)]
        set y1 [expr $y1/($cmval($id,3)-$cmval($id,12))*$neg($ml)]
        set y1 [expr ($y1-$xmin)*$xscale+$l]
        set z1 [expr ($cmval($id,3)+$cmval($id,7,1)-$zmin)*$zscale+$m]
        set y2 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set z2 $z1
        $parent_w create line $y1 $z1 $y2 $z2

        #positive side
        set y1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        set z1 [expr ($cmval($id,3)-$zmin)*$zscale+$m]
        set y2 [expr ($pos($ml)-$xmin)*$xscale+$l]
        set z2 $z1
        if $ratio*$pos($ml)>$cmval($id,0) {
          #lower edge of opening beyond RMAX
          set y3 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
          set z3 [expr $cmval($id,0)*($cmval($id,3)-$cmval($id,12))/$pos($ml)]
          set z3 [expr $z3-($cmval($id,3)-$cmval($id,12))]
          set z3 [expr ($z3-$zmin)*$zscale+$m]
          #draw a triangle
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 -fill $color \
                                   -outline black
        } else {
          #lower edge of opening < RMAX
          set y3 [expr ($ratio*$pos($ml)-$xmin)*$xscale+$l]
          set z3 [expr ($cmval($id,3)+$cmval($id,4)-$zmin)*$zscale+$m]
          set y4 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
          set z4 $z3
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 $y4 $z4 -fill $color\
                                   -outline black
        }
        #draw lines to indicate screws if within RMAX
        set y1 [expr $cmval($id,3)+$cmval($id,4)-$cmval($id,7,1)-$cmval($id,12)]
        set y1 [expr $y1/($cmval($id,3)-$cmval($id,12))*$pos($ml)]
        if $y1<$cmval($id,0) {
          set y1 [expr ($y1-$xmin)*$xscale+$l]
          set z1 [expr ($cmval($id,3)+$cmval($id,4)-$cmval($id,7,1)-$zmin)*$zscale+$m]
          set y2 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
          set z2 $z1
          $parent_w create line $y1 $z1 $y2 $z2
        }
        #assume upper screws within RMAX
        set y1 [expr $cmval($id,3)+$cmval($id,7,1)-$cmval($id,12)]
        set y1 [expr $y1/($cmval($id,3)-$cmval($id,12))*$pos($ml)]
        set y1 [expr ($y1-$xmin)*$xscale+$l]
        set z1 [expr ($cmval($id,3)+$cmval($id,7,1)-$zmin)*$zscale+$m]
        set y2 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        set z2 $z1
        $parent_w create line $y1 $z1 $y2 $z2
      }
    }; # end of if $ml>0
}

proc add_VARMLC_sides {id yscale zscale ymin zmin zmax l m parent_w} {
    global cmval colorlist medium nmed meds_used y z rmin rmax zf ztop nleaf
    global colornum
    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,16) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,18) $medium($j)]==0 {
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

    # add polygons for each leaf which is intersected by X=0 (leaves ||
    # to Y) or Y=0 (leaves || to X)

    # store some often used quantities first
    #zf: focus of leaf sides
    #ls: position of first tongue at ztop
    set zf $cmval($id,13)
    set ls $cmval($id,6)

    #ztop: top of material
    #zbot: bot. of material
    #ztl: top of leaf, excl. screw
    #zbl: bot. of leaf, excl. screw
    #ztt: top of tongue
    #zbt: bot. of tongue
    #ztg: top of groove
    #zbg: bot. of groove

    set ztop $cmval($id,3)
    set zbot [expr $ztop+$cmval($id,4)]
    set ztl [expr $ztop+$cmval($id,7,1)]
    set zbl [expr $zbot-$cmval($id,7,1)]
    if {$cmval($id,8,2)==0} {
        set ztt [expr $ztop+($cmval($id,4)-$cmval($id,8,1))/2]
    } else {
        set ztt $cmval($id,8,2)
    }
    set zbt [expr $ztt+$cmval($id,8,1)]
    if {$cmval($id,9,2)==0} {
        set ztg [expr $ztop+($cmval($id,4)-$cmval($id,9,1))/2]
    } else {
        set ztg $cmval($id,9,2)
    }
    set zbg [expr $ztg+$cmval($id,9,1)]

    #leaf gap width at ztop
    set lgw $cmval($id,10)

    set color [lindex $colorlist $med(leaves)]
    set rmin -$cmval($id,0)
    set rmax $cmval($id,0)

    # set up an array of neg and pos for each leaf
    set i 1; #keeps track of opening group
    set k 0; #leaf k
    while {$k < $nleaf($id)} {
	for {set j 1} {$j<=$cmval($id,14,2,$i)} {incr j} {
	    incr k
	    set neg($k) $cmval($id,14,0,$i)
	    set pos($k) $cmval($id,14,1,$i)
	}
	incr i
    }

    # see if X=0 or Y=0 intersects leaf
    # note that even if a portion of the end goes through 0, we will
    # show the entire cross section

    set i 0; #keeps track of leaf
    set k 1; #keeps track of width group
    while {$i < $nleaf($id)} {
      for {set j 1} {$j<=$cmval($id,5,0,$k)} {incr j} {
        incr i

        #all positions below are relative to ls, or the starting position
        #of the leaf tongue projected to ztop and are in the positive direction
        #they are calculated here because most depend on the leaf width, which
        #depends on what width group the leaf is in
        #yll: left side of leaf
        #yrl: right side of leaf
        #yls: left side of screw
        #yrs: right side of screw
        #ylg: left side of groove

        set yll $cmval($id,8,0)
        set yrl [expr $yll+$cmval($id,5,1,$k)]
        set yls [expr $yll+($cmval($id,5,1,$k)-$cmval($id,7,0))/2]
        set yrs [expr $yls+$cmval($id,7,0)]
        set ylg [expr $yrl-$cmval($id,9,0)]

	if { $pos($i)==$neg($i) || $pos($i)<=0 || $neg($i)>=0 } {

            #start with upper left of tongue
            set z(1) $ztt
            set y(1) [expr $ls*($ztt-$zf)/($ztop-$zf)]
            set y(1) [max_nrc $rmin $y(1)]
            set y(1) [min_nrc $rmax $y(1)]

            #lower left of tongue
            set z(3) $zbt
            set y(3) [expr $ls*($zbt-$zf)/($ztop-$zf)]
            set y(3) [max_nrc $rmin $y(3)]
            set y(3) [min_nrc $rmax $y(3)]

            #define midpoint between upper and lower left of tongue

            set_mid_pt 2 $ls $ztt $zbt

            #lower right of tongue
            set z(4) $z(3)
            set y(4) [expr ($ls+$yll)*($zbt-$zf)/($ztop-$zf)]
            set y(4) [max_nrc $rmin $y(4)]
            set y(4) [min_nrc $rmax $y(4)]

            #lower left of leaf
            set z(6) $zbl
            set y(6) [expr ($ls+$yll)*($zbl-$zf)/($ztop-$zf)]
            set y(6) [max_nrc $rmin $y(6)]
            set y(6) [min_nrc $rmax $y(6)]

            set_mid_pt 5 [expr $ls+$yll] $zbt $zbl

            #upper left of bottom screw
            set z(7) $z(6)
            set y(7) [expr ($ls+$yls)*($zbl-$zf)/($ztop-$zf)]
            set y(7) [max_nrc $rmin $y(7)]
            set y(7) [min_nrc $rmax $y(7)]

            #lower left of bottom screw
            set z(9) $zbot
            set y(9) [expr ($ls+$yls)*($zbot-$zf)/($ztop-$zf)]
            set y(9) [max_nrc $rmin $y(9)]
            set y(9) [min_nrc $rmax $y(9)]

            set_mid_pt 8 [expr $ls+$yls] $zbl $zbot

            #lower right of bottom screw
            set z(10) $z(9)
            set y(10) [expr ($ls+$yrs)*($zbot-$zf)/($ztop-$zf)]
            set y(10) [max_nrc $rmin $y(10)]
            set y(10) [min_nrc $rmax $y(10)]

            #upper right of bottom screw
            set z(12) $zbl
            set y(12) [expr ($ls+$yrs)*($zbl-$zf)/($ztop-$zf)]
            set y(12) [max_nrc $rmin $y(12)]
            set y(12) [min_nrc $rmax $y(12)]

            set_mid_pt 11 [expr $ls+$yrs] $zbl $zbot

            #lower right of leaf
            set z(13) $z(12)
            set y(13) [expr ($ls+$yrl)*($zbl-$zf)/($ztop-$zf)]
            set y(13) [max_nrc $rmin $y(13)]
            set y(13) [min_nrc $rmax $y(13)]

            #lower right of groove
            set z(15) $zbg
            set y(15) [expr ($ls+$yrl)*($zbg-$zf)/($ztop-$zf)]
            set y(15) [max_nrc $rmin $y(15)]
            set y(15) [min_nrc $rmax $y(15)]

            set_mid_pt 14 [expr $ls+$yrl] $zbg $zbl

            #lower left of groove
            set z(16) $z(15)
            set y(16) [expr ($ls+$ylg)*($zbg-$zf)/($ztop-$zf)]
            set y(16) [max_nrc $rmin $y(16)]
            set y(16) [min_nrc $rmax $y(16)]

            #upper left of groove
            set z(18) $ztg
            set y(18) [expr ($ls+$ylg)*($ztg-$zf)/($ztop-$zf)]
            set y(18) [max_nrc $rmin $y(18)]
            set y(18) [min_nrc $rmax $y(18)]

            set_mid_pt 17 [expr $ls+$ylg] $ztg $zbg

            #upper right of groove
            set z(19) $z(18)
            set y(19) [expr ($ls+$yrl)*($ztg-$zf)/($ztop-$zf)]
            set y(19) [max_nrc $rmin $y(19)]
            set y(19) [min_nrc $rmax $y(19)]

            #top right of leaf
            set z(21) $ztl
            set y(21) [expr ($ls+$yrl)*($ztl-$zf)/($ztop-$zf)]
            set y(21) [max_nrc $rmin $y(21)]
            set y(21) [min_nrc $rmax $y(21)]

            set_mid_pt 20 [expr $ls+$yrl] $ztl $ztg

            #lower right of top screw
            set z(22) $z(21)
            set y(22) [expr ($ls+$yrs)*($ztl-$zf)/($ztop-$zf)]
            set y(22) [max_nrc $rmin $y(22)]
            set y(22) [min_nrc $rmax $y(22)]

             #upper right of top screw
            set z(24) $ztop
            set y(24) [expr $ls+$yrs]
            set y(24) [max_nrc $rmin $y(24)]
            set y(24) [min_nrc $rmax $y(24)]

            set_mid_pt 23 [expr $ls+$yrs] $ztop $ztl

            #upper left of top screw
            set z(25) $z(24)
            set y(25) [expr $ls+$yls]
            set y(25) [max_nrc $rmin $y(25)]
            set y(25) [min_nrc $rmax $y(25)]

            #lower left of top screw
            set z(27) $ztl
            set y(27) [expr ($ls+$yls)*($ztl-$zf)/($ztop-$zf)]
            set y(27) [max_nrc $rmin $y(27)]
            set y(27) [min_nrc $rmax $y(27)]

            set_mid_pt 26 [expr $ls+$yls] $ztop $ztl

            #upper left of leaf
            set z(28) $z(27)
            set y(28) [expr ($ls+$yll)*($ztl-$zf)/($ztop-$zf)]
            set y(28) [max_nrc $rmin $y(28)]
            set y(28) [min_nrc $rmax $y(28)]

            #upper right of tongue
            set z(30) $ztt
            set y(30) [expr ($ls+$yll)*($ztt-$zf)/($ztop-$zf)]
            set y(30) [max_nrc $rmin $y(30)]
            set y(30) [min_nrc $rmax $y(30)]

            set_mid_pt 29 [expr $ls+$yll] $ztl $ztt

            # do a loop to set y and z scale and relative position
            for {set jj 1} {$jj<31} {incr jj} {
                set y($jj) [expr ($y($jj)-$ymin)*$yscale+$l]
                set z($jj) [expr ($z($jj)-$zmin)*$zscale+$m]
            }

            #now draw the 30-point polygon
            $parent_w create polygon $y(1) $z(1) $y(2) $z(2) $y(3) $z(3) $y(4) $z(4) $y(5) $z(5)\
      $y(6) $z(6) $y(7) $z(7) $y(8) $z(8) $y(9) $z(9) $y(10) $z(10)\
      $y(11) $z(11) $y(12) $z(12) $y(13) $z(13) $y(14) $z(14) $y(15) $z(15)\
      $y(16) $z(16) $y(17) $z(17) $y(18) $z(18) $y(19) $z(19) $y(20) $z(20)\
      $y(21) $z(21) $y(22) $z(22) $y(23) $z(23) $y(24) $z(24) $y(25) $z(25)\
      $y(26) $z(26) $y(27) $z(27) $y(28) $z(28) $y(29) $z(29) $y(30) $z(30)\
                               -fill $color -outline black
        }
        #set left of next tongue
        set ls [expr $ls+$yrl+$lgw-$yll]
      }
      incr k
    }
}

proc set_mid_pt {ind ytop zhi zlo} {
     global y z ztop zf rmin rmax

# this procedure defines a midpoint of index $ind for the polygon defining
# the leaf side view

     if {$y([expr $ind+1])<=$rmin && $y([expr $ind-1])>$rmin} {
         set y($ind) $rmin
         set z($ind) [expr $rmin/$ytop*($ztop-$zf)+$zf]
     } elseif {$y([expr $ind-1])<=$rmin && $y([expr $ind+1])>$rmin} {
         set y($ind) $rmin
         set z($ind) [expr $rmin/$ytop*($ztop-$zf)+$zf]
     } elseif {$y([expr $ind-1])>=$rmax && $y([expr $ind+1])<$rmax} {
         set y($ind) $rmax
         set z($ind) [expr $rmax/$ytop*($ztop-$zf)+$zf]
     } elseif {$y([expr $ind+1])>=$rmax && $y([expr $ind-1])<$rmax} {
         set y($ind) $rmax
         set z($ind) [expr $rmax/$ytop*($ztop-$zf)+$zf]
     } elseif {$y([expr $ind+1])<=$rmin && $y([expr $ind-1])<=$rmin} {
         set y($ind) $rmin
         set z($ind) [expr ($zhi+$zlo)/2]
     } elseif {$y([expr $ind+1])>=$rmax && $y([expr $ind-1])>=$rmax} {
         set y($ind) $rmax
         set z($ind) [expr ($zhi+$zlo)/2]
     } else {
         set z($ind) [expr ($zhi+$zlo)/2]
         set y($ind) [expr $ytop*($z($ind)-$zf)/($ztop-$zf)]
     }
}

proc help_ignoregaps { w } {

# help procedure put here because it is only for VARMLC

    set text {
If this input is set on (IGNOREGAPS=1), then all air gaps (ie between\
leaves and Z-direction gaps caused by the presence of carriage screws) will be ignored\
when doing range rejection in the leaves as long as:

particle X position < min. X of leaf openings (not including leaf ends)\
or > max. X of leaf openings (not including leaf ends) if leaves are || to X

or:

particle Y position < min. Y of leaf openings (not including leaf ends)\
or > max. Y of leaf openings (not including leaf ends) if leaves are || to Y.

This option makes\
range rejection very efficient in the leaves and can speed up simulation time\
in VARMLC by a factor of 2, while preserving exact transport in \
the leaf ends.\
However, if you have significant air gaps between the leaves, then it is\
recommended that you run with this option off (IGNOREGAPS=0; the default)\
to have exact transport throughout the entire multi-leaf collimator.}
    help_dialog $w.help "Help" $text info 0 OK
}
