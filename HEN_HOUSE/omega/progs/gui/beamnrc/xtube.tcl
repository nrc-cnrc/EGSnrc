#**************************************************************************
#
# $Id: xtube.tcl,v 1.4 2008/02/08 22:32:45 bwalters Exp $
#
# Started from Revision 1.5 of
#      /usr/people/omega/progs/gui/beamnrc/RCS/xtube.tcl,v
# last edited 2002-09-27 11:08:34-04
#
#*************************************************************************
#                                                                        #
#         Copyright 1999 by the National Research Council                #
#         of Canada, all rights reserved.  This code is part             #
#         of the BEAM code system for Monte carlo simulation             #
#         of radiotherapy treatment units. It was developed              #
#         at the National Research Council of Canada as part             #
#         of the OMEGA collaborative project with the University         #
#         of Wisconsin.  The system is described in:                     #
#           BEAM: A Monte Carlo code to simulate radiotherapy            #
#           treatment units                                              #
#           D.W.O. Rogers, B.A. Faddegon, G.X. Ding, C.-M. Ma,           #
#           J. Wei and T.R.Mackie                                        #
#           Medical Physics 22 (1995) 503 -- 524.                        #
#         in the                                                         #
#           BEAM Users Manual                                   #
#           D.W.O. Rogers, C.-M. Ma, B. Walters, G.X. Ding,              #
#                                 D.  Sheikh-Bagheri and G. Zhang        #
#           NRC Report PIRS-509A (rev D)                                 #
#         and in the many other NRC reports cited in the Users Manual    #
#                                                                        #
#         As well as the authors of the above paper and report           #
#         Joanne Treurniet of NRC made significant contributions to the  #
#         code system, in particular the GUIs and EGS_Windows            #
#         Mark Holmes, Brian Geiser and Paul Reckwerdt of Wiscosin       #
#         were important parts of the overall OMEGA project within which #
#         the BEAM code system was developed.                            #
#         The BEAM code system (excluding the EGS4 portions of it)       #
#         is copyright and subject to the BEAM General License           #
#         which is included in the distribution.                         #
#         The BEAM code system is provided as is without any warranty or #
#         guarantee of any kind, either expressed or implied,            #
#         as to its accuracy or ability to perform particular            #
#         calculations.                                                  #
#   Copyright National Research Council of Canada, Ottawa,
#   BEAM Code System 1999, all rights reserved
#   This code is covered by the BEAM General License
#

proc init_XTUBE { id } {
    global cmval

    for {set i 0} {$i<2} {incr i} {
	set cmval($id,$i) {}
    }
    set cmval($id,2,0) {}
    set cmval($id,2,1) {}
    for {set i 3} {$i<=4} {incr i} {
	set cmval($id,$i) {}
    }
    # I'm not sure 20 will do...
    for {set j 0} {$j<=20} {incr j} {
	set cmval($id,5,$j) {}
        set cmval($id,5,$j,1) 0
        set cmval($id,8,0,$j) {}
        set cmval($id,8,1,$j) {}
	for {set k 0} {$k<=3} {incr k} {
	    set cmval($id,6,$k,$j) {}
            set cmval($id,9,$k,$j) {}
	}
	set cmval($id,7,$j) {}
        set cmval($id,10,$j) {}
    }
    for {set k 0} {$k<=3} {incr k} {
	set cmval($id,8,$k) {}
    }
    for {set k 0} {$k<=3} {incr k} {
	set cmval($id,9,$k) {}
    }
    set cmval($id,10) {}
    set cmval($id,11) {}
}

proc read_XTUBE { fileid id } {
    global cmval GUI_DIR cm_ident

    # read a trash line
    gets $fileid data
    # if $data doesn't start with *, it's not a BEAM comment line and there's 
    # something wrong with the file.  Later, let it search for a line
    # beginning with * and start reading again from there.
    set data [string trimleft $data]
    if [string compare [string range $data 0 0] "*"]!=0 {
	tk_dialog .error "Read Error" "Error in the input file when starting\
		to read XTUBE $cm_ident($id).  The inputs don't begin where\
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

    # read zmin, zthick 2
    gets $fileid data
    set data [get_val $data cmval $id,2,0]
    set data [get_val $data cmval $id,2,1]

    # read angle 3
    gets $fileid data
    set data [get_val $data cmval $id,3]

    # read nxtube
    gets $fileid data
    set data [get_val $data cmval $id,4]

    # For each layer, in backward order, read thick, ecut etc., material
    for {set i $cmval($id,4)} {$i>=1} {set i [expr $i-1]} {
	gets $fileid data
	set data [get_val $data cmval $id,5,$i]
        if {$i==$cmval($id,4)} {
           set data [get_val $data cmval $id,5,$i,1]
        }
	gets $fileid data
	for {set j 0} {$j<=3} {incr j} {
	    set data [get_val $data cmval $id,6,$j,$i]
	}
	gets $fileid data
	set data [get_str $data cmval $id,7,$i]
    }
    set i $cmval($id,4)
    if {$cmval($id,5,$i,1)==1} {
       set j $cmval($id,4)
       gets $fileid data
       for {set k 0} {$k<=1} {incr k} {
          set data [get_val $data cmval $id,8,$k,$j]
       }
       gets $fileid data
       for {set k 0} {$k<=3} {incr k} {
          set data [get_val $data cmval $id,9,$k,$j]
       }
       gets $fileid data
       set data [get_str $data cmval $id,10,$j]
    }
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,8,$j]
    }
    gets $fileid data
    set data [get_str $data cmval $id,11]
    gets $fileid data
    for {set j 0} {$j<=3} {incr j} {
	set data [get_val $data cmval $id,9,$j]
    }
    gets $fileid data
    set data [get_str $data cmval $id,10]
}

proc edit_XTUBE { id zmax } {
    global cmval GUI_DIR helvfont values help_xtube_text

    catch { destroy .xtube$id }
    toplevel .xtube$id
    wm title .xtube$id "Edit XTUBE, CM\#$id"
    # MAIN WINDOW - PUT ON THE MAIN STUFF; A >> BUTTON BY NXTUBE.
    frame .xtube$id.frm -bd 4
    set top .xtube$id.frm

    frame $top.title
    label $top.title.lab -text "X-ray tube" -font $helvfont
    pack $top.title.lab 
    pack $top.title -pady 5

    #XTUBE_macros.mortran:REPLACE {$MAX_N_$XTUBE} WITH {10}
    get_1_default XTUBE $top "maximum number of layers"

    label $top.zmax -text $zmax -font $helvfont
    pack $top.zmax -pady 5

    add_rmax_square $top.inp0 $id,0
    add_title $top.inp1 $id,1
    add_zmin $top.inp2 $id,2,0

    frame $top.inp3 -bd 4
    label $top.inp3.label -text {Thickness of xtube in Z-direction}
    entry $top.inp3.inp -textvariable cmval($id,2,1) -width 8
    pack $top.inp3.label -anchor w -side left
    pack $top.inp3.inp -anchor e -side right -fill x -expand true
    pack $top.inp3 -side top -fill x

    frame $top.inp4 -bd 4
    label $top.inp4.label \
	    -text {Angle between target surface and Z-axis (degrees)}
    entry $top.inp4.inp -textvariable cmval($id,3) -width 8
    pack $top.inp4.label -anchor w -side left
    pack $top.inp4.inp -anchor e -side right -fill x -expand true
    pack $top.inp4 -side top -fill x

    frame $top.inp5 -bd 4
    label $top.inp5.label -text {Number of layers in the target}
    entry $top.inp5.inp -textvariable cmval($id,4) -width 8
    button $top.inp5.nextproc -text " Define layers >> "\
	    -command "define_xtube $id"
    pack $top.inp5.label -anchor w -side left
    pack $top.inp5.inp -side left -padx 10
    pack $top.inp5.nextproc -anchor e -side right -fill x -expand true
    pack $top.inp5 -side top -fill x

    frame $top.bottom -bd 5
    frame $top.bottom.left
    frame $top.bottom.right
    
    label $top.bottom.left.lab -text "Region in front of target"\
	    -font $helvfont
    pack $top.bottom.left.lab -side top 

    add_ecut $top.bottom.left.f1 $id,8,0
    add_pcut $top.bottom.left.f2 $id,8,1
    add_dose $top.bottom.left.f3 $id,8,2
    add_latch $top.bottom.left.f4 $id,8,3
    add_material $top.bottom.left.f5 $id,11
	    
    label $top.bottom.right.lab -text "Region behind target (target holder)"\
	    -font $helvfont
    pack $top.bottom.right.lab -side top 

    add_ecut $top.bottom.right.f1 $id,9,0
    add_pcut $top.bottom.right.f2 $id,9,1
    add_dose $top.bottom.right.f3 $id,9,2
    add_latch $top.bottom.right.f4 $id,9,3
    add_material $top.bottom.right.f5 $id,10

    pack $top.bottom.left -side left -anchor n -padx 5
    pack $top.bottom.right -side right -anchor n -padx 5

    pack $top.bottom -side top

    frame $top.buts -bd 4
    button $top.buts.helpb -text "Help" -command\
	    "help_gif .xtube$id.help {$help_xtube_text} help_xtube"\
	    -relief groove -bd 8
    button $top.buts.okb -text "OK" -command "destroy .xtube$id"\
	    -relief groove -bd 8
    button $top.buts.prev -text "Preview" -command "show_XTUBE $id"\
	    -relief groove -bd 8
    pack $top.buts.helpb $top.buts.okb $top.buts.prev -side left -padx 10
    pack $top.buts -pady 15
    pack $top
}

proc define_xtube { id } {
    global cmval helvfont values

    # allow a maximum of 4/window, the proceed to the next window.
    set ntube $cmval($id,4)
    set nwindow [expr $ntube/4]
    if [expr $ntube%4]>0 {
	incr nwindow
    }
    for {set i 1} {$i<=$nwindow} {incr i} {
	catch { destroy .xtube$id.baby$i }
	toplevel .xtube$id.baby$i
	set w .xtube$id.baby$i
	wm title $w "Define xtube, window $i"	
	frame $w.mainfrm
	set top $w.mainfrm
	for {set ii 1} {$ii<=4} {incr ii} {
	    set index [expr 4*($i-1)+$ii]
	    if $index>$ntube {break}

            if {$index==$ntube} {
              set topold $top
              set top $top.l1
              frame $top
            }
 
            frame $top.f$i-$ii -bd 4

	    # define the 6 values that go with each tube; but on a groove frame

            if {$index==$ntube} {
              label $top.lab -text "Layer $index (outermost layer)"\
                   -font $helvfont
              pack $top.lab -side top
            } else {
	      label $top.f$i-$ii.lab -text "Layer $index" -font $helvfont
              pack $top.f$i-$ii.lab -side top
            }

            if {$index==$ntube} {
              checkbutton $top.f$i-$ii.f6 -text "Define extra central region"\
                 -variable cmval($id,5,$index,1) \
 -command "enable_disable_xtra_inputs $id $index $i $ii"
              pack $top.f$i-$ii.f6 -side top -pady 5 -anchor w
            }

	    frame $top.f$i-$ii.f0
	    set w $top.f$i-$ii.f0
	    label $w.lab -text "Layer thickness (cm)"
	    pack $w.lab -anchor w -side left 
	    entry $w.ent -textvariable cmval($id,5,$index) -width 10
	    pack $w.ent -anchor e -side right -fill x -expand true 
	    pack $top.f$i-$ii.f0 -side top -fill x

	    add_ecut $top.f$i-$ii.f1 $id,6,0,$index
	    add_pcut $top.f$i-$ii.f2 $id,6,1,$index
	    add_dose $top.f$i-$ii.f3 $id,6,2,$index
	    add_latch $top.f$i-$ii.f4 $id,6,3,$index
	    add_material $top.f$i-$ii.f5 $id,7,$index
	    
            if {$index==$ntube} {
              #add option to have extra central region in outermost layer
              frame $top.f$i-$ii-2 -bd 4
              label $top.f$i-$ii-2.lab -text "For extra central region:"\
                  -font $helvfont
              pack $top.f$i-$ii-2.lab -side top

              frame $top.f$i-$ii-2.wh
              frame $top.f$i-$ii-2.wh.w
              label $top.f$i-$ii-2.wh.w.lab -text "Y width (cm)"
              pack $top.f$i-$ii-2.wh.w.lab -anchor w -side left
              entry $top.f$i-$ii-2.wh.w.ent -textvariable cmval($id,8,0,$index)
              pack $top.f$i-$ii-2.wh.w.ent -anchor e -side right -fill x \
                            -expand true
              frame $top.f$i-$ii-2.wh.h
              label $top.f$i-$ii-2.wh.h.lab -text "Z height (cm)"
              pack $top.f$i-$ii-2.wh.h.lab -anchor w -side left
              entry $top.f$i-$ii-2.wh.h.ent -textvariable cmval($id,8,1,$index)
              pack $top.f$i-$ii-2.wh.h.ent -anchor e -side right -fill x \
                            -expand true
              pack $top.f$i-$ii-2.wh.w $top.f$i-$ii-2.wh.h -side left
              pack $top.f$i-$ii-2.wh -side top

              add_ecut $top.f$i-$ii-2.f1 $id,9,0,$index
              add_pcut $top.f$i-$ii-2.f2 $id,9,1,$index
              add_dose $top.f$i-$ii-2.f3 $id,9,2,$index
              add_latch $top.f$i-$ii-2.f4 $id,9,3,$index
              add_material $top.f$i-$ii-2.f5 $id,10,$index
          
              pack $top.f$i-$ii $top.f$i-$ii-2 -side left -anchor s
              pack $top -side top

              enable_disable_xtra_inputs $id $index $i $ii

              set top $topold
            } else {
              pack $top.f$i-$ii -side top
            }
        
	    frame $top.sep$i-$ii -bd 4 -width 100 -height 2 -relief groove 
	    pack $top.sep$i-$ii -side top -fill x -pady 5
	}
	button .xtube$id.baby$i.okb -text "OK" -command "destroy .xtube$id.baby$i"\
		-relief groove -bd 8
	pack $top .xtube$id.baby$i.okb -pady 10
	if [expr 4*($i-1)+$ii]>$ntube {break}
    }
}

proc write_XTUBE {fileid id} {
    global cmval cm_names cm_ident cm_type

    puts $fileid "$cmval($id,0), RMAX"
    puts $fileid $cmval($id,1)
    puts $fileid "$cmval($id,2,0), $cmval($id,2,1), ZMIN, ZTHICK"
    puts $fileid "$cmval($id,3), ANGLE"
    puts $fileid "$cmval($id,4), # LAYERS"
    
    for {set i $cmval($id,4)} {$i>=1} {set i [expr $i-1]} {
        set str {}
        set str "$str$cmval($id,5,$i), "
        if {$i==$cmval($id,4)} {
          set str "$str$cmval($id,5,$i,1)"
        }
	puts $fileid $str
	set str {}
	set str "$str$cmval($id,6,0,$i), "
	set str "$str$cmval($id,6,1,$i), "
	set str "$str$cmval($id,6,2,$i), "
	set str "$str$cmval($id,6,3,$i), "
	puts $fileid $str
	puts $fileid $cmval($id,7,$i)
    }
    set i $cmval($id,4)
    if {$cmval($id,5,$i,1)==1} {
        set j $cmval($id,4)
        set str {}  
        set str "$str$cmval($id,8,0,$j), "
        set str "$str$cmval($id,8,1,$j), "
        puts $fileid $str
        set str {}
        set str "$str$cmval($id,9,0,$j), "
        set str "$str$cmval($id,9,1,$j), "
        set str "$str$cmval($id,9,2,$j), "
        set str "$str$cmval($id,9,3,$j), "
        puts $fileid $str
        puts $fileid $cmval($id,10,$j)
    } 
    set str {}
    set str "$str$cmval($id,8,0), "
    set str "$str$cmval($id,8,1), "
    set str "$str$cmval($id,8,2), "
    set str "$str$cmval($id,8,3), "
    puts $fileid $str
    puts $fileid $cmval($id,11)
    set str {}
    set str "$str$cmval($id,9,0), "
    set str "$str$cmval($id,9,1), "
    set str "$str$cmval($id,9,2), "
    set str "$str$cmval($id,9,3), "
    puts $fileid $str
    puts $fileid $cmval($id,10)
}


proc show_XTUBE { id } {
    global cmval xrange zrange cm_ticks
    
    catch { destroy .xtube$id.show }
    toplevel .xtube$id.show
    wm title .xtube$id.show "Preview"
    
    set cm_ticks($id,x) 6
    set cm_ticks($id,z) 10

    # have to make an initial xrange, zrange:
    catch {
	set xrange(0) -$cmval($id,0) 
	set xrange(1) $cmval($id,0) 
	set zrange(0) [get_zmax [expr $id-1]]
	set zrange(1) [get_zmax $id]
    }

    if [catch { draw_XTUBE $id }]==1 {
	destroy .xtube$id.show
	tk_dialog .xtube$id.error "Preview error" "There was an error in\
		attempting to display this CM.  Please make sure that\
		all numerical values are defined." error 0 OK
	return
    }

    frame .xtube$id.show.buts
    button .xtube$id.show.buts.range -text "Change plot properties" -command\
	    "change_cm_range $id .xtube$id.show xz" -relief groove -bd 8
    button .xtube$id.show.buts.print -text "Print..." -command\
	    "print_canvas .xtube$id.show.can 600 600" -relief groove -bd 8
    button .xtube$id.show.buts.done -text "Done" -command\
	    "destroy .xtube$id.show" -relief groove -bd 8
    pack .xtube$id.show.buts.range .xtube$id.show.buts.print\
	    .xtube$id.show.buts.done -side left -padx 10
    pack .xtube$id.show.buts -side bottom -anchor s -pady 15
}
    
proc draw_XTUBE { id } {
    global cmval zrange xrange xscale zscale helvfont l m cm_ticks

    catch { destroy .xtube$id.show.can }

    # put the canvas up
    set ncan 1
    set width 450
    canvas .xtube$id.show.can -width [expr $ncan*($width+150)]\
	    -height [expr $ncan*($width+150)]
    
    set xscale [expr $width/abs($xrange(1)-$xrange(0))]
    set zscale [expr $width/abs($zrange(1)-$zrange(0))]
    set l 100.0
    set m 50.0

    add_air $id .xtube$id.show.can $xrange(0) $zrange(0) $xscale $zscale $l $m

    add_XTUBE $id $xscale $zscale $xrange(0) $zrange(0)\
	    $l $m .xtube$id.show.can

    coverup $l $m $width .xtube$id.show.can

    set curx 0
    set cury 0
    label .xtube$id.show.can.xy -text [format "(%6.3f, %6.3f)" $curx $cury]\
	    -bg white -bd 5 -width 16 -font $helvfont
    .xtube$id.show.can create window [expr $l/2+$width] [expr 2*$m+$width]\
	    -window .xtube$id.show.can.xy
    bind .xtube$id.show.can <Motion> {
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
	    $xscale $zscale .xtube$id.show.can

    pack .xtube$id.show.can -side top -anchor n
}

proc add_XTUBE {id xscale zscale xmin zmin l m parent_w} {
    global colorlist medium nmed cmval meds_used values colornum

    # assign numbers to the holder, layers and air
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set med($i) $colornum 
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,7,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum] 
		set meds_used($j) 1
		break
	    }
	}
    }
    set med(holder) $colornum 
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,10) $medium($j)]==0 {
	    set med(holder) [min_nrc $j $colornum] 
	    set meds_used($j) 1
	    break
	}
    }
    set med(front) $colornum 
    for {set j 0} {$j<=$nmed} {incr j} {
	if { [string compare $cmval($id,11) $medium($j)]==0 } {
	    set med(front) [min_nrc $j $colornum] 
	    set meds_used($j) 1
	    break
	}
    }
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
        if { [string compare $values(2) $medium($j)]==0 } {
            set med(air) [min_nrc $j $colornum]
            set meds_used($j) 1
            break
        }
    }
    set i $cmval($id,4)
    if {$cmval($id,5,$i,1)==1} {
       set med(xtra) $colornum
       for {set j 0} {$j<=$nmed} {incr j} {
        if [string compare $cmval($id,10,$i) $medium($j)]==0 {
            set med(xtra) [min_nrc $j $colornum]
            set meds_used($j) 1
            break
        }
       }
    }

    # rectangle for holder
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set z(1) [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z(2) [expr ($cmval($id,2,0)+$cmval($id,2,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(holder)]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

    # polygons for layers

    set alpha [expr $cmval($id,3)*3.14159/180.]

    # find x-direction thickness of each layer
    set sumthick 0
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set thick($i) [expr $cmval($id,5,$i)/cos($alpha)]
	set sumthick [expr $sumthick+$thick($i)]
    }

    set xtop1 [expr $cmval($id,2,1)*tan($alpha)/2.0 - $sumthick]
    set xbot1 [expr -$cmval($id,2,1)*tan($alpha)/2.0 - $sumthick]
    set z(1) [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set z(2) [expr ($cmval($id,2,0)+$cmval($id,2,1)-$zmin)*$zscale+$m]

    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set xtop2 [expr $xtop1+$thick($i)]
	set xbot2 [expr $xbot1+$thick($i)]
	set x(1) [expr ($xtop1-$xmin)*$xscale+$l]
	set x(2) [expr ($xtop2-$xmin)*$xscale+$l]
	set x(3) [expr ($xbot1-$xmin)*$xscale+$l]
	set x(4) [expr ($xbot2-$xmin)*$xscale+$l]
	set color [lindex $colorlist $med($i)]
	$parent_w create poly $x(1) $z(1) $x(2) $z(1) $x(4) $z(2)\
		$x(3) $z(2) -fill $color -outline {}
	set xtop1 $xtop2
	set xbot1 $xbot2
        if {$i==$cmval($id,4) && $cmval($id,5,$i,1)==1} {
           set ztop_xtra [expr ($cmval($id,2,0)+($cmval($id,2,1)-$cmval($id,8,1,$i))/2.0-$zmin)*$zscale+$m]
           set zbot_xtra [expr ($cmval($id,2,0)+($cmval($id,2,1)+$cmval($id,8,1,$i))/2.0-$zmin)*$zscale+$m]
           set xtop2_xtra [expr $cmval($id,8,1,$i)*tan($alpha)/2.0]
           set xbot2_xtra -$xtop2_xtra
           set xtop1_xtra [expr $xtop2_xtra-$thick($i)]
           set xbot1_xtra [expr $xbot2_xtra-$thick($i)]
           set xtop2_xtra [expr ($xtop2_xtra-$xmin)*$xscale+$l]
           set xbot2_xtra [expr ($xbot2_xtra-$xmin)*$xscale+$l]
           set xtop1_xtra [expr ($xtop1_xtra-$xmin)*$xscale+$l]
           set xbot1_xtra [expr ($xbot1_xtra-$xmin)*$xscale+$l]
           set color [lindex $colorlist $med(xtra)]
           $parent_w create poly $xtop1_xtra $ztop_xtra $xtop2_xtra $ztop_xtra\
               $xbot2_xtra $zbot_xtra $xbot1_xtra $zbot_xtra -fill $color \
               -outline {}
        }
    }

    # polygon for front: xtop2, xbot2 are the starting points, z(1), z(2) are ok
    set x(1) [expr ($xtop2-$xmin)*$xscale+$l]
    set x(2) [expr ($xbot2-$xmin)*$xscale+$l]
    set x(3) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set color [lindex $colorlist $med(front)]
    $parent_w create poly $x(1) $z(1) $x(3) $z(1) $x(3) $z(2)\
	    $x(2) $z(2) -fill $color -outline {}
}

proc add_XTUBE_yz {id xscale zscale xmin zmin l m parent_w} {
    global colorlist medium nmed cmval meds_used values colornum

    # assign numbers to the holder, layers and air
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set med($i) $colornum 
	for {set j 0} {$j<=$nmed} {incr j} {
	    if [string compare $cmval($id,7,$i) $medium($j)]==0 {
		set med($i) [min_nrc $j $colornum] 
		set meds_used($j) 1
		break
	    }
	}
    }
    set med(holder) $colornum 
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,10) $medium($j)]==0 {
	    set med(holder) [min_nrc $j $colornum] 
	    set meds_used($j) 1
	    break
	}
    }
    set med(front) $colornum 
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $cmval($id,11) $medium($j)]==0 {
	    set med(front) [min_nrc $j $colornum] 
	    set meds_used($j) 1
	    break
	}
    }
    set med(air) $colornum
    for {set j 0} {$j<=$nmed} {incr j} {
        if { [string compare $values(2) $medium($j)]==0 } {
            set med(air) [min_nrc $j $colornum]
            set meds_used($j) 1
            break
        }
    }
    set i $cmval($id,4)
    if {$cmval($id,5,$i,1)==1} {
       set med(xtra) $colornum
       for {set j 0} {$j<=$nmed} {incr j} {
        if [string compare $cmval($id,10,$i) $medium($j)]==0 {
            set med(xtra) [min_nrc $j $colornum]
            set meds_used($j) 1
            break
        }
       }
    }

    set alpha [expr $cmval($id,3)*3.14159/180.]
    # rectangle for front, whole thing
    set x(1) [expr (-$cmval($id,0)-$xmin)*$xscale+$l]
    set x(2) [expr ($cmval($id,0)-$xmin)*$xscale+$l]
    set z(1) [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set z(2) [expr ($cmval($id,2,0)+$cmval($id,2,1)-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(front)]
    $parent_w create rect $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

    # find x-direction thickness of each layer, xi=di/cos(alpha)
    set sumthick 0.0
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set xi($i) [expr $cmval($id,5,$i)/cos($alpha)]
	set sumthick [expr $sumthick+$xi($i)]
    }

    set xmax [expr $cmval($id,2,1)*tan($alpha)/2.0]
    set x0 [expr $xmax - $sumthick]
    set z0 [expr $x0/tan($alpha)]
    # rectangle for holder
    set z(1) [expr ($cmval($id,2,0)-$zmin)*$zscale+$m]
    set z(2) [expr ($cmval($id,2,0)+$z0-$zmin)*$zscale+$m]
    set color [lindex $colorlist $med(holder)]
    $parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}

    # put in a rectangle for each layer
    # create an array of the z-coordinates starting after the holder
    set temp $x0
    set zstart(0) $z0 
    for {set i 1} {$i<=$cmval($id,4)} {incr i} {
	set zstart($i) [expr ($temp+$xi($i))/tan($alpha)]
	set temp [expr $temp+$xi($i)]
    }

    for {set i 0} {$i<$cmval($id,4)} {incr i} {
	set z(1) [expr ($zstart($i)+$cmval($id,2,0)-$zmin)*$zscale+$m]
	set z(2) [expr ($zstart([expr $i+1])+$cmval($id,2,0)-$zmin)*$zscale+$m]
	set color [lindex $colorlist $med([expr $i+1])]
	$parent_w create rectangle $x(1) $z(1) $x(2) $z(2) -fill $color -outline {}
        if {$i==[expr $cmval($id,4)-1] && $cmval($id,5,$cmval($id,4),1)==1} {
           set j $cmval($id,4)
           set ymin_xtra [expr (-$cmval($id,8,0,$j)-$xmin)*$xscale+$l]
           set ymax_xtra [expr ($cmval($id,8,0,$j)-$xmin)*$xscale+$l]
           set ztop_xtra [max_nrc [expr $cmval($id,2,0)+($cmval($id,2,1)-$cmval($id,8,1,$j))/2.0] [expr $zstart($i)+$cmval($id,2,0)]]
           set ztop_xtra [expr ($ztop_xtra-$zmin)*$zscale+$m]
           set zbot_xtra [expr ($cmval($id,2,0)+$cmval($id,2,1)/2.0-$zmin)*$zscale+$m]
           set color [lindex $colorlist $med(xtra)]
           $parent_w create rectangle $ymin_xtra $ztop_xtra $ymax_xtra \
              $zbot_xtra -fill $color -outline {}
        }
    }

}

proc enable_disable_xtra_inputs {id index i ii} {
global cmval
 if {$cmval($id,5,$index,1)==1} {
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.wh.w.lab configure -fg black
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.wh.w.ent configure -state normal
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.wh.h.lab configure -fg black
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.wh.h.ent configure -state normal 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f1.lab configure -fg black
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f1.ent configure -state normal
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f2.lab configure -fg black
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f2.ent configure -state normal
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f3.lab configure -fg black
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f3.ent configure -state normal
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f4.lab configure -fg black
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f4.ent configure -state normal
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f5.lab configure -fg black
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f5.ent configure -state normal
 } else {
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.wh.w.lab configure -fg grey 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.wh.w.ent configure -state disabled 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.wh.h.lab configure -fg grey 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.wh.h.ent configure -state disabled 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f1.lab configure -fg grey 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f1.ent configure -state disabled 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f2.lab configure -fg grey 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f2.ent configure -state disabled 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f3.lab configure -fg grey 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f3.ent configure -state disabled 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f4.lab configure -fg grey 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f4.ent configure -state disabled 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f5.lab configure -fg grey 
   .xtube$id.baby$i.mainfrm.l1.f$i-$ii-2.f5.ent configure -state disabled 
 }
}
