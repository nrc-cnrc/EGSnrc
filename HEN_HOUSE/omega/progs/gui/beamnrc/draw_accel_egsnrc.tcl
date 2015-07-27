
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: draw accelerator
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


proc draw_accel {} {
    global num_cm cmval helvfont  cm_names cm_type values
    global xzoom yzoom zzoom xrange_acc yrange_acc zrange_acc smhelvfont view
    global colorlist nmed medium meds_used scrlwidth scrlheight colornum

    # first we have to determine what the limits of the accelerator
    # are in x and in z, same as in create_accel.tcl because if it can't
    # get these values it can't draw it.

    if [catch {expr $zrange_acc(0)}]==1 {
	set zrange_acc(0) $values(24)
    }
    if [catch {expr $zrange_acc(1)}]==1 {
	set zrange_acc(1) [get_zmax $num_cm]
	if [string compare $zrange_acc(1)  ""]==0 {
	    tk_dialog .error "Z Axis Undefined" "The end of the last CM has\
		    to be defined in order to determine the scale of the preview.  \
		    Please completely define the last CM to continue." error 0 OK
	    return
	}
    }
    set xrange_acc(1) 0.0
    for {set id 1} {$id<=$num_cm} {incr id} {
	set xmax $cmval($id,0)
	if $xmax>$xrange_acc(1) {
	    set xrange_acc(1) $xmax
	}
    }
    set xrange_acc(0) [expr -$xrange_acc(1)]

    set yrange_acc(1) 0.0
    for {set id 1} {$id<=$num_cm} {incr id} {
	set ymax $cmval($id,0)
	if $ymax>$yrange_acc(1) {
	    set yrange_acc(1) $ymax
	}
    }
    set yrange_acc(0) [expr -$yrange_acc(1)]

    # the zoom is how many pixels represent 1 unit of length/20
    set zzoom 1
    # make the accelerator show in the space provided.
    set yzoom [expr (1024/2.0)/(($yrange_acc(1)-$yrange_acc(0))*20.)]
    set xzoom [expr (1024/2.0)/(($xrange_acc(1)-$xrange_acc(0))*20.)]

    # the user can select a xz view of a yz view with a menubutton; set default
    if [catch {expr $view}]==1 {
	set view xz
    }

    catch {destroy .accel}
    toplevel .accel
    wm title .accel "Accelerator Preview"
    wm geometry .accel +10+10

    frame .accel.topw -bd 4

    frame .accel.topw.frm

    scrollbar .accel.topw.frm.scrlz -command ".accel.topw.frm.can yview"
    scrollbar .accel.topw.frm.scrlx -command ".accel.topw.frm.can xview" -orient horizontal

    pack .accel.topw.frm.scrlz -side right -fill y
    pack .accel.topw.frm.scrlx -side bottom -fill x

    redraw_accel

    pack .accel.topw.frm -side right -anchor n

    # left frame will hold a legend of the colours used and the menubutton
    # for xz or yz view
    frame .accel.topw.leg

    label .accel.topw.leg.lab -text "LEGEND" -font $helvfont
    pack .accel.topw.leg.lab -pady 5

    message .accel.topw.leg.mess -text "Click on the colour swatch\
	    to change the colour of a material.  If a material shows\
	    as black on the preview, \
	    the material is either not available in the PEGS4 data file\
            selected or there are not enough colours in the default colour\
            list to give it a colour." \
	    -font $helvfont -width 200
    pack .accel.topw.leg.mess -pady 5 -anchor w

    # create a frame to hold a grid
    frame .accel.topw.leg.frm
    set w .accel.topw.leg.frm
    set j 0
    for {set i 0} {$i<=$nmed} {incr i} {
	if $meds_used($i)==1 {
	    incr j
	    canvas $w.c$i -width 10 -height 10 -bg [lindex $colorlist [min_nrc $i $colornum]]
	    bind $w.c$i <Button-1> "change_color $w.c$i $i;\
		    tkwait window $w.c$i.select; redraw_accel"
	    label $w.l$i -text $medium($i) -font $smhelvfont
	    grid config $w.c$i -row $j -column 0
	    grid config $w.l$i -row $j -column 1 -sticky w
	}
    }
    pack $w -padx 10 -pady 10

    pack .accel.topw.leg -side left -anchor w

    pack .accel.topw -side top

    frame .accel.bfrm
    menubutton .accel.bfrm.menu -text "$view view" -menu \
	    .accel.bfrm.menu.m -bd 4 -relief raised -indicatoron 1
    menu .accel.bfrm.menu.m -tearoff no
    .accel.bfrm.menu.m add command -label "xz view" -command "set view xz; \
	    .accel.bfrm.menu configure -text {xz view}; redraw_accel"
    .accel.bfrm.menu.m add command -label "yz view" -command "set view yz; \
	    .accel.bfrm.menu configure -text {yz view}; redraw_accel"

    button .accel.bfrm.range -text " Plot properties... " -command\
	    "change_accel_range" -relief groove -bd 8
    button .accel.bfrm.print -text " Print... " -command\
	    "print_canvas .accel.topw.frm.can $scrlwidth $scrlheight"\
	    -relief groove -bd 8
    button .accel.bfrm.done -text " Close " -command\
	    "destroy .accel" -relief groove -bd 8
    pack .accel.bfrm.menu .accel.bfrm.range .accel.bfrm.print .accel.bfrm.done\
	    -side left -padx 10 -fill x -expand 1
    pack .accel.bfrm -side bottom -pady 5 -fill x

}

proc redraw_accel {} {
    global zrange_acc yrange_acc xrange_acc values medium nmed colorlist
    global num_cm cmval helvfont cm_names cm_type cm_ident nyticks
    global xzoom yzoom zzoom meds_used inp_base GUI_DIR nxticks nzticks view
    global scrlwidth scrlheight colornum

    catch {destroy .accel.topw.frm.can}

    # get the colour of the background medium and initialize meds_used
    for {set j 0} {$j<=$nmed} {incr j} {
	set meds_used($j) 0
    }
    set med $colornum
    #default color is black if we cannot find the medium or if the
    #medium number is >= max index of colorlist
    for {set j 0} {$j<=$nmed} {incr j} {
	if [string compare $values(2) $medium($j)]==0 {
	    set med [min_nrc $j $med]
	    break
	}
    }
    set bgcolour [lindex $colorlist $med]

    # scrlwidth, scrlheight are what is actually used, REAL size of the canvas
    set zneed [expr $zrange_acc(1)-$zrange_acc(0)]
    set yneed [expr $yrange_acc(1)-$yrange_acc(0)]
    set xneed [expr $xrange_acc(1)-$xrange_acc(0)]
    if [string compare $view xz]==0 {
	set scrlwidth [expr $xzoom*$xneed*20.0]
    } else {
	set scrlwidth [expr $yzoom*$yneed*20.0]
    }
    set scrlheight [expr $zzoom*$zneed*20.0]
    if $scrlwidth<300 {
	set scrlwidth 300
    }
    if $scrlheight<300 {
	set scrlheight 300
    }
    # width, height are what shows inside the scrollbars
    set width [expr 1024/2.]
    # crop off extra width
    if $width>$scrlwidth {
	set width $scrlwidth
    }
    set height [expr 1024*3./4.]
    if $height>$scrlheight {
	set height $scrlheight
    }

    canvas .accel.topw.frm.can -width $width -height $height\
	    -yscrollcommand ".accel.topw.frm.scrlz set"\
	    -xscrollcommand ".accel.topw.frm.scrlx set"\
	    -scrollregion "0 0 $scrlwidth $scrlheight" -relief sunken -bd 4

    pack .accel.topw.frm.can -side left -expand 1 -fill both

    set l 70.0
    set m 50.0

    set xscale [expr ($scrlwidth-100.0)/double(abs($xrange_acc(1)-$xrange_acc(0)))]
    set yscale [expr ($scrlwidth-100.0)/double(abs($yrange_acc(1)-$yrange_acc(0)))]
    set zscale [expr ($scrlheight-$m-50.0)/double(abs($zrange_acc(1)-$zrange_acc(0)))]

    # fill in the entire canvas with bgcolour
    .accel.topw.frm.can create rectangle 0 0 $scrlwidth $scrlheight -fill $bgcolour\
	    -outline $bgcolour

    if [string compare $view xz]==0 {
	for {set i 1} {$i<=$num_cm} {incr i} {
	    set name $cm_names($cm_type($i))
        if { [string compare $name "MLC"]==0 ||\
                 [string compare $name "MLCQ"]==0 ||\
                 [string compare $name "MLCE"]==0} {
		if $cmval($i,2)==0 {
		    if [catch {add_${name}_sides $i $xscale $zscale $xrange_acc(0)\
			    $zrange_acc(0) $zrange_acc(1) $l $m\
			    .accel.topw.frm.can}]==1 {
			tk_dialog .accelerr "Error in accelerator preview"\
				"An error ocurred while trying to display\
				the $name $cm_ident($i) preview." info 0 OK
		    }
		} else {
		    if [catch {add_${name}_ends $i $xscale $zscale $xrange_acc(0)\
			    $zrange_acc(0) $zrange_acc(1) $l $m\
			    .accel.topw.frm.can}]==1 {
			tk_dialog .accelerr "Error in accelerator preview"\
				"An error ocurred while trying to display\
				the $name $cm_ident($i) preview." info 0 OK
		    }
		}
	    } elseif { [string compare $name "VARMLC"]==0 ||\
                        [string compare $name "DYNVMLC"]==0 ||\
                       [string compare $name "SYNCVMLC"]==0 ||\
                        [string compare $name "SYNCMLCE"]==0 ||\
                        [string compare $name "SYNCHDMLC"]==0} {
                if $cmval($i,2,0)==0 {
                    if [catch {add_${name}_sides $i $xscale $zscale $xrange_acc(0)\
                            $zrange_acc(0) $zrange_acc(1) $l $m\
                            .accel.topw.frm.can}]==1 {
                        tk_dialog .accelerr "Error in accelerator preview"\
                                "An error ocurred while trying to display\
                                the $name $cm_ident($i) preview." info 0 OK
                    }
                } else {
                    if [catch {add_${name}_ends $i $xscale $zscale $xrange_acc(0)\
                            $zrange_acc(0) $zrange_acc(1) $l $m\
                            .accel.topw.frm.can}]==1 {
                        tk_dialog .accelerr "Error in accelerator preview"\
                                "An error ocurred while trying to display\
                                the $name $cm_ident($i) preview." info 0 OK
                    }
                }
            } else {
		if [catch {add_$name $i $xscale $zscale $xrange_acc(0)\
			$zrange_acc(0) $l $m .accel.topw.frm.can}]==1 {
		    tk_dialog .accelerr "Error in accelerator preview"\
			    "An error ocurred while trying to display\
			    the $name $cm_ident($i) preview." info 0 OK
		}
	    }
	}
    } elseif [string compare $view yz]==0 {
        # ALWAYS DRAW ARCCHM FIRST!  (OTHERWISE IT COVERS UP ALL CMS ABOVE IT)
        for {set i $num_cm} {$i>=1} {incr i -1} {
            set name $cm_names($cm_type($i))
            if [string compare $name "ARCCHM"]==0 {
                if [catch {add_${name}_yz $i $yscale $zscale $yrange_acc(0)\
                        $zrange_acc(0) $l $m .accel.topw.frm.can}]==1 {
		    tk_dialog .accelerr "Error in accelerator preview"\
			    "An error ocurred while trying to display\
			    the $name $cm_ident($i) preview." info 0 OK
		}
            }
        }

	for {set i 1} {$i<=$num_cm} {incr i} {
	    set name $cm_names($cm_type($i))
	    if { [string compare $name "MLC"]==0 ||\
                 [string compare $name "MLCQ"]==0 ||\
                 [string compare $name "MLCE"]==0} {
		if $cmval($i,2)==1 {
		    if [catch {add_${name}_sides $i $yscale $zscale $yrange_acc(0)\
			    $zrange_acc(0) $zrange_acc(1) $l $m \
			    .accel.topw.frm.can}]==1 {
			tk_dialog .accelerr "Error in accelerator preview"\
				"An error ocurred while trying to display\
				the $name $cm_ident($i) preview." info 0 OK
		    }
		} else {
		    if [catch {add_${name}_ends $i $yscale $zscale $yrange_acc(0)\
			    $zrange_acc(0) $zrange_acc(1) $l $m \
			    .accel.topw.frm.can}]==1 {
			tk_dialog .accelerr "Error in accelerator preview"\
				"An error ocurred while trying to display\
				the $name $cm_ident($i) preview." info 0 OK
		    }
		}
	    } elseif { [string compare $name "VARMLC"]==0 ||\
                       [string compare $name "DYNVMLC"]==0 ||\
                       [string compare $name "SYNCVMLC"]==0 ||\
                       [string compare $name "SYNCMLCE"]==0 ||\
                       [string compare $name "SYNCHDMLC"]==0} {
                if $cmval($i,2,0)==1 {
                    if [catch {add_${name}_sides $i $yscale $zscale $yrange_acc(0)\
                            $zrange_acc(0) $zrange_acc(1) $l $m \
                            .accel.topw.frm.can}]==1 {
                        tk_dialog .accelerr "Error in accelerator preview"\
                                "An error ocurred while trying to display\
                                the $name $cm_ident($i) preview." info 0 OK
                    }
                } else {
                    if [catch {add_${name}_ends $i $yscale $zscale $yrange_acc(0)\
                            $zrange_acc(0) $zrange_acc(1) $l $m \
                            .accel.topw.frm.can}]==1 {
                        tk_dialog .accelerr "Error in accelerator preview"\
                                "An error ocurred while trying to display\
                                the $name $cm_ident($i) preview." info 0 OK
                    }
                }
            } elseif { [string compare $name "SLABS"]==0 || \
		    [string compare $name "CONS3R"]==0 || \
		    [string compare $name "CONESTAK"]==0 || \
		    [string compare $name "FLATFILT"]==0 || \
		    [string compare $name "CHAMBER"]==0 } {
		if [catch {add_$name $i $yscale $zscale $yrange_acc(0)\
			$zrange_acc(0) $l $m .accel.topw.frm.can}]==1 {
		    tk_dialog .accelerr "Error in accelerator preview"\
			    "An error ocurred while trying to display\
			    the $name $cm_ident($i) preview." info 0 OK
		}

	    } elseif [string compare $name "ARCCHM"]!=0 {
		if [catch {add_${name}_yz $i $yscale $zscale $yrange_acc(0)\
			$zrange_acc(0) $l $m .accel.topw.frm.can}]==1 {
		    tk_dialog .accelerr "Error in accelerator preview"\
			    "An error ocurred while trying to display\
			    the $name $cm_ident($i) preview." info 0 OK
		}
	    }
	}
    }

    set canvas .accel.topw.frm.can
    $canvas create rectangle -10 -10 $l [expr $scrlheight-$m]\
	    -fill white -outline white
    $canvas create rectangle $l -10 [expr $scrlwidth+10] $m -fill white\
	    -outline white
    $canvas create rectangle [expr $scrlwidth-30] $m [expr $scrlwidth+10]\
	    [expr $scrlheight+10] -fill white -outline white
    $canvas create rectangle -10 [expr $scrlheight+10] [expr $scrlwidth-30]\
	    [expr $scrlheight-$m] -fill white -outline white

    # put on the NRC-CNRC logo, bottom right corner
    .accel.topw.frm.can create bitmap [expr $scrlwidth-70] [expr $scrlheight-20]\
	    -bitmap @$GUI_DIR/graphics/nrc-cnrc.xbm
    set date [clock format [clock seconds] -format %D]
    #set date [exec date +%D]
    #set date {}

    # COMMENT OUT THE NEXT LINE TO REMOVE DATE/FILENAME FROM THE CANVAS
    .accel.topw.frm.can create text [expr $scrlwidth/2.0] 15 \
	    -text "$inp_base.egsinp   $date" -font $helvfont


    # put r markers on x-axis
    if [catch {expr $nxticks}]==1 {
	set nxticks [expr int($scrlwidth/75.0)]
    }
    if [catch {expr $nyticks}]==1 {
	set nyticks [expr int($scrlwidth/75.0)]
    }
    if [string compare $view xz]==0 {
	set dr [expr ($xrange_acc(1)-$xrange_acc(0))/double($nxticks-1)]
	for {set i 0} {$i<$nxticks} {incr i} {
	    set r [expr ($i*$dr)*$xscale+$l]
	    .accel.topw.frm.can create line $r [expr $m-5] $r [expr $m+5]
	    .accel.topw.frm.can create text $r [expr $m-15] -text \
		    [format "%3.3f" [expr $xrange_acc(0)+$i*$dr]] -font $helvfont
	}
    } else {
	set dr [expr ($yrange_acc(1)-$yrange_acc(0))/double($nyticks-1)]
	for {set i 0} {$i<$nyticks} {incr i} {
	    set r [expr ($i*$dr)*$yscale+$l]
	    .accel.topw.frm.can create line $r [expr $m-5] $r [expr $m+5]
	    .accel.topw.frm.can create text $r [expr $m-15] -text \
		    [format "%3.3f" [expr $yrange_acc(0)+$i*$dr]] -font $helvfont
	}
    }
    # put on z markers on z-axis: number of ticks is based on the scrlheight
    if [catch {expr $nzticks}]==1 {
	set nzticks [expr int($scrlheight/75.0)]
    }
    set awidth [expr $scrlwidth-100.0]
    set dz [expr ($zrange_acc(1)-$zrange_acc(0))/double($nzticks-1)]
    for {set i 0} {$i<$nzticks} {incr i} {
	# add a marker on the z-axis:
	set y2 [expr ($i*$dz)*$zscale+$m]
	.accel.topw.frm.can create line [expr $awidth/2.0-5+$l] $y2\
		[expr $awidth/2.0+5+$l] $y2
	.accel.topw.frm.can create text 35.0 $y2 -text\
		[format "%3.3f" [expr $zrange_acc(0)+$i*$dz]] -font $helvfont
    }

    # put on x/z axes
    # z down the center:
    .accel.topw.frm.can create line [expr $awidth/2.0+$l] $m\
	    [expr $awidth/2.0+$l] [expr $scrlheight-15] -arrow last
    .accel.topw.frm.can create text [expr $awidth/2.0+$l] [expr $scrlheight-5]\
	    -text "z"
    # x from left to right at top:
    .accel.topw.frm.can create line $l $m [expr $scrlwidth-15] $m -arrow last
    if [string compare $view xz]==0 {
	.accel.topw.frm.can create text [expr $scrlwidth-8] $m -text "x"
    } else {
	.accel.topw.frm.can create text [expr $scrlwidth-8] $m -text "y"
    }

    # reconfigure this button to put in the correct scrlwidth, scrlheight vals.
    catch { .accel.bfrm.print configure -command "print_canvas\
	    .accel.topw.frm.can $scrlwidth $scrlheight" }
}

proc change_accel_range {} {
    global zzoom yzoom xzoom helvfont xrange_acc yrange_acc zrange_acc
    global nxticks nyticks nzticks
    catch {destroy .zoom}
    toplevel .accel.zoom
    wm title .accel.zoom "Plot properties"

    frame .accel.zoom.frm
    set w .accel.zoom.frm

    label $w.xl -text "X"
    label $w.yl -text "Y"
    label $w.zl -text "Z"
    grid config $w.xl -row 0 -column 1
    grid config $w.yl -row 0 -column 2
    grid config $w.zl -row 0 -column 3

    label $w.zoom -text "zoom"
    grid config $w.zoom -row 1 -column 0 -sticky e
    entry $w.xzt -textvariable xzoom -width 8
    entry $w.yzt -textvariable yzoom -width 8
    entry $w.zzt -textvariable zzoom -width 8
    grid config $w.xzt -row 1 -column 1
    grid config $w.yzt -row 1 -column 2
    grid config $w.zzt -row 1 -column 3

    label $w.min -text "Min. value"
    grid config $w.min -row 2 -column 0 -sticky e
    entry $w.minxrange -textvariable xrange_acc(0) -width 8
    entry $w.minyrange -textvariable yrange_acc(0) -width 8
    entry $w.minzrange -textvariable zrange_acc(0) -width 8
    grid config $w.minxrange -row 2 -column 1
    grid config $w.minyrange -row 2 -column 2
    grid config $w.minzrange -row 2 -column 3

    label $w.max -text "Max. value"
    grid config $w.max -row 3 -column 0 -sticky e
    entry $w.maxxrange -textvariable xrange_acc(1) -width 8
    entry $w.maxyrange -textvariable yrange_acc(1) -width 8
    entry $w.maxzrange -textvariable zrange_acc(1) -width 8
    grid config $w.maxxrange -row 3 -column 1
    grid config $w.maxyrange -row 3 -column 2
    grid config $w.maxzrange -row 3 -column 3

    label $w.ticks -text "Number of ticks"
    grid config $w.ticks -row 4 -column 0 -sticky e
    entry $w.xticks -textvariable nxticks -width 8
    entry $w.yticks -textvariable nyticks -width 8
    entry $w.zticks -textvariable nzticks -width 8
    grid config $w.xticks -row 4 -column 1
    grid config $w.yticks -row 4 -column 2
    grid config $w.zticks -row 4 -column 3

    pack .accel.zoom.frm -pady 5
    frame .accel.zoom.buts
    button .accel.zoom.buts.app -text "Apply" \
	    -command {redraw_accel} -relief groove -bd 8
    button .accel.zoom.buts.appclose -text "Apply and close" \
	    -command {destroy .accel.zoom; redraw_accel} -relief groove -bd 8
    button .accel.zoom.buts.cancel -text "Close" \
	    -command {destroy .accel.zoom} -relief groove -bd 8
    pack .accel.zoom.buts.app .accel.zoom.buts.appclose\
	    .accel.zoom.buts.cancel -side left -padx 10
    pack .accel.zoom.buts -pady 5
}

proc get_zmax { id } {

    global cmval cm_type top_identical bot_identical chm_identical values

    set zmax {}

    if $id==0 {
	set zmax $values(24)
    } else {
	set pindex $cm_type($id)
	if $pindex==1 {
	    # slabs
	    set zmax $cmval($id,3)
	    for {set i 1} {$i<=$cmval($id,2)} {incr i} {
		if [catch { set zmax [expr $zmax + $cmval($id,4,0,$i)]}]==1 {
		    set zmax {}; break
		}
	    }
	} elseif { $pindex==2 || $pindex==15 } {
	    # cons3r or sidetube
	    catch { set zmax [expr $cmval($id,2) + $cmval($id,3)]}
	} elseif $pindex==3 {
	    # conestak
	    set zmax $cmval($id,2,0)
	    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
		if [catch { set zmax [expr $zmax + $cmval($id,4,0,$i)]}]==1 {
		    set zmax {}; break
		}
	    }
	} elseif $pindex==4 {
	    # flatfilt
	    set zmax $cmval($id,2)
	    for {set i 1} {$i<=$cmval($id,3)} {incr i} {
		if [catch { set zmax [expr $zmax + $cmval($id,4,1,$i)]}]==1 {
		    set zmax {}; break
		}
	    }
	} elseif $pindex==5 {
	    # chamber
	    set zmax $cmval($id,2)

	    if $top_identical($id)==0 {
		for {set i 1} {$i<=$cmval($id,3,0)} {incr i} {
		    catch { set zmax [expr $zmax+$cmval($id,4,0,0,$i)] }
		}
	    } else {
		catch {set zmax [expr $zmax+$cmval($id,4,0,0,1)*$cmval($id,3,0)]}
	    }

	    if $chm_identical($id)==0 {
		for {set i 1} {$i<=$cmval($id,3,1)} {incr i} {
		    catch {set zmax [expr $zmax+$cmval($id,5,1,0,$i)]}
		}
	    } elseif $chm_identical($id)==1 {
		catch {set zmax [expr $zmax+$cmval($id,5,1,0,1)*$cmval($id,3,1)]}
	    } elseif $chm_identical($id)==2 {
		for {set i 1} {$i<=$cmval($id,3,1)} {incr i} {
		    catch { set zmax [expr $zmax+\
			    $cmval($id,5,1,1,$i)*$cmval($id,5,1,0,$i)]}
		}
	    }

	    if $bot_identical($id)==0 {
		for {set i 1} {$i<=$cmval($id,3,2)} {incr i} {
		    catch {set zmax [expr $zmax+$cmval($id,6,0,0,$i)]}
		}
	    } else {
		catch {set zmax [expr $zmax+$cmval($id,6,0,0,1)*$cmval($id,3,2)]}
	    }
	} elseif $pindex==6 {
	    # jaws
	    set zmax $cmval($id,4,1,$cmval($id,2))
	} elseif { $pindex==7 || $pindex==8 } {
	    # applicat or circapp
	    set zmax $cmval($id,2)
	} elseif $pindex==9 {
	    set zmax $cmval($id,3,1,$cmval($id,2,0))
	} elseif $pindex==10 {
	    set zmax $cmval($id,2,1)
	} elseif { $pindex==11 || $pindex==17 || $pindex==18 \
               || $pindex==20 || $pindex==24 || $pindex==25} {
            #mlc,mlcq or varmlc or dynvmlc or syncvmlc or synchdmlc
	    catch { set zmax [expr $cmval($id,3)+$cmval($id,4)]}
	} elseif $pindex==12 {
	    catch { set zmax [expr $cmval($id,2)+$cmval($id,3,3)]}
	} elseif { $pindex==13 || $pindex==14 } {
	    # mirror or xtube
	    catch { set zmax [expr $cmval($id,2,0)+$cmval($id,2,1)]}
	} elseif $pindex==16 {
	    # arcchm
	    set zmax $cmval($id,12)
	} elseif { $pindex==19 || $pindex==23} {
            #mlce, syncmlce: slightly more complicated because of rotations
            #calculation assumes no point in leaves goes lower than bottom
            #left and right corners after rotation
            set xref(4) -$cmval($id,7,1)
            set xref(5) [expr $cmval($id,7,1)-$cmval($id,6)]
            set zref(4) $cmval($id,4,1)
            set zref(5) $cmval($id,4,1)
            set zref(1) $cmval($id,4,0)
            set zmax $cmval($id,4,1)
            for {set j 1} {$j<= [expr $cmval($id,3)/2]} {incr j} {
               #for leaves on +ve side of || axis
               set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
               if {$j==1} {
                  set translr1 $translr
               }
               if {[expr -$cmval($id,7,0)+$translr1]<0} {
                  #centremost leaves will overlap (unlikely that rotation
                  #will change this)
                  set translr [expr (2*$j-1)*($cmval($id,8,0)/2.)]
               }
               set rotr \
                 [expr atan((2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
               for {set k 4} {$k<=5} {incr k} {
                  set xtempr [expr cos($rotr)*$xref($k)+sin($rotr)*\
                               ($zref($k)-$zref(1))+$translr]
                  set ztempr [expr $zref(1)-sin($rotr)*$xref($k)+\
                               cos($rotr)*($zref($k)-$zref(1))]
                  set z($k) [expr $zref(1)-sin($cmval($id,9))*$xtempr+\
                               cos($cmval($id,9))*($ztempr-$zref(1))]
                  if {$z($k)>$zmax} {
                     set zmax $z($k)
                  }
               }
               #for leaves on -ve side of || axis
               set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)*\
                        $cmval($id,4,0)/$cmval($id,8,1)]
               if {$j==1} {
                  set transll1 $transll
               }
               if {[expr $cmval($id,7,0)+$transll1]>0} {
                  #centremost leaves will overlap (unlikely that rotation
                  #will change this)
                  set transll [expr -(2*$j-1)*($cmval($id,8,0)/2.)]
               }
               set rotl \
                 [expr atan(-(2*$j-1)*($cmval($id,8,0)/2.)/$cmval($id,8,1))]
               for {set k 4} {$k<=5} {incr k} {
                  set xtempl [expr cos($rotl)*$xref($k)+sin($rotl)*\
                                 ($zref($k)-$zref(1))+$transll]
                  set ztempl [expr $zref(1)-sin($rotl)*$xref($k)+\
                                 cos($rotl)*($zref($k)-$zref(1))]
                  set z($k) [expr $zref(1)-sin($cmval($id,9))*$xtempl+\
                                 cos($cmval($id,9))*($ztempl-$zref(1))]
                  if {$z($k)>$zmax} {
                     set zmax $z($k)
                  }
               }
            }
        } elseif {$pindex==21 || $pindex == 22}  {
            # dynjaws or syncjaws
            set zmax $cmval($id,4,1,$cmval($id,2,0))
        }
    }
    return $zmax
}


