
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: SYNCHDMLC
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
#  Contributors:    Blake Walters
#                   Reid Townson
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


proc init_SYNCHDMLC { id } {
    global cmval ngroups nleaf sync_file nfull nhlf nqtr check_full_qtr

    set cmval($id,0) {}
    set cmval($id,1) {}
    set cmval($id,2,0) 0
    set cmval($id,2,1) 1
    set cmval($id,2,2) 0
    set cmval($id,3) {}
    set cmval($id,4) {}
    # for each leaf group, set no. and width / edited to have target and isocenter half and quarter leafs
    for {set i 0} {$i<15} {incr i} {
        set cmval($id,5,$i) {}
        set cmval($id,6,$i) {}
        set cmval($id,7,$i) {}
	set cmval($id,30,$i) {}
        set cmval($id,31,$i) {}
    }
    for {set i 1} {$i<=100} {incr i} {
      for {set j 0} {$j<3} {incr j} {
        set cmval($id,8,$j,$i) {}
      }
    }
    for {set i 9} {$i<14} {incr i} {
       set cmval($id,$i) {}
    }
    # for each leaf set: neg, pos, num
    for {set i 1} {$i<=100} {incr i} {
        set cmval($id,14,0,$i) {}
        set cmval($id,14,1,$i) {}
        set cmval($id,14,2,$i) {}
	set cmval($id,14,3,$i) {}
	set cmval($id,14,4,$i) {}
    }
    for {set i 0} {$i<4} {incr i} {
        set cmval($id,15,$i) {}
    }
    set cmval($id,16) {}
    for {set i 0} {$i<5} {incr i} {
        set cmval($id,17,$i) {}
    }
    set cmval($id,18) {}
    for {set i 0} {$i<4} {incr i} {
        set cmval($id,19,$i) {}
    }
    set cmval($id,20) {}
    set ngroups($id) 1
    set nleaf($id) {}
    # no. of groups of FULL leaves
    set nfull 0
    # no. of groups of HALF leaf pairs
    set nhlf 0
    # no. of groups of QUARTER leaf pairs
    set nqtr 0
    # set below 1 to do a dim. check between QUARTER and FULL leaves
    set check_full_qtr 0
    set sync_file($id) {}
}

proc read_SYNCHDMLC { fileid id } {
    global cmval GUI_DIR ngroups cm_ident nleaf sync_file nfull nhlf nqtr check_full_qtr

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read SYNCHDMLC $cm_ident($id).  The inputs don't begin where\
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
    for {set i 0} {$i<3} {incr i} {
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

    # read dimensions of full leaf
    gets $fileid data
    for {set i 0} {$i<15} {incr i} {
        set data [get_val $data cmval $id,5,$i]
    }

    # read dimensions of target half leaf
    gets $fileid data
    for {set i 0} {$i<15} {incr i} {
        set data [get_val $data cmval $id,6,$i]
    }

    # read dimensions of isocenter half leaf
    gets $fileid data
    for {set i 0} {$i<15} {incr i} {
        set data [get_val $data cmval $id,7,$i]
    }

    # read dimensions of target quarter leaf
    gets $fileid data
    for {set i 0} {$i<15} {incr i} {
        set data [get_val $data cmval $id,30,$i]
    }

    # read dimensions of isocenter quarter leaf
    gets $fileid data
    for {set i 0} {$i<15} {incr i} {
        set data [get_val $data cmval $id,31,$i]
    }


    # read numleaf, leaf type for each group--8
    set nleaf($id) 0
    set nfull 0
    set nhlf 0
    set nqtr 0
    for {set i 1} {$i<=$cmval($id,2,1)} {incr i} {
      gets $fileid data
      for {set j 0} {$j<2} {incr j} {
	set data [get_val $data cmval $id,8,$j,$i]
      }
      incr nleaf($id) $cmval($id,8,0,$i)
      if {$cmval($id,8,1,$i)==1} {
         incr nfull
      } elseif {$cmval($id,8,1,$i)==2} {
         incr nhlf
      } elseif {$cmval($id,8,1,$i)==3} {
         incr nqtr
      }
      if {$i > 1} {
          if { ($cmval($id,8,1,$i)==1 && $cmval($id,8,1,[expr $i-1])==3) ||
              ($cmval($id,8,1,$i)==3 && $cmval($id,8,1,[expr $i-1])==1) } {
             set check_full_qtr 1
          }
      }
    }

    # read start 9
    gets $fileid data
    set data [get_val $data cmval $id,9]
    # read leafgap 10
    gets $fileid data
    set data [get_val $data cmval $id,10]

    # read endtype 11
    gets $fileid data
    set data [get_val $data cmval $id,11]

    # read focus or radius of leaf ends 12
    gets $fileid data
    set data [get_val $data cmval $id,12]

    # read focus of leaf sides 13
    gets $fileid data
    set data [get_val $data cmval $id,13]

    if {$cmval($id,2,2)==1 | $cmval($id,2,2)==2} {
      gets $fileid sync_file($id)
    } else {
    # read in the leaf coordinates, neg,pos,num.  When the sum over num
    # is > numleaf, stop reading them. 14
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
    }

    # read ecut etc in air and air gaps 15 16
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
	set data [get_val $data cmval $id,15,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,16]

    # read ecut etc in collimator leaves 17 18
    gets $fileid data
    for {set i 0} {$i<5} {incr i} {
	set data [get_val $data cmval $id,17,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,18]

    # read ecut etc in driving screw holes 19 20
    gets $fileid data
    for {set i 0} {$i<4} {incr i} {
        set data [get_val $data cmval $id,19,$i]
    }
    gets $fileid data
    set data [get_str $data cmval $id,20]
}

proc edit_SYNCHDMLC { id zmax } {
    global cmval GUI_DIR helvfont cm_ident ngroups help_synchdmlc_text
    global nleaf default1 nmed medium values sync_file inp_file_dir med_per_column

    catch { destroy .synchdmlc$id }
    toplevel .synchdmlc$id
    wm title .synchdmlc$id "Edit SYNCHDMLC, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY SYNCHDMLC.
    frame .synchdmlc$id.top -bd 4
    set top .synchdmlc$id.top


    #label $top.mainlabel -text "Multi-leaf collimator $cm_ident($id)" \
#	    -font $helvfont
   # pack $top.mainlabel -side top -padx 10

    # SYNCHDMLC_macros.mortran:REPLACE {$MAXLEAF} WITH {64}
    # read the macro file for CM SYNCHDMLC to get defaults and put a label on the
    # main edit window.  Has to be different from the others because there's
    # no SYNCHDMLC in the default name.

    global omega helvfont
    set filename $omega/beamnrc/CMs/SYNCHDMLC_macros.mortran
    set default1 {}
    if { [file readable $filename] } {
	set fileid [open $filename "r"]
	set i 0
	while {$i<200 & [string compare $default1 {}]==0} {
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

   # label $top.default1 -text "The default maximum number\
#	    of leaves is $default1." -font $helvfont
 #   pack $top.default1 -side top

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp3 $id,3

    # leaf direction radiobuttons + selection to ignore air gaps
    # for range rejection in leaves
    frame $top.inp2 -bd 2
    frame $top.inp2.left
    frame $top.inp2.left.ll
    checkbutton $top.inp2.left.ll.help -bitmap @$GUI_DIR/help_icon.xbm \
          -command "help_leaf_mfg_specs_synchdmlc [winfo parent $top.inp2.left]"
    pack $top.inp2.left.ll.help -padx 5
    frame $top.inp2.left.l
    label $top.inp2.left.l.l1 -text {Leaves || to Y}
    label $top.inp2.left.l.l2 -text {Leaves || to X}
    pack $top.inp2.left.l.l1 $top.inp2.left.l.l2 -side top
    frame $top.inp2.left.m
    radiobutton $top.inp2.left.m.r1 -text "default orient."\
            -variable cmval($id,2,0) -value 0
    radiobutton $top.inp2.left.m.r2 -text "default orient."\
            -variable cmval($id,2,0) -value 1
    pack $top.inp2.left.m.r1 $top.inp2.left.m.r2 -side top
    frame $top.inp2.left.r
    radiobutton $top.inp2.left.r.r1 -text "mfg orient."\
            -variable cmval($id,2,0) -value 2
    radiobutton $top.inp2.left.r.r2 -text "mfg orient."\
            -variable cmval($id,2,0) -value 3
    pack $top.inp2.left.r.r1 $top.inp2.left.r.r2 -side top
    pack $top.inp2.left.ll $top.inp2.left.l $top.inp2.left.m $top.inp2.left.r -side left

    frame $top.inp2.right
    checkbutton $top.inp2.right.c1 -text "Ignore air gaps for range rejection" -variable cmval($id,17,4)
    button $top.inp2.right.help -bitmap @$GUI_DIR/help_icon.xbm \
            -command "help_ignoregaps_synchdmlc [winfo parent $top.inp2.right]"
    pack $top.inp2.right.help $top.inp2.right.c1 -side right
    pack $top.inp2.left $top.inp2.right -side left -expand true -fill x
    pack $top.inp2 -side top -expand true -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label -text {Thickness of the leaves (cm)}
    entry $top.inp4.inp -textvariable cmval($id,4) -width 10
    pack $top.inp4.label -anchor w -side left
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    frame $top.nleaf -bd 4
    label $top.nleaf.label -text {Number of leaves}
    entry $top.nleaf.inp -textvariable nleaf($id) -width 10
    pack $top.nleaf.label -anchor w -side left
    pack $top.nleaf.inp -anchor e -side right -fill x -expand true
    pack $top.nleaf -side top -fill x

    frame $top.leafdef -bd 4
        button $top.leafdef.type -text "Define leaf types >>" -command \
            "define_synchdmlc_types $id"
    button $top.leafdef.xsections -text "Define leaf cross-sections >>" -command \
            "define_synchdmlc_leaves $id"
    pack $top.leafdef.type $top.leafdef.xsections -side left -fill x -expand true
    pack $top.leafdef -side top -fill x -expand true

    frame $top.inp7 -bd 4
    label $top.inp7.label -text {Starting position of leaf sides}
    entry $top.inp7.inp -textvariable cmval($id,9) -width 10
    pack $top.inp7.label -anchor w -side left
    pack $top.inp7.inp -anchor e -side right -fill x -expand true
    pack $top.inp7 -side top -fill x

    frame $top.inp16 -bd 4
    label $top.inp16.label -text {Width of air gap (cm)}
    entry $top.inp16.inp -textvariable cmval($id,10) -width 10
    pack $top.inp16.label -anchor w -side left
    pack $top.inp16.inp -anchor e -side right -fill x -expand true
    pack $top.inp16 -side top -fill x

    frame $top.endparam

    frame $top.endparam.inp17 -bd 4
    label $top.endparam.inp17.label -text {Leaf end type}
    end_param_menu_synchdmlc $top.endparam.inp17.inp $id
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

    frame $top.openings -bd 4
    label $top.openings.label -text {Select field type:}

    frame $top.openings.inps
    button $top.openings.inps.help -bitmap @$GUI_DIR/help_icon.xbm \
            -command "help_fieldtype [winfo parent $top.openings.inps]"
    frame $top.openings.inps.static
    radiobutton $top.openings.inps.static.r1 -text "Static"\
       -variable cmval($id,2,2) -value 0 \
    -command "$top.openings.inps.static.but configure -state normal;\
            $top.openings.inps.dyn.file.inp.txt configure -state disabled;\
            $top.openings.inps.dyn.file.inp.browse configure -state disabled;\
            $top.openings.inps.dyn.file.lab configure -fg grey"
    button $top.openings.inps.static.but -text "Define leaf openings >>"\
             -command "define_synchdmlc_openings $id"
    if { $cmval($id,2,2)==1 | $cmval($id,2,2)==2 } {
       $top.openings.inps.static.but configure -state disabled
    }
    pack $top.openings.inps.static.r1 $top.openings.inps.static.but -side left
    frame $top.openings.inps.dyn
    frame $top.openings.inps.dyn.sel
    radiobutton $top.openings.inps.dyn.sel.r2 -text "Dynamic"\
       -variable cmval($id,2,2) -value 1 \
       -command "$top.openings.inps.dyn.file.inp.txt configure -state normal;\
           $top.openings.inps.dyn.file.inp.browse configure -state normal;\
           $top.openings.inps.dyn.file.lab configure -fg black;\
           $top.openings.inps.static.but configure -state disabled"
    radiobutton $top.openings.inps.dyn.sel.r3 -text "Step-and-shoot"\
       -variable cmval($id,2,2) -value 2 \
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
    if { $cmval($id,2,2)==0 } {
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

    pack $top -side top -pady 0

#now get ecut,pcut,materials,etc

    frame .synchdmlc$id.bottom -bd 0

    frame .synchdmlc$id.bottom.ecut
    button .synchdmlc$id.bottom.ecut.b -text "?" -command \
           "help_ecut .synchdmlc$id.bottom.ecut"
    label .synchdmlc$id.bottom.ecut.lab -text \
          "Electron cutoff energy (default ECUTIN) (MeV)"
    pack .synchdmlc$id.bottom.ecut.b -anchor w -side left
    pack .synchdmlc$id.bottom.ecut.lab -side left -padx 5
    grid configure .synchdmlc$id.bottom.ecut -column 0 -row 1 -sticky e

    frame .synchdmlc$id.bottom.pcut
    button .synchdmlc$id.bottom.pcut.b -text "?" \
          -command "help_pcut .synchdmlc$id.bottom.pcut"
    label .synchdmlc$id.bottom.pcut.lab -text \
          "Photon cutoff energy (default PCUTIN) (MeV)"
    pack .synchdmlc$id.bottom.pcut.b -anchor w -side left
    pack .synchdmlc$id.bottom.pcut.lab -side left -padx 5
    grid configure .synchdmlc$id.bottom.pcut -column 0 -row 2 -sticky e

    frame .synchdmlc$id.bottom.dose
    button .synchdmlc$id.bottom.dose.b -text "?" \
         -command "help_dose .synchdmlc$id.bottom.dose"
    label .synchdmlc$id.bottom.dose.lab -text "Dose zone (0 for no scoring)"
    pack .synchdmlc$id.bottom.dose.b -anchor w -side left
    pack .synchdmlc$id.bottom.dose.lab -side left -padx 5
    grid configure .synchdmlc$id.bottom.dose -column 0 -row 3 -sticky e

    frame .synchdmlc$id.bottom.latch
    button .synchdmlc$id.bottom.latch.b -text "?" \
        -command "help_latch .synchdmlc$id.bottom.latch"
    label .synchdmlc$id.bottom.latch.lab -text "Associate with LATCH bit"
    pack .synchdmlc$id.bottom.latch.b -anchor w -side left
    pack .synchdmlc$id.bottom.latch.lab -padx 5 -side left
    grid configure .synchdmlc$id.bottom.latch -column 0 -row 4 -sticky e

    frame .synchdmlc$id.bottom.med
    button .synchdmlc$id.bottom.med.b -text "?" \
        -command "help_material .synchdmlc$id.bottom.med"
    label .synchdmlc$id.bottom.med.lab -text "Material"
    pack .synchdmlc$id.bottom.med.b -anchor w -side left
    pack .synchdmlc$id.bottom.med.lab -padx 5 -side left
    grid configure .synchdmlc$id.bottom.med -column 0 -row 5 -sticky e

    label .synchdmlc$id.bottom.t0 -text "Opening(s) + air gaps" \
                       -font $helvfont
    label .synchdmlc$id.bottom.t1 -text "Collimator leaves" \
                       -font $helvfont
    label .synchdmlc$id.bottom.t2 -text "Driving screw holes" \
                       -font $helvfont
    grid configure .synchdmlc$id.bottom.t0 -column 1 -row 0
    grid configure .synchdmlc$id.bottom.t1 -column 2 -row 0
    grid configure .synchdmlc$id.bottom.t2 -column 3 -row 0

    #now, do ecut for all regions
    set k 0
    foreach i {15 17 19} {
      incr k
      if [string compare $cmval($id,$i,0) ""]==0 {
        set cmval($id,$i,0) $values(ecut)
      }
      entry .synchdmlc$id.bottom.ecut$k -textvariable cmval($id,$i,0) -width 20
      grid configure .synchdmlc$id.bottom.ecut$k -column $k -row 1
    }

    #pcut for all regions
    set k 0
    foreach i {15 17 19} {
      incr k
      if [string compare $cmval($id,$i,1) ""]==0 {
        set cmval($id,$i,1) $values(pcut)
      }
      entry .synchdmlc$id.bottom.pcut$k -textvariable cmval($id,$i,1) -width 20
      grid configure .synchdmlc$id.bottom.pcut$k -column $k -row 2
    }

    #dose zone for all regions
    set k 0
    foreach i {15 17 19} {
      incr k
      entry .synchdmlc$id.bottom.dose$k -textvariable cmval($id,$i,2) -width 20
      grid configure .synchdmlc$id.bottom.dose$k -column $k -row 3
    }

    #latch bit for all regions
    set k 0
    foreach i {15 17 19} {
      incr k
      entry .synchdmlc$id.bottom.latch$k -textvariable cmval($id,$i,3) -width 20
      grid configure .synchdmlc$id.bottom.latch$k -column $k -row 4
    }

    #medium for all regions
    set k 0
    foreach i {16 18 20} {
      incr k
      if [catch {string compare $cmval($id,$i) {} }]==1 {
        set cmval($id,$i) {}
      }
      menubutton .synchdmlc$id.bottom.med$k -text $cmval($id,$i) -menu \
            .synchdmlc$id.bottom.med$k.m -bd 1 -relief raised -indicatoron 1\
            -width 15
      menu .synchdmlc$id.bottom.med$k.m -tearoff no
      for {set iopt 0} {$iopt <= $nmed} {incr iopt} {
        .synchdmlc$id.bottom.med$k.m add command -label $medium($iopt) \
            -command "set_material .synchdmlc$id.bottom.med$k $iopt $id,$i" -columnbreak [expr $iopt % $med_per_column==0]
      }
      grid configure .synchdmlc$id.bottom.med$k -column $k -row 5
    }

    pack .synchdmlc$id.bottom -side top -pady 5

    frame .synchdmlc$id.buts -bd 1
    button .synchdmlc$id.buts.okb -text "OK" -command "destroy .synchdmlc$id"\
	    -relief groove -bd 8
    button .synchdmlc$id.buts.helpb -text "Help" -command\
	    "help_gif .synchdmlc$id.help {$help_synchdmlc_text} help_synchdmlc"\
	    -relief groove -bd 8
#don't have a preview yet, get it working after course, will call show_SYNCHDMLC
    button .synchdmlc$id.buts.prev -text "Preview" -command\
                          "show_SYNCHDMLC $id" -relief groove -bd 8
#	    { tk_dialog .nopreview "No Preview" \
#             "No preview available yet." info 0 OK } -relief groove -bd 8
    pack .synchdmlc$id.buts.helpb .synchdmlc$id.buts.okb .synchdmlc$id.buts.prev -side left -padx 10
    pack .synchdmlc$id.buts -pady 10
}

proc end_param_menu_synchdmlc { w id } {
    global cmval

    set inw .synchdmlc$id.top.endparam.inp18.label

    if { $cmval($id,11)==1 } {
      menubutton $w -text "straight" -menu  $w.m -bd 1 \
                  -relief raised -indicatoron 1 -width 20
    } else {
      menubutton $w -text "rounded" -menu  $w.m -bd 1 \
                  -relief raised -indicatoron 1 -width 20
    }
    menu $w.m -tearoff no
    $w.m add command -label "rounded" \
               -command "set_endval_synchdmlc $id 0 $w;\
                         $inw configure -text {Radius of leaf ends}"
    $w.m add command -label "straight" \
               -command "set_endval_synchdmlc $id 1 $w;\
                         $inw configure -text {Zfocus of leaf ends}"
}

proc set_endval_synchdmlc { id val w } {
    global cmval

    set cmval($id,11) $val

    if { $val==1 } {
        $w configure -text "straight"
    } else {
        $w configure -text "rounded"
    }
}
proc define_synchdmlc_types { id } {
    global cmval helvfont ngroups tow fromw help_synchdmlc_text
    global nleaf default1

# procedure for defining leaf types and, in the process, the total
# number of leaves

    if {$nleaf($id)==""} {
       tk_dialog .setnleaf "Set leaf types" "Before you can specify\
       the leaf types, you must specify the number of leaves." warning 0 OK
       return
    }

    catch { destroy .synchdmlc$id.childt }
    toplevel .synchdmlc$id.childt
    set top .synchdmlc$id.childt
    wm title $top "Define leaf types"

    frame $top.f -bd 10
    set w $top.f

    message $w.message -text "Specify leaf types: FULL or HALF TARGET/ISOCENTER or \
            QUARTER TARGET/ISOCENTER leaves.\
            Add a row to start a\
            new group.  When a new row is added the next leaf in the order will\
            automatically appear in the 'from leaf' box."\
            -width 400 -font $helvfont
    pack $w.message -side top -anchor n -fill x -expand true

    frame $w.grid -bd 4
    label $w.grid.l0 -text "from leaf"
    label $w.grid.l1 -text "to leaf"
    label $w.grid.l2 -text "FULL"
    label $w.grid.l3 -text "HALF TARGET/ISOCENTER"
    label $w.grid.l4 -text "QUARTER TARGET/ISOCENTER"

    grid configure $w.grid.l0 -row 0 -column 0
    grid configure $w.grid.l1 -row 0 -column 1 -padx 5
    grid configure $w.grid.l2 -row 0 -column 2 -padx 5
    grid configure $w.grid.l3 -row 0 -column 3 -padx 5
    grid configure $w.grid.l4 -row 0 -column 4 -padx 5

    set tow(0) 0
    for {set j 1} {$j<=$cmval($id,2,1)} {incr j} {
      set fromw($j) [expr $tow([expr $j-1])+1]
      if [catch {set tow($j) [expr $tow([expr $j-1])+$cmval($id,8,0,$j)]}]==1 {
            set tow($j) {}
      }
      entry $w.grid.e0$j -textvariable fromw($j) -width 8
      grid configure $w.grid.e0$j -row $j -column 0
      entry $w.grid.e1$j -textvariable tow($j) -width 8
      grid configure $w.grid.e1$j -row $j -column 1 -padx 5
      radiobutton $w.grid.e2$j -variable cmval($id,8,1,$j) -value 1
      grid configure $w.grid.e2$j -row $j -column 2
      radiobutton $w.grid.e3$j -variable cmval($id,8,1,$j) -value 2
      grid configure $w.grid.e3$j -row $j -column 3
      radiobutton $w.grid.e4$j -variable cmval($id,8,1,$j) -value 3
      grid configure $w.grid.e4$j -row $j -column 4
    }
    pack $w.grid -side top

    pack $w
    frame $top.b
    button $top.b.addb -text "Add a row" -command "add_synchdmlc_row_type $id"\
            -relief groove -bd 8
    button $top.b.delb -text "Delete last row"\
            -command "del_synchdmlc_row_type $id" -relief groove -bd 8
    button $top.b.okb -text "OK"\
            -command "save_synchdmlc_type $id" -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
          "help_gif .synchdmlc$id.childt.help {$help_synchdmlc_text} help_synchdmlc"\
            -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb $top.b.helpb -side left -padx 10
    pack $top.b -pady 10
}

proc define_synchdmlc_leaves { id } {
# defines the cross-section dimensions for FULL, ISOCENTER and TARGET HALF, ISOCENTER and TARGET QUARTER
# leaf types
    global cmval helvfont help_synchdmlc_full_text help_synchdmlc_tar_text
    global help_synchdmlc_iso_text
    global help_synchdmlc_qtar_text help_synchdmlc_qiso_text

    catch { destroy .synchdmlc$id.childx }
    toplevel .synchdmlc$id.childx
    set top .synchdmlc$id.childx
    wm title $top "Define leaf cross-sections"

    frame $top.f -bd 10
    set w $top.f

    if {$cmval($id,2,0)==1 || $cmval($id,2,0)==3} {
        set porient X
        set xorient Y
    } else {
        set porient Y
        set xorient X
    }

    message $w.message -text "           Define the $xorient cross-sections of\
            the leaf types you need to model your MLC:
            For the Millenium MLC introduce values for: \
               FULL, HALF TARGET, HALF ISOCENTER leaves.
            For the HD MLC introduce values for: \
              HALF TARGET , HALF ISOCENTER, QUARTER TARGET and QUARTER ISOCENTER leaves.
            Option to leave dimensions for unused leaf types blank.
            For help, click on the \"?\"\
            button for the appropriate leaf type." -width 800 -font $helvfont
    pack $w.message -side top -anchor n -fill x -expand true

    frame $w.grid -bd 4

    frame $w.grid.full
    button $w.grid.full.b -text "?" -command\
  "help_gif .synchdmlc$id.fullhelp {$help_synchdmlc_full_text} help_dynvmlc_full"
    label $w.grid.full.t -text "FULL leaves"
    pack $w.grid.full.b $w.grid.full.t -side left

    frame $w.grid.target
    button $w.grid.target.b -text "?" -command\
  "help_gif .synchdmlc$id.tarhelp {$help_synchdmlc_tar_text} help_dynvmlc_tar"
    label $w.grid.target.t -text "HALF TARGET leaves"
    pack $w.grid.target.b $w.grid.target.t -side left

    frame $w.grid.isocenter
    button $w.grid.isocenter.b -text "?" -command\
  "help_gif .synchdmlc$id.isohelp {$help_synchdmlc_iso_text} help_dynvmlc_iso"
    label $w.grid.isocenter.t -text "HALF ISOCENTER leaves"
    pack $w.grid.isocenter.b $w.grid.isocenter.t -side left

    frame $w.grid.targetq
    button $w.grid.targetq.b -text "?" -command\
  "help_gif .synchdmlc$id.tarqhelp {$help_synchdmlc_qtar_text} help_dynvmlc_tar"
    label $w.grid.targetq.t -text "QUARTER TARGET leaves"
    pack $w.grid.targetq.b $w.grid.targetq.t -side left

    frame $w.grid.isocenterq
    button $w.grid.isocenterq.b -text "?" -command\
  "help_gif .synchdmlc$id.isoqhelp {$help_synchdmlc_qiso_text} help_dynvmlc_iso"
    label $w.grid.isocenterq.t -text "QUARTER ISOCENTER leaves"
    pack $w.grid.isocenterq.b $w.grid.isocenterq.t -side left


    grid configure $w.grid.full -row 0 -column 0 -columnspan 2 -sticky e
    grid configure $w.grid.target -row 0 -column 2 -columnspan 2 -sticky e
    grid configure $w.grid.isocenter -row 0 -column 4 -columnspan 2 -sticky e
    grid configure $w.grid.targetq -row 0 -column 6 -columnspan 2 -sticky e
    grid configure $w.grid.isocenterq -row 0 -column 8 -columnspan 2 -sticky e

    #labels and entries for FULL leaf

    label $w.grid.wl -text "$xorient widths (cm):"

    grid configure $w.grid.wl -row 1 -column 0 -columnspan 6 -sticky w \
       -ipady 10

    label $w.grid.fl0 -text "leaf (excl. tongue)"
    entry $w.grid.fe0 -textvariable cmval($id,5,0) -width 8
    label $w.grid.fl1 -text "tongue"
    entry $w.grid.fe1 -textvariable cmval($id,5,1) -width 8
    label $w.grid.fl2 -text "groove"
    entry $w.grid.fe2 -textvariable cmval($id,5,2) -width 8
    label $w.grid.fl3 -text "tip"
    entry $w.grid.fe3 -textvariable cmval($id,5,3) -width 8
    label $w.grid.fl4 -text "upper support rail"
    entry $w.grid.fe4 -textvariable cmval($id,5,4) -width 8
    label $w.grid.fl5 -text "lower support rail"
    entry $w.grid.fe5 -textvariable cmval($id,5,5) -width 8

    for {set i 0} {$i<6} {incr i} {
       grid configure $w.grid.fl$i -row [expr $i+2] -column 0 -sticky e
       grid configure $w.grid.fe$i -row [expr $i+2] -column 1
    }

    label $w.grid.zl -text "Z positions (cm):"

    grid configure $w.grid.zl -row 8 -column 0 -columnspan 6 -sticky w \
         -ipady 10

    label $w.grid.fl6 -text "top of tip"
    entry $w.grid.fe6 -textvariable cmval($id,5,6) -width 8
    label $w.grid.fl7 -text "top of leaf"
    entry $w.grid.fe7 -textvariable cmval($id,5,7) -width 8
    label $w.grid.fl8 -text "bottom of tongue"
    entry $w.grid.fe8 -textvariable cmval($id,5,8) -width 8
    label $w.grid.fl9 -text "bottom of groove"
    entry $w.grid.fe9 -textvariable cmval($id,5,9) -width 8
    label $w.grid.fl10 -text "top of driving screw hole"
    entry $w.grid.fe10 -textvariable cmval($id,5,10) -width 8
    label $w.grid.fl11 -text "bottom of driving screw hole"
    entry $w.grid.fe11 -textvariable cmval($id,5,11) -width 8
    label $w.grid.fl13 -text "top of support rail"
    entry $w.grid.fe13 -textvariable cmval($id,5,13) -width 8
    label $w.grid.fl14 -text "bottom of support rail"
    entry $w.grid.fe14 -textvariable cmval($id,5,14) -width 8

    for {set i 6} {$i<12} {incr i} {
        grid configure $w.grid.fl$i -row [expr $i+3] -column 0 -sticky e
        grid configure $w.grid.fe$i -row [expr $i+3] -column 1
    }
    for {set i 13} {$i<15} {incr i} {
        grid configure $w.grid.fl$i -row [expr $i+2] -column 0 -sticky e
        grid configure $w.grid.fe$i -row [expr $i+2] -column 1
    }

    frame $w.grid.holedist
    button $w.grid.holedist.b -text "?" -command \
    "help_screwdist [winfo parent $w.grid.holedist]"
    label $w.grid.holedist.l -text "$porient distance of driving screw hole from leaf tip (cm):"
    pack $w.grid.holedist.b $w.grid.holedist.l -side left
    grid configure $w.grid.holedist -row 17 -column 0 -columnspan 6 -sticky w\
         -ipady 10

    entry $w.grid.fe12 -textvariable cmval($id,5,12) -width 8
    grid configure $w.grid.fe12 -row 18 -column 1

    #labels and entries for TARGET HALF leaf

    label $w.grid.tl0 -text "leaf (excl. tongue)"
    entry $w.grid.te0 -textvariable cmval($id,6,0) -width 8
    label $w.grid.tl1 -text "tongue"
    entry $w.grid.te1 -textvariable cmval($id,6,1) -width 8
    label $w.grid.tl2 -text "groove"
    entry $w.grid.te2 -textvariable cmval($id,6,2) -width 8
    label $w.grid.tl3 -text "tip"
    entry $w.grid.te3 -textvariable cmval($id,6,3) -width 8
    label $w.grid.tl4 -text "upper support rail"
    entry $w.grid.te4 -textvariable cmval($id,6,4) -width 8
    label $w.grid.tl5 -text "lower support rail"
    entry $w.grid.te5 -textvariable cmval($id,6,5) -width 8

    for {set i 0} {$i<6} {incr i} {
       grid configure $w.grid.tl$i -row [expr $i+2] -column 2 -sticky e
       grid configure $w.grid.te$i -row [expr $i+2] -column 3
    }

    label $w.grid.tl6 -text "top of support rail"
    entry $w.grid.te6 -textvariable cmval($id,6,6) -width 8
    label $w.grid.tl7 -text "bottom of support rail"
    entry $w.grid.te7 -textvariable cmval($id,6,7) -width 8
    label $w.grid.tl8 -text "top of driving screw hole"
    entry $w.grid.te8 -textvariable cmval($id,6,8) -width 8
    label $w.grid.tl9 -text "  bottom of driving screw hole"
    entry $w.grid.te9 -textvariable cmval($id,6,9) -width 8
    label $w.grid.tl11 -text "bottom of tongue"
    entry $w.grid.te11 -textvariable cmval($id,6,11) -width 8
    label $w.grid.tl12 -text "top of groove"
    entry $w.grid.te12 -textvariable cmval($id,6,12) -width 8
    label $w.grid.tl13 -text "bottom of leaf"
    entry $w.grid.te13 -textvariable cmval($id,6,13) -width 8
    label $w.grid.tl14 -text "bottom of tip"
    entry $w.grid.te14 -textvariable cmval($id,6,14) -width 8

    for {set i 6} {$i<10} {incr i} {
        grid configure $w.grid.tl$i -row [expr $i+3] -column 2 -sticky e
        grid configure $w.grid.te$i -row [expr $i+3] -column 3
    }
    for {set i 11} {$i<15} {incr i} {
        grid configure $w.grid.tl$i -row [expr $i+2] -column 2 -sticky e
        grid configure $w.grid.te$i -row [expr $i+2] -column 3
    }

    entry $w.grid.te10 -textvariable cmval($id,6,10) -width 8
    grid configure $w.grid.te10 -row 18 -column 3

    #labels and entries for ISOCENTER HALF leaf

    label $w.grid.il0 -text "leaf (excl. tongue)"
    entry $w.grid.ie0 -textvariable cmval($id,7,0) -width 8
    label $w.grid.il1 -text "tongue"
    entry $w.grid.ie1 -textvariable cmval($id,7,1) -width 8
    label $w.grid.il2 -text "groove"
    entry $w.grid.ie2 -textvariable cmval($id,7,2) -width 8
    label $w.grid.il3 -text "tip"
    entry $w.grid.ie3 -textvariable cmval($id,7,3) -width 8
    label $w.grid.il4 -text "upper support rail"
    entry $w.grid.ie4 -textvariable cmval($id,7,4) -width 8
    label $w.grid.il5 -text "lower support rail"
    entry $w.grid.ie5 -textvariable cmval($id,7,5) -width 8

    for {set i 0} {$i<6} {incr i} {
       grid configure $w.grid.il$i -row [expr $i+2] -column 4 -sticky e
       grid configure $w.grid.ie$i -row [expr $i+2] -column 5
    }

    label $w.grid.il6 -text "top of tip"
    entry $w.grid.ie6 -textvariable cmval($id,7,6) -width 8
    label $w.grid.il7 -text "top of leaf"
    entry $w.grid.ie7 -textvariable cmval($id,7,7) -width 8
    label $w.grid.il8 -text "top of tongue"
    entry $w.grid.ie8 -textvariable cmval($id,7,8) -width 8
    label $w.grid.il9 -text "bottom of groove"
    entry $w.grid.ie9 -textvariable cmval($id,7,9) -width 8
    label $w.grid.il10 -text "top of driving screw hole"
    entry $w.grid.ie10 -textvariable cmval($id,7,10) -width 8
    label $w.grid.il11 -text "bottom of driving screw hole"
    entry $w.grid.ie11 -textvariable cmval($id,7,11) -width 8
    label $w.grid.il13 -text "top of support rail"
    entry $w.grid.ie13 -textvariable cmval($id,7,13) -width 8
    label $w.grid.il14 -text "bottom of support rail"
    entry $w.grid.ie14 -textvariable cmval($id,7,14) -width 8

    for {set i 6} {$i<12} {incr i} {
        grid configure $w.grid.il$i -row [expr $i+3] -column 4 -sticky e
        grid configure $w.grid.ie$i -row [expr $i+3] -column 5
    }
    for {set i 13} {$i<15} {incr i} {
        grid configure $w.grid.il$i -row [expr $i+2] -column 4 -sticky e
        grid configure $w.grid.ie$i -row [expr $i+2] -column 5
    }

    entry $w.grid.ie12 -textvariable cmval($id,7,12) -width 8
    grid configure $w.grid.ie12 -row 18 -column 5

    #labels and entries for TARGET QUARTER leaf

    label $w.grid.tql0 -text "leaf (excl. tongue)"
    entry $w.grid.tqe0 -textvariable cmval($id,30,0) -width 8
    label $w.grid.tql1 -text "tongue"
    entry $w.grid.tqe1 -textvariable cmval($id,30,1) -width 8
    label $w.grid.tql2 -text "groove"
    entry $w.grid.tqe2 -textvariable cmval($id,30,2) -width 8
    label $w.grid.tql3 -text "tip"
    entry $w.grid.tqe3 -textvariable cmval($id,30,3) -width 8
    label $w.grid.tql4 -text "upper support rail"
    entry $w.grid.tqe4 -textvariable cmval($id,30,4) -width 8
    label $w.grid.tql5 -text "lower support rail"
    entry $w.grid.tqe5 -textvariable cmval($id,30,5) -width 8

    for {set i 0} {$i<6} {incr i} {
       grid configure $w.grid.tql$i -row [expr $i+2] -column 6 -sticky e
       grid configure $w.grid.tqe$i -row [expr $i+2] -column 7
    }

    label $w.grid.tql6 -text "top of support rail"
    entry $w.grid.tqe6 -textvariable cmval($id,30,6) -width 8
    label $w.grid.tql7 -text "bottom of support rail"
    entry $w.grid.tqe7 -textvariable cmval($id,30,7) -width 8
    label $w.grid.tql8 -text "top of driving screw hole"
    entry $w.grid.tqe8 -textvariable cmval($id,30,8) -width 8
    label $w.grid.tql9 -text "  bottom of driving screw hole"
    entry $w.grid.tqe9 -textvariable cmval($id,30,9) -width 8
    label $w.grid.tql11 -text "bottom of tongue"
    entry $w.grid.tqe11 -textvariable cmval($id,30,11) -width 8
    label $w.grid.tql12 -text "top of groove"
    entry $w.grid.tqe12 -textvariable cmval($id,30,12) -width 8
    label $w.grid.tql13 -text "bottom of leaf"
    entry $w.grid.tqe13 -textvariable cmval($id,30,13) -width 8
    label $w.grid.tql14 -text "bottom of tip"
    entry $w.grid.tqe14 -textvariable cmval($id,30,14) -width 8

    for {set i 6} {$i<10} {incr i} {
        grid configure $w.grid.tql$i -row [expr $i+3] -column 6 -sticky e
        grid configure $w.grid.tqe$i -row [expr $i+3] -column 7
    }
    for {set i 11} {$i<15} {incr i} {
        grid configure $w.grid.tql$i -row [expr $i+2] -column 6 -sticky e
        grid configure $w.grid.tqe$i -row [expr $i+2] -column 7
    }

    entry $w.grid.tqe10 -textvariable cmval($id,30,10) -width 8
    grid configure $w.grid.tqe10 -row 18 -column 7

    #labels and entries for ISOCENTER QUARTER leaf

    label $w.grid.iql0 -text "leaf (excl. tongue)"
    entry $w.grid.iqe0 -textvariable cmval($id,31,0) -width 8
    label $w.grid.iql1 -text "tongue"
    entry $w.grid.iqe1 -textvariable cmval($id,31,1) -width 8
    label $w.grid.iql2 -text "groove"
    entry $w.grid.iqe2 -textvariable cmval($id,31,2) -width 8
    label $w.grid.iql3 -text "tip"
    entry $w.grid.iqe3 -textvariable cmval($id,31,3) -width 8
    label $w.grid.iql4 -text "upper support rail"
    entry $w.grid.iqe4 -textvariable cmval($id,31,4) -width 8
    label $w.grid.iql5 -text "lower support rail"
    entry $w.grid.iqe5 -textvariable cmval($id,31,5) -width 8

    for {set i 0} {$i<6} {incr i} {
       grid configure $w.grid.iql$i -row [expr $i+2] -column 8 -sticky e
       grid configure $w.grid.iqe$i -row [expr $i+2] -column 9
    }

    label $w.grid.iql6 -text "top of tip"
    entry $w.grid.iqe6 -textvariable cmval($id,31,6) -width 8
    label $w.grid.iql7 -text "top of leaf"
    entry $w.grid.iqe7 -textvariable cmval($id,31,7) -width 8
    label $w.grid.iql8 -text "top of tongue"
    entry $w.grid.iqe8 -textvariable cmval($id,31,8) -width 8
    label $w.grid.iql9 -text "bottom of groove"
    entry $w.grid.iqe9 -textvariable cmval($id,31,9) -width 8
    label $w.grid.iql10 -text "top of driving screw hole"
    entry $w.grid.iqe10 -textvariable cmval($id,31,10) -width 8
    label $w.grid.iql11 -text "bottom of driving screw hole"
    entry $w.grid.iqe11 -textvariable cmval($id,31,11) -width 8
    label $w.grid.iql13 -text "top of support rail"
    entry $w.grid.iqe13 -textvariable cmval($id,31,13) -width 8
    label $w.grid.iql14 -text "bottom of support rail"
    entry $w.grid.iqe14 -textvariable cmval($id,31,14) -width 8

    for {set i 6} {$i<12} {incr i} {
        grid configure $w.grid.iql$i -row [expr $i+3] -column 8 -sticky e
        grid configure $w.grid.iqe$i -row [expr $i+3] -column 9
    }
    for {set i 13} {$i<15} {incr i} {
        grid configure $w.grid.iql$i -row [expr $i+2] -column 8 -sticky e
        grid configure $w.grid.iqe$i -row [expr $i+2] -column 9
    }

    entry $w.grid.iqe12 -textvariable cmval($id,31,12) -width 8
    grid configure $w.grid.iqe12 -row 18 -column 9

    pack $w.grid -side top

    pack $w

    frame $top.cd
    button $top.cd.check -text "Check dimensions" -command \
      "check_dimensions_synchdmlc $id" -relief groove -bd 8
    button $top.cd.done -text "Done" -command\
            "destroy .synchdmlc$id.childx" -relief groove -bd 8
    pack $top.cd.check $top.cd.done -side left -padx 5
    pack $top.cd -pady 10 -side top

}

proc check_dimensions_synchdmlc { id } {
    global cmval helvfont dimtext totprob check_full_qtr

#checks dimensions of all leaf types, within the leaf and against leaves
#that it must fit together with

    #check FULL leaf dimensions

    set dimtext "For FULL leaf:
---------------"

    set wlgprob 0
    set wttprob 0
    set wrbrtprob 0
    set totprob 0

    #first check to see that all are defined
    check_def_synchdmlc $id 5

    if {"$cmval($id,5,0)"!="" && "$cmval($id,5,2)"!=""} {
        if {$cmval($id,5,2)>$cmval($id,5,0)} {
          set dimtext "$dimtext
- groove width ($cmval($id,5,2) cm)  must be <= leaf width ($cmval($id,5,0) cm)"
          set wlgprob 1
          incr totprob
        }
    }
    if {"$cmval($id,5,1)"!="" && "$cmval($id,5,3)"!=""} {
        if {$cmval($id,5,3)<$cmval($id,5,1)} {
          set dimtext "$dimtext
- tip width ($cmval($id,5,3) cm) must be >= tongue width ($cmval($id,5,1) cm)"
          set wttprob 1
          incr totprob
        }
    }
    if {"$cmval($id,5,3)"!="" && "$cmval($id,5,0)"!="" && \
        "$cmval($id,5,1)"!=""} {
        if {$cmval($id,5,3)>[expr $cmval($id,5,0)+$cmval($id,5,1)]} {
          set dimtext "$dimtext
- tip width ($cmval($id,5,3) cm) must be <= leaf + tongue width (\
[expr $cmval($id,5,0)+$cmval($id,5,1)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,5,4)"!="" && "$cmval($id,5,0)"!="" &&\
        "$cmval($id,5,2)"!=""} {
        if {$cmval($id,5,4)>[expr $cmval($id,5,0)-$cmval($id,5,2)] &&\
          $wlgprob==0} {
          set dimtext "$dimtext
- width of top support rail ($cmval($id,5,4) cm) must be <= leaf - groove width \
                     ([expr $cmval($id,5,0)-$cmval($id,5,2)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,5,4)"!="" && "$cmval($id,5,3)"!="" &&\
         "$cmval($id,5,1)"!=""} {
        if {$cmval($id,5,4)<[expr $cmval($id,5,3)-$cmval($id,5,1)] &&\
            $wttprob==0} {
          set dimtext "$dimtext
- width of top support rail ($cmval($id,5,4) cm) must be >= tip - tongue width \
                     ([expr $cmval($id,5,3)-$cmval($id,5,1)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,5,4)"!="" && "$cmval($id,5,5)"!=""} {
        if {$cmval($id,5,5)>$cmval($id,5,4)} {
          set dimtext "$dimtext
- width of bottom support rail ($cmval($id,5,5) cm) must be <= width of top\
                     support rail ($cmval($id,5,4) cm)"
          set wrbrtprob 1
          incr totprob
        }
    }
    if {"$cmval($id,5,4)"!="" && "$cmval($id,5,5)"!="" &&\
        "$cmval($id,5,3)"!="" && "$cmval($id,5,1)"!=""} {
        if {[expr $cmval($id,5,4)-$cmval($id,5,5)]<\
            [expr $cmval($id,5,3)-$cmval($id,5,1)] && $wrbrtprob==0 &&\
            $wttprob==0} {
          set dimtext "$dimtext
- top rail - bottom rail width ([expr $cmval($id,5,4)-$cmval($id,5,5)] cm) must be >= tip - tongue \
                     width ([expr $cmval($id,5,3)-$cmval($id,5,1)] cm)"
          incr totprob
        }
    }

    #now check Z positions
    check_z_posn $id 5

    if {$totprob==0} {
        set dimtext "$dimtext
- no dimension errors"
    }

    #check TARGET HALF leaf dimensions

    set dimtext "$dimtext

For HALF TARGET leaf:
---------------"

    set totprob 0

    #first check to see that all are defined
    check_def_synchdmlc $id 6

    #width checks

    if {"$cmval($id,6,0)"!="" && "$cmval($id,6,2)"!=""} {
        if {$cmval($id,6,2)>$cmval($id,6,0)} {
          set dimtext "$dimtext
- groove width ($cmval($id,6,2) cm) must be <= leaf width ($cmval($id,6,0) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,6,3)"!="" &&\
         "$cmval($id,6,0)"!="" && "$cmval($id,6,2)"!=""} {
        if {$cmval($id,6,3)>[expr $cmval($id,6,0)-$cmval($id,6,2)]} {
          set dimtext "$dimtext
- tip width ($cmval($id,6,3) cm) must be <= leaf - groove width \
([expr $cmval($id,6,0)-$cmval($id,6,2)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,6,5)"!="" && "$cmval($id,6,2)"!=""} {
        if {$cmval($id,6,5)<$cmval($id,6,2)} {
          set dimtext "$dimtext
- width of bottom support rail ($cmval($id,6,5) cm) must be \
>= groove width ($cmval($id,6,2) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,6,5)"!="" && "$cmval($id,6,2)"!="" &&\
        "$cmval($id,6,3)"!=""} {
        if {$cmval($id,6,5)>[expr $cmval($id,6,2)+$cmval($id,6,3)]} {
          set dimtext "$dimtext
- width of bottom support rail ($cmval($id,6,5) cm) must be <= groove + tip width\
([expr $cmval($id,6,2)+$cmval($id,6,3)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,6,4)"!="" && "$cmval($id,6,5)"!=""} {
        if {$cmval($id,6,4)>$cmval($id,6,5)} {
          set dimtext "$dimtext
- width of top support rail ($cmval($id,6,4) cm) must be <= width of bottom support rail \
($cmval($id,6,5) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,6,4)"!="" && "$cmval($id,6,5)"!="" &&\
        "$cmval($id,6,2)"!=""} {
        if {$cmval($id,6,4)<[expr $cmval($id,6,5)-$cmval($id,6,2)]} {
          set dimtext "$dimtext
- width of top support rail ($cmval($id,6,4) cm) must be >= bottom support rail - groove width \
([expr $cmval($id,6,5)-$cmval($id,6,2)] cm)"
          incr totprob
        }
    }

    #now check Z positions
    check_z_posn $id 6

    if {$totprob==0} {
        set dimtext "$dimtext
- no dimension errors"
    }

    #check ISOCENTER HALF leaf dimensions

    set dimtext "$dimtext

For HALF ISOCENTER leaf:
---------------"

    set totprob 0

    #first check to see that all are defined
    check_def_synchdmlc $id 7

    #width checks

    set totprob 0
    set wlgprob 0

    if {"$cmval($id,7,0)"!="" && "$cmval($id,7,2)"!=""} {
        if {$cmval($id,7,2)>$cmval($id,7,0)} {
          set dimtext "$dimtext
- groove width ($cmval($id,7,2) cm)must be <= leaf width ($cmval($id,7,0) cm)"
          incr totprob
          set wlgprob 1
        }
    }
    if {"$cmval($id,7,0)"!="" && "$cmval($id,7,2)"!="" && \
        "$cmval($id,7,3)"!=""} {
        if {$cmval($id,7,3)>[expr $cmval($id,7,0)-$cmval($id,7,2)] &&\
          $wlgprob==0} {
          set dimtext "$dimtext
- tip width ($cmval($id,7,3) cm) must be <= leaf - groove width \
([expr $cmval($id,7,0)-$cmval($id,7,2)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,7,0)"!="" && "$cmval($id,7,1)"!="" && \
        "$cmval($id,7,4)"!="" && "$cmval($id,7,2)"!=""} {
        if {$cmval($id,7,4)>[expr $cmval($id,7,0)+$cmval($id,7,1)\
            -$cmval($id,7,2)] && $wlgprob==0} {
          set dimtext "$dimtext
- width of top support rail ($cmval($id,7,4) cm) must be <= leaf + tongue - groove width \
([expr $cmval($id,7,0)+$cmval($id,7,1)-$cmval($id,7,2)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,7,1)"!="" && "$cmval($id,7,3)"!="" && \
        "$cmval($id,7,4)"!=""} {
        if {$cmval($id,7,4)<[expr $cmval($id,7,1)+$cmval($id,7,3)]} {
          set dimtext "$dimtext
- width of top support rail ($cmval($id,7,4) cm)must be >= tongue + tip width \
([expr $cmval($id,7,1)+$cmval($id,7,3)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,7,4)"!="" && "$cmval($id,7,5)"!=""} {
        if {$cmval($id,7,5)>$cmval($id,7,4)} {
          set dimtext "$dimtext
- width of bottom support rail ($cmval($id,7,5) cm) must be <= width of top support rail\
($cmval($id,7,4) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,7,4)"!="" && "$cmval($id,7,5)"!="" &&\
        "$cmval($id,7,1)"!=""} {
        if {$cmval($id,7,5)<[expr $cmval($id,7,4)-$cmval($id,7,1)]} {
          set dimtext "$dimtext
- width of bottom support rail ($cmval($id,7,5) cm) must be >= top support rail - tongue width\
([expr $cmval($id,7,4)-$cmval($id,7,1)] cm)"
          incr totprob
        }
    }

    #now check Z positions
    check_z_posn $id 7

    if {$totprob==0} {
        set dimtext "$dimtext
- no dimension errors"
    }

    #check TARGET QUARTER leaf dimensions

    set dimtext "$dimtext

For QUARTER TARGET leaf:
---------------"

    set totprob 0

    #first check to see that all are defined
    check_def_synchdmlc $id 30

    #width checks

    if {"$cmval($id,30,0)"!="" && "$cmval($id,30,2)"!=""} {
        if {$cmval($id,30,2)>$cmval($id,30,0)} {
          set dimtext "$dimtext
- groove width must be ($cmval($id,30,2) cm) <= leaf width ($cmval($id,30,0) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,30,3)"!="" &&\
         "$cmval($id,30,0)"!="" && "$cmval($id,30,2)"!=""} {
        if {$cmval($id,30,3)>[expr $cmval($id,30,0)-$cmval($id,30,2)]} {
          set dimtext "$dimtext
- tip width ($cmval($id,30,3) cm) must be <= leaf - groove width \
([expr $cmval($id,30,0)-$cmval($id,30,2)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,30,2)"!="" && "$cmval($id,30,5)"!=""} {
        if {$cmval($id,30,5)<$cmval($id,30,2)} {
          set dimtext "$dimtext
- width of bottom support rail ($cmval($id,30,5) cm) must be >= groove width ($cmval($id,30,2) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,30,5)"!="" && "$cmval($id,30,2)"!="" &&\
        "$cmval($id,30,3)"!=""} {
        if {$cmval($id,30,5)>[expr $cmval($id,30,2)+$cmval($id,30,3)]} {
          set dimtext "$dimtext
- width of bottom support rail ($cmval($id,30,5) cm) must be <= groove + tip width\
([expr $cmval($id,30,2)+$cmval($id,30,3)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,30,4)"!="" && "$cmval($id,30,5)"!=""} {
        if {$cmval($id,30,4)>$cmval($id,30,5)} {
          set dimtext "$dimtext
- width of top support rail must be ($cmval($id,30,4) cm) <= width of bottom support rail \
($cmval($id,30,5) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,30,4)"!="" && "$cmval($id,30,5)"!="" &&\
        "$cmval($id,30,2)"!=""} {
        if {$cmval($id,30,4)<[expr $cmval($id,30,5)-$cmval($id,30,2)]} {
          set dimtext "$dimtext
- width of top support rail ($cmval($id,30,4) cm) must be >= bottom support rail - groove width \
([expr $cmval($id,30,5)-$cmval($id,30,2)] cm)"
          incr totprob
        }
    }

    #now check Z positions
    check_z_posn $id 30

    if {$totprob==0} {
        set dimtext "$dimtext
- no dimension errors"
    }

    #check ISOCENTER HALF leaf dimensions

    set dimtext "$dimtext

For QUARTER ISOCENTER leaf:
---------------"

    set totprob 0

    #first check to see that all are defined
    check_def_synchdmlc $id 31

    #width checks

    set totprob 0
    set wlgprob 0

    if {"$cmval($id,31,0)"!="" && "$cmval($id,31,2)"!=""} {
        if {$cmval($id,31,2)>$cmval($id,31,0)} {
          set dimtext "$dimtext
- groove width ($cmval($id,31,2) cm) must be <= leaf width ($cmval($id,31,0) cm)"
          incr totprob
          set wlgprob 1
        }
    }
    if {"$cmval($id,31,0)"!="" && "$cmval($id,31,2)"!="" && \
        "$cmval($id,31,3)"!=""} {
        if {$cmval($id,31,3)>[expr $cmval($id,31,0)-$cmval($id,31,2)] &&\
          $wlgprob==0} {
          set dimtext "$dimtext
- tip width ($cmval($id,31,3) cm) must be <= leaf - groove width \
([expr $cmval($id,31,0)-$cmval($id,31,2)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,31,0)"!="" && "$cmval($id,31,1)"!="" && \
        "$cmval($id,31,4)"!="" && "$cmval($id,31,2)"!=""} {
        if {$cmval($id,31,4)>[expr $cmval($id,31,0)+$cmval($id,31,1)\
            -$cmval($id,31,2)] && $wlgprob==0} {
          set dimtext "$dimtext
- width of top support rail must be <= leaf + tongue - groove width \
([expr $cmval($id,31,0)+$cmval($id,31,1)-$cmval($id,31,2)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,31,1)"!="" && "$cmval($id,31,3)"!="" && \
        "$cmval($id,31,4)"!=""} {
        if {$cmval($id,31,4)<[expr $cmval($id,31,1)+$cmval($id,31,3)]} {
          set dimtext "$dimtext
- width of top support rail ($cmval($id,31,4) cm) must be >= tongue + tip width \
([expr $cmval($id,31,1)+$cmval($id,31,3)] cm)"
          incr totprob
        }
    }
    if {"$cmval($id,31,4)"!="" && "$cmval($id,31,5)"!=""} {
        if {$cmval($id,31,5)>$cmval($id,31,4)} {
          set dimtext "$dimtext
- width of bottom support rail ($cmval($id,31,5) cm) must be <= width of top support rail\
($cmval($id,31,4) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,31,4)"!="" && "$cmval($id,31,5)"!="" &&\
        "$cmval($id,31,1)"!=""} {
        if {$cmval($id,31,5)<[expr $cmval($id,31,4)-$cmval($id,31,1)]} {
          set dimtext "$dimtext
- width of bottom support rail must be ($cmval($id,31,5) cm) >= top support rail - tongue width\
([expr $cmval($id,31,4)-$cmval($id,31,1)] cm)"
          incr totprob
        }
    }

    #now check Z positions
    check_z_posn $id 31

    if {$totprob==0} {
        set dimtext "$dimtext
- no dimension errors"
    }

    #check dimensions between leaf types

    set dimtext "$dimtext

Between leaf types:
---------------"

    set totprob

    #they're all Z position checks

#CONDITIONS BETWEEN HALF LEAVES
    if {"$cmval($id,7,8)"!="" && "$cmval($id,6,12)"!=""} {
        if {$cmval($id,7,8)<=$cmval($id,6,12)} {
          set dimtext "$dimtext
- Z of top of tongue of HALF ISOCENTER ($cmval($id,7,8) cm) must be > Z of \
top of groove of HALF TARGET ($cmval($id,6,12) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,7,9)"!="" && "$cmval($id,6,11)"!=""} {
        if {$cmval($id,6,11)>=$cmval($id,7,9)} {
          set dimtext "$dimtext
- Z of bottom of tongue of HALF TARGET ($cmval($id,6,11) cm) must be < Z of \
bottom of groove of HALF ISOCENTER ($cmval($id,7,9) cm)"
          incr totprob
        }
    }
#CONDITIONS BETWEEN FULL AND HALF LEAVES
    if {"$cmval($id,7,9)"!="" && "$cmval($id,5,8)"!=""} {
        if {$cmval($id,7,9)<=$cmval($id,5,8)} {
          set dimtext "$dimtext
- Z of bottom of groove of HALF ISOCENTER ($cmval($id,7,9) cm) must be > Z \
of bottom of tongue of FULL ($cmval($id,5,8) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,6,11)"!="" && "$cmval($id,5,9)"!=""} {
        if {$cmval($id,6,11)>=$cmval($id,5,9)} {
          set dimtext "$dimtext
- Z of bottom of tongue of HALF TARGET ($cmval($id,6,11) cm) must be < Z of \
bottom of groove of FULL ($cmval($id,5,9) cm)"
          incr totprob
        }
    }
# CONDITIONS BETWEEN QUARTER LEAVES
    if {"$cmval($id,31,8)"!="" && "$cmval($id,30,9)"!=""} {
        if {$cmval($id,31,8)<=$cmval($id,30,9)} {
          set dimtext "$dimtext
- Z of top of tongue of QUARTER ISOCENTER ($cmval($id,31,8) cm) must be > Z \
of top of groove of QUARTER TARGET ($cmval($id,30,9) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,30,11)"!="" && "$cmval($id,31,9)"!=""} {
        if {$cmval($id,30,11)>=$cmval($id,31,9)} {
          set dimtext "$dimtext
- Z of bottom of tongue of QUARTER TARGET ($cmval($id,30,11) cm) must be < Z \
of bottom of groove of QUARTER ISOCENTER ($cmval($id,31,9) cm)"
          incr totprob
        }
    }
#CONDITIONS BETWEEN HALF AND QUARTER LEAVES
    if {"$cmval($id,31,9)"!="" && "$cmval($id,6,11)"!=""} {
        if {$cmval($id,31,9)<=$cmval($id,6,11)} {
          set dimtext "$dimtext
- Z of bottom of groove of QUARTER ISOCENTER ($cmval($id,31,9) cm) must be > Z \
of bottom of tongue of HALF TARGET ($cmval($id,6,11) cm)"
          incr totprob
        }
    }
    if {"$cmval($id,30,11)"!="" && "$cmval($id,7,9)"!=""} {
        if {$cmval($id,30,11)>=$cmval($id,7,9)} {
          set dimtext "$dimtext
- Z of bottom of tongue of QUARTER TARGET ($cmval($id,30,11) cm) must be < Z \
of bottom of groove of HALF ISOCENTER ($cmval($id,7,9) cm)"
          incr totprob
        }
    }
#AND BETWEEN QUARTER AND FULL LEAVES--could happen
    if {check_full_qtr == 1 && "$cmval($id,31,9)"!="" && "$cmval($id,5,8)"!=""} {
        if {$cmval($id,31,9)<=$cmval($id,5,8)} {
          set dimtext "$dimtext
- Z of bottom of groove of QUARTER ISOCENTER ($cmval($id,31,9) cm) must be > Z \
of bottom of tongue of FULL ($cmval($id,5,8) cm)"
          incr totprob
        }
    }
    if {check_full_qtr == 1 && "$cmval($id,30,11)"!="" && "$cmval($id,5,9)"!=""} {
        if {$cmval($id,30,11)>=$cmval($id,5,9)} {
          set dimtext "$dimtext
- Z of bottom of tongue of QUARTER TARGET ($cmval($id,30,11) cm) must be < Z \
of bottom of groove of FULL ($cmval($id,5,9) cm)"
          incr totprob
        }
    }

    if {$totprob==0} {
        set dimtext "$dimtext
- no dimension errors"
    }

    #set up scrolling textbox with messages
    # put the text in a yscroll window frame

    toplevel .checkdim -bd 8
    wm title .checkdim "Leaf dimension check"
    wm geometry .checkdim +100+100

    frame .checkdim.m -bg white

    set height 30

    text .checkdim.m.text -font $helvfont -width 60 -height $height -wrap word\
         -bd 5 -yscrollcommand ".checkdim.m.v_scroll set" -bg white
    .checkdim.m.text insert 1.0 $dimtext
    .checkdim.m.text configure -state disabled

    # vertical scrollbar:
    scrollbar .checkdim.m.v_scroll -command ".checkdim.m.text yview" -bg white

    pack .checkdim.m.text -side left
    pack .checkdim.m.v_scroll -side right -fill y

    pack .checkdim.m
    frame .checkdim.sep -bd 4 -width 100 -height 2 -relief groove
    pack .checkdim.sep -pady 10 -fill x
    button .checkdim.okb -text "OK" -command "destroy .checkdim" \
            -relief groove -bd 8
    pack .checkdim.okb -pady 10
}

proc check_def_synchdmlc { id leafind } {
    global cmval dimtext totprob

#procedure to check if leaf cross-section dimensions are actually
#defined

    if {"$cmval($id,$leafind,0)"==""} {
       set dimtext "$dimtext
- define leaf width"
       incr totprob
    }
    if {"$cmval($id,$leafind,1)"==""} {
       set dimtext "$dimtext
- define tongue width"
       incr totprob
    }
    if {"$cmval($id,$leafind,2)"==""} {
       set dimtext "$dimtext
- define groove width"
       incr totprob
    }
    if {"$cmval($id,$leafind,3)"==""} {
       set dimtext "$dimtext
- define tip width"
       incr totprob
    }
    if {"$cmval($id,$leafind,4)"==""} {
       set dimtext "$dimtext
- define width of top support rail"
       incr totprob
    }
    if {"$cmval($id,$leafind,5)"==""} {
       set dimtext "$dimtext
- define width of bottom support rail"
       incr totprob
    }
    if {"$cmval($id,$leafind,5)"==""} {
       set dimtext "$dimtext
- define width of bottom support rail"
       incr totprob
    }

    #here's where things differ from leaf to leaf
    if {"$cmval($id,$leafind,6)"==""} {
       if {$leafind!=6 && $leafind!=30} {
         set dimtext "$dimtext
- define Z of top of tip"
       } else {
         set dimtext "$dimtext
- define Z of top of support rail"
       }
       incr totprob
    }
    if {"$cmval($id,$leafind,7)"==""} {
       if {$leafind!=6 && $leafind!=30} {
         set dimtext "$dimtext
- define Z of top of leaf"
       } else {
         set dimtext "$dimtext
- define Z of bottom of support rail"
       }
       incr totprob
    }
    if {"$cmval($id,$leafind,8)"==""} {
       if {$leafind==5} {
         set dimtext "$dimtext
- define Z of bottom of tongue"
       } elseif {$leafind!=5 && $leafind!=7 && $leafind!=31} {
         set dimtext "$dimtext
- define Z of top of driving screw hole"
       } else {
         set dimtext "$dimtext
- define Z of top of tongue"
       }
       incr totprob
    }
    if {"$cmval($id,$leafind,9)"==""} {
       if {$leafind!=6 && $leafind!=30} {
         set dimtext "$dimtext
- define Z of bottom of groove"
       } else {
         set dimtext "$dimtext
- define Z of bottom of driving screw hole"
       }
       incr totprob
    }
     if {"$cmval($id,$leafind,10)"==""} {
       if {$leafind!=6 && $leafind!=30} {
         set dimtext "$dimtext
- define Z of top of driving screw hole"
       } else {
         set dimtext "$dimtext
- define Y distance of driving screw hole from leaf tip"
       }
       incr totprob
    }
    if {"$cmval($id,$leafind,11)"==""} {
       if {$leafind!=6 && $leafind!=30} {
         set dimtext "$dimtext
- define Z of bottom of driving screw hole"
       } else {
         set dimtext "$dimtext
- define Z of bottom of tongue"
       }
       incr totprob
    }
    if {"$cmval($id,$leafind,12)"==""} {
       if {$leafind!=6 && $leafind!=30} {
         set dimtext "$dimtext
- define Y distance of driving screw hole from leaf tip"
       } else {
         set dimtext "$dimtext
- define Z of top of groove"
       }
       incr totprob
    }
    if {"$cmval($id,$leafind,13)"==""} {
       if {$leafind!=6 && $leafind!=30} {
         set dimtext "$dimtext
- define Z of top of support rail"
       } else {
         set dimtext "$dimtext
- define Z of bottom of leaf"
       }
       incr totprob
    }
    if {"$cmval($id,$leafind,14)"==""} {
       if {$leafind!=6 && $leafind!=30} {
         set dimtext "$dimtext
- define Z of bottom of support rail"
       } else {
         set dimtext "$dimtext
- define Z of bottom of tip"
       }
       incr totprob
    }
    return
}

proc check_z_posn { id leafind } {
    global cmval dimtext totprob

#procedure to check Z positions in leaf cross-sections
#defined

      if {"$cmval($id,$leafind,6)"!="" && "$cmval($id,3)"!=""} {
        if {$cmval($id,$leafind,6)<$cmval($id,3)} {
          if {$leafind!=6 && $leafind!=30} {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of tip ($cmval($id,$leafind,6) cm) must be \
>= ZMIN ($cmval($id,3) cm)"
          } else {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of top support rail ($cmval($id,$leafind,6) cm) must be \
>= ZMIN ($cmval($id,3) cm)"
          }
          incr totprob
        }
      }
      if {"$cmval($id,$leafind,6)"!="" && "$cmval($id,$leafind,7)"!=""} {
        if {$cmval($id,$leafind,7)<=$cmval($id,$leafind,6)} {
          if {$leafind!=6 && $leafind!=30} {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of leaf ($cmval($id,$leafind,7) cm) must be \
> Z of top of tip ($cmval($id,$leafind,6) cm)"
          } else {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom support rail ($cmval($id,$leafind,7) cm) must be \
>= Z of top support rail ($cmval($id,$leafind,6) cm)"
          }
          incr totprob
        }
      }
      if {"$cmval($id,$leafind,7)"!="" && "$cmval($id,$leafind,8)"!=""} {
        if {$cmval($id,$leafind,7)>=$cmval($id,$leafind,8)} {
           if {$leafind==5} {
             set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of tongue ($cmval($id,$leafind,8) cm) must be \
> Z of top of leaf ($cmval($id,$leafind,7) cm)"
           } elseif {$leafind==6} {
             set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of tongue ($cmval($id,$leafind,8) cm) must be\
 > Z of top of leaf ($cmval($id,$leafind,7) cm)"
           } elseif {$leafind==30} {
             set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of tongue ($cmval($id,$leafind,8) cm) must be \
> Z of top of leaf ($cmval($id,$leafind,7) cm)"
           } elseif {$leafind==7} {
             set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of driving screw hole ($cmval($id,$leafind,8) cm) must be \
> Z of bottom of support rail ($cmval($id,$leafind,7) cm)"
           }
           incr totprob
        }
      }
       if {"$cmval($id,$leafind,8)"!="" && "$cmval($id,$leafind,9)"!=""} {
        if {$cmval($id,$leafind,8)>=$cmval($id,$leafind,9)} {
           if {$leafind==5} {
             set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of groove ($cmval($id,$leafind,9) cm) must be \
> Z of bottom of tongue ($cmval($id,$leafind,8) cm)"
           } elseif {$leafind==7} {
             set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of groove ($cmval($id,$leafind,9) cm) must be \
> Z of top of tongue ($cmval($id,$leafind,8) cm)"
           } elseif {$leafind==6} {
             set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of driving screw hole ($cmval($id,$leafind,9) cm) must be \
> Z of top of driving screw hole($cmval($id,$leafind,8) cm)"
           } elseif {$leafind==30} {
             set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of driving screw hole ($cmval($id,$leafind,9) cm) must be \
> Z of top of driving screw hole($cmval($id,$leafind,8) cm)"
           }
           incr totprob
        }
      }
     if {$leafind==31 && "$cmval($id,31,8)"!="" && "$cmval($id,31,9)"!=""} {
      if {$cmval($id,31,8)<$cmval($id,31,9)} {
         set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of tongue ($cmval($id,31,9) cm) must be \
>= Z of bottom of groove ($cmval($id,31,8) cm)"
           }
           incr totprob
      }
      if {"$cmval($id,$leafind,9)"!="" && "$cmval($id,$leafind,10)"!=""} {
       if {$cmval($id,$leafind,10)<=$cmval($id,$leafind,9)} {
          if {$leafind!=6 && $leafind!=30} {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of driving screw hole ($cmval($id,$leafind,10) cm) must be \
> Z of bottom of groove ($cmval($id,$leafind,9) cm)"
           }
           incr totprob
         }
      }
     if {"$cmval($id,$leafind,10)"!="" && "$cmval($id,$leafind,11)"!=""} {
      if {$cmval($id,$leafind,11)<$cmval($id,$leafind,10)} {
         if {$leafind!=6 && $leafind!=30} {
          set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of driving screw hole ($cmval($id,$leafind,11) cm) must be \
> Z of top of driving screw hole ($cmval($id,$leafind,10) cm)"
           }
           incr totprob
         }
      }

      if {"$cmval($id,$leafind,11)"!="" && "$cmval($id,$leafind,12)"!=""} {
      if {$cmval($id,$leafind,12)<$cmval($id,$leafind,11)} {
         if {$leafind!=5 && $leafind!=7 && $leafind!=31} {
          set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of groove ($cmval($id,$leafind,12) cm) must be \
> Z of bottom of tongue ($cmval($id,$leafind,11) cm)"
           }
           incr totprob
         }
       }
      if {"$cmval($id,$leafind,11)"!="" && "$cmval($id,$leafind,13)"!=""} {
       if {$cmval($id,$leafind,13)<$cmval($id,$leafind,11)} {
          if {$leafind!=6 && $leafind!=30} {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of top of support rail ($cmval($id,$leafind,13) cm) must be \
> Z of bottom of driving screw hole ($cmval($id,$leafind,11) cm)"
           }
           incr totprob
         }
      }
      if {"$cmval($id,$leafind,9)"!="" && "$cmval($id,$leafind,11)"!=""} {
       if {$cmval($id,$leafind,11)<$cmval($id,$leafind,9)} {
          if {$leafind!=5 && $leafind!=7 && $leafind!=31} {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of tongue($cmval($id,$leafind,11) cm) must be \
>= Z of bottom of driving screw hole ($cmval($id,$leafind,9) cm)"
           }
        incr totprob
        }
      }
      if {"$cmval($id,$leafind,12)"!="" && "$cmval($id,$leafind,13)"!=""} {
       if {$cmval($id,$leafind,13)<$cmval($id,$leafind,12)} {
          if {$leafind!=6 && $leafind!=30} {
           set dimtext "$dimtext
-NEEDS ATTENTION: Y distance of driving screw hole from leaf tip is expected \
be much smaller than ($cmval($id,$leafind,12) cm)"
           } else {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of leaf ($cmval($id,$leafind,13) cm) must be\
 >= Z of top of groove ($cmval($id,$leafind,12) cm)"
           }
         incr totprob
       }
     }
     if {"$cmval($id,$leafind,13)"!="" && "$cmval($id,$leafind,14)"!=""} {
      if {$cmval($id,$leafind,14)<$cmval($id,$leafind,13)} {
         if {$leafind!=6 && $leafind!=30} {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom support rail ($cmval($id,$leafind,14) cm) must be \
>= Z of top support rail ($cmval($id,$leafind,13) cm)"
         } else {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of tip ($cmval($id,$leafind,14) cm) must be \
>= Z of bottom of leaf ($cmval($id,$leafind,13) cm)"
         }
         incr totprob
        }
      }
      if {"$cmval($id,3)"!="" && "$cmval($id,4)"!="" && \
          "$cmval($id,$leafind,14)"!=""} {
       if {$cmval($id,$leafind,14)>[expr $cmval($id,3)+$cmval($id,4)]} {
          if {$leafind!=6 && $leafind!=30} {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom support rail must be <= ZMIN+ZTHICK\
 ([expr $cmval($id,3)+$cmval($id,4)] cm)"
          } else {
           set dimtext "$dimtext
- NEEDS ATTENTION: Z of bottom of tip must be <= ZMIN+ZTHICK \
([expr $cmval($id,3)+$cmval($id,4)] cm)"
          }
          incr totprob
        }
      }
}

proc define_synchdmlc_openings { id } {
    global cmval helvfont ngroups nleaf to from help_synchdmlc_text

    if {$nleaf($id)==""} {
       tk_dialog .setnleaf "Set number of leaves" "Before you can specify\
       the leaf openings, you must specify the number of leaves." warning 0 OK
       return
    }

    catch { destroy .synchdmlc$id.child }
    toplevel .synchdmlc$id.child
    set top .synchdmlc$id.child
    wm title $top "Define leaf openings"

    frame $top.f -bd 10
    set w $top.f

    if {$cmval($id,2,0)==1 || $cmval($id,2,0)==3} {
	set orientation X
    } else {
	set orientation Y
    }

    if [string compare $ngroups($id) ""]==0 {
	set ngroups($id) 1
    }

    message $w.message -text "Specify the minimum and maximum $orientation\
	    value (for straight, focused leaf ends, this is defined at ZMIN)\
            for the opening in \
	    each leaf in the SYNCHDMLC by groups here.  Add a row to start a\
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
    button $top.b.addb -text "Add a row" -command "add_synchdmlc_row $id"\
	    -relief groove -bd 8
    button $top.b.delb -text "Delete last row"\
            -command "del_synchdmlc_row $id" -relief groove -bd 8
    button $top.b.okb -text "OK" -command "save_synchdmlc $id" -relief groove -bd 8
    button $top.b.helpb -text "Help" -command\
	    "help_gif .synchdmlc$id.child.help {$help_synchdmlc_text} help_synchdmlc"\
	    -relief groove -bd 8
    pack $top.b.addb $top.b.delb $top.b.okb $top.b.helpb -side left -padx 10
    pack $top.b -pady 10
}

proc save_synchdmlc { id } {
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

    destroy .synchdmlc$id.child

}

proc save_synchdmlc_type { id } {
    global cmval fromw tow nleaf nhlf nfull nqtr check_full_qtr

    if {$tow($cmval($id,2,1))>$nleaf($id)} {
       tk_dialog .toomanytypegroups "Too many leaves" "You have specified too many\
       leaf types for the number of leaves ($nleaf($id))." warning 0 OK
       return
    } elseif {$tow($cmval($id,2,1))<$nleaf($id)} {
       tk_dialog .toofewtypegroups "Too few leaves" "You have specified too few\
       leaf types for the number of leaves ($nleaf($id))." warning 0 OK
       return
    }

    set nhlf 0
    set nqtr 0
    set nfull 0
    set check_full_qtr 0
    for {set j 1} {$j<=$cmval($id,2,1)} {incr j} {
        if {[catch {set cmval($id,8,0,$j) [expr $tow($j)-$fromw($j)+1]}]==1} {
           tk_dialog .nope "No no no" "The leaf types for group $j\
                    have been improperly set or not set at all.\
                    Go back and fix them." warning 0 OK
        }

        if {$cmval($id,8,1,$j)==1} {
            incr nfull
        } elseif {$cmval($id,8,1,$j)==2} {
            incr nhlf
        } elseif {$cmval($id,8,1,$j)==3} {
            incr nqtr
        }
        if {$j > 1} {
          if {($cmval($id,8,1,$j)==1 && $cmval($id,8,1,[expr $j-1])==3) ||
              ($cmval($id,8,1,$j)==3 && $cmval($id,8,1,[expr $j-1])==1)} {
             set check_full_qtr 1
          }
        }
    }

    destroy .synchdmlc$id.childt

}

proc add_synchdmlc_row { id } {
    global ngroups from to cmval nleaf

    set w .synchdmlc$id.child.f

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

proc add_synchdmlc_row_type { id } {
    global cmval fromw tow nleaf

    set w .synchdmlc$id.childt.f

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
    if [catch {set tow($j) [expr $tow([expr $j-1])+$cmval($id,8,0,$j)]}]==1 {
        set tow($j) {}
    }
    entry $w.grid.e0$j -textvariable fromw($j) -width 8
    grid configure $w.grid.e0$j -row $j -column 0
    entry $w.grid.e1$j -textvariable tow($j) -width 8
    grid configure $w.grid.e1$j -row $j -column 1 -padx 5
    radiobutton $w.grid.e2$j -variable cmval($id,8,1,$j) -value 1
    grid configure $w.grid.e2$j -row $j -column 2 -padx 5
    radiobutton $w.grid.e3$j -variable cmval($id,8,1,$j) -value 2
    grid configure $w.grid.e3$j -row $j -column 3
    radiobutton $w.grid.e4$j -variable cmval($id,8,1,$j) -value 3
    grid configure $w.grid.e4$j -row $j -column 4
}

proc del_synchdmlc_row { id } {
    global ngroups

    set w .synchdmlc$id.child.f

    for {set i 0} {$i<4} {incr i} {
         destroy $w.grid.e$i$ngroups($id)
    }
    incr ngroups($id) -1
}

proc del_synchdmlc_row_type { id } {
    global cmval

    set w .synchdmlc$id.childt.f

    for {set i 0} {$i<5} {incr i} {
         destroy $w.grid.e$i$cmval($id,2,1)
    }
    incr cmval($id,2,1) -1
}

proc write_SYNCHDMLC {fileid id} {
    global cmval cm_names cm_ident cm_type ngroups sync_file nfull nhlf nqtr

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2,0), $cmval($id,2,1), $cmval($id,2,2), ORIENT, NGROUP, MODE"
    puts $fileid "$cmval($id,3), ZMIN"
    puts $fileid "$cmval($id,4), ZTHICK"

    #FULL leaf dimensions
    set str {}
    for {set i 0} {$i<15} {incr i} {
        set str "$str$cmval($id,5,$i), "
    }
    if {$nfull == 0 | $str!=", , , , , , , , , , , , , , , "} {
       puts $fileid $str
    } else {
# output a guess based on leaves clearing each other
	puts $fileid "0.4, 0.04, 0.04 0.1, 0.3, 0.1, [expr $cmval($id,3)],\
[expr $cmval($id,3)+0.5], [expr $cmval($id,3)+2.5], [expr $cmval($id,3)+3.7],\
[expr $cmval($id,3)+3.9], [expr $cmval($id,3)+4.3], 1.7, [expr $cmval($id,3)+5.5],\
[expr $cmval($id,3)+5.8],"
    }

    #TARGET HALF leaf dimensions
    #no guess for half leaf dimensions
    set str {}
    for {set i 0} {$i<15} {incr i} {
        set str "$str$cmval($id,6,$i), "
    }
    puts $fileid $str

    #ISOCENTER HALF leaf dimensions
    set str {}
    for {set i 0} {$i<15} {incr i} {
        set str "$str$cmval($id,7,$i), "
    }
    puts $fileid $str

    #TARGET QUARTER leaf dimensions
    set str {}
    for {set i 0} {$i<15} {incr i} {
        set str "$str$cmval($id,30,$i), "
    }
    if {$nqtr==0 | $str!=", , , , , , , , , , , , , , , "} {
       puts $fileid $str
    } else {
#put in a guess based on leaves clearing each other
       puts $fileid "0.11, 0.037, 0.037, 0.04, 0.07, 0.08, [expr $cmval($id,3)+0.1],\
[expr $cmval($id,3)+0.3], [expr $cmval($id,3)+0.9], [expr $cmval($id,3)+1.2],\
1.7, [expr $cmval($id,3)+3.3], [expr $cmval($id,3)+3.5], [expr $cmval($id,3)+6],\
[expr $cmval($id,3)+6.7],"
    }

    #ISOCENTER QUARTER leaf dimensions
    set str {}
    for {set i 0} {$i<15} {incr i} {
        set str "$str$cmval($id,31,$i), "
    }
    if {$nqtr==0 | $str!=", , , , , , , , , , , , , , , "} {
       puts $fileid $str
    } else {
#a guess based on leaf clearance
       puts $fileid "0.11, 0.036, 0.0365, 0.032, 0.08, 0.07, [expr $cmval($id,3)+0.2],\
[expr $cmval($id,3)+0.3], [expr $cmval($id,3)+3.7], [expr $cmval($id,3)+3.6],\
[expr $cmval($id,3)+5], [expr $cmval($id,3)+6], 1.7, [expr $cmval($id,3)+6.5],\
[expr $cmval($id,3)+6.8],"
    }

    for {set j 1} {$j<=$cmval($id,2,1)} {incr j} {
        puts $fileid "$cmval($id,8,0,$j), $cmval($id,8,1,$j)"
    }
    puts $fileid "$cmval($id,9), START"
    puts $fileid "$cmval($id,10), LEAFGAP"
    puts $fileid "$cmval($id,11), ENDTYPE"
    puts $fileid "$cmval($id,12), ZFOCUS or RADIUS of leaf ends"
    puts $fileid "$cmval($id,13), ZFOCUS of leaf sides"

    if { $cmval($id,2,2)==1 | $cmval($id,2,2)==2 } {
        puts $fileid "$sync_file($id)"
    } else {
    for {set j 1} {$j<=$ngroups($id)} {incr j} {
	puts $fileid "$cmval($id,14,0,$j), $cmval($id,14,1,$j), $cmval($id,14,2,$j)"
    }
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

    set str {}
    for {set i 0} {$i<4} {incr i} {
        set str "$str$cmval($id,19,$i), "
    }
    puts $fileid $str
    puts $fileid $cmval($id,20)
}


proc show_SYNCHDMLC { id } {
    global cmval helvfont medium nmed xrange yrange zrange med cm_ticks nleaf

    catch { destroy .synchdmlc$id.show }
    toplevel .synchdmlc$id.show
    wm title .synchdmlc$id.show "Preview"

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
    frame .synchdmlc$id.show.frm

    draw_SYNCHDMLC $id

    frame .synchdmlc$id.show.buts
    button .synchdmlc$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .synchdmlc$id.show xyz" -relief groove -bd 8
    button .synchdmlc$id.show.buts.print1 -text "Print xy..." -command\
	    "print_canvas .synchdmlc$id.show.frm.can.one 450 450" -relief groove -bd 8
    button .synchdmlc$id.show.buts.print2 -text "Print xz..." -command\
	    "print_canvas .synchdmlc$id.show.frm.can.two 450 450" -relief groove -bd 8
    button .synchdmlc$id.show.buts.print3 -text "Print yz..." -command\
	    "print_canvas .synchdmlc$id.show.frm.can.three 450 450" -relief groove -bd 8
    button .synchdmlc$id.show.buts.done -text "Done" -command\
	    "destroy .synchdmlc$id.show" -relief groove -bd 8
    pack .synchdmlc$id.show.buts.range .synchdmlc$id.show.buts.print1\
	    .synchdmlc$id.show.buts.print2 .synchdmlc$id.show.buts.print3\
	    .synchdmlc$id.show.buts.done -side left -padx 10
    pack .synchdmlc$id.show.buts -side bottom -anchor s -pady 15

}

proc draw_SYNCHDMLC { id } {
    global helvfont zrange yrange xrange zscale yscale xscale l m
    global cmval cm_ticks nleaf

    catch { destroy .synchdmlc$id.show.frm.can }

    # put the canvas up
    set ncan 3
    set width 180.0
    set canwidth [expr $width+150.0]
    set scrlheight [expr 2*$canwidth]
    set scrlwidth [expr 2*$canwidth]
    set winwidth [expr [winfo screenwidth .]*4.0/5.0]
    set winheight [expr [winfo screenheight .]*4.0/5.0]

    if $scrlwidth>$winwidth {
	catch { destroy .synchdmlc$id.show.frm.scrlz }
	catch { destroy .synchdmlc$id.show.frm.scrlx }
	scrollbar .synchdmlc$id.show.frm.scrlz -command\
		".synchdmlc$id.show.frm.can yview"
	scrollbar .synchdmlc$id.show.frm.scrlx -command\
		".synchdmlc$id.show.frm.can xview" -orient horizontal
	pack .synchdmlc$id.show.frm.scrlz -side right -fill y
	pack .synchdmlc$id.show.frm.scrlx -side bottom -fill x
	canvas .synchdmlc$id.show.frm.can -width $winwidth -height $winheight\
		-yscrollcommand ".synchdmlc$id.show.frm.scrlz set"\
		-xscrollcommand ".synchdmlc$id.show.frm.scrlx set"\
		-scrollregion "0 0 $scrlwidth $scrlheight"
    } else {
	canvas .synchdmlc$id.show.frm.can -width $scrlwidth -height $scrlheight
    }

    # Put some text in the upper left corner, just to fill the gap.
    if {$cmval($id,2,2)==1 | $cmval($id,2,2)==2} {
      .synchdmlc$id.show.frm.can create text 160 160 -text "xz\
            cross-section shows only those leaves that\
            are intersected by y=0, and yz cross-section shows only those\
            leaves intersected by x=0.  In the case where only a portion\
            of the leaf is intersected by the axis, the entire cross-section\
            is shown.  The xy view is at a plane at Z=ZMIN\
            (straight leaf ends) or Z=ZMIN+ZTHICK/2 (cylindrical\
            leaf ends).

NOTE: This is a dynamic or step-and-shoot delivery, with leaf opening\
      coordinates defined in a file.  Leaf opening coordinates are shown for the\
      first field defined in this file."\
            -font $helvfont -width 300
    } else {
    .synchdmlc$id.show.frm.can create text 160 160 -text "xz\
            cross-section shows only those leaves that\
	    are intersected by y=0, and yz cross-section shows only those\
            leaves intersected by x=0.  In the case where only a portion\
            of the leaf is intersected by the axis, the entire cross-section\
            is shown.  The xy view is at a plane at Z=ZMIN\
            (straight leaf ends) or Z=ZMIN+ZTHICK/2 (cylindrical\
            leaf ends)."\
	    -font $helvfont -width 300
    }

    set xscale [expr $width/double(abs($xrange(1)-$xrange(0)))]
    set yscale [expr $width/double(abs($yrange(1)-$yrange(0)))]
    set zscale [expr $width/double(abs($zrange(1)-$zrange(0)))]

    set l 100.0
    set m 50.0

    # XY, upper right

    canvas .synchdmlc$id.show.frm.can.one -width $canwidth -height $canwidth

    add_SYNCHDMLC_xy $id $xscale $yscale $xrange(0) $yrange(0)\
	    $l $m .synchdmlc$id.show.frm.can.one

    set curx 0
    set cury 0
    label .synchdmlc$id.show.frm.can.one.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .synchdmlc$id.show.frm.can.one create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .synchdmlc$id.show.frm.can.one.xy
    bind .synchdmlc$id.show.frm.can.one <Motion> {
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
    coverup $l $m $width .synchdmlc$id.show.frm.can.one

    add_axes xy $cm_ticks($id,x) $cm_ticks($id,y) $l $m $width $xrange(0)\
	    $xrange(1) $yrange(0) $yrange(1)\
	    $xscale $yscale .synchdmlc$id.show.frm.can.one

    .synchdmlc$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth/2.0] -window .synchdmlc$id.show.frm.can.one

    # XZ, lower left: if leaves // to y, show leaf sides,
    # else if leaves // to x, show only one leaf at y=0 (leaf ends).

    canvas .synchdmlc$id.show.frm.can.two -width $canwidth -height $canwidth

    add_air $id .synchdmlc$id.show.frm.can.two $xrange(0) $zrange(0)\
	    $xscale $zscale $l $m

    if {$cmval($id,2,0)==0 || $cmval($id,2,0)==2} {
	add_SYNCHDMLC_sides $id $xscale $zscale $xrange(0) $zrange(0) $zrange(1)\
		$l $m .synchdmlc$id.show.frm.can.two
    } else {
        add_SYNCHDMLC_ends $id $xscale $zscale $xrange(0) $zrange(0) $zrange(1)\
		$l $m .synchdmlc$id.show.frm.can.two
    }

    coverup $l $m $width .synchdmlc$id.show.frm.can.two

    label .synchdmlc$id.show.frm.can.two.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .synchdmlc$id.show.frm.can.two create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .synchdmlc$id.show.frm.can.two.xy
    bind .synchdmlc$id.show.frm.can.two <Motion> {
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
	    $xscale $zscale .synchdmlc$id.show.frm.can.two

    .synchdmlc$id.show.frm.can create window [expr $canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .synchdmlc$id.show.frm.can.two

    # YZ, lower right

    canvas .synchdmlc$id.show.frm.can.three -width $canwidth -height $canwidth

    add_air $id .synchdmlc$id.show.frm.can.three $yrange(0) $zrange(0)\
	    $yscale $zscale $l $m

    if {$cmval($id,2,0)==0 || $cmval($id,2,0)==2} {
	add_SYNCHDMLC_ends $id $yscale $zscale $yrange(0) $zrange(0) $zrange(1)\
		$l $m .synchdmlc$id.show.frm.can.three
    } else {
	add_SYNCHDMLC_sides $id $yscale $zscale $yrange(0) $zrange(0) $zrange(1)\
		$l $m .synchdmlc$id.show.frm.can.three
    }

    coverup $l $m $width .synchdmlc$id.show.frm.can.three

    label .synchdmlc$id.show.frm.can.three.xy -text \
	    [format "(%6.3f, %6.3f)" $curx $cury] -bg white\
	    -bd 5 -width 16 -font $helvfont
    .synchdmlc$id.show.frm.can.three create window [expr $l/2+$width]\
	    [expr 2*$m+$width] -window .synchdmlc$id.show.frm.can.three.xy
    bind .synchdmlc$id.show.frm.can.three <Motion> {
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
	    $yscale $zscale .synchdmlc$id.show.frm.can.three

    .synchdmlc$id.show.frm.can create window [expr $canwidth+$canwidth/2.0]\
	    [expr $canwidth+$canwidth/2.0] -window .synchdmlc$id.show.frm.can.three

    pack .synchdmlc$id.show.frm.can -side top -anchor n
    pack .synchdmlc$id.show.frm -side top -anchor n
}

proc add_SYNCHDMLC_xy {id xscale yscale xmin ymin l m parent_w} {
    global cmval colorlist medium nmed nleaf colornum sync_file nfull nhlf nqtr

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

    # set up an array of leaf types
    set k 0; #leaf k
    for {set i 1} {$i<=$cmval($id,2,1)} {incr i} {
        for {set j 1} {$j<=$cmval($id,8,0,$i)} {incr j} {
          incr k
          if {$cmval($id,8,1,$i)==1} {
             set type($k) 1
          } elseif {$cmval($id,8,1,$i)==2} {
             if {$k > 1 && ($type([expr $k-1])==4 || $type([expr $k-1])==2)} {
               set type($k) 3
             } else {
               set type($k) 2
             }
          } elseif {$cmval($id,8,1,$i)==3} {
             if {$k > 1 && ($type([expr $k-1])==4 || $type([expr $k-1])==2)} {
               set type($k) 5
             } else {
               set type($k) 4
             }
          }
        }
    }

    # set up an array of neg and pos for each leaf
    if {$cmval($id,2,2)==1 | $cmval($id,2,2)==2} {
      set fileid [open $sync_file($id) "r"]
      #first three lines are title, no. of fields, muindex of first field
      gets $fileid data
      gets $fileid data
      gets $fileid data
      set leafnum 0; set groupnum 0;
      while {$leafnum<$nleaf($id)} {
            incr groupnum
            gets $fileid data
            for {set i 0} {$i<3} {incr i} {
                set data [get_val $data cmval $id,14,$i,$groupnum]
            }
            if { $cmval($id,14,2,$groupnum)<=0 } {
                set cmval($id,14,2,$groupnum) 1
            }
            incr leafnum $cmval($id,14,2,$groupnum)
       }
    }
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
    #now flip leaf opening sequence if ORIENT=2 or 3
    if {$cmval($id,2,0) == 2 || $cmval($id,2,0) == 3} {
        for {set i 1} {$i <= $nleaf($id)/2} {incr i} {
          set tmp_coord $neg($i)
          set neg($i) $neg([expr $nleaf($id)-$i+1])
          set neg([expr $nleaf($id)-$i+1]) $tmp_coord
          set tmp_coord $pos($i)
          set pos($i) $pos([expr $nleaf($id)-$i+1])
          set pos([expr $nleaf($id)-$i+1]) $tmp_coord
        }
    }

    set color [lindex $colorlist $med(leaves)]

    # if it's parallel to x, neg and pos are x-values, else neg and pos are
    # y-values.  Define a rectangle by (a,b) to (c,d).
    # the first leaf starts at START

    #set up arrays of widths that can be seen from the top view
    #FULL leaf
  if {$nfull > 0} {
    set w(1,1) $cmval($id,5,3)
    set w(1,2) [expr $cmval($id,5,0)+$cmval($id,5,1)-$cmval($id,5,3) \
                     -$cmval($id,5,2)]
    set w(1,3) $cmval($id,5,2)
    set numw(1) 3
    #also, store tongue, groove and total widths for later use
    set wt(1) $cmval($id,5,1)
    set wg(1) $cmval($id,5,2)
    set wtot(1) [expr $cmval($id,5,0)+$cmval($id,5,1)]
  }
  if {$nhlf > 0} {
    #TARGET HALF leaf
    set w(2,1) [expr $cmval($id,6,0)+$cmval($id,6,1)-$cmval($id,6,5) \
                     +$cmval($id,6,4)]
    set w(2,2) [expr $cmval($id,6,5)-$cmval($id,6,4)]
    set numw(2) 2
    set wt(2) $cmval($id,6,1)
    set wg(2) $cmval($id,6,2)
    set wtot(2) [expr $cmval($id,6,0)+$cmval($id,6,1)]
    #ISOCENTER HALF leaf
    set w(3,1) $cmval($id,7,1)
    set w(3,2) $cmval($id,7,3)
    set w(3,3) [expr $cmval($id,7,0)-$cmval($id,7,2)-$cmval($id,7,3)]
    set w(3,4) $cmval($id,7,2)
    set numw(3) 4
    set wt(3) $cmval($id,7,1)
    set wg(3) $cmval($id,7,2)
    set wtot(3) [expr $cmval($id,7,0)+$cmval($id,7,1)]
  }
  if {$nqtr > 0} {
    #TARGET QUARTER leaf
    set w(4,1) [expr $cmval($id,30,0)+$cmval($id,30,1)-$cmval($id,30,5) \
                     +$cmval($id,30,4)]
    set w(4,2) [expr $cmval($id,30,5)-$cmval($id,30,4)]
    set numw(4) 2
    set wt(4) $cmval($id,30,1)
    set wg(4) $cmval($id,30,2)
    set wtot(4) [expr $cmval($id,30,0)+$cmval($id,30,1)]
    #ISOCENTER QUARTER leaf
    set w(5,1) $cmval($id,31,1)
    set w(5,2) $cmval($id,31,3)
    set w(5,3) [expr $cmval($id,31,0)-$cmval($id,31,2)-$cmval($id,31,3)]
    set w(5,4) $cmval($id,31,2)
    set numw(5) 4
    set wt(5) $cmval($id,31,1)
    set wg(5) $cmval($id,31,2)
    set wtot(5) [expr $cmval($id,31,0)+$cmval($id,31,1)]
  }
    if {$cmval($id,2,0)==1 || $cmval($id,2,0)==3} {
	# leaves parallel to x
	set lstart $cmval($id,9)
	set rmin [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
	set rmax [expr ($cmval($id,0)-$xmin)*$xscale+$l]
	for {set i 1} {$i <= $nleaf($id)} {incr i} {
            # now make rectangles for the top view of the leaf
            set color [lindex $colorlist $med(leaves)]

            set a [expr ($neg($i)-$xmin)*$xscale+$l]
            if {$cmval($id,2,0)==3} {
               set b [expr (-$lstart-$ymin)*$yscale+$m]
            } else {
               set b [expr ($lstart-$ymin)*$yscale+$m]
            }
            set c [expr ($pos($i)-$xmin)*$xscale+$l]
            if {$cmval($id,2,0)==3} {
               set d [expr (-($lstart+$wtot($type($i)))-$ymin)*$yscale+$m]
            } else {
               set d [expr ($lstart+$wtot($type($i))-$ymin)*$yscale+$m]
            }
            $parent_w create rectangle $rmin $b $a $d -fill $color -outline black
            $parent_w create rectangle $rmax $b $c $d -fill $color -outline black

            if {$i<$nleaf($id)} {
              if {[expr $cmval($id,10)]>$wt($type([expr $i+1]))} {
                  #airgap is > tongue width
                  #draw the air space
                  set color [lindex $colorlist $med(in)]
                  set a [expr ($neg($i)-$xmin)*$xscale+$l]
                  if {$cmval($id,2,0)==3} {
                     set b [expr (-($lstart+$wtot($type($i)))-$ymin)*$yscale+$m]
                  } else {
                     set b [expr ($lstart+$wtot($type($i))-$ymin)*$yscale+$m]
                  }
                  set c [expr ($pos($i)-$xmin)*$xscale+$l]
                  if {$cmval($id,2,0)==3} {
                     set d [expr (-($lstart+$wtot($type($i))+$cmval($id,10) \
                               -$wt($type([expr $i+1])))-$ymin)*$yscale+$m]
                  } else {
                     set d [expr ($lstart+$wtot($type($i))+$cmval($id,10) \
                               -$wt($type([expr $i+1]))-$ymin)*$yscale+$m]
                  }
                  $parent_w create rectangle $rmin $b $a $d -fill $color -outline {}
                  $parent_w create rectangle $rmax $b $c $d -fill $color -outline {}
              }
              set lstart [expr $lstart+$wtot($type($i))+$cmval($id,10) \
                               -$wt($type([expr $i+1]))]
            }
        }
    } else {
	# leaves parallel to y
        set lstart $cmval($id,9)
        set rmin [expr (-$cmval($id,0)-$ymin)*$yscale+$m]
        set rmax [expr ($cmval($id,0)-$ymin)*$yscale+$m]
        for {set i 1} {$i <= $nleaf($id)} {incr i} {
            set color [lindex $colorlist $med(leaves)]
            # now make rectangles for the top view of the leaf

            set a [expr ($neg($i)-$ymin)*$yscale+$m]
            if {$cmval($id,2,0)==2} {
               set b [expr (-$lstart-$xmin)*$xscale+$l]
            } else {
               set b [expr ($lstart-$xmin)*$xscale+$l]
            }
            set c [expr ($pos($i)-$ymin)*$yscale+$m]
            if {$cmval($id,2,0)==2} {
               set d [expr (-($lstart+$wtot($type($i)))-$xmin)*$xscale+$l]
            } else {
               set d [expr ($lstart+$wtot($type($i))-$xmin)*$xscale+$l]
            }
            $parent_w create rectangle $b $rmax $d $c -fill $color -outline black
            $parent_w create rectangle $b $a $d $rmin -fill $color -outline black
            if {$i<$nleaf($id)} {
              if {[expr $cmval($id,10)]>$wt($type([expr $i+1]))} {
                  #airgap is > tongue width
                  #draw the air space
                  set color [lindex $colorlist $med(in)]
                  set a [expr ($neg($i)-$ymin)*$yscale+$m]
                  if {$cmval($id,2,0)==2} {
                     set b [expr (-$lstart-$xmin)*$xscale+$l]
                  } else {
                     set b [expr ($lstart-$xmin)*$xscale+$l]
                  }
                  set c [expr ($pos($i)-$ymin)*$yscale+$m]
                  if {$cmval($id,2,0)==2} {
                     set d [expr (-($lstart+$wtot($type($i))+$cmval($id,10) \
                               -$wt($type([expr $i+1])))-$xmin)*$xscale+$l]
                  } else {
                     set d [expr ($lstart+$wtot($type($i))+$cmval($id,10) \
                               -$wt($type([expr $i+1]))-$xmin)*$xscale+$l]
                  }
                  $parent_w create rectangle $b $rmax $d $c -fill $color -outline {}
                  $parent_w create rectangle $b $a $d $rmin -fill $color -outline {}
              }
              set lstart [expr $lstart+$wtot($type($i))+$cmval($id,10) \
                               -$wt($type([expr $i+1]))]
            }
        }
    }
}

proc add_SYNCHDMLC_ends {id xscale zscale xmin zmin zmax l m parent_w} {
    global cmval colorlist medium nmed meds_used nleaf colornum sync_file nfull nqtr nhlf

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
    set med(screws) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
        if [string compare $cmval($id,20) $medium($j)]==0 {
            set med(screws) [min_nrc $j $colornum]
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

    # set up an array of leaf types
    set k 0; #leaf k
    for {set i 1} {$i<=$cmval($id,2,1)} {incr i} {
        for {set j 1} {$j<=$cmval($id,8,0,$i)} {incr j} {
          incr k
          if {$cmval($id,8,1,$i)==1} {
             set type($k) 1
          } elseif {$cmval($id,8,1,$i)==2} {
             if {$k > 1 && ($type([expr $k-1])==4 || $type([expr $k-1])==2)} {
               set type($k) 3
             } else {
               set type($k) 2
             }
          } elseif {$cmval($id,8,1,$i)==3} {
	     if {$k > 1 && ($type([expr $k-1])==4 || $type([expr $k-1])==2)} {
               set type($k) 5
	     } else {
               set type($k) 4
             }
          }
        }
    }

    # set up an array of neg and pos for each leaf
    if {$cmval($id,2,2)==1 | $cmval($id,2,2)==2} {
      set fileid [open $sync_file($id) "r"]
      #first three lines are title, no. of fields, muindex of first field
      gets $fileid data
      gets $fileid data
      gets $fileid data
      set leafnum 0; set groupnum 0;
      while {$leafnum<$nleaf($id)} {
            incr groupnum
            gets $fileid data
            for {set i 0} {$i<3} {incr i} {
                set data [get_val $data cmval $id,14,$i,$groupnum]
            }
            if { $cmval($id,14,2,$groupnum)<=0 } {
                set cmval($id,14,2,$groupnum) 1
            }
            incr leafnum $cmval($id,14,2,$groupnum)
       }
    }
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
    #now flip leaf opening sequence if ORIENT=2 or 3
    if {$cmval($id,2,0) == 2 || $cmval($id,2,0) == 3} {
        for {set i 1} {$i <= $nleaf($id)/2} {incr i} {
          set tmp_coord $neg($i)
          set neg($i) $neg([expr $nleaf($id)-$i+1])
          set neg([expr $nleaf($id)-$i+1]) $tmp_coord
          set tmp_coord $pos($i)
          set pos($i) $pos([expr $nleaf($id)-$i+1])
          set pos([expr $nleaf($id)-$i+1]) $tmp_coord
        }
    }

    #set up an array of widths and Z positions for the leaf types
    #FULL leaf
  if {$nfull > 0 } {
    set wtot(1) [expr $cmval($id,5,0)+$cmval($id,5,1)]
    set wt(1) $cmval($id,5,1)
    set numzf(1) 5
    set zf(1,1) $cmval($id,5,6)
    set zf(1,2) $cmval($id,5,8)
    set zf(1,3) $cmval($id,5,13)
    set zf(1,4) $cmval($id,5,14)
    set zf(1,5) [expr $cmval($id,3)+$cmval($id,4)]
    set numzb(1) 2
    set zb(1,1) $cmval($id,5,7)
    set zb(1,2) $cmval($id,5,9)
    set holetop(1) $cmval($id,5,10)
    set holebot(1) $cmval($id,5,11)
    set holepos(1) $cmval($id,5,12)
  }
  if {$nhlf > 0} {
    #TARGET HALF leaf
    set wtot(2) [expr $cmval($id,6,0)+$cmval($id,6,1)]
    set wt(2) $cmval($id,6,1)
    set numzf(2) 4
    set zf(2,1) $cmval($id,3)
    set zf(2,2) $cmval($id,6,11)
    set zf(2,3) $cmval($id,6,13)
    set zf(2,4) $cmval($id,6,14)
    set numzb(2) 3
    set zb(2,1) $cmval($id,6,6)
    set zb(2,2) $cmval($id,6,7)
    set zb(2,3) $cmval($id,6,12)
    set holetop(2) $cmval($id,6,8)
    set holebot(2) $cmval($id,6,9)
    set holepos(2) $cmval($id,6,10)
    #ISOCENTER HALF leaf
    set wtot(3) [expr $cmval($id,7,0)+$cmval($id,7,1)]
    set wt(3) $cmval($id,7,1)
    set numzf(3) 5
    set zf(3,1) $cmval($id,7,6)
    set zf(3,2) $cmval($id,7,8)
    set zf(3,3) $cmval($id,7,13)
    set zf(3,4) $cmval($id,7,14)
    set zf(3,5) [expr $cmval($id,3)+$cmval($id,4)]
    set numzb(3) 2
    set zb(3,1) $cmval($id,7,7)
    set zb(3,2) $cmval($id,7,9)
    set holetop(3) $cmval($id,7,10)
    set holebot(3) $cmval($id,7,11)
    set holepos(3) $cmval($id,7,12)
 }
 if {$nqtr > 0} {
    #TARGET QUARTER leaf
    set wtot(4) [expr $cmval($id,30,0)+$cmval($id,30,1)]
    set wt(4) $cmval($id,30,1)
    set numzf(4) 4
    set zf(4,1) $cmval($id,3)
    set zf(4,2) $cmval($id,30,11)
    set zf(4,3) $cmval($id,30,13)
    set zf(4,4) $cmval($id,30,14)
    set numzb(4) 3
    set zb(4,1) $cmval($id,30,6)
    set zb(4,2) $cmval($id,30,7)
    set zb(4,3) $cmval($id,30,12)
    set holetop(4) $cmval($id,30,8)
    set holebot(4) $cmval($id,30,9)
    set holepos(4) $cmval($id,30,10)
    #ISOCENTER QUARTER leaf
    set wtot(5) [expr $cmval($id,31,0)+$cmval($id,31,1)]
    set wt(5) $cmval($id,31,1)
    set numzf(5) 5
    set zf(5,1) $cmval($id,31,6)
    set zf(5,2) $cmval($id,31,8)
    set zf(5,3) $cmval($id,31,13)
    set zf(5,4) $cmval($id,31,14)
    set zf(5,5) [expr $cmval($id,3)+$cmval($id,4)]
    set numzb(5) 2
    set zb(5,1) $cmval($id,31,7)
    set zb(5,2) $cmval($id,31,9)
    set holetop(5) $cmval($id,31,10)
    set holebot(5) $cmval($id,31,11)
    set holepos(5) $cmval($id,31,12)
  }
    #now find out which leaves are intersected by Y=0 (if leaves || to Y)
    #or X=0 (if leaves || to X)
    #if only part of a leaf intersects 0, show the entire leaf
    set ml 0
    set lstart $cmval($id,9)
    for {set i 1} {$i <= $nleaf($id)} {incr i} {
        if {$lstart<=0 && [expr $lstart+$wtot($type($i))]>=0} {
           set ml $i
           break
        }
        if {$i<$nleaf($id)} {
           set lstart [expr $lstart+$wtot($type($i))+$cmval($id,10)- \
                    $wt($type([expr $i+1]))]
        }
    }

    if {$ml>0} {
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
        set alp1 [expr asin(($z0leaf-$zf($type($ml),1))/$r0leaf)]
        set alp2 \
              [expr asin(($z0leaf-$zf($type($ml),$numzf($type($ml))))/$r0leaf)]
        set alp1d [expr $alp1*180/3.1415927]
        set alp2d [expr $alp2*180/3.1415927]
        $parent_w create arc $a $c $b $d -start $alp1d \
            -extent [expr $alp2d-$alp1d]\
	    -fill $color -style pieslice -outline {}
        $parent_w create arc $a $c $b $d -start $alp1d \
            -extent [expr $alp2d-$alp1d]\
            -fill $color -style arc -outline black

        # 2: positive bounding box
        set a [expr ($pos($ml)-$xmin)*$xscale+$l]
        set b [expr ($pos($ml)+2*$r0leaf-$xmin)*$xscale+$l]
        $parent_w create arc $a $c $b $d -start [expr 180-$alp1d]\
	  -extent [expr $alp1d-$alp2d] -fill $color -style pieslice -outline {}
        $parent_w create arc $a $c $b $d -start [expr 180-$alp1d]\
          -extent [expr $alp1d-$alp2d] -fill $color -style arc -outline black

        # 3: polygons to fill in the rest
        set a [expr ($neg($ml)-$r0leaf+$r0leaf*cos($alp2)-$xmin)*$xscale+$l]
        set b [expr ($neg($ml)-$r0leaf+$r0leaf*cos($alp1)-$xmin)*$xscale+$l]
        set c [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set d [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        set e [expr ($zf($type($ml),1)-$zmin)*$zscale+$m]
        set f [expr ($zf($type($ml),$numzf($type($ml)))-$zmin)*$zscale+$m]
        # negative
        $parent_w create poly $a $f $b $e $c $e $c $f -fill $color -outline {}
        $parent_w create line $c $e $b $e
        $parent_w create line $c $f $a $f
        # positive
        set a [expr ($pos($ml)+$r0leaf-$r0leaf*cos($alp2)-$xmin)*$xscale+$l]
        set b [expr ($pos($ml)+$r0leaf-$r0leaf*cos($alp1)-$xmin)*$xscale+$l]
        $parent_w create poly $a $f $b $e $d $e $d $f -fill $color -outline {}
        $parent_w create line $b $e $d $e
        $parent_w create line $a $f $d $f

        #4: now, draw solid lines to show structures visible from -ve side
#        for {set i 2} {$i<=[expr $numzf($type($ml))-1]} {incr i} {
#           set a [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
#           set b [expr ($cmval($id,0)-$xmin)*$xscale+$l]
#           set c [expr ($neg($ml)-$r0leaf+sqrt(pow($r0leaf,2)-\
#                  pow($z0leaf-$zf($type($ml),$i),2))-$xmin)*$xscale+$l]
#           set d [expr ($pos($ml)+$r0leaf-sqrt(pow($r0leaf,2)-\
#                  pow($z0leaf-$zf($type($ml),$i),2))-$xmin)*$xscale+$l]
#           set e [expr ($zf($type($ml),$i)-$zmin)*$zscale+$m]
           #negative
#           $parent_w create line $a $e $c $e
           #positive
#           $parent_w create line $d $e $b $e
#        }

        #5: draw gray lines to show structures on +ve side
        #take this out for now because it's confusing
#        for {set i 1} {$i<=$numzb($type($ml))} {incr i} {
#           set a [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
#           set b [expr ($cmval($id,0)-$xmin)*$xscale+$l]
#           set c [expr ($neg($ml)-$r0leaf+sqrt(pow($r0leaf,2)-\
#                  pow($z0leaf-$zb($type($ml),$i),2))-$xmin)*$xscale+$l]
#           set d [expr ($pos($ml)+$r0leaf-sqrt(pow($r0leaf,2)-\
#                  pow($z0leaf-$zb($type($ml),$i),2))-$xmin)*$xscale+$l]
#           set e [expr ($zb($type($ml),$i)-$zmin)*$zscale+$m]
           #negative
#          $parent_w create line $a $e $c $e -fill gray
           #positive
#           $parent_w create line $d $e $b $e -fill gray
#        }
      } else { # straight, focused leaf ends, similar to MLC
        #use filled polygons for the leaves

        set ration [expr $neg($ml)/($cmval($id,3)-$cmval($id,12))]
        set ratiop [expr $pos($ml)/($cmval($id,3)-$cmval($id,12))]

        #negative side
        set y1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set z1 [expr ($zf($type($ml),1)-$zmin)*$zscale+$m]
        set y2 [expr ($ration*($zf($type($ml),1)-$cmval($id,12))-$xmin)*\
                $xscale+$l]
        set z2 $z1
        if {[expr $ration*($zf($type($ml),$numzf($type($ml)))-$cmval($id,12))]\
             <-$cmval($id,0)} {
          #lower edge of opening beyond -RMAX
          set y3 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
          set z3 [expr -$cmval($id,0)*($cmval($id,3)-$cmval($id,12))/$neg($ml)]
          set z3 [expr $z3+$cmval($id,12)]
          set z3 [expr ($z3-$zmin)*$zscale+$m]
          #draw a triangle
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 -fill $color \
                                   -outline black
        } else {
          #lower edge of opening > -RMAX
          set y3 [expr ($ration*($zf($type($ml),$numzf($type($ml)))-$cmval($id,12))\
                  -$xmin)*$xscale+$l]
          set z3 [expr ($zf($type($ml),$numzf($type($ml)))-$zmin)*$zscale+$m]
          set y4 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
          set z4 $z3
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 $y4 $z4 -fill $color\
                                   -outline black
        }
        #draw solid black lines to indicate structure visible from -ve side
#        for {set i 2} {$i<=[expr $numzf($type($ml))-1]} {incr i} {
#          if {[expr $ration*($zf($type($ml),$i)-$cmval($id,12))]\
#             >-$cmval($id,0)} {
#            set y1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
#            set z1 [expr ($zf($type($ml),$i)-$zmin)*$zscale+$m]
#            set y2 [expr $ration*($zf($type($ml),$i)-$cmval($id,12))]
#            set y2 [expr ($y2-$xmin)*$xscale+$l]
#            set z2 $z1
#            $parent_w create line $y1 $z1 $y2 $z2
#          }
#        }
        #draw gray lines to indicate structure visible from +ve side
#        for {set i 1} {$i<=$numzb($type($ml))} {incr i} {
#          if {[expr $ration*($zb($type($ml),$i)-$cmval($id,12))]\
#             >-$cmval($id,0)} {
#            set y1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
#            set z1 [expr ($zb($type($ml),$i)-$zmin)*$zscale+$m]
#            set y2 [expr $ration*($zb($type($ml),$i)-$cmval($id,12))]
#            set y2 [expr ($y2-$xmin)*$xscale+$l]
#            set z2 $z1
#            $parent_w create line $y1 $z1 $y2 $z2 -fill gray
#          }
#        }

        #positive side
        set y1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        set z1 [expr ($zf($type($ml),1)-$zmin)*$zscale+$m]
        set y2 [expr ($ratiop*($zf($type($ml),1)-$cmval($id,12))-$xmin)*\
                $xscale+$l]
        set z2 $z1
        if {[expr $ratiop*($zf($type($ml),$numzf($type($ml)))-$cmval($id,12))]\
             >$cmval($id,0)} {
          #lower edge of opening beyond RMAX
          set y3 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
          set z3 [expr $cmval($id,0)*($cmval($id,3)-$cmval($id,12))/$pos($ml)]
          set z3 [expr $z3+$cmval($id,12)]
          set z3 [expr ($z3-$zmin)*$zscale+$m]
          #draw a triangle
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 -fill $color \
                                   -outline black
        } else {
          #lower edge of opening < RMAX
          set y3 [expr ($ratiop*($zf($type($ml),$numzf($type($ml)))-$cmval($id,12))\
                  -$xmin)*$xscale+$l]
          set z3 [expr ($zf($type($ml),$numzf($type($ml)))-$zmin)*$zscale+$m]
          set y4 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
          set z4 $z3
          $parent_w create polygon $y1 $z1 $y2 $z2 $y3 $z3 $y4 $z4 -fill $color\
                                   -outline black
        }
        #draw solid black lines to indicate structure visible from -ve side
#        for {set i 2} {$i<=[expr $numzf($type($ml))-1]} {incr i} {
#          if {[expr $ratiop*($zf($type($ml),$i)-$cmval($id,12))]\
#             <$cmval($id,0)} {
#            set y1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
#            set z1 [expr ($zf($type($ml),$i)-$zmin)*$zscale+$m]
#            set y2 [expr $ratiop*($zf($type($ml),$i)-$cmval($id,12))]
#            set y2 [expr ($y2-$xmin)*$xscale+$l]
#            set z2 $z1
#            $parent_w create line $y1 $z1 $y2 $z2
#          }
#        }
        #draw gray lines to indicate structure visible from +ve side
#        for {set i 1} {$i<=$numzb($type($ml))} {incr i} {
#          if {[expr $ratiop*($zb($type($ml),$i)-$cmval($id,12))]\
#             <$cmval($id,0)} {
#            set y1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
#            set z1 [expr ($zb($type($ml),$i)-$zmin)*$zscale+$m]
#            set y2 [expr $ratiop*($zb($type($ml),$i)-$cmval($id,12))]
#            set y2 [expr ($y2-$xmin)*$xscale+$l]
#            set z2 $z1
#            $parent_w create line $y1 $z1 $y2 $z2 -fill gray
#          }
#        }
      }
      #now, draw rectangles for driving screw holes
      #does not depend on end type
      set color [lindex $colorlist $med(screws)]

      #negative side first
      if {[expr $neg($ml)-$holepos($type($ml))]>-$cmval($id,0)} {
        set z1 [expr ($holetop($type($ml))-$zmin)*$zscale+$m]
        set y1 [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
        set z2 [expr ($holebot($type($ml))-$zmin)*$zscale+$m]
        set y2 [expr ($neg($ml)-$holepos($type($ml))-$xmin)*$xscale+$l]
        $parent_w create rectangle $y1 $z1 $y2 $z2 -fill $color -outline black
      }
      #positive side
      if {[expr $pos($ml)+$holepos($type($ml))]<$cmval($id,0)} {
        set z1 [expr ($holetop($type($ml))-$zmin)*$zscale+$m]
        set y1 [expr ($cmval($id,0)-$xmin)*$xscale+$l]
        set z2 [expr ($holebot($type($ml))-$zmin)*$zscale+$m]
        set y2 [expr ($pos($ml)+$holepos($type($ml))-$xmin)*$xscale+$l]
        $parent_w create rectangle $y1 $z1 $y2 $z2 -fill $color -outline black
      }
    }; # end of if $ml>0
}

proc add_SYNCHDMLC_sides {id yscale zscale ymin zmin zmax l m parent_w} {
    global cmval colorlist medium nmed meds_used y z rmin rmax zf ztop nleaf
    global colornum sync_file nfull nhlf nqtr
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
    set med(screws) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
        if [string compare $cmval($id,20) $medium($j)]==0 {
            set med(screws) [min_nrc $j $colornum]
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

    #set up an array of -ve and +ve opening coordinates
    if {$cmval($id,2,2)==1 | $cmval($id,2,2)==2} {
      set fileid [open $sync_file($id) "r"]
      #first three lines are title, no. of fields, muindex of first field
      gets $fileid data
      gets $fileid data
      gets $fileid data
      set leafnum 0; set groupnum 0;
      while {$leafnum<$nleaf($id)} {
            incr groupnum
            gets $fileid data
            for {set i 0} {$i<3} {incr i} {
                set data [get_val $data cmval $id,14,$i,$groupnum]
            }
            if { $cmval($id,14,2,$groupnum)<=0 } {
                set cmval($id,14,2,$groupnum) 1
            }
            incr leafnum $cmval($id,14,2,$groupnum)
       }
    }
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
    #now flip leaf opening sequence if ORIENT=2 or 3
    if {$cmval($id,2,0) == 2 || $cmval($id,2,0) == 3} {
        for {set i 1} {$i <= $nleaf($id)/2} {incr i} {
          set tmp_coord $neg($i)
          set neg($i) $neg([expr $nleaf($id)-$i+1])
          set neg([expr $nleaf($id)-$i+1]) $tmp_coord
          set tmp_coord $pos($i)
          set pos($i) $pos([expr $nleaf($id)-$i+1])
          set pos([expr $nleaf($id)-$i+1]) $tmp_coord
        }
    }

    # set up an array of leaf types
        set k 0; #leaf k
    for {set i 1} {$i<=$cmval($id,2,1)} {incr i} {
        for {set j 1} {$j<=$cmval($id,8,0,$i)} {incr j} {
          incr k
          if {$cmval($id,8,1,$i)==1} {
             set type($k) 1
          } elseif {$cmval($id,8,1,$i)==2} {
             if {$k > 1 && ($type([expr $k-1])==4 || $type([expr $k-1])==2)} {
               set type($k) 3
             } else {
               set type($k) 2
             }
          } elseif {$cmval($id,8,1,$i)==3} {
             if {$k > 1 && ($type([expr $k-1])==4 || $type([expr $k-1])==2)} {
               set type($k) 5
             } else {
               set type($k) 4
             }
          }
        }
    }


    #for {set i 1} {$i<=$cmval($id,2,1)} {incr i} {
    #	for {set j 1} {$j<=$cmval($id,8,0,$i)} {incr j} {
    #      incr k
    #      if {$cmval($id,8,1,$i)==1} {
    #         set type($k) 1
    #      } elseif {$cmval($id,8,1,$i)==2} {
    #         if {[expr fmod($j,2)] != 0} {
    #           set type($k) 2
    #         } else {
    #           set type($k) 3
    #         }
    #      }
    #    }
    #}

    #leaf gap width at ztop
    set lgw $cmval($id,10)
    #z focus of leaf sides
    set zf $cmval($id,13)
    #zmin
    set ztop $cmval($id,3)
    #zmax
    set zbot [expr $ztop+$cmval($id,4)]
    #left-most point of first leaf at zmin
    set ls $cmval($id,9)

    #set up arrays of y and z positions for the 5 leaf types
    #this will make it easier to draw them later
    #y positions are all relative to ls, the left-most edge of the
    #tongue (at zmin)

    #FULL leaf
  if {$nfull > 0 } {
    set wtot(1) [expr $cmval($id,5,1)+$cmval($id,5,0)]
    set wtongue(1) $cmval($id,5,1)
    #upper left corner of tip
    set zl(1,1) $cmval($id,5,6)
    set yl(1,1) 0
    #lower left corner of tongue
    set zl(1,2) $cmval($id,5,8)
    set yl(1,2) 0
    #lower right corner of tongue
    set zl(1,3) $cmval($id,5,8)
    set yl(1,3) $cmval($id,5,1)
    #upper left corner of support rail
    set zl(1,4) $cmval($id,5,13)
    set yl(1,4) $cmval($id,5,1)
    #upper right corner of support rail
    set zl(1,5) $cmval($id,5,13)
    set yl(1,5) [expr $cmval($id,5,1)+$cmval($id,5,4)]
    #lower right corner of support rail
    set zl(1,6) $cmval($id,5,14)
    set yl(1,6) [expr $cmval($id,5,1)+$cmval($id,5,4)]
    #lower left corner of support rail
    set zl(1,7) $cmval($id,5,14)
    set yl(1,7) [expr $cmval($id,5,1)+$cmval($id,5,4)-$cmval($id,5,5)]
    #lower left corner of leaf
    set zl(1,8) $zbot
    set yl(1,8) [expr $cmval($id,5,1)+$cmval($id,5,4)-$cmval($id,5,5)]
    #lower right corner of leaf
    set zl(1,9) $zbot
    set yl(1,9) [expr $cmval($id,5,1)+$cmval($id,5,0)]
    #lower right of groove
    set zl(1,10) $cmval($id,5,9)
    set yl(1,10) [expr $cmval($id,5,1)+$cmval($id,5,0)]
    #lower left of groove
    set zl(1,11) $cmval($id,5,9)
    set yl(1,11) [expr $yl(1,10)-$cmval($id,5,2)]
    #upper right of leaf
    set zl(1,12) $cmval($id,5,7)
    set yl(1,12) $yl(1,11)
    #lower right of tip
    set zl(1,13) $cmval($id,5,7)
    set yl(1,13) $cmval($id,5,3)
    #upper right of tip
    set zl(1,14) $cmval($id,5,6)
    set yl(1,14) $cmval($id,5,3)
    #upper left of screw hole
    set hz(1,1) $cmval($id,5,10)
    set hy(1,1) $cmval($id,5,1)
    #lower left of screw hole
    set hz(1,2) $cmval($id,5,11)
    set hy(1,2) $cmval($id,5,1)
    #lower right of screw hole
    set hz(1,3) $cmval($id,5,11)
    set hy(1,3) [expr $cmval($id,5,0)+$cmval($id,5,1)]
    #upper right of screw hole
    set hz(1,4) $cmval($id,5,10)
    set hy(1,4) [expr $cmval($id,5,0)+$cmval($id,5,1)]
  }
  if {$nhlf > 0} {
    #TARGET HALF leaf
    set wtot(2) [expr $cmval($id,6,1)+$cmval($id,6,0)]
    set wtongue(2) $cmval($id,6,1)
    #upper left corner of leaf
    set zl(2,1) $ztop
    set yl(2,1) 0
    #lower left corner of tongue
    set zl(2,2) $cmval($id,6,11)
    set yl(2,2) 0
    #lower right corner of tongue
    set zl(2,3) $cmval($id,6,11)
    set yl(2,3) $cmval($id,6,1)
    #lower left corner of leaf
    set zl(2,4) $cmval($id,6,13)
    set yl(2,4) $cmval($id,6,1)
    #upper left corner of tip
    set zl(2,5) $cmval($id,6,13)
    set yl(2,5) [expr $cmval($id,6,1)+$cmval($id,6,0)-$cmval($id,6,2)-\
                $cmval($id,6,3)]
    #lower left of tip
    set zl(2,6) $cmval($id,6,14)
    set yl(2,6) $yl(2,5)
    #lower right of tip
    set zl(2,7) $cmval($id,6,14)
    set yl(2,7) [expr $yl(2,5)+$cmval($id,6,3)]
    #upper left of groove
    set zl(2,8) $cmval($id,6,12)
    set yl(2,8) $yl(2,7)
    #upper right of groove
    set zl(2,9) $cmval($id,6,12)
    set yl(2,9) [expr $cmval($id,6,1)+$cmval($id,6,0)]
    #lower right of support rail
    set zl(2,10) $cmval($id,6,7)
    set yl(2,10) [expr $cmval($id,6,1)+$cmval($id,6,0)]
    #lower left of support rail
    set zl(2,11) $cmval($id,6,7)
    set yl(2,11) [expr $cmval($id,6,1)+$cmval($id,6,0)-$cmval($id,6,5)]
    #upper left of support rail
    set zl(2,12) $cmval($id,6,6)
    set yl(2,12) $yl(2,11)
    #upper right of support rail
    set zl(2,13) $cmval($id,6,6)
    set yl(2,13) [expr $yl(2,12)+$cmval($id,6,4)]
    #upper right of leaf
    set zl(2,14) $ztop
    set yl(2,14) $yl(2,13)
    #upper left of screw hole
    set hz(2,1) $cmval($id,6,8)
    set hy(2,1) 0
    #lower left of screw hole
    set hz(2,2) $cmval($id,6,9)
    set hy(2,2) 0
    #lower right of screw hole
    set hz(2,3) $cmval($id,6,9)
    set hy(2,3) [expr $cmval($id,6,0)+$cmval($id,6,1)]
    #upper right of screw hole
    set hz(2,4) $cmval($id,6,8)
    set hy(2,4) [expr $cmval($id,6,0)+$cmval($id,6,1)]

    #ISOCENTER HALF leaf
    set wtot(3) [expr $cmval($id,7,1)+$cmval($id,7,0)]
    set wtongue(3) $cmval($id,7,1)
    #upper left corner of tip
    set zl(3,1) $cmval($id,7,6)
    set yl(3,1) $cmval($id,7,1)
    #upper right corner of tongue
    set zl(3,2) $cmval($id,7,8)
    set yl(3,2) $cmval($id,7,1)
    #upper left corner of tongue
    set zl(3,3) $cmval($id,7,8)
    set yl(3,3) 0
    #upper left corner of support rail
    set zl(3,4) $cmval($id,7,13)
    set yl(3,4) 0
    #upper right corner of support rail
    set zl(3,5) $cmval($id,7,13)
    set yl(3,5) $cmval($id,7,4)
    #lower right corner of support rail
    set zl(3,6) $cmval($id,7,14)
    set yl(3,6) $cmval($id,7,4)
    #lower left corner of support rail
    set zl(3,7) $cmval($id,7,14)
    set yl(3,7) [expr $cmval($id,7,4)-$cmval($id,7,5)]
    #lower left corner of leaf
    set zl(3,8) $zbot
    set yl(3,8) $yl(3,7)
    #lower right corner of leaf
    set zl(3,9) $zbot
    set yl(3,9) [expr $cmval($id,7,1)+$cmval($id,7,0)]
    #lower right of groove
    set zl(3,10) $cmval($id,7,9)
    set yl(3,10) [expr $cmval($id,7,1)+$cmval($id,7,0)]
    #lower left of groove
    set zl(3,11) $cmval($id,7,9)
    set yl(3,11) [expr $yl(3,10)-$cmval($id,7,2)]
    #upper right of leaf
    set zl(3,12) $cmval($id,7,7)
    set yl(3,12) $yl(3,11)
    #lower right of tip
    set zl(3,13) $cmval($id,7,7)
    set yl(3,13) [expr $cmval($id,7,1)+$cmval($id,7,3)]
    #upper right of tip
    set zl(3,14) $cmval($id,7,6)
    set yl(3,14) $yl(3,13)
    #upper left of screw hole
    set hz(3,1) $cmval($id,7,10)
    set hy(3,1) 0
    #lower left of screw hole
    set hz(3,2) $cmval($id,7,11)
    set hy(3,2) 0
    #lower right of screw hole
    set hz(3,3) $cmval($id,7,11)
    set hy(3,3) [expr $cmval($id,7,0)+$cmval($id,7,1)]
    #upper right of screw hole
    set hz(3,4) $cmval($id,7,10)
    set hy(3,4) [expr $cmval($id,7,0)+$cmval($id,7,1)]
 }
 if {$nqtr > 0} {
    #TARGET QUARTER leaf
    set wtot(4) [expr $cmval($id,30,1)+$cmval($id,30,0)]
    set wtongue(4) $cmval($id,30,1)
    #upper left corner of leaf
    set zl(4,1) $ztop
    set yl(4,1) 0
    #lower left corner of tongue
    set zl(4,2) $cmval($id,30,11)
    set yl(4,2) 0
    #lower right corner of tongue
    set zl(4,3) $cmval($id,30,11)
    set yl(4,3) $cmval($id,30,1)
    #lower left corner of leaf
    set zl(4,4) $cmval($id,30,13)
    set yl(4,4) $cmval($id,30,1)
    #upper left corner of tip
    set zl(4,5) $cmval($id,30,13)
    set yl(4,5) [expr $cmval($id,30,1)+$cmval($id,30,0)-$cmval($id,30,2)-\
                $cmval($id,30,3)]
    #lower left of tip
    set zl(4,6) $cmval($id,30,14)
    set yl(4,6) $yl(4,5)
    #lower right of tip
    set zl(4,7) $cmval($id,30,14)
    set yl(4,7) [expr $yl(4,5)+$cmval($id,30,3)]
    #upper left of groove
    set zl(4,8) $cmval($id,30,12)
    set yl(4,8) $yl(4,7)
    #upper right of groove
    set zl(4,9) $cmval($id,30,12)
    set yl(4,9) [expr $cmval($id,30,1)+$cmval($id,30,0)]
    #lower right of support rail
    set zl(4,10) $cmval($id,30,7)
    set yl(4,10) [expr $cmval($id,30,1)+$cmval($id,30,0)]
    #lower left of support rail
    set zl(4,11) $cmval($id,30,7)
    set yl(4,11) [expr $cmval($id,30,1)+$cmval($id,30,0)-$cmval($id,30,5)]
    #upper left of support rail
    set zl(4,12) $cmval($id,30,6)
    set yl(4,12) $yl(4,11)
    #upper right of support rail
    set zl(4,13) $cmval($id,30,6)
    set yl(4,13) [expr $yl(4,12)+$cmval($id,30,4)]
    #upper right of leaf
    set zl(4,14) $ztop
    set yl(4,14) $yl(4,13)
    #upper left of screw hole
    set hz(4,1) $cmval($id,30,8)
    set hy(4,1) 0
    #lower left of screw hole
    set hz(4,2) $cmval($id,30,9)
    set hy(4,2) 0
    #lower right of screw hole
    set hz(4,3) $cmval($id,30,9)
    set hy(4,3) [expr $cmval($id,30,0)+$cmval($id,30,1)]
    #upper right of screw hole
    set hz(4,4) $cmval($id,30,8)
    set hy(4,4) [expr $cmval($id,30,0)+$cmval($id,30,1)]

    #ISOCENTER QUARTER leaf
    set wtot(5) [expr $cmval($id,31,1)+$cmval($id,31,0)]
    set wtongue(5) $cmval($id,31,1)
    #upper left corner of tip
    set zl(5,1) $cmval($id,31,6)
    set yl(5,1) $cmval($id,31,1)
    #upper right corner of tongue
    set zl(5,2) $cmval($id,31,8)
    set yl(5,2) $cmval($id,31,1)
    #upper left corner of tongue
    set zl(5,3) $cmval($id,31,8)
    set yl(5,3) 0
    #upper left corner of support rail
    set zl(5,4) $cmval($id,31,13)
    set yl(5,4) 0
    #upper right corner of support rail
    set zl(5,5) $cmval($id,31,13)
    set yl(5,5) $cmval($id,31,4)
    #lower right corner of support rail
    set zl(5,6) $cmval($id,31,14)
    set yl(5,6) $cmval($id,31,4)
    #lower left corner of support rail
    set zl(5,7) $cmval($id,31,14)
    set yl(5,7) [expr $cmval($id,31,4)-$cmval($id,31,5)]
    #lower left corner of leaf
    set zl(5,8) $zbot
    set yl(5,8) $yl(5,7)
    #lower right corner of leaf
    set zl(5,9) $zbot
    set yl(5,9) [expr $cmval($id,31,1)+$cmval($id,31,0)]
    #lower right of groove
    set zl(5,10) $cmval($id,31,9)
    set yl(5,10) [expr $cmval($id,31,1)+$cmval($id,31,0)]
    #lower left of groove
    set zl(5,11) $cmval($id,31,9)
    set yl(5,11) [expr $yl(5,10)-$cmval($id,31,2)]
    #upper right of leaf
    set zl(5,12) $cmval($id,31,7)
    set yl(5,12) $yl(5,11)
    #lower right of tip
    set zl(5,13) $cmval($id,31,7)
    set yl(5,13) [expr $cmval($id,31,1)+$cmval($id,31,3)]
    #upper right of tip
    set zl(5,14) $cmval($id,31,6)
    set yl(5,14) $yl(5,13)
    #upper left of screw hole
    set hz(5,1) $cmval($id,31,10)
    set hy(5,1) 0
    #lower left of screw hole
    set hz(5,2) $cmval($id,31,11)
    set hy(5,2) 0
    #lower right of screw hole
    set hz(5,3) $cmval($id,31,11)
    set hy(5,3) [expr $cmval($id,31,0)+$cmval($id,31,1)]
    #upper right of screw hole
    set hz(5,4) $cmval($id,31,10)
    set hy(5,4) [expr $cmval($id,31,0)+$cmval($id,31,1)]
  }
    set rmin -$cmval($id,0)
    set rmax $cmval($id,0)

    # see if X=0 or Y=0 intersects leaf
    # note that even if a portion of the end goes through 0, we will
    # show the entire cross section

    for {set i 1} {$i <= $nleaf($id)} {incr i} {

        set color [lindex $colorlist $med(leaves)]

	if { $pos($i)==$neg($i) || $pos($i)<=0 || $neg($i)>=0 } {

          #draw 14-point polygon
          for {set j 1} {$j<=14} {incr j} {

            set z($j) $zl($type($i),$j)
            set y($j) [expr ($ls+$yl($type($i),$j))*($z($j)-$zf)/($ztop-$zf)]
            set y($j) [max_nrc $rmin $y($j)]
            set y($j) [min_nrc $rmax $y($j)]

            #flip cross-sections around Z if ORIENT = 2 or 3 (hope this works!)
            if {$cmval($id,2,0)==2 || $cmval($id,2,0)==3} {
                set y($j) [expr -1*$y($j)]
            }

            # set y and z scale and relative position
            set y($j) [expr ($y($j)-$ymin)*$yscale+$l]
            set z($j) [expr ($z($j)-$zmin)*$zscale+$m]
          }

          #now draw the polygon
          $parent_w create polygon $y(1) $z(1) $y(2) $z(2) $y(3) $z(3) \
           $y(4) $z(4) $y(5) $z(5) $y(6) $z(6) $y(7) $z(7) $y(8) $z(8) \
           $y(9) $z(9) $y(10) $z(10) $y(11) $z(11) $y(12) $z(12) $y(13) $z(13)\
           $y(14) $z(14) -fill $color -outline black

          #draw the driving screw hole
          set color [lindex $colorlist $med(screws)]

          #draw 4-point polygon
          for {set j 1} {$j<=4} {incr j} {

            set z($j) $hz($type($i),$j)
            set y($j) [expr ($ls+$hy($type($i),$j))*($z($j)-$zf)/($ztop-$zf)]
            set y($j) [max_nrc $rmin $y($j)]
            set y($j) [min_nrc $rmax $y($j)]
            if {$cmval($id,2,0)==2 || $cmval($id,2,0)==3} {
                set y($j) [expr -1*$y($j)]
            }

            # set y and z scale and relative position
            set y($j) [expr ($y($j)-$ymin)*$yscale+$l]
            set z($j) [expr ($z($j)-$zmin)*$zscale+$m]
          }

          #draw the polygon
          $parent_w create polygon $y(1) $z(1) $y(2) $z(2) $y(3) $z(3) \
           $y(4) $z(4) -fill $color -outline black
        }
        #set left of next tongue
        if {$i<$nleaf($id)} {
          set ls [expr $ls+$wtot($type($i))+$lgw-$wtongue($type([expr $i+1]))]
        }
    }
}

proc set_mid_pt_dynmlc {ind ytop zhi zlo} {
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

proc help_ignoregaps_synchdmlc { w } {

# help procedure put here because it is only for SYNCHDMLC and VARMLC

    set text {
If this input is set on (IGNOREGAPS=1), then all air gaps (ie between\
leaves) and driving screw holes will be ignored\
when doing range rejection in the leaves as long as:

particle X position < min. X of leaf openings (not including leaf ends)\
or > max. X of leaf openings (not including leaf ends) if leaves are || to X

or:

particle Y position < min. Y of leaf openings (not including leaf ends)\
or > max. Y of leaf openings (not including leaf ends) if leaves are || to Y.

This option makes\
range rejection very efficient in the leaves and can speed up simulation time\
in SYNCHDMLC by a factor of 2, while preserving exact transport in \
the leaf ends.\
However, if you have significant air gaps between the leaves and/or you\
are concerned about the effects of the driving screw holes then it is\
recommended that you run with this option off (IGNOREGAPS=0; the default)\
to have exact transport throughout the entire multi-leaf collimator.}
    help_dialog $w.help "Help" $text info 0 OK
}

proc help_leaf_mfg_specs_synchdmlc { w } {

    set text {
Choose "default orientation" for leaf cross-sections and numbering as shown in\
the BEAMnrc manual for leaves parallel to Y (ORIENT=0) or X (ORIENT=1).  Choose\
"mfg orientation" to reflect leaves parallel to Y about the X-axis (ORIENT=2)\
or to reflect leaves parallel to X about the Y-axis (ORIENT=3).  The\
"mfg orientation" option effectively reflects leaf banks so that the MLC\
coordinate system accords with that used in the manufacturer (Varian)\
specifications for the 120MLC and HD120 multileaf collimators.\
Note that because the leaf banks have been reflected, leaf opening sequences\
are automaticaly "flipped," so that opening coordinates for leaf i are instead\
applied to leaf N-i+1, where N is the total no. of leaves.  See the BEAMnrc\
Manual for more details.}
    help_dialog $w.help "Help" $text info 0 OK
}

proc help_screwdist { w } {

# help procedure put here because it is only for SYNCHDMLC

    set text {
Driving screw holes (cross-section defined\
above) start this distance from the leaf end and extend to -RMAX (for -ve\
leaves) or RMAX (for +ve leaves) in the direction parallel to the leaf\
opening direction.  Distance from leaf end is defined at Z = ZMIN + ZTHICK/2\
(ie mid way through the leaf thickness) for leaves with cylindrical (rounded)\
ends and at Z = ZMIN for leaves with straight, focused ends.  Note that these\
values of Z are also where you specify the openings in the leaves.}
    help_dialog $w.help "Help" $text info 0 OK
}

proc help_fieldtype { w } {

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
of the leaf opening for field I.  NUM instructs SYNCHDMLC to apply the\
accompanying values of NEG and POS to NUM adjacent leaves. NUM defaults\
to 1.  NEG POS NUM inputs start with leaf no. 1.\
See the general SYNCHDMLC help for leaf numbering scheme.

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
