#!/usr/bin/wish
###############################################################################
#
#  EGSnrc previewRZ rz geometry viewer for windows
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
#  Contributors:    Dave Rogers
#                   Blake Walters
#                   Ernesto Mainegra-Hing
#
###############################################################################
#
#  First line needs to be changed to point at where wish is on your system.
#  On SUSE Linux it is under /usr/X11/bin/wish but on our previous system it
#  was under /usr/bin/wish. Both are now aliased to the same location at NRC.
#
#  This is a Tcl script to view an RZ geometry for any of the NRC RZ user
#  codes. It reads the input file directly. The input file name (without
#  the .egsinp extension) should be passed as an argument.

#  This is set to work with input.egsinp files for the EGSnrc system. It can
#  be used with EGS4 user codes which use the GET_INPUT subroutine to input
#  the standard NRC RZ geometry by changing egsinp to egs4inp at 2 locations.
#
###############################################################################


set debug 0
set show_outline 1
set ticktype 0

if $argc!=1 {
    puts "usage: preview <input filename>"
    exit
}


# set the colors for graphic - VACUUM is always grey -- there are 44 here
set colorlist  {
grey          #ffff00000000 #a410b861ffff #ffffafc80000 #ffffea600000
#ffffffff0000 #a69985c6a699 #159fffff0000 #0000ffffafc8 #0000ffffffff
#0000c567ffff #00008acfffff #00000000ffff #afc80000ffff #ea600000ffff
#ffff0000c567 #ffffa410a410 #ffffdcf3a410 #ffffffffa410 #a410ffffa410
#a410ffffdcf3 #a410ffffffff #a410f554ffff #ffff75300000 #a410a410ffff
#dcf3a410ffff #ffffa410ffff #ffffa410b450 #aa0f00000000 #aa0f58ca0000
#aa0f95bd0000 #aa0faa0f0000 #0000aa0f0000 #0000aa0faa0f #0000712baa0f
#00000000aa0f #99ce0000aa0f #aa0f0000aa0f #fffff473dd5a  white
#e6e6e6e6fafa #4040e0e0d0d0 #aa0f00003848 black
}

#procedure endstr below returns the index of the last character in a string
#it will cut off parts of the string beginning with ";" or "#"
#assuming they are comments--BW

proc endstr { str } {

      if { [string first "#" $str]==-1 } {
        if { [string first ";" $str]==-1 } {
            return [string length $str]
        } else {
            return [expr [string first ";" $str] - 1]
        }
      } else {
        return [expr [string first "#" $str] - 1]
      }

}

proc get_1_val { data varname } {
    # procedure to get a single number.  Should have no comma.
    global $varname
    set data [string trimleft $data]
    set data [string trimright $data]

    # find the first space after the number
    set indx [string first " " $data]
    if $indx<0 {set indx [string length $data]}
    set val [string range $data 0 $indx]
    if [catch {set val [expr $val]}]==1 {
	puts "Couldn't read $varname: check if there's an unnecessary comma."
	set val 0
    }
    set ${varname} $val

    return $data
}

proc get_multi_vals { data varname max } {
    # procedure to get multiple numbers out of data.  Separated by commas.
    global $varname fileid

    set data [string trimleft $data]
    set data [string trimright $data]

    # There may be multiple lines.  For up to $max, look for a comma,
    # get the number, and reduce $data.
    for {set i 1} {$i<=$max} {incr i} {
	set indx [string first , $data]
	if { $indx==[expr [string length $data] -1] & $indx!=-1} {
	    # The first comma in $data is the last comma in $data, hence the
	    # number is the last on this line to be read in.  Have to check
	    # if there's another line of numbers.
	    # Read in the last number
	    set val [string range $data 0 [incr indx -1] ]
	    if [catch {set val [expr $val]}]==1 {
		set val 0
	    }
	    # Get the next line and test it.
	    gets $fileid data
	    set data [string trimleft $data]
	    set data [string trimright $data]
	    if [string first = $data]>-1 {
		# There's an = on the next line, so it's not continuing.
		break
	    }
	} elseif { $indx<0 } {
	    # There was no comma found. Get the number and return.
	    set indx [string first " " $data]
	    if $indx<0 {set indx [string length $data]}
	    set val [string range $data 0 [incr indx -1] ]
	    if [catch {set val [expr $val]}]==1 {
		set val 0
	    }
	    set ${varname}($i) $val
	    break
	} else {
	    set val [string range $data 0 [incr indx -1] ]
	    if [catch {set val [expr $val]}]==1 {
		set val 0
	    }
	    set data [string range $data [incr indx 2] [string length $data] ]
	    set data [string trimleft $data]
	}
	set ${varname}($i) $val
    }
    return $i
}

proc get_multi_str { data varname max } {
    # procedure to get multiple numbers out of data.  Separated by commas.
    global $varname fileid

    # I'm going to assume here that all comments are started by a '#' or a ';'
    # First prune off comments
    set indx [string first "#" $data]
    if $indx<0 { set indx [string first ";" $data]}
    if $indx<0 { set indx [string length $data]}
    set data [string range $data 0 [expr $indx-1] ]

    # Now trim off whitespace
    set data [string trimleft $data]
    set data [string trimright $data]

    # Now get what's there.
    for {set i 1} {$i<=$max} {incr i} {
	set indx [string first , $data]
	if $indx<0 {
	    # There's no comma on the line, which means this is the last of it.
	    set indx2 [string first " " $data]
	    if $indx2<0 {set indx2 [string length $data]}
	    set val [string range $data 0 $indx2 ]
	    set ${varname}($i) $val
	    break
	} else {
	    # We have a comma.  This may mean we have more on the line or
	    # we have more on the next line or that's it.
	    # Strategy: get what's there up until that comma.
	    set val [string range $data 0 [incr indx -1] ]
	    # Now trim it off and see what's left
	    set data [string range $data [expr $indx+2] [string length $data]]
	    set data [string trimleft $data]
	    # Is there anything left in $data?
	    if [string length $data]==0 {
		# nope, line over.  go to the next line and begin again.
		gets $fileid data
		# First prune off comments
		set indx [string first "#" $data]
		if $indx<0 { set indx [string first ";" $data]}
		if $indx<0 { set indx [string length $data]}
		set data [string range $data 0 [expr $indx-1] ]
		set data [string trimleft $data]
		set data [string trimright $data]
		if [string first = $data]>-1 {
		    break
		}
	    }
	    set ${varname}($i) $val
	}
    }
    return $i
}

proc read_inputfile {} {

    global inputfile fileid radii depth zff medium startreg stopreg mednum
    global ndepth nrad nmed mmed nslab thick startz stopz startr stopr debug

    # read an input file ** in Aaron's morphed format **
    # For we need to search for the keyword METHOD OF INPUT first for
    # Groups, Individual, or Cavity information.

    set fileid [open $inputfile r]

    # The first thing is to mark the position of the start of the geometry
    set startpos -1
    while { [eof $fileid] != 1 } {
	gets $fileid data
	set str $data
	set str [string trimleft $str]
	set str [string trimright $str]
	if [string compare $str ":start geometrical inputs:"]==0 {
	    set startpos [tell $fileid]
	    break
	}
    }
    if $startpos==-1 {
	puts "This input file is not in the morphed format!  Exiting."
	exit
    }

    while { [eof $fileid] != 1 } {
	gets $fileid data
	set str [string range $data 0 [string first = $data]]
	set str [string trimleft $str]
	set str [string trimright $str]
	if [string compare $str "METHOD OF INPUT="]==0 {
	    set method [string range $data [expr [string first = $data] + 1]\
		    [endstr $data]]
	    set method [string trimleft $method]
	    set method [string trimright $method]
	    set method [string tolower $method]
	    break
	}
    }

    # Z OF FRONT FACE
    seek $fileid $startpos start
    while { [eof $fileid] != 1 } {
	gets $fileid data
	set str [string range $data 0 [string first = $data]]
	set str [string trimleft $str]
	set str [string trimright $str]
	if [string compare $str "Z OF FRONT FACE="]==0 {
	    set str2 [string range $data [expr [string first = $data] + 1]\
		    [endstr $data]]
	    set str2 [get_1_val $str2 zff]
	    break
	}
    }
    if $debug==1 { puts "Z of front face = $zff" }

    if [string compare $method "individual"]==0 {
	# DEPTH BOUNDARIES
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "DEPTH BOUNDARIES="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			 [endstr $data]]
		set ndepth [get_multi_vals $str2 depth 100]
		break
	    }
	}
    } else {
	# method=Groups
	# NSLAB, SLAB THICKNESS
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "NSLAB="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			 [endstr $data]]
		set nnslab [get_multi_vals $str2 nslab 100]
		break
	    }
	}
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "SLAB THICKNESS="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			 [endstr $data]]
		set nnslab [get_multi_vals $str2 thick 100]
		break
	    }
	}
	# Now set ndepth, depth based on this.
	set ndepth 0
	for {set i 1} {$i<=$nnslab} {incr i} {
	    set ndepth [expr $ndepth+$nslab($i)]
	}
	set z $zff
	set count 0
	for {set j 1} {$j<=$nnslab} {incr j} {
	    for {set i 1} {$i<=$nslab($j)} {incr i} {
		incr count
		set z [expr $z+$thick($j)]
		set depth($count) $z
	    }
	}
    }
    if $debug==1 {
	for {set i 1} {$i<=$ndepth} {incr i} {
	    puts "depth($i)=$depth($i)"
	}
    }

    # Get the radii, valid for all methods
    seek $fileid $startpos start
    while { [eof $fileid] != 1 } {
	gets $fileid data
	set str [string range $data 0 [string first = $data]]
	set str [string trimleft $str]
	set str [string trimright $str]
	if [string compare $str "RADII="]==0 {
	    set str2 [string range $data [expr [string first = $data] + 1]\
		    [endstr $data]]
	    set nrad [get_multi_vals $str2 radii 100]
	    break
	}
    }
    if $debug==1 {
	for {set i 1} {$i<=$nrad} {incr i} {
	    puts "radii($i)=$radii($i)"
	}
    }

    # Now it's time to read in the media...
    set medium(0) VACUUM
    set nmed 0
    seek $fileid $startpos start
    while { [eof $fileid] != 1 } {
	gets $fileid data
	set str [string range $data 0 [string first = $data]]
	set str [string trimleft $str]
	set str [string trimright $str]
	if [string compare $str "MEDIA="]==0 {
	    set str2 [string range $data [expr [string first = $data] + 1]\
		    [endstr $data]]
	    set nmed [get_multi_str $str2 medium 100]
	    break
	}
    }
    if $debug==1 {
	for {set i 1} {$i<=$nmed} {incr i} {
	    puts "medium($i)=$medium($i)"
	}
    }

    # DESCRIPTION BY
    seek $fileid $startpos start
    while { [eof $fileid] != 1 } {
	gets $fileid data
	set str [string range $data 0 [string first = $data]]
	set str [string trimleft $str]
	set str [string trimright $str]
	if [string compare $str "DESCRIPTION BY="]==0 {
	    set descrip [string range $data [expr [string first = $data] + 1]\
		    [endstr $data]]
	    set descrip [string trimleft $descrip]
	    set descrip [string trimright $descrip]
	    set descrip [string tolower $descrip]
	    break
	}
    }
    if $debug==1 { puts "description by= $descrip" }

    # Read MEDNUM
    seek $fileid $startpos start
    while { [eof $fileid] != 1 } {
	gets $fileid data
	set str [string range $data 0 [string first = $data]]
	set str [string trimleft $str]
	set str [string trimright $str]
	if [string compare $str "MEDNUM="]==0 {
	    set str2 [string range $data [expr [string first = $data] + 1]\
		    [endstr $data]]
	    set nummed [get_multi_vals $str2 mednum 10000]
	    break
	}
    }
    if $debug==1 {
	for {set i 1} {$i<=$nummed} {incr i} {
	    puts "mednum($i)=$mednum($i)"
	}
    }

    if [string compare $descrip regions]==0 {
	# Get the mednum, start and stop regions
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "START REGION="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			[endstr $data]]
		set nummed [get_multi_vals $str2 startreg 10000]
		break
	    }
	}
	if $debug==1 {
	    for {set i 1} {$i<=$nummed} {incr i} {
		puts "startreg($i)=$startreg($i)"
	    }
	}
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "STOP REGION="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			[endstr $data]]
		set nummed [get_multi_vals $str2 stopreg 10000]
		break
	    }
	}
	if $debug==1 {
	    for {set i 1} {$i<=$nummed} {incr i} {
		puts "stopreg($i)=$stopreg($i)"
	    }
	}
	# Set up the array of media for each region
	set k 1
	for {set i 1} {$i<=$nrad} {incr i} {
	    for {set j 1} {$j<=$ndepth} {incr j} {
		incr k
		set mmed($k) 1
	    }
	}
	for {set i 1} {$i<=$nummed} {incr i} {
	    if { $startreg($i)!=$stopreg($i) & $stopreg($i)!=0 } {
		for {set r1 $startreg($i)} {$r1<=$stopreg($i)} {incr r1} {
		    set mmed($r1) $mednum($i)
		}
	    } else {
		set mmed($startreg($i)) $mednum($i)
	    }
	}
    } else {
	# Description by planes.
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "START ZSLAB="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			[endstr $data]]
		set nummed [get_multi_vals $str2 startz 10000]
		break
	    }
	}
	if $debug==1 {
	    for {set i 1} {$i<=$nummed} {incr i} {
		puts "startz($i)=$startz($i)"
	    }
	}
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "STOP ZSLAB="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			[endstr $data]]
		set nummed [get_multi_vals $str2 stopz 10000]
		break
	    }
	}
	if $debug==1 {
	    for {set i 1} {$i<=$nummed} {incr i} {
		puts "stopz($i)=$stopz($i)"
	    }
	}
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "START RING="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			[endstr $data]]
		set nummed [get_multi_vals $str2 startr 10000]
		break
	    }
	}
	if $debug==1 {
	    for {set i 1} {$i<=$nummed} {incr i} {
		puts "startr($i)=$startr($i)"
	    }
	}
	seek $fileid $startpos start
	while { [eof $fileid] != 1 } {
	    gets $fileid data
	    set str [string range $data 0 [string first = $data]]
	    set str [string trimleft $str]
	    set str [string trimright $str]
	    if [string compare $str "STOP RING="]==0 {
		set str2 [string range $data [expr [string first = $data] + 1]\
			[endstr $data]]
		set nummed [get_multi_vals $str2 stopr 10000]
		break
	    }
	}
	if $debug==1 {
	    for {set i 1} {$i<=$nummed} {incr i} {
		puts "stopr($i)=$stopr($i)"
	    }
	}

	# Set up the array of media for each region
	set k 1
	for {set i 1} {$i<=$nrad} {incr i} {
	    for {set j 1} {$j<=$ndepth} {incr j} {
		incr k
		set mmed($k) 1
	    }
	}
	for {set i 1} {$i<=$nummed} {incr i} {
	    for {set iz $startz($i)} {$iz<=$stopz($i)} {incr iz} {
		for {set ir $startr($i)} {$ir<=$stopr($i)} {incr ir} {
		    set k [expr ($ir-1)*$ndepth+$iz+1]
		    set mmed($k) $mednum($i)
		}
	    }
	}
    }

    close $fileid
}

proc show_preview {} {
    global xzoom zzoom xrange_acc zrange_acc scrlwidth scrlheight
    global colorlist nmed medium depth radii ndepth nrad zff

    # first we have to determine what the limits of the cavity/chamber
    # are in x and in z.

    set zrange_acc(0) $zff
    set zrange_acc(1) $depth($ndepth)
    set xrange_acc(1) $radii($nrad)
    set xrange_acc(0) 0.0

    # the zoom is how many pixels represent 1 unit of length
    set dx [expr $xrange_acc(1)-$xrange_acc(0)]
    set dz [expr $zrange_acc(1)-$zrange_acc(0)]
    set xzoom [expr int(([winfo screenwidth .]/2.0)/$dx)]
    set zzoom [expr int(([winfo screenheight .]/2.0)/$dz)]
    set ratio [expr $dx/$dz]
    if $ratio>1000 {
        set xzoom [expr $zzoom*1000/$ratio]
    } elseif $ratio>5.0 {
	set xzoom $zzoom
    } elseif { $ratio>1.0 & $ratio<5.0 } {
	set zzoom $xzoom
    } elseif { $ratio<1.0 & $ratio>0.2 } {
	set xzoom $zzoom
    } elseif { $ratio<0.2 & $ratio>=0.001 } {
        set zzoom $xzoom
    } else {
        set zzoom [expr $xzoom*1000*$ratio]
    }

    frame .topw -bd 4
    frame .topw.frm

    scrollbar .topw.frm.scrlz -command ".topw.frm.can yview"
    scrollbar .topw.frm.scrlx -command ".topw.frm.can xview" -orient horizontal

    pack .topw.frm.scrlz -side right -fill y
    pack .topw.frm.scrlx -side bottom -fill x

    redraw_cavity

    pack .topw.frm -side right -anchor n

    # left frame will hold a legend of the colours used
    frame .topw.leg

    label .topw.leg.lab -text "LEGEND"
    pack .topw.leg.lab -pady 5

    message .topw.leg.mess -text "Click on the colour swatch to\
            change the colour of a material." -width 200
    pack .topw.leg.mess -pady 5 -anchor w

    # create a frame to hold a grid
    frame .topw.leg.frm
    set w .topw.leg.frm
    for {set i 1} {$i<=$nmed} {incr i} {
	canvas $w.c$i -width 10 -height 10 -bg [lindex $colorlist $i]
	bind $w.c$i <Button-1> "change_color $w.c$i $i;\
		tkwait window $w.c$i.select; redraw_cavity"
	label $w.l$i -text $medium($i)
	grid config $w.c$i -row $i -column 0
	grid config $w.l$i -row $i -column 1 -sticky w
    }
    pack $w -padx 10 -pady 10

    label .topw.leg.mess2 -text "DEPTHS AND RADII (cm)"
    pack .topw.leg.mess2 -pady 10

    frame .topw.leg.vals
    set w .topw.leg.vals
    # Lets put a maximum column depth of 15:
    label $w.l0 -text "Depths"
    grid config $w.l0 -row 0 -column 0 \
	    -columnspan [expr int(ceil($ndepth/15.))] -ipadx 10
    set ncol 0
    set nrow 0
    for {set i 1} {$i<=$ndepth} {incr i} {
	incr nrow
	label $w.l$i -text $depth($i)
	grid config $w.l$i -row $nrow -column $ncol -sticky w
	if $nrow>=15 {
	    set nrow 0
	    incr ncol
	}
    }
    incr ncol
    label $w.pad -text "  "
    grid config $w.pad -row 0 -column $ncol
    incr ncol
    label $w.ll0 -text "Radii"
    grid config $w.ll0 -row 0 -column $ncol \
	    -columnspan [expr int(ceil($nrad/15.))] -ipadx 10
    set nrow 0
    for {set i 1} {$i<=$nrad} {incr i} {
	incr nrow
	label $w.ll$i -text $radii($i)
	grid config $w.ll$i -row $nrow -column $ncol -sticky w
	if $nrow>=15 {
	    set nrow 0
	    incr ncol
	}
    }
    pack $w -padx 10

    pack .topw.leg -side left -anchor w

    pack .topw -side top

    frame .bfrm
    button .bfrm.range -text " Plot properties... " -command\
	    "change_cavity_range" -relief groove -bd 8
    button .bfrm.print -text " Print... " -command\
	    "print_canvas .topw.frm.can $scrlwidth $scrlheight"\
	    -relief groove -bd 8
    button .bfrm.done -text " Exit " -command {exit} -relief groove -bd 8
    pack .bfrm.range .bfrm.print .bfrm.done \
	    -side left -padx 10 -fill x -expand 1
    pack .bfrm -side bottom -pady 5 -fill x

}

proc redraw_cavity {} {
    global zrange_acc xrange_acc medium nmed colorlist mmed debug
    global xzoom zzoom meds_used nxticks nzticks show_outline
    global scrlwidth scrlheight inputfile nrad radii ndepth depth zff
    global ticktype

    catch {destroy .topw.frm.can}

    # scrlwidth, scrlheight are what is actually used, REAL size of the canvas
    set zneed [expr $zrange_acc(1)-$zrange_acc(0)]
    set xneed [expr $xrange_acc(1)-$xrange_acc(0)]
    set scrlwidth [expr $xzoom*$xneed]
    set scrlheight [expr $zzoom*$zneed]

    # width, height are what shows inside the scrollbars
    set width [expr [winfo screenwidth .]/2.]
    # crop off extra width
    if $width>$scrlwidth {
	set width $scrlwidth
    }
    set height [expr [winfo screenheight .]*3./4.]
    if $height>$scrlheight {
	set height $scrlheight
    }

    canvas .topw.frm.can -width $width -height $height\
	    -yscrollcommand ".topw.frm.scrlz set"\
	    -xscrollcommand ".topw.frm.scrlx set"\
	    -scrollregion "0 0 $scrlwidth $scrlheight" -relief sunken -bd 4

    pack .topw.frm.can -side left -expand 1 -fill both

    set l 70.0
    set m 50.0

    set xscale [expr ($scrlwidth-100.0)/double(abs($xrange_acc(1)-$xrange_acc(0)))]
    set zscale [expr ($scrlheight-$m-50.0)/double(abs($zrange_acc(1)-$zrange_acc(0)))]

    # fill in the entire canvas with bgcolour
    .topw.frm.can create rectangle 0 0 $scrlwidth $scrlheight -fill white\
	    -outline white

    set canvas .topw.frm.can

    # Draw the rectangles...
    set zmin $zrange_acc(0)
    set xmin $xrange_acc(0)
    set xul 0.0
    for {set i 1} {$i<=$nrad} {incr i} {
	set xlr $radii($i)
	set zul $zff
	for {set j 1} {$j<=$ndepth} {incr j} {
	    set zlr $depth($j)
	    set med $mmed([expr ($i-1)*$ndepth+$j+1])
	    set xlr2 [expr ($xlr-$xmin)*$xscale+$l]
	    set zlr2 [expr ($zlr-$zmin)*$zscale+$m]
	    set xul2 [expr ($xul-$xmin)*$xscale+$l]
	    set zul2 [expr ($zul-$zmin)*$zscale+$m]
	    if $debug==1 { puts " $xul $zul $xlr $zlr " }
	    if $show_outline==1 {
		set outcolour black
	    } else {
		set outcolour [lindex $colorlist $med]
	    }
	    $canvas create rectangle $xul2 $zul2 $xlr2 $zlr2\
		    -fill [lindex $colorlist $med] -outline $outcolour
	    set zul $zlr
	}
	set xul $xlr
    }

    $canvas create rectangle -5 -5 $l [expr $scrlheight-$m]\
	    -fill white -outline {}
    $canvas create rectangle $l -5 [expr $scrlwidth+5] $m -fill white -outline {}
    $canvas create rectangle [expr $scrlwidth-30] $m [expr $scrlwidth+5]\
	    [expr $scrlheight+5] -fill white -outline {}
    $canvas create rectangle -5 [expr $scrlheight+5] [expr $scrlwidth-30]\
	    [expr $scrlheight-$m] -fill white -outline {}

    # put on the NRC-CNRC logo, bottom right corner
     .topw.frm.can create text [expr $scrlwidth-70] [expr $scrlheight-20]\
 	-text "NRC-CNRC" -font "-*-times-bold-i-normal--*-200-*"
    #if the above not present on your system, find a font that is
    #or just comment out the two lines- its only PR!

     #set date [exec date +%D]
     set date [clock format [clock seconds] -format %D]

     # COMMENT OUT THE NEXT LINE TO REMOVE DATE/FILENAME FROM THE CANVAS
     .topw.frm.can create text [expr $scrlwidth/2.0] 15 \
     	    -text "$inputfile    $date"

    # put r markers on x-axis
    if {$ticktype==0} {
      if [catch {expr $nxticks}]==1 {
	set nxticks [expr int($scrlwidth/75.0)]
      }
      if $nxticks<2 { set nxticks 2 }
      set dr [expr ($xrange_acc(1)-$xrange_acc(0))/double($nxticks-1)]
      for {set i 0} {$i<$nxticks} {incr i} {
	set r [expr ($i*$dr)*$xscale+$l]
	.topw.frm.can create line $r [expr $m-5] $r [expr $m+5]
	.topw.frm.can create text $r [expr $m-15] -text \
		[format "%5.5f" [expr $xrange_acc(0)+$i*$dr]]
      }
    } elseif {$ticktype==1} {
      #put ticks at radial boundaries
      set radii(0) 0
      for {set i 0} {$i<=$nrad} {incr i} {
        if {$radii($i)>=$xrange_acc(0) && $radii($i)<=$xrange_acc(1)} {
           set r [expr ($radii($i)-$xrange_acc(0))*$xscale+$l]
           .topw.frm.can create line $r [expr $m-5] $r $m
           .topw.frm.can create text $r [expr $m-15] -text \
                [format "%5.5f" $radii($i)]
        }
      }
    }

    # put on z markers on z-axis: number of ticks is based on the scrlheight
    if {$ticktype==0} {
      if [catch {expr $nzticks}]==1 {
	set nzticks [expr int($scrlheight/75.0)]
      }
      if $nzticks<2 { set nzticks 2 }
      set dz [expr ($zrange_acc(1)-$zrange_acc(0))/double($nzticks-1)]
      for {set i 0} {$i<$nzticks} {incr i} {
	# add a marker on the z-axis:
	set y2 [expr ($i*$dz)*$zscale+$m]
	.topw.frm.can create line [expr -5+$l] $y2\
		[expr 5+$l] $y2
	.topw.frm.can create text 35.0 $y2 -text\
		[format "%5.5f" [expr $zrange_acc(0)+$i*$dz]]
      }
    } elseif {$ticktype==1} {
      #put ticks at depth boundaries
      set depth(0) $zff
      for {set i 0} {$i<=$ndepth} {incr i} {
        if {$depth($i)>=$zrange_acc(0) && $depth($i)<=$zrange_acc(1)} {
           set y2 [expr ($depth($i)-$zrange_acc(0))*$zscale+$m]
           .topw.frm.can create line [expr $l-5] $y2 $l $y2
           .topw.frm.can create text 35.0 $y2 -text \
                [format "%5.5f" $depth($i)]
        }
      }
    }


    # put on x/z axes
    # z down the left:
    .topw.frm.can create line $l $m $l [expr $scrlheight-15] -arrow last
    .topw.frm.can create text $l [expr $scrlheight-5] -text "z"
    # x from left to right at top:
    .topw.frm.can create line $l $m [expr $scrlwidth-15] $m -arrow last
    .topw.frm.can create text [expr $scrlwidth-8] $m -text "r"

    # reconfigure this button to put in the correct scrlwidth, scrlheight vals.
    catch { .bfrm.print configure -command "print_canvas\
	    .topw.frm.can $scrlwidth $scrlheight" }
}


proc print_canvas { w canwidth canheight } {
     global file prcolour orient fname prname

    catch { destroy .print }
    toplevel .print
    wm title .print "Print options"
    label .print.title -text "Print Options"
    pack .print.title -pady 5

    message .print.mess -text "If you are printing the entire accelerator, \
	    you may want to scale the canvas to a smaller size before\
	    printing/exporting, so that the fonts are readable in the\
	    postscript output." -width 400
#    pack .print.mess -pady 5

    frame .print.frm

    frame .print.frm.colour -relief groove -bd 2
    radiobutton .print.frm.colour.r1 -text "Colour" -variable prcolour\
	    -value color
    radiobutton .print.frm.colour.r2 -text "Greyscale" -variable prcolour\
	    -value gray
    pack .print.frm.colour.r1 .print.frm.colour.r2  -anchor w

    frame .print.frm.orient -relief groove -bd 2
    radiobutton .print.frm.orient.r0 -text "Portrait" -variable orient -value 0
    radiobutton .print.frm.orient.r1 -text "Landscape" -variable orient -value 1
    pack .print.frm.orient.r0 .print.frm.orient.r1 -anchor w

    pack .print.frm.colour .print.frm.orient -side left -padx 10 -pady 5
    pack .print.frm

    frame .print.f2 -relief groove -bd 2
    radiobutton .print.f2.file0 -text "To printer:" -variable file -value 0
    radiobutton .print.f2.file1 -text "To file:" -variable file -value 1
    entry .print.f2.prname -textvariable prname -width 15
    entry .print.f2.fname -textvariable fname -width 15
    grid config .print.f2.file0 -row 0 -column 0 -sticky w
    grid config .print.f2.file1 -row 1 -column 0 -sticky w
    grid config .print.f2.prname -row 0 -column 1 -sticky w
    grid config .print.f2.fname -row 1 -column 1 -sticky w
    pack .print.f2 -pady 5

    frame .print.buts
    button .print.buts.pb -text "Print" -command "print_cmd $w\
            $canwidth $canheight; destroy .print" -relief groove -bd 8
    button .print.buts.cb -text "Cancel" -command "destroy .print"\
	    -relief groove -bd 8
    pack .print.buts.pb .print.buts.cb -side left -padx 5
    pack .print.buts -pady 10

}

proc print_cmd { w canwidth canheight } {
    global env file prcolour orient fname prname

    if [string compare $prcolour {}]==0 {
	tk_dialog .printerror "Print error" "You haven't set whether you\
        want colour or greyscale.  Please set it and try\
        again." error 0 OK
        return 0
    }
    if [catch {expr $file}]==1 {
	tk_dialog .printerror "Print error" "You haven't set whether you\
        want to print to the printer or to a file.  Please set it and try\
        again." error 0 OK
        return 0
    }
    if {$file==1} {
	set fff $fname
    } else {
	set fff $env(HOME)/cavrztemppsfile.ps
    }

    set canwidth [expr $canwidth+50]
    set canheight [expr $canheight+50]

    if [catch {expr $orient}]==1 {
	tk_dialog .printerror "Print error" "You haven't set whether you\
        want to print landscape or portrait.  Please set it and try\
        again." error 0 OK
        return 0
    }
    if $orient==0 {
	# portrait output

	set scalew [expr $canwidth/8.0]
	set scaleh [expr $canheight/10.5]

	if $scaleh>$scalew {
	    $w postscript -colormode $prcolour -rotate $orient\
		    -file $fff -height $canheight -width $canwidth\
		    -pageheight 10.5i
	} else {
	    $w postscript -colormode $prcolour -rotate $orient\
		    -file $fff -height $canheight -width $canwidth\
		    -pagewidth 8.0i
	}
    } else {
	# landscape output
	set scalew [expr $canwidth/10.5]
	set scaleh [expr $canheight/8.0]

	if $scaleh>$scalew {
	    $w postscript -colormode $prcolour -rotate $orient\
		    -file $fff -height $canheight -width $canwidth\
		    -pageheight 8.i
	} else {
	    $w postscript -colormode $prcolour -rotate $orient\
		    -file $fff -height $canheight -width $canwidth\
		    -pagewidth 10.5i
	}
    }


    if $file==0 {
	# send to printer
	if [string compare $prname {}]==0 {
	    tk_dialog .printerr "Printer unspecified" "You haven't specified\
		    a printer.  The print job has not been sent." error 0 OK
	} else {
	    exec lpr -P$prname $fff
	}
	exec rm $fff
    }
   return 1
}


proc change_cavity_range {} {
    global zzoom xzoom xrange_acc zrange_acc show_outline
    global nxticks nzticks ticktype
    catch {destroy .zoom}
    toplevel .zoom
    wm title .zoom "Plot properties"

    frame .zoom.frm
    set w .zoom.frm

    label $w.xl -text "R"
    label $w.zl -text "Z"
    grid config $w.xl -row 0 -column 1
    grid config $w.zl -row 0 -column 2

    label $w.zoom -text "# pixels representing 1cm"
    grid config $w.zoom -row 1 -column 0 -sticky e
    entry $w.xzt -textvariable xzoom -width 8
    entry $w.zzt -textvariable zzoom -width 8
    grid config $w.xzt -row 1 -column 1
    grid config $w.zzt -row 1 -column 2

    label $w.min -text "Minimum value"
    grid config $w.min -row 2 -column 0 -sticky e
    entry $w.minxrange -textvariable xrange_acc(0) -width 8
    entry $w.minzrange -textvariable zrange_acc(0) -width 8
    grid config $w.minxrange -row 2 -column 1
    grid config $w.minzrange -row 2 -column 2

    label $w.max -text "Maximum value"
    grid config $w.max -row 3 -column 0 -sticky e
    entry $w.maxxrange -textvariable xrange_acc(1) -width 8
    entry $w.maxzrange -textvariable zrange_acc(1) -width 8
    grid config $w.maxxrange -row 3 -column 1
    grid config $w.maxzrange -row 3 -column 2

    radiobutton $w.numticks -text "Number of ticks" -variable ticktype\
             -value 0 -command "$w.xticks configure -state normal -fg black;\
                                $w.zticks configure -state normal -fg black"
    grid config $w.numticks -row 4 -column 0 -sticky w
    entry $w.xticks -textvariable nxticks -width 8
    entry $w.zticks -textvariable nzticks -width 8
    grid config $w.xticks -row 4 -column 1
    grid config $w.zticks -row 4 -column 2

    if {$ticktype==1} {
      $w.xticks configure -state disabled -fg grey
      $w.zticks configure -state disabled -fg grey
    }

    radiobutton $w.tab -text "Ticks at region boundaries" -variable ticktype\
             -value 1 -command "$w.xticks configure -state disabled -fg grey;\
                                $w.zticks configure -state disabled -fg grey"
    grid configure $w.tab -row 5 -column 0 -columnspan 3 -sticky w

    pack .zoom.frm -pady 5

    checkbutton .zoom.outlinecheck -text "Show region outlines" \
	    -variable show_outline -onvalue 1 -offvalue 0
    pack .zoom.outlinecheck -pady 5

    frame .zoom.buts
    button .zoom.buts.app -text "Apply" \
	    -command {redraw_cavity} -relief groove -bd 8
    button .zoom.buts.appclose -text "Apply and close" \
	    -command {destroy .zoom; redraw_cavity} -relief groove -bd 8
    button .zoom.buts.cancel -text "Close" \
	    -command {destroy .zoom} -relief groove -bd 8
    pack .zoom.buts.app .zoom.buts.appclose\
	    .zoom.buts.cancel -side left -padx 10
    pack .zoom.buts -pady 5
}

proc change_color { parent_w i } {
    global colorlist red green blue tempcolor
    # get the rgb value for the colour selected
    set tempcolor [lindex $colorlist $i]
    set colist [winfo rgb . $tempcolor]
    set red [lindex $colist 0]
    set green [lindex $colist 1]
    set blue [lindex $colist 2]

    catch {destroy $parent_w.select}

    toplevel $parent_w.select -bd 5
    set top $parent_w.select
    wm title $top "Select colour"
    wm geometry $top +250+300

    frame $top.frm
    frame $top.frm.left
    frame $top.frm.right

    set w $top.frm.left

    frame $w.grid
    # use rgb colour scales
    label $w.grid.red -text "Red"
    label $w.grid.green -text "Green"
    label $w.grid.blue -text "Blue"
    grid config $w.grid.red -row 0 -column 0
    grid config $w.grid.green -row 1 -column 0
    grid config $w.grid.blue -row 2 -column 0
    scale $w.grid.sred -from 0 -to 65535 -length 300 -orient horizontal\
	    -command "update_palette $parent_w red"
    scale $w.grid.sgreen -from 0 -to 65535 -length 300 -orient horizontal\
	    -command "update_palette $parent_w green"
    scale $w.grid.sblue -from 0 -to 65535 -length 300 -orient horizontal\
	    -command "update_palette $parent_w blue"
    $w.grid.sred set $red
    $w.grid.sgreen set $green
    $w.grid.sblue set $blue
    grid config $w.grid.sred -row 0 -column 1
    grid config $w.grid.sgreen -row 1 -column 1
    grid config $w.grid.sblue -row 2 -column 1

    pack $w.grid

    pack $w -side left

    set w $top.frm.right
    label $w.lab -text "Selected colour:"
    pack $w.lab -anchor w
    entry $w.ent -textvariable tempcolor -bg white -state disabled
    pack $w.ent -anchor w
    canvas $w.can -width 100 -height 100
    pack $w.can -fill both -expand true -pady 10

    pack $w

    pack $top.frm -pady 5

    frame $top.buts
    button $top.buts.accept -text "Accept" -relief groove -bd 8 -command\
	    "set_color $parent_w $i; destroy $parent_w.select"
    button $top.buts.cancel -text "Cancel" -relief groove -bd 8 -command\
	    "destroy $parent_w.select"
    pack $top.buts.accept $top.buts.cancel -side left -padx 10
    pack $top.buts -pady 10

}

proc update_palette {parent_w col value} {
    global red green blue tempcolor

    switch $col {
        red { set red $value }
        green { set green $value }
        blue { set blue $value }
        grey { set red $value }
    }

    set tempcolor [format "#%4.4x%4.4x%4.4x" $red $green $blue]
    $parent_w.select.frm.right.can configure -bg $tempcolor
}

proc set_color { parent_w i } {
    global tempcolor colorlist
    set colorlist [lrm $colorlist $i $tempcolor]
    $parent_w configure -bg $tempcolor
}

proc lrm { list pos item } {
    if {$pos >= 0} {
	return [lreplace $list $pos $pos $item]
    }
    return $list
}


# HERE'S THE START OF THE SCRIPT

set inputfile [lindex $argv 0]
if [string compare [file extension $inputfile] .egsinp]!=0 {
    set inputfile "$inputfile.egsinp"
}
if [file exists $inputfile]==1 {
    read_inputfile
} else {
    puts "The file $inputfile doesn't exist!"
    exit
}

wm title . "Preview $inputfile"
set width [winfo screenwidth .]
set height [winfo screenheight .]
set rootx [expr ($width/2) - 400]
set rooty [expr ($height/2) -400]
wm geometry . +$rootx+$rooty

#define a main menubar
#frame .mbar -relief raised -bd 1
#menubutton .mbar.file -text "File" -menu .mbar.file.menu
#menu .mbar.file.menu
#.mbar.file.menu add command -label "Load another cavity" \
#	-command "load_new_cavity"
#.mbar.file.menu add command -label "Exit" -command { exit }
#pack .mbar.file -side left -anchor w -padx 5
#pack .mbar -side top -anchor n -fill both

show_preview




