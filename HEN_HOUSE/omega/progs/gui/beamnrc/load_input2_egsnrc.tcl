
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: load input
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


proc load_input {} {
    # if an input file is already loaded, ask if they want to discard it.
    # if OK, then look for BEAM in inp_file_dir.  If there, start browser at
    # inp_file_dir, else start browser at $start_dir.

    global home inp_file rootx rooty inp_file_dir start_dir GUI_DIR maxvals
    global values

    set result 0
    if [string compare $inp_file ""]!=0 {
	# There's an input file already loaded.  Ask if they
	# want to discard it.
	set result [tk_dialog .okay "Discard?" "The input file\
		currently loaded will be discarded.  Are you sure?" \
		question 0 Yes Cancel  ]
    }
    if $result==0 {
	# Okay, discard it.
        source $GUI_DIR/beamnrc_params.tcl
	set inp_file {}
	.inploaded configure -text {}
	if [file exists $inp_file_dir]==0 {
	    # inp_file_dir doesn't exist yet. Reset to start_dir.
	    set inp_file_dir $start_dir
	}
	query_filename set_inp_file "$start_dir" egsinp
	# query_filename will get a filename and directory tree
	# and send them to set_inp_file...
    }
}

proc set_inp_file {tree filen} {
    # if there is no accelerator loaded, try to get the right module file
    # from the input file directory tree.  If BEAM is in it, load the
    # module and call read_input, else tell the user that they can't do it.

    global home inp_file inp_base mod_file mod_base num_cm inp_file_dir

    set inp_file [file join $tree $filen]
    set inp_base [file rootname $filen]

    if $num_cm==0 {
	# THERE'S NO ACCELERATOR LOADED.  GET THE MATCHING ACCELERATOR
	# FROM THE DIRECTORY NAME (ALREADY CHECKED IN LOAD_INPUT).
	set start_index [string last "BEAM" $tree]

	if $start_index==-1 {
	    tk_dialog .noBEAM "No accelerator found" "The accelerator\
		    corresponding to this input file could not be\
		    located based on it's directory tree.  Please load\
		    an accelerator and try again." warning 0 OK
	    set inp_file {}
	    set inp_base {}
	    return
	}
	incr start_index 5
	set stop_index [string length $tree]
	set mod_base [string range $tree $start_index $stop_index]
	set mod_file $tree/../beamnrc/spec_modules/$mod_base.module
	set inp_file_dir $tree
	set result [load_module]
	if $result==1 {
	    tk_dialog .edit "Loaded accelerator" "The accelerator\
		    $mod_base.module has been loaded." warning 0 OK
	    read_input
	} else {
	    set inp_file {}
	    set inp_base {}
	    set mod_file {}
	    set mod_base {}
	    set inp_file_dir {}
	}
    } else {
	read_input
    }
}


proc get_val { data varname i} {
    # procedure to get a number, with a way to deal with a comment with no comma
    global $varname

    set data [string trimleft $data]
    set data [string trimright $data]

    set indx [string first , $data]
    if { $indx<0 } {
	# There was no comma found.
	# Now see if there's a space anywhere in the remaining string.
	if [string first " " $data]>0 {
	    # A space was found.  Set indx to that position.
	    set indx [string first " " $data]
	} else {
	    # Just take the whole string as the number.
	    set indx [string length $data]
	}
    } else {
	# A comma was found, make sure it wasn't IN a comment.
	set spc [string first " " $data]
	if { $spc>-1 & $spc<$indx } {
	    # The space occurs BEFORE the comma, indicating a botch.
	    set indx [expr $spc+1]
	}
    }
    set val [string range $data 0 [incr indx -1] ]
#below is a special case for large NCASE
    if { $i==10 && $val>1000000000 && [string first . $val]<0 } {
          set val $val.
    }
    if [catch {set val [expr $val]}]==1 {
	set val 0
    }
    set data [string range $data [incr indx 2] [string length $data] ]
    set data [string trimleft $data]
    set ${varname}($i) $val

    return $data
}

proc get_str { data varname i} {
    # procedure to get a string, data holds the whole thing
    global $varname medium nmed nmednotfound

    set val [string trimleft $data]
    set val [string trimright $val]
    set ${varname}($i) $val

    # Check if str is in the array medium
    set found 0
    for {set i 0} {$i<=$nmed} {incr i} {
	if [string compare $val $medium($i)]==0 {
	    set found 1
	    break
	}
    }
    if $found==0 { incr nmednotfound }

    return $data
}

proc get_str_arr { data varname i} {
    # procedure to get an array of string values separated by a comma
    global $varname

    set data [string trimleft $data]
    set indx [string first , $data]
    if { $indx<0 } {
        # There was no comma found.  Trim whitespace off the end of the line.
        set data [string trimright $data]
        # Now see if there's a space anywhere in the remaining string.
        if [string first " " $data]>0 {
            # A space was found.  Set indx to that position.
            set indx [string first " " $data]
        } else {
            # Just take the whole string as the value
            set indx [string length $data]
        }
    }
    set val [string range $data 0 [incr indx -1] ]
    set val [string trimright $val]
    set data [string range $data [incr indx 2] [string length $data] ]
    set data [string trimleft $data]
    set ${varname}($i) $val

    return $data
}

proc read_input {} {

    global values numopts options monoen ifluor_opts srcopts Ein_val
    global ioutsp ioutsp_opts l_n_exc l_n_inc spec_file spcnam15
    global force_bdd exc inc num_cm s1opt s3opt spcnam21 spcnam31
    global inp_file inp_base cm_names cm_type cm_ident s9vals nmednotfound
    global src19input dbrem
    global level ibcmp_min ibcmp_max iphter_min iphter_max
    global iraylr_min iraylr_max iedgfl_min iedgfl_max got_egsnrc_input
    global isrc_dbs rsrc_dbs ssdsrc_dbs zsrc_dbs
    global arr nuser_inputs user_inputs
    global the_beam_code the_input_file the_pegs_file
    global alpha24 beta24 dist24 i_dsb splitcm_dsb dsb_delta

    # read an input file ** in the OLD FORMAT **

    set nmednotfound 0

    if [file exists $inp_file]==1 {
	set fileid [open $inp_file r]
	.inploaded configure -text "Accelerator parameters $inp_base loaded"
    } else {
        tk_dialog .edit "Sorry, try again..." "That input file doesn't exist! \
                Try again." warning 0 OK
	return
    }

    #media definition inputs for pegsless
    get_pegsless_inputs $fileid

    # FIRST DO THE MAIN INPUTS
    # FOR EACH, MUST SPLIT UP THE DATA STRING INTO NUMBERS

    #TITLE
    gets $fileid data; # READS ONE LINE FROM THE FILE
    set values(1) $data
    # check if the end of the title has "#!GUI1.0" from 81 to 88.
    # if so, generated by the GUI.  If not, issue a warning that this file
    # wasn't generated by the GUI, so if it's flawed it's your problem.
    set index [string first "#!GUI1.0" $values(1)]
    if $index>=0 {
        # Trim it off the end
        set values(1) [string range $values(1) 0 [expr $index-1] ]
    } else {
	tk_dialog .not_gui "Not GUI generated" "Warning!  This file was not\
		generated by the GUI.  If the input file format is old, you\
		may find that some parameters are incorrect.  Please check\
		all parameters carefully." warning 0 OK
    }
    # Trim to 80 chars
    set values(1) [string range $values(1) 0 80]

    # MATERIAL
    gets $fileid data
    set data [get_str $data values 2]

    gets $fileid data
    #   IWATCH,ISTORE,IRESTART,IO_OPT,IDAT,LATCH_OPTION,IZLAST
    for {set i 3} {$i <= 9} {incr i} {
	set data [get_val $data values $i]
	if { $i==3 & [expr $values($i)]<0 } {
	    set values(3) 5
	    set special_N [expr -$values($i)]
	} elseif { $i==5 & $values(5)==2 } {
	    set values(5) 0
	} elseif { $i==8 & $values(8)==0 } {
	    set values(8) 2
	}
	if [expr $numopts($i) > 0] {
	    set values($i) $options($i,$values($i))
	}
    }

    gets $fileid data
    #   NCASE,IXX,JXX,TIMMAX,IBRSPL,NBRSPL,IRRLTT,ICM_SPLIT
    foreach i {10 11 12 13 14 15 16 40} {
	set data [get_val $data values $i]
	if {$i==14} {
	    # Could be selective brem splitting (if = 29)
	    if $values($i)==29 {
		# selective brem no longer supported
                tk_dialog .error "Warning" "This input file specified selective bremsstrahlung splitting\
                    (IBRSPL=29), which is no longer supported.  Will be saved with no bremsstrahlung splitting\
                    unless modified." info 0 OK
		set values($i) $options($i,0)
		# have to get the next data line anyway so we can handle it
		gets $fileid selbrem
            # could be DBS
            } elseif $values($i)==2 {
                #DBS, order is a bit screwed up since SBS came first
                set values($i) $options($i,2)
                # get the next data line
                gets $fileid dirbrem
                for {set k 1} {$k<=6} {incr k} {
                    set indx [string first , $dirbrem]
                    if { $indx<0 } { set indx [string length $dirbrem] }
                    set dbrem($k) [string range $dirbrem 0 [incr indx -1] ]
                    set dirbrem [string range $dirbrem [incr indx 2]\
                            [string length $dirbrem] ]
                    set dirbrem [string trimleft $dirbrem]
                }
	    } else {
		set values($i) $options($i,$values($i))
	    }
	} elseif {$i==16} {
            #russian roulette, "on" is actually irrltt=2, but we must
            #also allow for irrltt=1
            if {$values($i)==1 || $values($i)==2} {
                set values($i) $options($i,1)
            } else {
                set values($i) $options($i,0)
            }
        } else {
            if [expr $numopts($i) > 0] {
                set values($i) $options($i,$values($i))
            }
        }
    }
    if [string compare $values(40) $options(40,0)]!=0 {
	# ICM_SPLIT>0, get the next line: NSPLIT_PHOT, NSPLIT_ELEC
	gets $fileid data
	set data [get_val $data values 40,phot]
	set data [get_val $data values 40,elec]
    }

    # READ SOURCE LINE
    gets $fileid data
    # here we have 6 numbers to read in.  The first is the charge.
    set data [get_val $data values 17]
    # make sure it's an integer first
    set values(17) [string trimright $values(17) "."]

    # if it's source 21's phase space thing, set it to 2 for option menu.
    # when writing file, it's reset to 9.
    if $values(17)>1 { set values(17) 2 }
    set values(17) $options(17,$values(17))
    # The second is the isource, followed
    # by the four source options.  Which parameters they are doesn't matter
    # until we put it up on the screen.
    set data [get_val $data values 18]
    # make sure it's an integer first (source number)
    set values(18) [expr [string trimright $values(18) "."]]
    if $values(18)==3 {set values(18) 2}
    if $values(18)==5 {set values(18) 3}
    if $values(18)==6 {set values(18) 4}
    if $values(18)==7 {set values(18) 5}
    if $values(18)==8 {set values(18) 6}
    if $values(18)==9 {set values(18) 7}
    if $values(18)==10 {set values(18) 8}
    if $values(18)==13 {set values(18) 9}
    if $values(18)==15 {set values(18) 10}
    if $values(18)==19 {set values(18) 11}
    if $values(18)==21 {set values(18) 12}
    if $values(18)==23 {set values(18) 13}
    if $values(18)==24 {set values(18) 14}
    if $values(18)==31 {set values(18) 15}
    # Now we have the source number.  If it's source 1, have to
    # worry about whether it's got a -ve rbeam, if so, it's circular.
    # also worry about source 3 for vertical or horizontal.

    # now get the source options
    for {set i 1} {$i <= 4} {incr i} {
	set data [get_val $data srcopts $i]
    }
    if {$values(18)==12 | $values(18)==14} {
	set srcopts(1) [expr int($srcopts(1))]
        #also get DBS stuff
        for {set i 1} {$i <= 4} {incr i} {
            set data [get_val $data arr $i]
        }
        set isrc_dbs $arr(1)
        set rsrc_dbs $arr(2)
        set ssdsrc_dbs $arr(3)
        set zsrc_dbs $arr(4)
    } elseif {$values(18)==13} {
        set data [get_val $data srcopts 5]
        set srcopts(1) [expr int($srcopts(1))]
        if {$srcopts(2)==1} {
           set isrc_dbs 1
        } else {
           set isrc_dbs 0
        }
        set alpha24 $srcopts(3)
        set beta24 $srcopts(4)
        set dist24 $srcopts(5)
    }
    set values(18) $options(18,$values(18))

    if [string compare "1 " [string range $values(18) 0 1]]==0 {
	# it's source 1
	if $srcopts(2)>=0 {
	    set s1opt "circular"
	} else {
	    set s1opt "rectangular"
            set srcopts(2) [expr abs($srcopts(2))]
	}
        #get other parameters
        for {set i 5} {$i <= 7} {incr i} {
           set data [get_val $data srcopts $i]
        }
    }
    if [string compare "3 " [string range $values(18) 0 1]]==0 {
	# it's source 3
	if $srcopts(2)>=0 {
	    set s3opt "vertical"
	} else {
	    set s3opt "horizontal"
	    set srcopts(2) [expr -$srcopts(2)]
	}
        #get dsb stuff
        for {set i 0} {$i < 3} {incr i} {
            set data [get_val $data arr $i]
        }
        set i_dsb $arr(0)
        set splitcm_dsb $arr(1)
        set dsb_delta $arr(2)
    }
    if [string compare "19" [string range $values(18) 0 1]]==0 {
	# it's source 19
        for {set i 5} {$i <= 6} {incr i} {
            set data [get_val $data srcopts $i]
        }
	if $srcopts(1)>=0 {
	    set src19input sigma
	} else {
	    set src19input fwhm
	    set srcopts(1) [expr -$srcopts(1)]
	}
        set srcopts(6) [expr abs($srcopts(6))]
        # in other words, the sign of the X-direction parameter
        # determines if both are in terms of sigma or FWHM
    }
    # if isource is 9, read a whole bunch of stuff...
    if [string compare $values(18) $options(18,7)]==0 {
	for {set i 1} {$i<=$srcopts(2)} {incr i} {
	    gets $fileid data
	    for {set j 1} {$j <= 3} {incr j} {
		set data [get_val $data s9vals $j,$i]
	    }
	}
    }
    # if isource is 15, read spcnam
    if [string compare $values(18) $options(18,10)]==0 {
	gets $fileid data
	set data [string trimleft $data]
	set data [string trimright $data]
	if [expr [string first , $data] > 0] {
	    set indx [string first , $data]
	    if { $indx<0 } { set indx [string length $data] }
	    set spcnam15 [string range $data 0 [incr indx -1] ]
	} else {
	    set spcnam15 $data
	}
    } else {
	set spcnam15 {}
    }
    # if isource is 21 or 24 read spcnam
    if {[string compare $values(18) $options(18,12)]==0 |
        [string compare $values(18) $options(18,14)]==0 } {
	gets $fileid data
	set data [string trimleft $data]
	set data [string trimright $data]
	if [expr [string first , $data] > 0] {
	    set indx [string first , $data]
	    if { $indx<0 } { set indx [string length $data] }
	    set spcnam21 [string range $data 0 [incr indx -1] ]
	} else {
	    set spcnam21 $data
	}
        if {[string compare $values(18) $options(18,14)]==0 } {
           # get rotational data
           gets $fileid data
           for {set i 1} {$i <= 3} {incr i} {
               set data [get_str_arr $data arr $i]
           }
           set alpha24 $arr(1)
           set beta24 $arr(2)
           set dist24 $arr(3)
        }
    } else {
	set spcnam21 {}
    }
    # if isource is 23, read the_beam_code, the_input_file, the_pegs_file
    if [string compare $values(18) $options(18,13)]==0 {
        gets $fileid data
        for {set i 1} {$i <= 3} {incr i} {
            set data [get_str_arr $data arr $i]
        }
        set the_beam_code $arr(1)
        set the_input_file $arr(2)
        set the_pegs_file $arr(3)
    } else {
        set the_beam_code {}
        set the_input_file {}
        set the_pegs_file {}
    }
    # if isource is 31, read spcnam
    if [string compare $values(18) $options(18,15)]==0 {
	gets $fileid data
	set data [string trimleft $data]
	set data [string trimright $data]
	if [expr [string first , $data] > 0] {
	    set indx [string first , $data]
	    if { $indx<0 } { set indx [string length $data] }
	    set spcnam31 [string range $data 0 [incr indx -1] ]
	} else {
	    set spcnam31 $data
	}
    } else {
	set spcnam31 {}
    }

    # if isource is < 21, read monoen (spectrum or mono)
    if { [string compare $values(18) $options(18,12)]!=0 & \
	    [string compare $values(18) $options(18,13)]!=0 &
            [string compare $values(18) $options(18,14)]!=0 &
             [string compare $values(18) $options(18,15)]!=0} {
	gets $fileid data
	set data [string trimleft $data]
	set data [string trimright $data]
	if [expr [string first , $data] > 0] {
	    set indx [string first , $data]
	    if { $indx<0 } { set indx [string length $data] }
	    set monoen [string range $data 0 [incr indx -1] ]
	    # trim off trailing whitespace by making it an integer
	    set monoen [expr $monoen]
	} else {
	    # trim off trailing whitespace by making it an integer
	    set data [expr $data]
	    set monoen $data
	}
	set monoen $options(monoen,$monoen)
    } else {
	set monoen {}
    }

    # if mono, read energy
    if [string match $monoen "monoenergetic"] {
	gets $fileid data
	if [expr [string first , $data] > 0] {
	    set indx [string first , $data]
	    if { $indx<0 } { set indx [string length $data] }
	    set Ein_val [string range $data 0 [incr indx -1] ]
	} else {
	    set Ein_val $data
	}
    } else {
	set Ein_val {}
    }
    # if spec, read filename
    if [string match $monoen "spectrum"] {
	gets $fileid data
	set data [string trimleft $data]
	set data [string trimright $data]
	if [expr [string first , $data] > 0] {
	    set indx [string first , $data]
	    if { $indx<0 } { set indx [string length $data] }
	    set spec_file [string range $data 0 [incr indx -1] ]
	} else {
	    set spec_file $data
	}
    } else {
	set spec_file {}
    }
    # if spectrum, read output type
    if [string match $monoen "spectrum"] {
	gets $fileid data
	set indx [string first , $data]
	if { $indx<0 } { set indx [string length $data] }
	set ioutsp [string range $data 0 [incr indx -1] ]
	set data [string range $data [incr indx 2] [string length $data] ]
	set data [string trimleft $data]
	# make sure it's an integer first
	set ioutsp [string trimright $ioutsp "."]
	set ioutsp $ioutsp_opts($ioutsp)
    } else {
	set ioutsp {}
    }

    # ECUT,PCUT,IREJCT_GLOBAL,ESAVE_GLOBAL
    gets $fileid data
    #strip off 2 dummy inputs
    for {set i 0} {$i < 2} {incr i} {
        set data [string range $data [expr [string first , $data]+1] [expr [string length $data]-1]]
    }
    #get ecut,pcut
    foreach i {ecut pcut} {
        set data [get_val $data values $i]
    }
    #strip off another dummy input
    set data [string range $data [expr [string first , $data]+1] [expr [string length $data]-1]]
    #now get irejct and esave_global
    for {set i 19} {$i <= 20} {incr i} {
	set data [get_val $data values $i]
        if {$i==19} {
           if {$values(19)==-1 | $values(19)==-2} {
          # the user has turned on augmented rr for DBS
             set dbrem(7) 1
             set values(19) [expr -$values(19)]
           }
        }
	if [expr $numopts($i) > 0] {
	    set values($i) $options($i,$values($i))
	}
    }

    # read photon forcing+bdds.
    gets $fileid data
    set data [get_val $data values 21]
    if [expr $numopts(21) > 0] {
	set values(21) $options(21,$values(21))
    }
    for {set i 1} {$i <= 4} {incr i} {
	set data [get_val $data force_bdd $i]
    }

    #READ SCORING PLANE INPUT
    #NSC_PLANES, (IPLANE_to_CM(I), I=1,NSC_PLANES)
    gets $fileid data
    set data [get_val $data values 22]

    for {set i 1} {$i <= $values(22) } {incr i} {
	set data [get_val $data values 22,1,$i]
    }

    # Repeat the next pair of lines for ISCORE=1,...,NSC_PLANES
    #     NSC_ZONES(ISCORE), MZONE_TYPE(ISCORE)
    # (RSCORE_ZONE(ISCORE,I), I=1,NSC_ZONES)  (only if NSC_ZONES(ISCORE)>0,
    #                                          up to 10/line)
    for {set i 1} {$i <= $values(22) } {incr i} {
	gets $fileid data
	set data [get_val $data values 22,2,$i]
	set data [get_val $data values 22,3,$i]
	if $values(22,3,$i)==0 {
	    set values(22,3,$i) "annular"
	} elseif $values(22,3,$i)==1 {
	    set values(22,3,$i) "square"
	} elseif $values(22,3,$i)==2 {
            set values(22,3,$i) "grid"
        }
        if {$values(22,3,$i)=="grid"} {
           gets $fileid data
           for {set j 1} {$j <=6} {incr j} {
              set data [get_val $data values 22,4,$i,$j]
           }
        } elseif {$values(22,2,$i)>0} {
	    set count $values(22,2,$i)
	    set low 1
	    while { $count>0 } {
		gets $fileid data
		if $count>10 {
		    set counter 10
		} else {
		    set counter $count
		}
		for {set j $low} {$j <= [expr $counter+$low]} {incr j} {
		    set data [get_val $data values 22,4,$i,$j]
		}
		incr count -10
		incr low 10
	    }
	}
    }

    #  ITDOSE_ON  = 0 (DEFAULT) only total dose is calculated
    #             = 1 total dose and dose components may be calculated
    # read photon forcing + bdds.
    gets $fileid data
    set data [get_val $data values 23]
    set values(23) $options(23,$values(23))

    # if itdose_on=1, get latch bit info
    if [string compare $values(23) $options(23,1)]==0 {
	# check if latch_option=1
	if [string compare $values(8) $options(8,1)]==0 {
	    tk_dialog .error "Warning" "This input file uses both non-inherited\
		    LATCH and contaminant dose scoring, which are\
		    incompatible." info 0 OK
	}
	# ICM_CONTAM, IQ_CONTAM (2I5)
	gets $fileid data
	set data [get_val $data values 23,1]
	# (23,2), contaminant type (photons or charged particles)
	set data [get_val $data values 23,2]
	if $values(23,2)==-1 {
	    tk_dialog .loaderror "Warning" "Contaminant dose was defined\
		    as -1 in input file. This is no longer allowed and it\
		    has been changed to 1 to represent charged\
		    particles." warning 0 OK
	    set values(23,2) $options(23,2,1)
	} else {
	    set values(23,2) $options(23,2,$values(23,2))
	}
	# LNEXC (I5)
	gets $fileid data
	set data [get_val $data values 23,3]
	# INITIALIZE EXC
	for { set i 1 } {$i<20} {incr i} {
	    for { set j 1 } {$j<=31} {incr j} {
		set exc($i,$j) {}
	    }
	}
	if [expr $values(23,3) > 0] {
	    # (L_N_EXC(I,J), J=1, 31 ) (31I5) (repeat LNEXC times, line by line)
	    for {set i 1} {$i <= $values(23,3)} {incr i} {
		gets $fileid data
		for {set j 1} {$j <= 31} {incr j} {
		    set data [get_val $data l_n_exc $i,$j]
		    set exc($i,$l_n_exc($i,$j)) 1
		}
	    }
	}
    }

    if [string compare $values(23) $options(23,1)]==0 {
	#     LNINC (I5)
	gets $fileid data
	set data [get_val $data values 23,4]

 # INITIALIZATION of INC --- Introduced by Palani Selvam, Carleton U
         for { set i 1 } {$i<20} {incr i} {
           for { set j 1 } {$j<=31} {incr j} {
         set inc($i,1,$j) {}
         set inc($i,2,$j) {}
            }
         }
       # End of initialization of INC --- by Palani

	if { $values(23,4)>0 } {
	    # (L_N_INC(I,J), J=1, 31 ) (31I5) (repeat LNINC times, line by line)
	    for {set i 1} {$i <= $values(23,4)} {incr i} {
		gets $fileid data
		set kind 1
		for {set j 1} {$j <= 31} {incr j} {
		    set data [get_val $data l_n_inc $i,$j]
		    if { [string compare $l_n_inc($i,$j) 0]==0 } {
			set kind 2
		    } else {
			set inc($i,$kind,$l_n_inc($i,$j)) 1
		    }
		}
	    }
	}
    }

    # Z_min_CM(1)  Z-coordinate of front surface for component module 1
    gets $fileid data
    set data [get_val $data values 24]

    for {set id 1} {$id<=$num_cm} {incr id} {
	set index $cm_type($id)
	init_$cm_names($index) $id
	read_$cm_names($index) $fileid $id
    }

    # now get the EGSnrc inputs
    foreach i {ecut pcut smaxir estepe ximax bca_algorithm skindepth_for_bca \
               transport_algorithm spin_effects ibrdst ibr_nist ibcmp iprdst\
               iphter iraylr iedgfl eii_flag photon_xsections comp_xsections\
               pair_nrc xsec_out} {
       get_egsnrc_parameter $i $fileid
    }

    if { $got_egsnrc_input(smaxir)==1 } {
    #we didn't get smax, set it depending on BCA and transport algorithm
       if { $values(bca_algorithm)=="EXACT" && \
            $values(transport_algorithm)=="PRESTA-II" } {
            set values(smaxir) 1e10
       }
    }

    #rejection plane inputs
    get_rejpln_inputs $fileid

    #BCSE inputs
    get_bcse_inputs $fileid

    #must have the delimiters :start user inputs: :stop user inputs:
    get_user_inputs $fileid

    catch {close $fileid}

    if $nmednotfound>0 {
	tk_dialog .nomed "Medium not found" "There were $nmednotfound\
		regions composed of a material not present in the PEGS4\
		data selected." info 0 OK
    }
}

proc get_egsnrc_parameter { iopt fileid } {
#procedure to read EGSnrc parameters from an input file
#must not fail if parameters aren't there
    global values inp_file got_egsnrc_input

    set got_egsnrc_input($iopt) 1

    if {$iopt=="ecut"} {
        set seekstring "global ecut"
    } elseif {$iopt=="pcut"} {
        set seekstring "global pcut"
    } elseif {$iopt=="smaxir"} {
        set seekstring "global smax"
    } elseif {$iopt=="estepe"} {
        set seekstring "estepe"
    } elseif {$iopt=="ximax"} {
        set seekstring "ximax"
    } elseif {$iopt=="bca_algorithm"} {
        set seekstring "boundary crossing algorithm"
    } elseif {$iopt=="skindepth_for_bca"} {
        set seekstring "skin depth for bca"
    } elseif {$iopt=="transport_algorithm"} {
        set seekstring "electron-step algorithm"
    } elseif {$iopt=="spin_effects"} {
        set seekstring "spin effects"
    } elseif {$iopt=="ibrdst"} {
        set seekstring "brems angular sampling"
    } elseif {$iopt=="ibr_nist"} {
        set seekstring "brems cross sections"
    } elseif {$iopt=="ibcmp"} {
        set seekstring "bound compton scattering"
    } elseif {$iopt=="iprdst"} {
        set seekstring "pair angular sampling"
    } elseif {$iopt=="iphter"} {
        set seekstring "photoelectron angular sampling"
    } elseif {$iopt=="iraylr"} {
        set seekstring "rayleigh scattering"
    } elseif {$iopt=="iedgfl"} {
        set seekstring "atomic relaxations"
    } elseif {$iopt=="eii_flag"} {
        set seekstring "electron impact ionization"
    } elseif {$iopt=="photon_xsections"} {
       set seekstring "photon cross sections"
    } elseif {$iopt=="pair_nrc"} {
       set seekstring "pair cross sections"
    } elseif {$iopt=="comp_xsections"} {
       set seekstring "compton cross sections"
    } elseif {$iopt=="xsec_out"} {
       set seekstring "photon cross-sections output"
    }

    catch {close $fileid}
    set fileid [open $inp_file r]

    while {[gets $fileid data]>-1} {
    #loop only ends when we're sure we have the line of data
       #switch data to lower case before = sign
       set eindex [string first "=" $data]
       set strlen [string length $data]
       if {$eindex>-1} {
         set data1 [string range $data 0 $eindex]
         set data2 [string range $data [expr $eindex+1] [expr $strlen-1]]
         set data1 [string tolower $data1]
         set data "$data1$data2"
       }
       set index [string first $seekstring $data]
       if {$index>-1} {
          #the seek string is on this line, make sure it is not preceeded
          #by #
          set cindex [string first "#" $data]
          if {$cindex==-1} {
             #no comment
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             if {$data!=""} {
               if {$iopt=="ecut" || $iopt=="pcut"} {
                set values(${iopt}) [max_nrc $values(${iopt}) $data]
               } else {
                set values(${iopt}) $data
               }
               set got_egsnrc_input($iopt) 0
             }
             break
          } elseif {$cindex > $index} {
             #comment after the data we need
             #strip off the comment
             set data [string range $data 0 [expr $cindex-1]]
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             if {$data!=""} {
               if {$iopt=="ecut" || $iopt=="pcut"} {
                set values(${iopt}) [max_nrc $values(${iopt}) $data]
               } else {
                set values(${iopt}) $data
               }
               set got_egsnrc_input($iopt) 0
             }
             break
          }
       }
    }


    if {$iopt=="ibcmp" || $iopt=="iphter" || $iopt=="iraylr" ||\
        $iopt=="iedgfl"} {
        if {$values(${iopt})=="On in regions" ||\
            $values(${iopt})=="Off in regions"} {
           if {$iopt=="ibcmp"} {
             set seekstron "bound compton start region"
             set seekstroff "bound compton stop region"
           } elseif {$iopt=="iphter"} {
             set seekstron "pe sampling start region"
             set seekstroff "pe sampling stop region"
           } elseif {$iopt=="iraylr"} {
             set seekstron "rayleigh start region"
             set seekstroff "rayleigh stop region"
           } elseif {$iopt=="iedgfl"} {
             set seekstron "relaxations start region"
             set seekstroff "relaxations stop region"
           }
           get_egsnrc_in_regions $iopt $seekstron min $fileid
           get_egsnrc_in_regions $iopt $seekstroff max $fileid
        }
    }

    if {$iopt=="iraylr" && $values(${iopt})=="custom"} {
        get_egsnrc_cust_rayl_data $fileid
    }
}

proc get_egsnrc_in_regions { iopt seekstring minmax fileid } {
#procedure to get upper/lower ranges of regions to turn on/off some of the
#egsnrc options
    global values inp_file
    global level ibcmp_min ibcmp_max iphter_min iphter_max
    global iraylr_min iraylr_max iedgfl_min iedgfl_max
    global templevel

    catch {close $fileid}
    set fileid [open $inp_file r]

    set data ""
    set templevel(${minmax}) 0

    while {[gets $fileid data]>-1} {
       #switch data to lower case before = sign
       set eindex [string first "=" $data]
       set strlen [string length $data]
       if {$eindex>-1} {
         set data1 [string range $data 0 $eindex]
         set data2 [string range $data [expr $eindex+1] [expr $strlen-1]]
         set data1 [string tolower $data1]
         set data "$data1$data2"
       }
       set index [string first $seekstring $data]
       if {$index>-1} {
         #the seek string is on this line, make sure it is not preceeded
         #by #
         set cindex [string first "#" $data]
         if {$cindex==-1} {
             #no comment
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             break
         } elseif {$cindex > $index} {
             #comment after the data we need
             #strip off the comment
             set data [string range $data 0 [expr $cindex-1]]
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             break
         }
       }
    }
    if {$data!=""} {
      #may have to concatenate several lines of input
      while {[string last , $data]==[expr [string length $data]-1]} {
        #should be another line of input
        gets $fileid data1
        set cindex [string first "#" $data1]
        if {$cindex>-1} {
          set data1 [string range $data1 0 [expr $cindex-1]]
        }
        set data1 [string trimleft $data1]
        set data1 [string trimright $data1]
        set char1 [string range $data1 0 0]
        if {$char1!="0" && $char1!="1" && $char1!="2" && $char1!="3" &&\
            $char1!="4" && $char1!="5" && $char1!="6" && $char1!="7" &&\
            $char1!="8" && $char1!="9"} {
            #this was the last line of start regions even though it
            #ended with a comma
            break
        } else {
           #otherwise, add data1 onto data
          set data "$data $data1"
        }
      }
      if {[string last , $data]!=[expr [string length $data]-1]} {
        #add a comma at the end to make the next loop easier
        set data "$data,"
      }
      while {[string first , $data]>-1} {
            incr templevel(${minmax})
            set cindex [string first , $data]
            set data1 [string range $data 0 [expr $cindex-1]]
            set data1 [string trimright $data1]
            set ${iopt}_${minmax}($templevel(${minmax})) $data1
            set strlen [string length $data]
            set data [string range $data [expr $cindex+1] [expr $strlen-1]]
            set data [string trimleft $data]
      }
    }
    if {$minmax=="max"} {
       #just in case the user entered a different number of start and stop
       #regions
       set level(${iopt}) [min_nrc $templevel(min) $templevel(max)]
    }
}

proc get_egsnrc_cust_rayl_data { fileid } {
#procedure to get media names and associated files specifying custom
#Rayleigh cross-sections
    global num_rayl_custom rayl_cust_med rayl_cust_file inp_file

    set i_cust_med 0
    set i_cust_file 0

  foreach seekstring {"ff media names" "ff file names"} {

    catch {close $fileid}
    set fileid [open $inp_file r]

    set data ""

    while {[gets $fileid data]>-1} {
       #switch data to lower case before = sign
       set eindex [string first "=" $data]
       set strlen [string length $data]
       if {$eindex>-1} {
         set data1 [string range $data 0 $eindex]
         set data2 [string range $data [expr $eindex+1] [expr $strlen-1]]
         set data1 [string tolower $data1]
         set data "$data1$data2"
       }
       set index [string first $seekstring $data]
       if {$index>-1} {
         #the seek string is on this line, make sure it is not preceeded
         #by #
         set cindex [string first "#" $data]
         if {$cindex==-1} {
             #no comment
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             break
         } elseif {$cindex > $index} {
             #comment after the data we need
             #strip off the comment
             set data [string range $data 0 [expr $cindex-1]]
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             break
         }
       }
    }
    if {$data!=""} {
      #may have to concatenate several lines of input
      while {[string last , $data]==[expr [string length $data]-1]} {
        #should be another line of input
        gets $fileid data1
        set cindex [string first "#" $data1]
        if {$cindex>-1} {
          set data1 [string range $data1 0 [expr $cindex-1]]
        }
        set data1 [string trimleft $data1]
        set data1 [string trimright $data1]
        if {$data1=="" || [string first "#" $data1]==0 \
            || [string first "=" $data]>-1} {
            #this was the last line of start regions even though it
            #ended with a comma
            break
        } else {
           #otherwise, add data1 onto data
          set data "$data $data1"
        }
      }
    }
    if {[string last , $data]!=[expr [string length $data]-1]} {
        #add a comma at the end to make the next loop easier
        set data "$data,"
    }
    while {[string first , $data]>-1} {
            set cindex [string first , $data]
            set data1 [string range $data 0 [expr $cindex-1]]
            set data1 [string trimright $data1]
            if {$seekstring=="ff media names"} {
                incr i_cust_med
                set rayl_cust_med($i_cust_med) $data1
            } elseif {$seekstring=="ff file names"} {
                incr i_cust_file
                set rayl_cust_file($i_cust_file) $data1
            }
            set strlen [string length $data]
            set data [string range $data [expr $cindex+1] [expr $strlen-1]]
            set data [string trimleft $data]
    }
  }
    if {$i_cust_med != $i_cust_file} {
        tk_dialog .custerr "User Input Warning" "In input file $inp_file\
                    the number of media for which custom Rayleigh data is to\
                    be supplied does not mach the number of custom Rayleigh\
                    data files specified." warning 0 OK
        set num_rayl_custom [max_nrc $i_cust_med $i_cust_file]
    } else {
        set num_rayl_custom $i_cust_med
    }
}

proc get_rejpln_inputs { fileid } {
#procedure to get use_rejpln, z_rejpln, rejection plane parameters
#for use with DBS
  global dbrem inp_file


  foreach seekstring {"use a rejection plane" \
        "z(cm) from zero reference plane"} {

     catch {close $fileid}
     set fileid [open $inp_file r]

     set data ""

     while {[gets $fileid data]>-1} {
       #switch data to lower case before = sign
       set eindex [string first "=" $data]
       set strlen [string length $data]
       if {$eindex>-1} {
         set data1 [string range $data 0 $eindex]
         set data2 [string range $data [expr $eindex+1] [expr $strlen-1]]
         set data1 [string tolower $data1]
         set data "$data1$data2"
       }
       set index [string first $seekstring $data]
       if {$index>-1} {
         #the seek string is on this line, make sure it is not preceeded
         #by #
         set cindex [string first "#" $data]
         if {$cindex==-1} {
             #no comment
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             break
         } elseif {$cindex > $index} {
             #comment after the data we need
             #strip off the comment
             set data [string range $data 0 [expr $cindex-1]]
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             break
         }
       }
     }
     if {$data!=""} {
           if {$seekstring=="use a rejection plane"} {
             set data [string tolower $data]
             if {$data=="on"} {
                set dbrem(use_rejpln) 1
             } else {
                set dbrem(use_rejpln) 0
             }
           } elseif {$seekstring=="z(cm) from zero reference plane"} {
             set dbrem(z_rejpln) $data
           }
     }
  }
}

proc get_bcse_inputs { fileid } {
#procedure to get BCSE-related inputs, On/Off, medium no. and
#enhancement factor
  global bcse_meds bcse_constant medused medindex values options
  global num_bcse_meds inp_file bcse_power bcse_rr

  #get all lines between :Start BCSE: and :Stop BCSE:

  catch {close $fileid}
  set fileid [open $inp_file r]

  set i_bcse_meds 0

  set start_bcse_inputs 0
  set nbcse_inputs 0
  while {[gets $fileid data]>-1} {
      set data1 [string tolower $data]
      set sindex [string first ":start bcse:" $data1]
      set eindex [string first ":stop bcse:" $data1]
      if {$eindex>-1} {
        break
      } elseif {$sindex>-1} {
        set start_bcse_inputs 1
      } elseif {$start_bcse_inputs==1} {
        incr nbcse_inputs
        set bcse_inputs($nbcse_inputs) "$data"
      }
  }
  if {$eindex<=-1 && $start_bcse_inputs==1} {
      # did not find the end delimiter
      set nbcse_inputs 0
      tk_dialog .userinp "BCSE Input Error" "In input file $inp_file\
                    bremsstrahlung cross-section enhancement (BCSE) inputs\
                    are indicated\
                    with the :Start BCSE:\
                    delimiter, but there is no matching :Stop BCSE:\
                    delimiter.  The values in the file will not be available\
                    in the GUI." warning 0 OK
  }


  foreach seekstring {"use bcse" "media to enhance"\
        "enhancement constant" "enhancement power"\
        "russian roulette"} {

     set data ""
     set ibcse_inputs 1

     while {$ibcse_inputs<=$nbcse_inputs} {
       set data $bcse_inputs($ibcse_inputs)
       #switch data to lower case before = sign
       set eindex [string first "=" $data]
       set strlen [string length $data]
       if {$eindex>-1} {
         set data1 [string range $data 0 $eindex]
         set data2 [string range $data [expr $eindex+1] [expr $strlen-1]]
         set data1 [string tolower $data1]
         set data "$data1$data2"
       }
       set index [string first $seekstring $data]
       if {$index>-1} {
         #the seek string is on this line, make sure it is not preceeded
         #by #
         set cindex [string first "#" $data]
         if {$cindex==-1} {
             #no comment
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             break
         } elseif {$cindex > $index} {
             #comment after the data we need
             #strip off the comment
             set data [string range $data 0 [expr $cindex-1]]
             #strip off the seek string
             set eindex [string first "=" $data]
             set strlen [string length $data]
             set data [string range $data [expr $eindex+1] [expr $strlen-1]]
             #strip off leading and trailing blanks
             set data [string trimleft $data]
             set data [string trimright $data]
             break
         }
       }
       incr ibcse_inputs
     }
     if {$data!=""} {
       if {$seekstring=="use bcse"} {
          set data [string tolower $data]
          if {$data=="on"} {
                set values(41) $options(41,1)
          } else {
                set values(41) $options(41,0)
          }
       } elseif {$seekstring=="russian roulette"} {
          set data [string tolower $data]
          if {$data=="on"} {
                set bcse_rr $options(16,1)
          } else {
                set bcse_rr $options(16,0)
          }
       } elseif {$seekstring=="enhancement constant"} {
          set bcse_constant $data
       } elseif {$seekstring=="enhancement power"} {
          set bcse_power $data
       } elseif {$seekstring=="media to enhance"} {
          incr ibcse_inputs
          while {[string last , $data]==[expr [string length $data]-1] &&\
                 $ibcse_inputs<=$nbcse_inputs} {
             #should be another line of input
             set data1 $bcse_inputs($ibcse_inputs)
             set cindex [string first "#" $data1]
             if {$cindex>-1} {
               set data1 [string range $data1 0 [expr $cindex-1]]
             }
             set data1 [string trimleft $data1]
             set data1 [string trimright $data1]
             if {$data1=="" || [string first "#" $data1]==0 \
                 || [string first "=" $data1]>-1} {
                 #this was the last line of start regions even though it
                 #ended with a comma
                 break
             } else {
                 #otherwise, add data1 onto data
                 set data "$data $data1"
             }
             incr ibcse_inputs
          }
          if {[string last , $data]!=[expr [string length $data]-1]} {
             #add a comma at the end to make the next loop easier
             set data "$data,"
          }
          while {[string first , $data]>-1} {
             set cindex [string first , $data]
             set data1 [string range $data 0 [expr $cindex-1]]
             set data1 [string trimright $data1]
             incr i_bcse_meds
             set bcse_meds($i_bcse_meds) $data1
             set strlen [string length $data]
             set data [string range $data [expr $cindex+1] [expr $strlen-1]]
             set data [string trimleft $data]
          }
       }
     }
  }
  set num_bcse_meds $i_bcse_meds
}


proc get_user_inputs { fileid } {

global nuser_inputs user_inputs inp_file

#procedure to get any custom user inputs between the delimiters
#:start user inputs:, :stop user inputs:

    catch {close $fileid}
    set fileid [open $inp_file r]

    set start_user_inputs 0
    set nuser_inputs 0
    while {[gets $fileid data]>-1} {
      set data1 [string tolower $data]
      set sindex [string first ":start user inputs:" $data1]
      set eindex [string first ":stop user inputs:" $data1]
      if {$eindex>-1} {
        break
      } elseif {$sindex>-1} {
        set start_user_inputs 1
      } elseif {$start_user_inputs==1} {
        incr nuser_inputs
        set user_inputs($nuser_inputs) "$data"
      }
    }

    if {$eindex<=-1 && $start_user_inputs==1} {
      # did not find the end delimiter
      set nuser_inputs 0
      tk_dialog .userinp "User Input Error" "In input file $inp_file\
                    you have indicated\
                    custom user inputs with the :Start user inputs:\
                    delimiter, but there is no matching :Stop user inputs:\
                    delimiter.  These inputs will not be written out when\
                    the input file is saved." warning 0 OK
    } elseif {$nuser_inputs>0} {
      tk_dialog .userinp "User Inputs Read" "Custom user inputs in\
                    input file $inp_file\
                    have been read in.\
                    You will not have access\
                    to these inputs in the GUI, but they will be output\
                    verbatim when the input file is saved." warning 0 OK
    }
}
