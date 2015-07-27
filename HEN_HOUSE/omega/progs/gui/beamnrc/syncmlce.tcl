
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: SYNCMLCE
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
#  Authors:         Tony Popescu, 2013
#                   Julio Lobo, 2013
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Frederic Tessier
#                   Blake Walters
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


proc init_SYNCMLCE { id } {
    global cmval ngroups sync_file

    set cmval($id,0) {}
    set cmval($id,1) {}
    set cmval($id,2,0) 0
    set cmval($id,2,1) 1
    set cmval($id,3) {}
    set cmval($id,4,0) {}
    set cmval($id,4,1) {}
    set cmval($id,5,0) {}
    set cmval($id,5,1) {}
    set cmval($id,6) {}
    set cmval($id,7,0) {}
    set cmval($id,7,1) {}
    set cmval($id,8,0) {}
    set cmval($id,8,1) {}
    set cmval($id,9) {}
    set cmval($id,10) 0
    set cmval($id,11) {}
    set cmval($id,11,0) {}
    set cmval($id,11,1) {}
    # for each leaf set: neg, pos, num
    for {set i 1} {$i<=100} {incr i} {
	set cmval($id,12,0,$i) {}
	set cmval($id,12,1,$i) {}
	set cmval($id,12,2,$i) {}
    }
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,13,$i) {}
    }
    set cmval($id,14) {}
    for {set i 0} {$i<4} {incr i} {
	set cmval($id,15,$i) {}
    }
    set cmval($id,16) {}
    set ngroups($id) 1

    set sync_file($id) {}
}

proc read_SYNCMLCE { fileid id } {
    global cmval GUI_DIR ngroups cm_ident sync_file

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read SYNCMLCE $cm_ident($id).  The inputs don't begin where\
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

    # read orient
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
        set data [get_val $data cmval $id,2,$i]
    }

    # read num_leaf
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # read zmin, zmax
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
        set data [get_val $data cmval $id,4,$i]
    }

    # read zstepl, zstepr
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
        set data [get_val $data cmval $id,5,$i]
    }

    # read tgw
    gets $fileid data
    set data [get_val $data cmval $id,6]

    # read x3, x4
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
        set data [get_val $data cmval $id,7,$i]
    }

    # read space, ssd
    gets $fileid data
    for {set i 0} {$i<2} {incr i} {
        set data [get_val $data cmval $id,8,$i]
    }

    # read lbrot
    gets $fileid data
    set data [get_val $data cmval $id,9]

    # read endtype
    gets $fileid data
    set data [get_val $data cmval $id,10]

    gets $fileid data
    if { $cmval($id,10)==1 } {
       # read zfocus,
       set data [get_val $data cmval $id,11]
    } else {
       # read leafradius, cil
       for {set i 0} {$i<2} {incr i} {
         set data [get_val $data cmval $id,11,$i]
       }
    }

    # read in the leaf coordinates, neg,pos,num.  When the sum over num
    # is > num_leaf, stop reading them.
    if {$cmval($id,2,1)==1 | $cmval($id,2,1)==2} {
      gets $fileid sync_file($id)
    } else {
        set numsum 0
        set ngroups($id) 0
        while {$numsum<$cmval($id,3)} {
	    incr ngroups($id)
	    gets $fileid data
	    for {set i 0} {$i<3} {incr i} {
	        set data [get_val $data cmval $id,12,$i,$ngroups($id)]
	    }
	    if { $cmval($id,12,2,$ngroups($id))<=0 } {
	        set cmval($id,12,2,$ngroups($id)) 1
	}
	incr numsum $cmval($id,12,2,$ngroups($id))
    }
    }
    # read ecut etc in air and air gaps
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,13,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,14]

    # read ecut etc in collimator leaves
    gets $fileid data
    for {set i 0} {$i<5} {incr i} {
	set data [get_val $data cmval $id,15,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,16]
}

proc edit_SYNCMLCE { id zmax } {
    global cmval GUI_DIR helvfont cm_ident ngroups help_syncmlce_text
    global default1 sync_file inp_file_dir

    catch { destroy .syncmlce$id }
    toplevel .syncmlce$id
    wm title .syncmlce$id "Edit SYNCMLCE, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY SYNCMLCE.
    frame .syncmlce$id.top -bd 4
    set top .syncmlce$id.top


    label $top.mainlabel -text "Multi-leaf collimator $cm_ident($id)" \
	    -font $helvfont
    pack $top.mainlabel -side top -padx 10

    # SYNCMLCE_macros.mortran:REPLACE {$MAXLEAF} WITH {64}
    # read the macro file for CM SYNCMLCE to get defaults and put a label on the
    # main edit window.  Has to be different from the others because there's
    # no SYNCMLCE in the default name.

    global omega helvfont
    set filename $omega/beamnrc/CMs/SYNCMLCE_macros.mortran
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

    add_rmax_square $top.rmax $id,0
    add_title $top.title $id,1

    # leaf direction radiobuttons
    frame $top.orient -bd 2
    frame $top.orient.left
    radiobutton $top.orient.left.r1 -text "Leaves parallel to y"\
            -variable cmval($id,2,0) -value 0
    radiobutton $top.orient.left.r2 -text "Leaves parallel to x"\
            -variable cmval($id,2,0) -value 1
    pack $top.orient.left.r1 $top.orient.left.r2 -side top
    pack $top.orient.left -side left -expand true -fill x
    pack $top.orient -side top -expand true -fill x

    frame $top.leaves -bd 4
    frame $top.leaves.nleaf
    label $top.leaves.nleaf.label -text {Number of leaves}
    entry $top.leaves.nleaf.inp -textvariable cmval($id,3) -width 10
    pack $top.leaves.nleaf.label -anchor w -side left
    pack $top.leaves.nleaf.inp -anchor e -side right -fill x -expand true

    frame $top.openings -bd 4
    label $top.openings.label -text {Select field type:}

    frame $top.openings.inps
    button $top.openings.inps.help -bitmap @$GUI_DIR/help_icon.xbm \
            -command "help_syncmlcefieldtype [winfo parent $top.openings.inps]"
    frame $top.openings.inps.static
    radiobutton $top.openings.inps.static.r1 -text "Static"\
       -variable cmval($id,2,1) -value 0 \
    -command "$top.openings.inps.static.but configure -state normal;\
            $top.openings.inps.dyn.file.inp.txt configure -state disabled;\
            $top.openings.inps.dyn.file.inp.browse configure -state disabled;\
            $top.openings.inps.dyn.file.lab configure -fg grey"
    button $top.openings.inps.static.but -text "Define leaf openings >>"\
             -command "define_syncmlce_openings $id"
    if { $cmval($id,2,1)==1 | $cmval($id,2,1)==2 } {
       $top.openings.inps.static.but configure -state disabled
    }
    pack $top.openings.inps.static.r1 $top.openings.inps.static.but -side left
    frame $top.openings.inps.dyn
    frame $top.openings.inps.dyn.sel
    radiobutton $top.openings.inps.dyn.sel.r2 -text "Dynamic"\
       -variable cmval($id,2,1) -value 1 \
       -command "$top.openings.inps.dyn.file.inp.txt configure -state normal;\
           $top.openings.inps.dyn.file.inp.browse configure -state normal;\
           $top.openings.inps.dyn.file.lab configure -fg black;\
           $top.openings.inps.static.but configure -state disabled"
    radiobutton $top.openings.inps.dyn.sel.r3 -text "Step-and-shoot"\
       -variable cmval($id,2,1) -value 2 \
       -command "$top.openings.inps.dyn.file.inp.txt configure -state normal;\
           $top.openings.inps.dyn.file.inp.browse configure -state normal;\
           $top.openings.inps.dyn.file.lab configure -fg black;\
           $top.openings.inps.static.but configure -state disabled"
    pack $top.openings.inps.dyn.sel.r2 $top.openings.inps.dyn.sel.r3 \
        -side top -anchor w
    frame $top.openings.inps.dyn.file
    label $top.openings.inps.dyn.file.lab \
           -text {File containing leaf opening data:}
    frame $top.openings.inps.dyn.file.inp
    entry $top.openings.inps.dyn.file.inp.txt -textvariable sync_file($id) -width 20
    button $top.openings.inps.dyn.file.inp.browse -text "Browse"\
             -command "browse set_syncfile $inp_file_dir $id"
    if { $cmval($id,2,1)==0 } {
        $top.openings.inps.dyn.file.lab configure -fg grey
        $top.openings.inps.dyn.file.inp.txt configure -state disabled
        $top.openings.inps.dyn.file.inp.browse configure -state disabled
    }
    pack $top.openings.inps.dyn.file.inp.txt $top.openings.inps.dyn.file.inp.browse -side left -expand true
    pack $top.openings.inps.dyn.file.lab $top.openings.inps.dyn.file.inp \
         -side top -anchor w
    pack $top.openings.inps.dyn.sel $top.openings.inps.dyn.file -side left
    pack $top.openings.inps.help $top.openings.inps.static \
         $top.openings.inps.dyn -side left -padx 10
    pack $top.openings.label $top.openings.inps -side top
    pack $top.openings -side top -fill x


#    button $top.leaves.def.open -text "Define leaf openings >>" -command \
#            "define_syncmlce_leaves $id"
#    pack $top.leaves.def.open -anchor w -side left -fill x -expand true
#    pack $top.leaves.nleaf -side left -fill x
#    pack $top.leaves.def -side left -padx 5 -fill x
#    pack $top.leaves -side top -fill x

    frame $top.inpzmin -bd 4
    label $top.inpzmin.label -text {min. Z of leaves (cm)}
    entry $top.inpzmin.inp -textvariable cmval($id,4,0)
    pack $top.inpzmin.label -anchor w -side left
    pack $top.inpzmin.inp -anchor e -side right -fill x -expand true
    pack $top.inpzmin -side top -fill x

    frame $top.inpzmax -bd 4
    label $top.inpzmax.label -text {max. Z of leaves (cm)}
    entry $top.inpzmax.inp -textvariable cmval($id,4,1)
    pack $top.inpzmax.label -anchor w -side left
    pack $top.inpzmax.inp -anchor e -side right -fill x -expand true
    pack $top.inpzmax -side top -fill x

    frame $top.inpzstepl -bd 4
    label $top.inpzstepl.label -text {Z posn. of left step (cm)}
    entry $top.inpzstepl.inp -textvariable cmval($id,5,0)
    pack $top.inpzstepl.label -anchor w -side left
    pack $top.inpzstepl.inp -anchor e -side right -fill x -expand true
    pack $top.inpzstepl -side top -fill x

    frame $top.inpzstepr -bd 4
    label $top.inpzstepr.label -text {Z posn. of right step (cm)}
    entry $top.inpzstepr.inp -textvariable cmval($id,5,1)
    pack $top.inpzstepr.label -anchor w -side left
    pack $top.inpzstepr.inp -anchor e -side right -fill x -expand true
    pack $top.inpzstepr -side top -fill x

    frame $top.tgw -bd 4
    label $top.tgw.label -text {width of step (cm)}
    entry $top.tgw.inp -textvariable cmval($id,6)
    pack $top.tgw.label -anchor w -side left
    pack $top.tgw.inp -anchor e -side right -fill x -expand true
    pack $top.tgw -side top -fill x

    frame $top.x3 -bd 4
    label $top.x3.label -text {top right coordinate of central leaf (cm)}
    entry $top.x3.inp -textvariable cmval($id,7,0) -width 10
    pack $top.x3.label -anchor w -side left
    pack $top.x3.inp -anchor e -side right -fill x -expand true
    pack $top.x3 -side top -fill x

    frame $top.x4 -bd 4
    label $top.x4.label -text {bottom right coordinate of central leaf (cm)}
    entry $top.x4.inp -textvariable cmval($id,7,1) -width 10
    pack $top.x4.label -anchor w -side left
    pack $top.x4.inp -anchor e -side right -fill x -expand true
    pack $top.x4 -side top -fill x

    frame $top.space -bd 4
    label $top.space.label -text {spacing between leaf centres at SSD (cm)}
    entry $top.space.inp -textvariable cmval($id,8,0) -width 10
    pack $top.space.label -anchor w -side left
    pack $top.space.inp -anchor e -side right -fill x -expand true
    pack $top.space -side top -fill x

    frame $top.ssd -bd 4
    label $top.ssd.label -text {SSD at which spacing defined (cm)}
    entry $top.ssd.inp -textvariable cmval($id,8,1) -width 10
    pack $top.ssd.label -anchor w -side left
    pack $top.ssd.inp -anchor e -side right -fill x -expand true
    pack $top.ssd -side top -fill x

    frame $top.lbrot -bd 4
    label $top.lbrot.label -text {Leaf bank rotation angle (radians)}
    entry $top.lbrot.inp -textvariable cmval($id,9) -width 10
    pack $top.lbrot.label -anchor w -side left
    pack $top.lbrot.inp -anchor e -side right -fill x -expand true
    pack $top.lbrot -side top -fill x

    frame $top.endparam

    frame $top.endparam.endtype -bd 4
    label $top.endparam.endtype.label -text {Leaf end type}
    end_param_menu_syncmlce $top.endparam.endtype.inp $id
    pack $top.endparam.endtype.label -anchor w -side left
    pack $top.endparam.endtype.inp -anchor e -side right -fill x -expand true


    frame $top.endparam.focrad -bd 4
    frame $top.endparam.focrad.zr
    if {$cmval($id,10)==1} {
        label $top.endparam.focrad.zr.label -text {Zfocus of leaf ends}
    } else {
        label $top.endparam.focrad.zr.label -text {Radius of leaf ends}
    }
    entry $top.endparam.focrad.zr.inp -textvariable cmval($id,11,0) -width 20
    pack $top.endparam.focrad.zr.label -anchor w -side left
    pack $top.endparam.focrad.zr.inp -anchor e -side right -fill x -expand true
    frame $top.endparam.focrad.cil
    label $top.endparam.focrad.cil.label -text {Z of cylinder origin (cm)}
    entry $top.endparam.focrad.cil.inp -textvariable cmval($id,11,1) -width 20
    pack $top.endparam.focrad.cil.label -anchor w -side left
    pack $top.endparam.focrad.cil.inp -anchor e -side right -fill x -expand true
    pack $top.endparam.focrad.zr $top.endparam.focrad.cil -side top -fill x
    if {$cmval($id,10)==1} {
        $top.endparam.focrad.cil.label configure -fg grey
        $top.endparam.focrad.cil.inp configure -state disabled
    }

    pack $top.endparam.endtype -anchor w -side left
    pack $top.endparam.focrad -anchor e -side right -fill x -expand true
    pack $top.endparam -side top -fill x

    pack $top -side top -pady 5

#now get ecut,pcut,materials,etc

    frame .syncmlce$id.bottom
    frame .syncmlce$id.bottom.left
    frame .syncmlce$id.bottom.right

    label .syncmlce$id.bottom.left.lab -text "Opening(s) and air gaps" \
                       -font $helvfont
    pack .syncmlce$id.bottom.left.lab -side top -pady 5
    add_ecut .syncmlce$id.bottom.left.f0 $id,13,0
    add_pcut .syncmlce$id.bottom.left.f1 $id,13,1
    add_dose .syncmlce$id.bottom.left.f2 $id,13,2
    add_latch .syncmlce$id.bottom.left.f3 $id,13,3
    add_material .syncmlce$id.bottom.left.f4 $id,14

    label .syncmlce$id.bottom.right.lab -text "Collimator leaves" -font $helvfont
    pack .syncmlce$id.bottom.right.lab -side top -pady 5
    add_ecut .syncmlce$id.bottom.right.f0 $id,15,0
    add_pcut .syncmlce$id.bottom.right.f1 $id,15,1
    add_dose .syncmlce$id.bottom.right.f2 $id,15,2
    add_latch .syncmlce$id.bottom.right.f3 $id,15,3
    add_material .syncmlce$id.bottom.right.f4 $id,16

    pack .syncmlce$id.bottom.left -side left -anchor n -padx 10
    pack .syncmlce$id.bottom.right -side right -anchor n -padx 10

    pack .syncmlce$id.bottom -side top -pady 5

    frame .syncmlce$id.buts -bd 4
    button .syncmlce$id.buts.okb -text "OK" -command "destroy .syncmlce$id"\
	    -relief groove -bd 8
    button .syncmlce$id.buts.helpb -text "Help" -command\
	    "help_gif .syncmlce$id.help {$help_syncmlce_text} help_syncmlce"\
	    -relief groove -bd 8
#don't have a preview yet, get it working after course, will call show_SYNCMLCE
    button .syncmlce$id.buts.prev -text "Preview" -command\
                          "show_SYNCMLCE $id" -relief groove -bd 8
#	    { tk_dialog .nopreview "No Preview" \
#             "No preview available yet." info 0 OK } -relief groove -bd 8
    pack .syncmlce$id.buts.helpb .syncmlce$id.buts.okb .syncmlce$id.buts.prev -side left -padx 10
    pack .syncmlce$id.buts -pady 10
}

proc end_param_menu_syncmlce { w id } {
    global cmval

    set inw .syncmlce$id.top.endparam.focrad.zr.label
    set inx .syncmlce$id.top.endparam.focrad.cil.label
    set iny .syncmlce$id.top.endparam.focrad.cil.inp
    if { $cmval($id,10)==1 } {
      menubutton $w -text "straight" -menu  $w.m -bd 1 \
                  -relief raised -indicatoron 1 -width 10
    } else {
      menubutton $w -text "cylindrical" -menu  $w.m -bd 1 \
                  -relief raised -indicatoron 1 -width 10
    }
    menu $w.m -tearoff no
    $w.m add command -label "cylindrical" \
               -command "set_endval_syncmlce $id 0 $w;\
                         $inw configure -text {Radius of leaf ends};\
                         $inx configure -fg black;\
                         $iny configure -state normal"
    $w.m add command -label "straight" \
               -command "set_endval_syncmlce $id 1 $w;\
                         $inw configure -text {Zfocus of leaf ends};\
                         $inx configure -fg grey;\
                         $iny configure -state disabled"
}

proc set_endval_syncmlce { id val w } {
    global cmval

    set cmval($id,10) $val

    if { $val==1 } {
        $w configure -text "straight"
    } else {
        $w configure -text "cylindrical"
    }
}

proc define_syncmlce_openings { id } {
    global cmval helvfont ngroups to from help_syncmlce_text

    if {$cmval($id,3)==""} {
       tk_dialog .setnleaf "Set number of leaves" "Before you can specify\
       the leaf openings, you must specify the number of leaves." warning 0 OK
       return
    }

    catch { destroy .syncmlce$id.child }
    toplevel .syncmlce$id.child
    set top .syncmlce$id.child
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
	    each leaf in the SYNCMLCE by groups here.  Add a row to start a\
	    new group.  When a new row is added the next leaf in the order will\
	    automatically appear in the 'from leaf' box.  This must repeat\
	    until you have defined the minimum and maximum for all\
	    $cmval($id,3) leaves." -width 400 -font $helvfont
    pack $w.message -side top -anchor n -fill x -expand true

    frame $w.grid -bd 4
    label $w.grid.l0 -text "from leaf"
    label $w.grid.l1 -text "to leaf"
    if { $cmval($id,10)==0 } {
      label $w.grid.l2 -text "min. $orientation of origin of leaf ends"
      label $w.grid.l3 -text "max. $orientation of origin of leaf ends"
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
	if [catch {set to($j) [expr $to([expr $j-1])+$cmval($id,12,2,$j)]}]==1 {
	    set to($j) {}
	}
	entry $w.grid.e0$j -textvariable from($j) -width 8
	grid configure $w.grid.e0$j -row $j -column 0
	entry $w.grid.e1$j -textvariable to($j) -width 8
	grid configure $w.grid.e1$j -row $j -column 1
	entry $w.grid.e2$j -textvariable cmval($id,12,0,$j) -width 15
	grid configure $w.grid.e2$j -row $j -column 2
	entry $w.grid.e3$j -textvariable cmval($id,12,1,$j) -width 15
	grid configure $w.grid.e3$j -row $j -column 3
    }
    pack $w.grid -side top

    pack $w
    frame $top.b
    button $top.b.addb -text "Add a row" -command "add_syncmlce_row $id"\
	    -relief groove -bd 8
    button $top.b.delb -text "Delete last row"\
            -command "del_syncmlce_row $id" -relief groove -bd 8
    button $top.b.okb -text "OK" -command "save_syncmlce $id" -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
	    "help_gif .syncmlce$id.child.help {$help_syncmlce_text} help_syncmlce"\
	    -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb $top.b.helpb -side left -padx 10
    pack $top.b -pady 10
}

proc save_syncmlce { id } {
    global cmval ngroups from to

    if {$to($ngroups($id))>$cmval($id,3)} {
       tk_dialog .toomanyopeninggroups "Too many leaves" "You have specified too many\
       leaf openings for the number of leaves ($cmval($id,3))." warning 0 OK
       return
    } elseif {$to($ngroups($id))<$cmval($id,3)} {
       tk_dialog .toofewopeninggroups "Too few leaves" "You have specified too few\
       leaf openings for the number of leaves ($cmval($id,3))." warning 0 OK
       return
    }

    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	if [catch {set cmval($id,12,2,$j) [expr $to($j)-$from($j)+1]}]==1 {
	    tk_dialog .nope "No no no" "The leaf geometries for group $j\
		    have been improperly set or not set at all.\
		    Go back and fix them." warning 0 OK
	}
    }

    destroy .syncmlce$id.child

}

proc add_syncmlce_row { id } {
    global ngroups from to cmval

    set w .syncmlce$id.child.f

    if $to($ngroups($id))>=$cmval($id,3) {
	# They've been finished already.  Refuse.
	tk_dialog .refuse "Too many" "You've already completed the leaves.\
		I can't add another row unless you change the last 'to'\
		box to a values less than $cmval($id,3) or increase the\
		number of leaves." warning 0 OK
	return
    }

    set j [incr ngroups($id)]
    set from($j) [expr $to([expr $j-1])+1]
    if [catch {set to($j) [expr $to([expr $j-1])+$cmval($id,12,2,$j)]}]==1 {
	set to($j) {}
    }
    entry $w.grid.e0$j -textvariable from($j) -width 8
    grid configure $w.grid.e0$j -row $j -column 0
    entry $w.grid.e1$j -textvariable to($j) -width 8
    grid configure $w.grid.e1$j -row $j -column 1
    entry $w.grid.e2$j -textvariable cmval($id,12,0,$j) -width 15
    grid configure $w.grid.e2$j -row $j -column 2
    entry $w.grid.e3$j -textvariable cmval($id,12,1,$j) -width 15
    grid configure $w.grid.e3$j -row $j -column 3

}

proc del_syncmlce_row { id } {
    global ngroups

    set w .syncmlce$id.child.f

    for {set i 0} {$i<4} {incr i} {
         destroy $w.grid.e$i$ngroups($id)
    }
    incr ngroups($id) -1
}

proc write_SYNCMLCE {fileid id} {
    global cmval cm_names cm_ident cm_type ngroups sync_file

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2,0), $cmval($id,2,1), ORIENT, MODE"
    puts $fileid "$cmval($id,3), NUM_LEAF"
    puts $fileid "$cmval($id,4,0), $cmval($id,4,1), ZMIN, ZMAX"
    puts $fileid "$cmval($id,5,0), $cmval($id,5,1), ZSTEPL, ZSTEPR"
    puts $fileid "$cmval($id,6), TGW"
    puts $fileid "$cmval($id,7,0), $cmval($id,7,1), X3, X4"
    puts $fileid "$cmval($id,8,0), $cmval($id,8,1), SPACE, SSD"
    puts $fileid "$cmval($id,9), LBROT"
    puts $fileid "$cmval($id,10), ENDTYPE"
    if { $cmval($id,10)==1 } {
       puts $fileid "$cmval($id,11), ZFOCUS"
    } else {
       puts $fileid "$cmval($id,11,0), $cmval($id,11,1), LEAFRADIUS, CIL"
    }

    if { $cmval($id,2,1)==1 | $cmval($id,2,1)==2 } {
        puts $fileid "$sync_file($id)"
    } else {
    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	puts $fileid "$cmval($id,12,0,$j), $cmval($id,12,1,$j), $cmval($id,12,2,$j)"
    }
    }

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,13,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,14)

    set str {}
    for {set i 0} {$i<4} {incr i} {
	set str "$str$cmval($id,15,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,16)

}


proc show_SYNCMLCE { id } {
    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks

    catch { destroy .syncmlce$id.show }
    toplevel .syncmlce$id.show
    wm title .syncmlce$id.show "Preview"

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
    frame .syncmlce$id.show.frm

    draw_SYNCMLCE $id

    frame .syncmlce$id.show.buts
    button .syncmlce$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .syncmlce$id.show xyz" -relief groove -bd 8
    button .syncmlce$id.show.buts.print1 -text "Print xy..." -command\
	    "print_canvas .syncmlce$id.show.frm.can.one 450 450" -relief groove -bd 8
    button .syncmlce$id.show.buts.print2 -text "Print xz..." -command\
	    "print_canvas .syncmlce$id.show.frm.can.two 450 450" -relief groove -bd 8
    button .syncmlce$id.show.buts.print3 -text "Print yz..." -command\
	    "print_canvas .syncmlce$id.show.frm.can.three 450 450" -relief groove -bd 8
    button .syncmlce$id.show.buts.done -text "Done" -command\
	    "destroy .syncmlce$id.show" -relief groove -bd 8
    pack .syncmlce$id.show.buts.range .syncmlce$id.show.buts.print1\
	    .syncmlce$id.show.buts.print2 .syncmlce$id.show.buts.print3\
	    .syncmlce$id.show.buts.done -side left -padx 10
    pack .syncmlce$id.show.buts -side bottom -anchor s -pady 15
}

proc draw_SYNCMLCE { id } {
    global helvfont zrange yrange xrange zscale yscale xscale l m
    global cmval cm_ticks transmod

    catch { destroy .syncmlce$id.show.frm.can }

    # put the canvas up
    set ncan 3
    set width 300.0
    set canwidth [expr $width+150.0]
    set scrlheight [expr 2*$canwidth]
    set scrlwidth [expr 2*$canwidth]
    set winwidth [expr [winfo screenwidth .]*4.0/5.0]
    set winheight [expr [winfo screenheight .]*4.0/5.0]

    if $scrlwidth>$winwidth {
	catch { destroy .syncmlce$id.show.frm.scrlz }
	catch { destroy .syncmlce$id.show.frm.scrlx }
	scrollbar .syncmlce$id.show.frm.scrlz -command\
		".syncmlce$id.show.frm.can yview"
	scrollbar .syncmlce$id.show.frm.scrlx -command\
		".syncmlce$id.show.frm.can xview" -orient horizontal
	pack .syncmlce$id.show.frm.scrlz -side right -fill y
	pack .syncmlce$id.show.frm.scrlx -side bottom -fill x
	canvas .syncmlce$id.show.frm.can -width $winwidth -height $winheight\
		-yscrollcommand ".syncmlce$id.show.frm.scrlz set"\
		-xscrollcommand ".syncmlce$id.show.frm.scrlx set"\
		-scrollregion "0 0 $scrlwidth $scrlheight"
    } else {
	canvas .syncmlce$id.show.frm.can -width $scrlwidth -height $scrlheight
    }

    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    set l 100.0
    set m 50.0

    # XY, upper right

    canvas .syncmlce$id.show.frm.can.one -width $canwidth -height $canwidth

    add_SYNCMLCE_xy $id $xscale $yscale $xrange(0) $yrange(0)\
	    $l $m .syncmlce$id.show.frm.can.one

    if { $cmval($id,2,1)==1 || $cmval($id,2,1)==2 } {

      if {$transmod==0} {
    # Put some text in the upper left corner, just to fill the gap.
      .syncmlce$id.show.frm.can create text 225 225 -text "This is a dynamic or\
              step-and-shoot input with the leaf opening coordinates for all fields\
              defined in a file.  Opening coordinates are shown only for the first\
              field defined in this file.

The preview of the leaf sides\
              shows either the leaf that passes through x (yz view) or y\
              (xz view) equal to zero or, in the case where two adjacent leaves\
              straddle the x or y axis, the leaf that is closest to x=0 or y=0.\
              The cross-section perpendicular to the leaves disregards leaf\
              ends.  The xy view shows only the leaf surfaces at Z=ZMIN."\
              -font $helvfont -width 300
      } else {
      .syncmlce$id.show.frm.can create text 225 225 -text "This is a dynamic or\
              step-and-shoot input with the leaf opening coordinates for all fields\
              defined in a file.  Opening coordinates are shown only for the first\
              field defined in this file.

The preview of the leaf sides\
              shows either the leaf that passes through x (yz view) or y\
              (xz view) equal to zero or, in the case where two adjacent leaves\
              straddle the x or y axis, the leaf that is closest to x=0 or y=0.\
              The cross-section perpendicular to the leaves disregards leaf\
              ends.  The xy view shows only the leaf surfaces at Z=ZMIN.\


WARNING: Leaves shown are simply translated to be SPACE cm apart\
              rather than using SPACE, SSD and requirment to focus at Z=0,\
            since the latter will result in overlapping leaves.  This will\
            also happen during a run with this input."\
            -font $helvfont -width 300
    }
   } else {
    if {$transmod==0} {
    # Put some text in the upper left corner, just to fill the gap.
      .syncmlce$id.show.frm.can create text 225 225 -text "The preview of the leaf sides\
              shows either the leaf that passes through x (yz view) or y\
              (xz view) equal to zero or, in the case where two adjacent leaves\
              straddle the x or y axis, the leaf that is closest to x=0 or y=0.\
              The cross-section perpendicular to the leaves disregards leaf\
              ends.  The xy view shows only the leaf surfaces at Z=ZMIN."\
              -font $helvfont -width 300
      } else {
      .syncmlce$id.show.frm.can create text 225 225 -text "The preview of the leaf sides\
              shows either the leaf that passes through x (yz view) or y\
              (xz view) equal to zero or, in the case where two adjacent leaves\
              straddle the x or y axis, the leaf that is closest to x=0 or y=0.\
              The cross-section perpendicular to the leaves disregards leaf\
              ends.  The xy view shows only the leaf surfaces at Z=ZMIN.\


WARNING: Leaves shown are simply translated to be SPACE cm apart\
              rather than using SPACE, SSD and requirment to focus at Z=0,\
            since the latter will result in overlapping leaves.  This will\
            also happen during a run with this input."\
            -font $helvfont -width 300
    }
   }

    set curx 0
    set cury 0
    label .syncmlce$id.show.frm.can.one.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .syncmlce$id.show.frm.can.one create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .syncmlce$id.show.frm.can.one.xy
    bind .syncmlce$id.show.frm.can.one <Motion> {
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
    coverup $l $m $width .syncmlce$id.show.frm.can.one

    add_axes xy $cm_ticks($id,x) $cm_ticks($id,y) $l $m $width $xrange(0)\
	    $xrange(1) $yrange(0) $yrange(1)\
	    $xscale $yscale .syncmlce$id.show.frm.can.one

    .syncmlce$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .syncmlce$id.show.frm.can.one

    # XZ, lower left: if leaves // to y, show leaf sides,
    # else if leaves // to x, show only one leaf at y=0 (leaf ends).

    canvas .syncmlce$id.show.frm.can.two -width $canwidth -height $canwidth

    add_air $id .syncmlce$id.show.frm.can.two $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    if $cmval($id,2,0)==0 {
	add_SYNCMLCE_sides $id $xscale $zscale $xrange(0) $zrange(0) $zrange(1)\
		$l $m .syncmlce$id.show.frm.can.two
    } else {
        add_SYNCMLCE_ends $id $xscale $zscale $xrange(0) $zrange(0) $zrange(1)\
		$l $m .syncmlce$id.show.frm.can.two
    }

    coverup $l $m $width .syncmlce$id.show.frm.can.two

    label .syncmlce$id.show.frm.can.two.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .syncmlce$id.show.frm.can.two create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .syncmlce$id.show.frm.can.two.xy
    bind .syncmlce$id.show.frm.can.two <Motion> {
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
	    $xscale $zscale .syncmlce$id.show.frm.can.two

    .syncmlce$id.show.frm.can create window [expr $canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .syncmlce$id.show.frm.can.two

    # YZ, lower right

    canvas .syncmlce$id.show.frm.can.three -width $canwidth -height $canwidth

    add_air $id .syncmlce$id.show.frm.can.three $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    if $cmval($id,2,0)==0 {
	add_SYNCMLCE_ends $id $yscale $zscale $yrange(0) $zrange(0) $zrange(1)\
		$l $m .syncmlce$id.show.frm.can.three
    } else {
	add_SYNCMLCE_sides $id $yscale $zscale $yrange(0) $zrange(0) $zrange(1)\
		$l $m .syncmlce$id.show.frm.can.three
    }

    coverup $l $m $width .syncmlce$id.show.frm.can.three

    label .syncmlce$id.show.frm.can.three.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .syncmlce$id.show.frm.can.three create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .syncmlce$id.show.frm.can.three.xy
    bind .syncmlce$id.show.frm.can.three <Motion> {
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
	    $yscale $zscale .syncmlce$id.show.frm.can.three

    .syncmlce$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .syncmlce$id.show.frm.can.three

    pack .syncmlce$id.show.frm.can -side top -anchor n
    pack .syncmlce$id.show.frm -side top -anchor n
}

proc add_SYNCMLCE_xy {id xscale yscale xmin ymin l m parent_w} {

    global cmval colorlist medium nmed colornum transmod sync_file

    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,14) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,16) $medium($j)]==0 {
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

    #if this is a dynamic or step-and-shoot input, then
    #read in opening coordinates of first field and put them
    #in the appropriate location in the cmval array
    if {$cmval($id,2,1)==1 || $cmval($id,2,1)==2} {
      set fileid [open $sync_file($id) "r"]
      #first three lines are title, no. of fields, muindex of first field
      gets $fileid data
      gets $fileid data
      gets $fileid data
      set nleaf 0; set ngroups 0;
      while {$nleaf<$cmval($id,3)} {
            incr ngroups
            gets $fileid data
            for {set i 0} {$i<3} {incr i} {
                set data [get_val $data cmval $id,12,$i,$ngroups]
            }
            if { $cmval($id,12,2,$ngroups)<=0 } {
                set cmval($id,12,2,$ngroups) 1
            }
            incr nleaf $cmval($id,12,2,$ngroups)
       }
    }


    # set up an array of neg and pos for each leaf
    set i 1; #group i
    set k 0; #leaf k
    while {$k < $cmval($id,3)} {
	for {set j 1} {$j<=$cmval($id,12,2,$i)} {incr j} {
	    incr k
            if {$cmval($id,10)==0} {
               # add 'n subtract leaf radius
               set neg($k) [expr $cmval($id,12,0,$i)+$cmval($id,11,0)]
               set pos($k) [expr $cmval($id,12,1,$i)-$cmval($id,11,0)]
            } else {
	       set neg($k) $cmval($id,12,0,$i)
	       set pos($k) $cmval($id,12,1,$i)
            }
	}
	incr i
    }

    set color [lindex $colorlist $med(leaves)]

    # if it's parallel to x, neg and pos are x-values, else neg and pos are
    # y-values.  Define a rectangle by (a,b) to (c,d).

    #define xref8, the top right corner of the imaginary central leaf
    #also calculate xref1, top left corner of the imaginary central leaf

    set xref8 $cmval($id,7,0)
    set xref1 [expr -$cmval($id,7,0)+$cmval($id,6)]
    set transmod 0
    #set to 1 if translation modified because of overlapping
                   #leaves

    if $cmval($id,2,0)==1 {
	# leaves parallel to x
	set rmin [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	set rmax [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        for {set j 1} {$j<=[expr $cmval($id,3)/2]} {incr j} {
           #for leaf on +ve Y side
           set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
           #for leaf on -ve Y side
           set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
           if {$j==1} {
              set translr1 $translr
              set transll1 $transll
           }
           if {[expr $xref1+$translr1]<[expr $xref8+$transll1]} {
               #centremost leaves will overlap (unlikely that rotation
               #will change this)
               set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)]
               set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)]
               set transmod 1
           }
           set rotr [expr atan((2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
           set xtempr8 [expr cos($rotr)*$xref8+$translr]
           set xtempr1 [expr cos($rotr)*$xref1+$translr]
           #now apply leaf bank rotation
           set xtempr8 [expr cos($cmval($id,9))*$xtempr8-\
                             sin($cmval($id,9))*sin($rotr)*$xref8]
           set xtempr1 [expr cos($cmval($id,9))*$xtempr1-\
                             sin($cmval($id,9))*sin($rotr)*$xref1]
           set a [expr ($xtempr8-$ymin)*$yscale+$m]
           set b [expr ($pos([expr $cmval($id,3)/2+$j])-$xmin)*$xscale+$l]
           set c [expr ($xtempr1-$ymin)*$yscale+$m]
           set d [expr ($neg([expr $cmval($id,3)/2+$j])-$xmin)*$xscale+$l]
           #draw leaf on +ve Y side
           $parent_w create rectangle $rmin $a $d $c -fill $color -outline black
           $parent_w create rectangle $rmax $a $b $c -fill $color -outline black

           set rotl [expr atan(-(2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
           set xtempl8 [expr cos($rotl)*$xref8+$transll]
           set xtempl1 [expr cos($rotl)*$xref1+$transll]
           #now apply leaf bank rotation
           set xtempl8 [expr cos($cmval($id,9))*$xtempl8-\
                             sin($cmval($id,9))*sin($rotl)*$xref8]
           set xtempl1 [expr cos($cmval($id,9))*$xtempl1-\
                             sin($cmval($id,9))*sin($rotl)*$xref1]
           set a [expr ($xtempl8-$ymin)*$yscale+$m]
           set b [expr ($pos([expr $cmval($id,3)/2+1-$j])-$xmin)*$xscale+$l]
           set c [expr ($xtempl1-$ymin)*$yscale+$m]
           set d [expr ($neg([expr $cmval($id,3)/2+1-$j])-$xmin)*$xscale+$l]
           #draw leaf on -ve Y side
           $parent_w create rectangle $rmin $a $d $c -fill $color -outline black
           $parent_w create rectangle $rmax $a $b $c -fill $color -outline black
        }
    } else {
	# leaves parallel to y
        set rmin [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
        set rmax [expr ($cmval($id,0)-$ymin)*$yscale+$m]
        for {set j 1} {$j<=[expr $cmval($id,3)/2]} {incr j} {
           #for leaf on +ve X side
           set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
           #for leaf on -ve Y side
           set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
           if {$j==1} {
              set translr1 $translr
              set transll1 $transll
           }
           if {[expr $xref1+$translr1]<[expr $xref8+$transll1]} {
               #centremost leaves will overlap (unlikely that rotation
               #will change this)
               set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)]
               set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)]
               set transmod 1
           }
           set rotr [expr atan((2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
           set xtempr8 [expr cos($rotr)*$xref8+$translr]
           set xtempr1 [expr cos($rotr)*$xref1+$translr]
           #now apply leaf bank rotation
           set xtempr8 [expr cos($cmval($id,9))*$xtempr8-\
                             sin($cmval($id,9))*sin($rotr)*$xref8]
           set xtempr1 [expr cos($cmval($id,9))*$xtempr1-\
                             sin($cmval($id,9))*sin($rotr)*$xref1]
           set a [expr ($xtempr8-$xmin)*$xscale+$l]
           set b [expr ($pos([expr $cmval($id,3)/2+$j])-$ymin)*$yscale+$m]
           set c [expr ($xtempr1-$xmin)*$xscale+$l]
           set d [expr ($neg([expr $cmval($id,3)/2+$j])-$ymin)*$yscale+$m]
           #draw leaf on +ve Y side
           $parent_w create rectangle $a $rmin $c $d -fill $color -outline black
           $parent_w create rectangle $a $rmax $c $b -fill $color -outline black

           set rotl [expr atan(-(2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
           set xtempl8 [expr cos($rotl)*$xref8+$transll]
           set xtempl1 [expr cos($rotl)*$xref1+$transll]
           #now apply leaf bank rotation
           set xtempl8 [expr cos($cmval($id,9))*$xtempl8-\
                             sin($cmval($id,9))*sin($rotl)*$xref8]
           set xtempl1 [expr cos($cmval($id,9))*$xtempl1-\
                             sin($cmval($id,9))*sin($rotl)*$xref1]
           set a [expr ($xtempl8-$xmin)*$xscale+$l]
           set b [expr ($pos([expr $cmval($id,3)/2+1-$j])-$ymin)*$yscale+$m]
           set c [expr ($xtempl1-$xmin)*$xscale+$l]
           set d [expr ($neg([expr $cmval($id,3)/2+1-$j])-$ymin)*$yscale+$m]
           #draw leaf on -ve Y side
           $parent_w create rectangle $a $rmin $c $d -fill $color -outline black
           $parent_w create rectangle $a $rmax $c $b -fill $color -outline black
       }
    }
}


proc add_SYNCMLCE_ends {id xscale zscale xmin zmin zmax l m parent_w} {
    global cmval colorlist medium nmed meds_used colornum sync_file

    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,14) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,16) $medium($j)]==0 {
	    set med(leaves) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    # add a rectangle
    set x1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z1 [expr ($cmval($id,4,0)-$zmin)*$zscale+$m]
    set x2 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z2 [expr ($cmval($id,4,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(in)]
    $parent_w create rectangle $x1 $z1 $x2 $z2 -fill $color -outline {}

    #if this is a dynamic or step-and-shoot input, then
    #read in opening coordinates of first field and put them
    #in the appropriate location in the cmval array
    if {$cmval($id,2,1)==1 || $cmval($id,2,1)==2} {
      set fileid [open $sync_file($id) "r"]
      #first three lines are title, no. of fields, muindex of first field
      gets $fileid data
      gets $fileid data
      gets $fileid data
      set nleaf 0; set ngroups 0;
      while {$nleaf<$cmval($id,3)} {
            incr ngroups
            gets $fileid data
            for {set i 0} {$i<3} {incr i} {
                set data [get_val $data cmval $id,12,$i,$ngroups]
            }
            if { $cmval($id,12,2,$ngroups)<=0 } {
                set cmval($id,12,2,$ngroups) 1
            }
            incr nleaf $cmval($id,12,2,$ngroups)
       }
    }

    # set up an array of neg and pos for each leaf
    set i 1; #opening group i
    set k 0; #leaf k
    while {$k < $cmval($id,3)} {
             for {set j 1} {$j<=$cmval($id,12,2,$i)} {incr j} {
                 incr k
                 set neg($k) $cmval($id,12,0,$i)
                 set pos($k) $cmval($id,12,1,$i)
             }
             incr i
    }

    #now find out which leaf is intersected by X=0 (if leaves || to Y)
    #or Y=0 (if leaves || to X)
    #if no leaf intersects 0, then use the one that is closest
    #first, define coordinates of central leaf, the following makes
    #translation/rotation easier
    set xref(8) $cmval($id,7,0)
    set xref(1) [expr -$cmval($id,7,0)+$cmval($id,6)]
    set xref(4) -$cmval($id,7,1)
    set xref(3) [expr -$cmval($id,7,0)+($cmval($id,7,0)-$cmval($id,7,1))/\
                      ($cmval($id,4,1)-$cmval($id,4,0))*\
                      ($cmval($id,5,0)-$cmval($id,4,0))]
    set xref(2) [expr $xref(3)+$cmval($id,6)]
    set xref(7) [expr $cmval($id,7,0)+($cmval($id,7,1)-$cmval($id,7,0))/\
                      ($cmval($id,4,1)-$cmval($id,4,0))*\
                      ($cmval($id,5,1)-$cmval($id,4,0))]
    set xref(6) [expr $xref(7)-$cmval($id,6)]
    set xref(5) [expr $cmval($id,7,1)-$cmval($id,6)]
    set zref(1) $cmval($id,4,0)
    set zref(2) $cmval($id,5,0)
    set zref(3) $cmval($id,5,0)
    set zref(4) $cmval($id,4,1)
    set zref(5) $cmval($id,4,1)
    set zref(6) $cmval($id,5,1)
    set zref(7) $cmval($id,5,1)
    set zref(8) $cmval($id,4,0)

    set ml 0

    for {set j 1} {$j<= [expr $cmval($id,3)/2]} {incr j} {
         #translation for leaves on right side of || axis
         set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
         #translation for leaves on left side of || axis
         set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
         if {$j==1} {
              set translr1 $translr
              set transll1 $transll
         }
         if {[expr $xref(1)+$translr1]<[expr $xref(8)+$transll1]} {
            #centremost leaves will overlap
            set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)]
            set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)]
         }
         set rotr [expr atan((2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
         set rotl [expr atan(-(2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
         for {set k 1} {$k<=8} {incr k} {
            set xtempr($k) [expr cos($rotr)*$xref($k)+sin($rotr)*\
                                 ($zref($k)-$zref(1))+$translr]
            set ztempr($k) [expr $zref(1)-sin($rotr)*$xref($k)+\
                                 cos($rotr)*($zref($k)-$zref(1))]
            set xtempr($k) [expr cos($cmval($id,9))*$xtempr($k)+\
                                 sin($cmval($id,9))*($ztempr($k)-$zref(1))]
            set xtempl($k) [expr cos($rotl)*$xref($k)+sin($rotl)*\
                                 ($zref($k)-$zref(1))+$transll]
            set ztempl($k) [expr $zref(1)-sin($rotl)*$xref($k)+\
                                 cos($rotl)*($zref($k)-$zref(1))]
            set xtempl($k) [expr cos($cmval($id,9))*$xtempl($k)+\
                                 sin($cmval($id,9))*($ztempl($k)-$zref(1))]
         }
         #now, check if either of these leaves intersects the || axis
         set xminr [min_nrc [min_nrc $xtempr(1) $xtempr(2)] \
                            [min_nrc $xtempr(3) $xtempr(4)]]
         set xmaxr [max_nrc [max_nrc $xtempr(5) $xtempr(6)] \
                            [max_nrc $xtempr(7) $xtempr(8)]]
         if {$xminr<=0 && $xmaxr>=0} {
             set ml [expr $cmval($id,3)/2+$j]
             break
         }
         set xminl [min_nrc [min_nrc $xtempl(1) $xtempl(2)] \
                            [min_nrc $xtempl(3) $xtempl(4)]]
         set xmaxl [max_nrc [max_nrc $xtempl(5) $xtempl(6)] \
                            [max_nrc $xtempl(7) $xtempl(8)]]
         if {$xminl<=0 && $xmaxl>=0} {
             set ml [expr $cmval($id,3)/2+1-$j]
             break
         }
         #if no intersection, store min. distance from || axis
         set mindist([expr $cmval($id,3)/2+$j]) $xminr
         set mindist([expr $cmval($id,3)/2+1-$j]) $xmaxl
    }

    if {$ml==0} {
       #see if adjacent leaves straddle || axis
       for {set j 1} {$j<=[expr $cmval($id,3)-1]} {incr j} {
          if {$mindist($j)<0 && $mindist([expr $j+1])>0} {
              #leaves straddle || axis, find out which is closer
              if {[expr abs($mindist($j))]<[expr abs($mindist([expr $j+1]))]} {
                  set ml $j
              } else {
                  set ml [expr $j+1]
              }
              break
          }
       }
    }

    if $ml>0 {
      set color [lindex $colorlist $med(leaves)]

      if $cmval($id,10)==0 { ;#cylindrical leaf ends, do similar to MLCQ
        set r0leaf $cmval($id,11,0)
        set z0leaf $cmval($id,11,1)

        # add an arc for the negative side and an arc for the positive side
        # 1: negative bounding box
        set a [expr ($neg($ml)-$r0leaf-$xmin)*$xscale+$l]
        set b [expr ($neg($ml)+$r0leaf-$xmin)*$xscale+$l]
        set c [expr ($z0leaf-$r0leaf-$zmin)*$zscale+$m]
        set d [expr ($z0leaf+$r0leaf-$zmin)*$zscale+$m]
        # angles:
        # angles:
        set alp1 [expr asin(($z0leaf-$cmval($id,4,0))/$r0leaf)]
        set alp2 [expr asin(($z0leaf-$cmval($id,4,1))/$r0leaf)]
        set alp1d [expr $alp1*180/3.14159]
        set alp2d [expr $alp2*180/3.14159]
        set color [lindex $colorlist $med(leaves)]
        $parent_w create arc $a $c $b $d -start $alp1d \
              -extent [expr $alp2d-$alp1d] -fill $color -outline black

        # 2: positive bounding box
        set a [expr ($pos($ml)-$r0leaf-$xmin)*$xscale+$l]
        set b [expr ($pos($ml)+$r0leaf-$xmin)*$xscale+$l]
        $parent_w create arc $a $c $b $d -start [expr 180-$alp1d]\
            -extent [expr $alp1d-$alp2d] -fill $color -outline black

        # 3: polygons to fill in the rest
        set a [expr ($neg($ml)+$r0leaf*cos($alp2)-$xmin)*$xscale+$l]
        set b [expr ($neg($ml)+$r0leaf*cos($alp1)-$xmin)*$xscale+$l]
        set c [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set d [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        set e [expr ($cmval($id,4,0)-$zmin)*$zscale+$m]
        set f [expr ($cmval($id,4,1)-$zmin)*$zscale+$m]
        # negative
        $parent_w create poly $a $f $b $e $c $e $c $f -fill $color -outline {}
        $parent_w create line $c $e $b $e
        $parent_w create line $c $f $a $f
        # positive
        set a [expr ($pos($ml)-$r0leaf*cos($alp2)-$xmin)*$xscale+$l]
        set b [expr ($pos($ml)-$r0leaf*cos($alp1)-$xmin)*$xscale+$l]
        $parent_w create poly $a $f $b $e $d $e $d $f -fill $color -outline {}
        $parent_w create line $b $e $d $e
        $parent_w create line $a $f $d $f

      } else { # straight, focused leaf ends, similar to MLC
        #use filled polygons for the leaves

        set ratio [expr $cmval($id,4,1)-$cmval($id,11)]
        set ratio [expr $ratio/($cmval($id,4,0)-$cmval($id,11))]

        #negative side
        set y1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set z1 [expr ($cmval($id,4,0)-$zmin)*$zscale+$m]
        set y2 [expr ($neg($ml)-$xmin)*$xscale+$l]
        set z2 $z1
        if $ratio*$neg($ml)<-$cmval($id,0) {
          #lower edge of opening beyond -RMAX
         set y3 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
         set z3 [expr -$cmval($id,0)*($cmval($id,4,0)-$cmval($id,11))/$neg($ml)]
         set z3 [expr $z3+$cmval($id,11)]
         set z3 [expr ($z3-$zmin)*$zscale+$m]
         #draw a triangle
         $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 -fill $color \
                                   -outline black
        } else {
          #lower edge of opening > -RMAX
         set y3 [expr ($ratio*$neg($ml)-$xmin)*$xscale+$l]
         set z3 [expr ($cmval($id,4,1)-$zmin)*$zscale+$m]
         set y4 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
         set z4 $z3
         $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 $y4 $z4 -fill $color\
                                   -outline black
        }

        #positive side
        set y1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        set z1 [expr ($cmval($id,4,0)-$zmin)*$zscale+$m]
        set y2 [expr ($pos($ml)-$xmin)*$xscale+$l]
        set z2 $z1
        if $ratio*$pos($ml)>$cmval($id,0) {
          #lower edge of opening beyond RMAX
          set y3 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
          set z3 [expr $cmval($id,0)*($cmval($id,4,0)-$cmval($id,11))/$pos($ml)]
          set z3 [expr $z3+$cmval($id,11)]
          set z3 [expr ($z3-$zmin)*$zscale+$m]
          #draw a triangle
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 -fill $color \
                                   -outline black
        } else {
          #lower edge of opening < RMAX
          set y3 [expr ($ratio*$pos($ml)-$xmin)*$xscale+$l]
          set z3 [expr ($cmval($id,4,1)-$zmin)*$zscale+$m]
          set y4 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
          set z4 $z3
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 $y4 $z4 -fill $color\
                                   -outline black
        }
      }
    }; # end of if $ml>0
}

proc add_SYNCMLCE_sides {id yscale zscale ymin zmin zmax l m parent_w} {
    global cmval colorlist medium nmed meds_used y z rmin rmax zf ztop
    global colornum sync_file
    # assign numbers to the media, in and out
    set med(in) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,14) $medium($j)]==0 {
	    set med(in) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    set med(leaves) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,16) $medium($j)]==0 {
	    set med(leaves) [min_nrc $j $colornum]
	    set meds_used($j) 1
	    break
	}
    }
    # add a rectangle
    set y1 [expr (-$cmval($id,0)-$ymin)*$yscale+$l]
    set z1 [expr ($cmval($id,4,1)-$zmin)*$zscale+$m]
    set y2 [expr ($cmval($id,0)-$ymin)*$yscale+$l]
    set z2 [expr ($cmval($id,4,0)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(in)]
    $parent_w create rectangle $y1 $z1 $y2 $z2 -fill $color -outline {}

    # add polygons for each leaf which is intersected by Y=0 (leaves ||
    # to Y) or X=0 (leaves || to X)

    # first, store the 8 points defining the central leaf
    set xref(8) $cmval($id,7,0)
    set xref(1) [expr -$cmval($id,7,0)+$cmval($id,6)]
    set xref(4) -$cmval($id,7,1)
    set xref(3) [expr -$cmval($id,7,0)+($cmval($id,7,0)-$cmval($id,7,1))/\
                      ($cmval($id,4,1)-$cmval($id,4,0))*\
                      ($cmval($id,5,0)-$cmval($id,4,0))]
    set xref(2) [expr $xref(3)+$cmval($id,6)]
    set xref(7) [expr $cmval($id,7,0)+($cmval($id,7,1)-$cmval($id,7,0))/\
                      ($cmval($id,4,1)-$cmval($id,4,0))*\
                      ($cmval($id,5,1)-$cmval($id,4,0))]
    set xref(6) [expr $xref(7)-$cmval($id,6)]
    set xref(5) [expr $cmval($id,7,1)-$cmval($id,6)]
    set zref(1) $cmval($id,4,0)
    set zref(2) $cmval($id,5,0)
    set zref(3) $cmval($id,5,0)
    set zref(4) $cmval($id,4,1)
    set zref(5) $cmval($id,4,1)
    set zref(6) $cmval($id,5,1)
    set zref(7) $cmval($id,5,1)
    set zref(8) $cmval($id,4,0)

    set color [lindex $colorlist $med(leaves)]
    set rmin -$cmval($id,0)
    set rmax $cmval($id,0)

    #if this is a dynamic or step-and-shoot input, then
    #read in opening coordinates of first field and put them
    #in the appropriate location in the cmval array
    if {$cmval($id,2,1)==1 || $cmval($id,2,1)==2} {
      set fileid [open $sync_file($id) "r"]
      #first three lines are title, no. of fields, muindex of first field
      gets $fileid data
      gets $fileid data
      gets $fileid data
      set nleaf 0; set ngroups 0;
      while {$nleaf<$cmval($id,3)} {
            incr ngroups
            gets $fileid data
            for {set i 0} {$i<3} {incr i} {
                set data [get_val $data cmval $id,12,$i,$ngroups]
            }
            if { $cmval($id,12,2,$ngroups)<=0 } {
                set cmval($id,12,2,$ngroups) 1
            }
            incr nleaf $cmval($id,12,2,$ngroups)
       }
    }

    # set up an array of neg and pos for each leaf
    set i 1; #keeps track of opening group
    set k 0; #leaf k
    while {$k < $cmval($id,3)} {
             for {set j 1} {$j<=$cmval($id,12,2,$i)} {incr j} {
                 incr k
                 if {$cmval($id,10)==0} {
                  # add 'n subtract leaf radius
                  set neg($k) [expr $cmval($id,12,0,$i)+$cmval($id,11,0)]
                  set pos($k) [expr $cmval($id,12,1,$i)-$cmval($id,11,0)]
                 } else {
                  set neg($k) $cmval($id,12,0,$i)
                  set pos($k) $cmval($id,12,1,$i)
                 }
             }
             incr i
    }

    # see if X=0 or Y=0 intersects leaf
    # note that even if a portion of the end goes through 0, we will
    # show the entire cross section

    for {set j 1} {$j <= [expr $cmval($id,3)/2]} {incr j} {

        set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
        set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
        if {$j==1} {
              set translr1 $translr
              set transll1 $transll
        }
        if {[expr $xref(1)+$translr1]<[expr $xref(8)+$transll1]} {
            #centremost leaves will overlap
            set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)]
            set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)]
        }
        set rotr [expr atan((2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
        set rotl [expr atan(-(2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]

        #check leaf on +ve side of || axis
	if { $pos([expr $cmval($id,3)/2+$j])==$neg([expr $cmval($id,3)/2+$j])\
                  || $pos([expr $cmval($id,3)/2+$j])<=0 || \
                     $neg([expr $cmval($id,3)/2+$j])>=0 } {
           #leaf intersects
           for {set k 1} {$k<=8} {incr k} {
             set xtempr [expr cos($rotr)*$xref($k)+sin($rotr)*\
                                 ($zref($k)-$zref(1))+$translr]
             set ztempr [expr $zref(1)-sin($rotr)*$xref($k)+\
                                 cos($rotr)*($zref($k)-$zref(1))]
             set x($k) [expr cos($cmval($id,9))*$xtempr+\
                                 sin($cmval($id,9))*($ztempr-$zref(1))]
             set x($k) [expr ($x($k)-$ymin)*$yscale+$l]
             set z($k) [expr $zref(1)-sin($cmval($id,9))*$xtempr+\
                                 cos($cmval($id,9))*($ztempr-$zref(1))]
             set z($k) [expr ($z($k)-$zmin)*$zscale+$m]
           }
           #now draw the 8-point polygon
           $parent_w create polygon $x(1) $z(1) $x(2) $z(2) $x(3) $z(3)\
             $x(4) $z(4) $x(5) $z(5) $x(6) $z(6) $x(7) $z(7) $x(8) $z(8)\
                               -fill $color -outline black
        }
        #check leaf on -ve side of || axis
        if { $pos([expr $cmval($id,3)/2+1-$j])==$neg([expr $cmval($id,3)/2+1-$j])\
                  || $pos([expr $cmval($id,3)/2+1-$j])<=0 || \
                     $neg([expr $cmval($id,3)/2+1-$j])>=0 } {
           #leaf intersects
           for {set k 1} {$k<=8} {incr k} {
              set xtempl [expr cos($rotl)*$xref($k)+sin($rotl)*\
                                 ($zref($k)-$zref(1))+$transll]
              set ztempl [expr $zref(1)-sin($rotl)*$xref($k)+\
                                 cos($rotl)*($zref($k)-$zref(1))]
              set x($k) [expr cos($cmval($id,9))*$xtempl+\
                                 sin($cmval($id,9))*($ztempl-$zref(1))]
              set x($k) [expr ($x($k)-$ymin)*$yscale+$l]
              set z($k) [expr $zref(1)-sin($cmval($id,9))*$xtempl+\
                                 cos($cmval($id,9))*($ztempl-$zref(1))]
              set z($k) [expr ($z($k)-$zmin)*$zscale+$m]
           }
           #now draw the 8-point polygon
           $parent_w create polygon $x(1) $z(1) $x(2) $z(2) $x(3) $z(3)\
             $x(4) $z(4) $x(5) $z(5) $x(6) $z(6) $x(7) $z(7) $x(8) $z(8)\
                               -fill $color -outline black

        }
    }
}

proc help_syncmlcefieldtype { w } {

#help text for selecting field/treatment type

    set text {
Select a static field to run the entire simulation with a fixed set\
of leaf opening coordinates.  These are defined using the\
"Define leaf openings >>" button.  In dynamic or step-and-shoot simulations\
the leaf opening coordinates are varied throughout the simulation.  For these\
field types, you must create a separate file containing the leaf opening\
data.  The format of the file is as follows:

NFIELD (I10)
FOR I=1,NFIELD[
  INDEX(I) (F15.0)
  NEG POS NUM (2F15.0,I5) -- repeat this line until
                             coordinates of all leaves
                             have been defined for field I
]

where: NFIELD is the total no. of fields (each field comprises a\
complete set of opening coordinates), INDEX(I) is a number in the\
range [0,1] which is compared to a random number, RND, at the start of\
each history.  If INDEX(I)>=RND, then field I\
is used.  Note that INDEX(I+1)>INDEX(I) and INDEX(NFIELD)=1.\
NEG and POS define the negative and positive coordinates\
(defined at the top of the MLC if leaves have straight focused ends or\
at half-way through the thickness of the MLC if leaves have rounded ends)\
of the leaf opening for field I.  NUM instructs DYNVMLC to apply the\
accompanying values of NEG and POS to NUM adjacent leaves. NUM defaults\
to 1.  NEG POS NUM inputs start with leaf no. 1.\
See the general DYNVMLC help for leaf numbering scheme.

For dynamic field simulations, the leaf opening coordinates are interpolated\
between field I and field I-1 based on (RND-INDEX(I-1))/(INDEX(I)-INDEX(I-1)).\
This simulates motion of the leaves while the beam is on.  Thus, for a\
dynamic field simulation, your file defining leaf opening coordinates must\
have the INDEX(1)=0.

In step-and-shoot simulations, the opening coordinates for field I as input\
in the file are used.  This simulates the case where the beam is turned off\
while the leaves are moved.}
   help_dialog $w.help "Help" $text info 0 OK
}
