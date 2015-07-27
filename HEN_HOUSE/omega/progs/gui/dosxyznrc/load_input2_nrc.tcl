
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: load input
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
#                   Ernesto Mainegra-Hing
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


proc load_input {} {
    # if an input file is already loaded, ask if they want to discard it.
    # if OK, start browser at $start_dir

    global start_dir inp_file

    set result 0
    if [string compare $inp_file ""]!=0 {
    # There's an input file already loaded.  Ask if they
    # want to discard it.
    set result [tk_dialog .okay "Discard?" "The input file\
        currently loaded will be discarded.  Are you sure?" \
        question 0 Yes No  ]
    }
    if $result==0 {
    # Okay, discard it.
        reset_parameters
    set inp_file {}
    .inploaded configure -text {}
    query_filename set_inp_file "$start_dir" egsinp

    # query_filename will get a filename and directory tree
    # and send them to set_inp_file...
    }
}

proc set_inp_file {tree filen} {
    # set the input file and call read_input to read the file.

    global inp_file inp_base inp_file_dir

    set inp_file [file join $tree $filen]
    set inp_base [file rootname $filen]
    set inp_file_dir $tree
    read_input

    # Enable the Edit Parameters option on the main menubar by destroying and
    # re-creating it.
    destroy .mbar.file.menu
    menu .mbar.file.menu
    .mbar.file.menu add command -label "Start a new input file" \
	    -command "reset_parameters; edit_parameters"
    .mbar.file.menu add command -label "Load a previous input file" \
	    -command "load_input"
    .mbar.file.menu add command -label "Edit parameters"\
	    -command "edit_parameters"
    .mbar.file.menu add command -label "Save input parameters" \
            -command "overwrite_input"
    .mbar.file.menu add command -label "Save input parameters as..." \
	    -command "save_input"
    .mbar.file.menu add separator
    .mbar.file.menu add command -label "Change PEGS4 file" \
	    -command "get_pegs4file"
    .mbar.file.menu add separator
    .mbar.file.menu add command -label "Exit" -command "exit_prompt"
}

proc get_val { data varname i} {
    # procedure to get a number, with a way to deal with a comment with no comma
    global $varname

    set indx [string first , $data]
    if { $indx<0 } {
	# There was no comma found.  Trim whitespace off the end of the line.
	set data [string trimright $data]
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
	set data [string trimright $data]
	set spc [string first " " $data]
	if { $spc>-1 & $spc<$indx } {
	    # The space occurs BEFORE the comma, indicating a botch.
	    set indx $spc
	}
    }
    set val [string range $data 0 [incr indx -1] ]
    if [catch {set val [expr $val]}]==1 {
	set val 0
    }
    set data [string range $data [incr indx 2] [string length $data] ]
    set data [string trimleft $data]
    set ${varname}($i) $val

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

proc get_str { data varname i} {
    # procedure to get a string, data holds the whole thing
    global $varname

    set val [string trimleft $data]
    set val [string trimright $val]
    set ${varname}($i) $val

    return $data
}

proc read_input {} {

    global values numopts options inp_file inp_base CTdataflag medium
    global ecutin pcutin smax estepm imax ivox gvox mvox izvox parnum
    global nrow izrow PhantFileName srcopts spec_file Ein enflag dsurround
    global medsur latbit inc exc nbit1 nbit2 izopts grouping ncpu dflag dflagopt
    global arr phspout phspoutopt
    global thphidef angfixed ang1 ang2 pang ivary numthphi nang
    global numsets nset iso1 iso2 iso3 ang1 ang2 ang3 dsource muI
    global iphant imuphspout
    global level ibcmp_min ibcmp_max iphter_min iphter_max
    global iraylr_min iraylr_max iedgfl_min iedgfl_max got_egsnrc_input
    global i_dbs r_dbs ssd_dbs z_dbs the_beam_code the_input_file the_pegs_file
    global the_vcu_code the_phsp_file the_vcu_input_file i_MLC
    global esplit

    # read an input file

    if [file exists $inp_file]==1 {
	set fileid [open $inp_file r]
	.inploaded configure -text "Input parameters $inp_base loaded"
    } else {
        tk_dialog .edit "Sorry, try again..." "That file doesn't exist! \
                Please try again." error 0 OK
	return
    }

    #media definition inputs for pegsless
    get_pegsless_inputs $fileid

    # FIRST DO THE MAIN INPUTS

    # "gets $fileid data" READS ONE LINE FROM THE FILE
    # FOR EACH, MUST SPLIT UP $data INTO NUMBERS (unless text)
    # the numbering of values follows that in the manual

    #TITLE
    gets $fileid data
    set values(1) $data
    # check if the end of the title has "#!GUI1.0" from 81 to 88.
    # if so, generated by the GUI.  If not, issue a warning that this file
    # wasn't generated by the GUI, so if it's flawed it's your fault.
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

    gets $fileid data
    set data [get_val $data values 2]

    if $values(2)>0 {
	# non-CT data input
	set CTdataflag 0

	# read in the media
	for {set i 1} {$i<=$values(2)} {incr i} {
	    gets $fileid data
	    set data [get_str $data medium $i]
	}

	# ecut, pcut, rest are dummies
	gets $fileid data
	set data [get_val $data arr 0]
	set values(ecut) $arr(0)

	set data [get_val $data arr 0]
	set values(pcut) $arr(0)

	# imax
	gets $fileid data
	for {set j 0} {$j<3} {incr j} {
	    set data [get_val $data imax $j]
	}
        set data [get_val $data arr 0]
        set iphant $arr(0)

	# read geometries:
	for {set j 0} {$j<3} {incr j} {
	    if $imax($j)<0 {
		# input in groups
		set imax($j) [expr -$imax($j)]
		set grouping($j) 1
		# minimum x boundary
		gets $fileid data
		set data [get_val $data gvox $j,0]
		for {set i 1} {$i<=$imax($j)} {incr i} {
		    # read in the wodth and number of voxels for group i
		    gets $fileid data
		    set data [get_val $data gvox $j,$i,1]
		    set data [get_val $data gvox $j,$i,2]
		}
	    } else {
		# read in individually
		set grouping($j) 0
		for {set i 0} {$i<=$imax($j)} {incr i} {
		    gets $fileid data
		    set data [get_val $data ivox $j,$i]
		}
	    }
	}

	# read in medium and density for groups of voxels, mvox
	# until a line with the first two values zero is reached (blank):
	set loop 1
	set nrow 0
	while {$loop==1} {
	    gets $fileid data
	    for {set i 1} {$i<=8} {incr i} {
		set data [get_val $data arr $i]
	    }
	    if {$arr(1)==0 && $arr(2)==0 } {
		set loop 0
	    } else {
		incr nrow
		set mvox($nrow,0,0) $arr(1)
		set mvox($nrow,0,1) $arr(2)
		set mvox($nrow,1,0) $arr(3)
		set mvox($nrow,1,1) $arr(4)
		set mvox($nrow,2,0) $arr(5)
		set mvox($nrow,2,1) $arr(6)
		set mvox($nrow,0) $medium($arr(7))
		set mvox($nrow,1) $arr(8)
	    }
	}

	# read a blank record, required for disabled ecutl,pcutl
	gets $fileid data

	# same for izscan, max20
	set values(27) $options(27,0)
	set loop 1
	set izrow 0
	while {$loop==1} {
	    gets $fileid data
	    for {set i 1} {$i<=8} {incr i} {
		set data [get_val $data arr $i]
	    }
	    if {$arr(1)==0 && $arr(2)==0 } {
		set loop 0
		if $arr(8)==1 {
		    # max20 is on
		    set values(27) $options(27,1)
		} else {
		    set values(27) $options(27,0)
		}
	    } else {
		incr izrow
		set izvox($izrow,0,0) $arr(1)
		set izvox($izrow,0,1) $arr(2)
		set izvox($izrow,1,0) $arr(3)
		set izvox($izrow,1,1) $arr(4)
		set izvox($izrow,2,0) $arr(5)
		set izvox($izrow,2,1) $arr(6)
		set izvox($izrow) $izopts([expr int($arr(7))])
		if $arr(8)==1 { set values(27) $options(27,1) }
	    }
	}
	# done for the non-CT data input
    } else {
	# CT-data input
	set CTdataflag 1

	gets $fileid data
	set PhantFileName $data

	gets $fileid data
	set data [get_val $data arr 0]
	set values(ecut) $arr(0)
	set data [get_val $data arr 0]
	set values(pcut) $arr(0)

	# zeroairdose, doseprint, max20 (values(25-27))
	gets $fileid data
	for {set i 25} {$i<=27} {incr i} {
	    set data [get_val $data values $i]
	    set values($i) $options($i,$values($i))
	}
    }

    # read source information
    gets $fileid data

    # here we have 10 numbers to read in.  The first is the charge.
    set data [get_val $data values 3]
    set values(3) [string trimright $values(3) "."]
    set values(3) $options(3,$values(3))

    # The second is the isource
    set data [get_val $data values 4]
    set values(4) [string trimright $values(4) "."]
    set isource $values(4)
    if [catch {set values(4) $options(4,$values(4))}]==1 {
	tk_dialog .inputerr "Input error" "The source number selected, \
		$values(4), is not a valid selection.  Input failed."\
		error 0 OK
	return
    }

    if {$isource==2} {
      for {set i 0} {$i<7} {incr i} {
        set data [get_val $data srcopts $i]
      }
      #now get the dbs stuff
      for {set i 0} {$i<4} {incr i} {
        set data [get_val $data arr $i]
      }
      set i_dbs $arr(0)
      set r_dbs $arr(1)
      set ssd_dbs $arr(2)
      set z_dbs $arr(3)
      # get esplit value
      set data [get_val $data arr 0]
      set esplit $arr(0)
    } elseif {$isource<7} {
      # Last the 8 source options, all numbers
      for {set i 0} {$i<8} {incr i} {
	set data [get_val $data srcopts $i]
      }
    } elseif {$isource==7 || $isource==8 || $isource==10} {
      #get the coordinates of the isocenter
      for {set i 0} {$i<3} {incr i} {
        set data [get_val $data srcopts $i]
      }
      #now get number of theta-phi pairs or groups of theta-phi pairs
      set data [get_val $data arr 0]
      set numthphi $arr(0)
      if {$numthphi<0} {
         set thphidef 1
         set numthphi [expr abs($numthphi)]
      } else {
         set thphidef 0
      }
      #get the rest of the inputs
      if {$isource==7} {
        for {set i 3} {$i<8} {incr i} {
          set data [get_val $data srcopts $i]
        }
      } elseif {$isource==8} {
        for {set i 3} {$i<5} {incr i} {
          set data [get_val $data srcopts $i]
        }
        #get dbs inputs
        for {set i 0} {$i<4} {incr i} {
          set data [get_val $data arr $i]
        }
        set i_dbs $arr(0)
        set r_dbs $arr(1)
        set ssd_dbs $arr(2)
        set z_dbs $arr(3)
        # get esplit value
        set data [get_val $data arr 0]
        set esplit $arr(0)
      } elseif {$isource==10} {
        for {set i 3} {$i<5} {incr i} {
          set data [get_val $data srcopts $i]
        }
        #get dbs inputs
        set data [get_val $data arr 0]
        set i_dbs $arr(0)
        # get esplit value
        set data [get_val $data arr 0]
        set esplit $arr(0)
      }
      #now get the actual angles/groups
      for {set i 1} {$i<=$numthphi} {incr i} {
        gets $fileid data
        if {$thphidef==0} {
           for {set j 0} {$j < 3} {incr j} {
              set data [get_val $data arr $j]
           }
           set ang1($i) $arr(0)
           set ang2($i) $arr(1)
           set pang($i) $arr(2)
        } elseif {$thphidef==1} {
           for {set j 0} {$j < 6} {incr j} {
              set data [get_val $data arr $j]
           }
           set ivary($i) $arr(0)
           set angfixed($i) $arr(1)
           set ang1($i) $arr(2)
           set ang2($i) $arr(3)
           set nang($i) $arr(4)
           set pang($i) $arr(5)
        }
      }
    } elseif {$isource==9} {
      for {set i 0} {$i<7} {incr i} {
        set data [get_val $data srcopts $i]
      }
      #now get the dbs stuff
      set data [get_val $data arr 0]
      set i_dbs $arr(0)
      # get esplit value
      set data [get_val $data arr 0]
      set esplit $arr(0)
    } elseif {$isource==20} {
      #first get number of settings
	  set data [get_val $data arr 0]
	  set numsets $arr(0)
	  #now get the rest of the inputs (dsource is now dynamic)
	  #for {set i 0} {$i<1} {incr i} {
	  #    set data [get_val $data srcopts $i]
	  #}
	  #now get the dbs stuff
	  for {set i 0} {$i<4} {incr i} {
          set data [get_val $data arr $i]
      }
	  set i_dbs $arr(0)
      set r_dbs $arr(1)
      set ssd_dbs $arr(2)
      set z_dbs $arr(3)
	  #get esplit value
	  set data [get_val $data arr 0]
	  set esplit $arr(0)
      #get imuphspout

      set data [get_val $data arr 0]
      set imuphspout $arr(0)
      if {$imuphspout>1 || $imusphspout<0} { set imuphspout 0 }
 	  #now get the actual settings
      for {set i 1} {$i<=$numsets} {incr i} {
	  gets $fileid data
	  for {set j 0} {$j < 8} {incr j} {
              set data [get_val $data arr $j]
	  }
	  set iso1($i) $arr(0)
	  set iso2($i) $arr(1)
	  set iso3($i) $arr(2)
	  set ang1($i) $arr(3)
	  set ang2($i) $arr(4)
	  set ang3($i) $arr(5)
	  set dsource($i) $arr(6)
	  set muI($i) $arr(7)
      }
    } elseif {$isource==21} {
	#first get number of settings
	set data [get_val $data arr 0]
	set numsets $arr(0)
	#now get the rest of the inputs (dsource is now dynamic)
	#for {set i 0} {$i<1} {incr i} {
	#    set data [get_val $data srcopts $i]
	#}
	#now get the dbs stuff
	set data [get_val $data arr 0]
	set i_dbs $arr(0)
	#get esplit value
	set data [get_val $data arr 0]
	set esplit $arr(0)
        #get imuphspout
        set data [get_val $data arr 0]
        set imuphspout $arr(0)
        if {$imuphspout>1 || $imuphspout<0} { set imuphspout 0 }
	#now get the actual settings
      for {set i 1} {$i<=$numsets} {incr i} {
	  gets $fileid data
	  for {set j 0} {$j < 8} {incr j} {
              set data [get_val $data arr $j]
	  }
	  set iso1($i) $arr(0)
	  set iso2($i) $arr(1)
	  set iso3($i) $arr(2)
	  set ang1($i) $arr(3)
	  set ang2($i) $arr(4)
	  set ang3($i) $arr(5)
	  set dsource($i) $arr(6)
	  set muI($i) $arr(7)
      }
    }

    # read enflag, mode(values(20)), medsur,dsurround, dflag
    gets $fileid data
    for {set i 1} {$i<=8} {incr i} {
	set data [get_val $data arr $i]
    }
    set enflag $arr(1)
    set values(20) $options(20,$arr(2))
    set medsur $medium($arr(3))
    set dsurround(1) $arr(4)
    set dflag $dflagopt($arr(5))
    set dsurround(2) $arr(6)
    set dsurround(3) $arr(7)
    set dsurround(4) $arr(8)

    if $enflag==0 {
	gets $fileid data
	set data [get_val $data arr 0]
	set Ein $arr(0)
	if $Ein==0 { set Ein "" }
    } else {
	gets $fileid spec_file
        if {$isource==9 || $isource==10} {
          for {set i 0} {$i<=2} {incr i} {
            set spec_file [get_str_arr $spec_file arr $i]
          }
          set the_beam_code $arr(0)
          set the_input_file $arr(1)
          set the_pegs_file $arr(2)
        }
	if {$isource==20} {
          for {set i 0} {$i<=2} {incr i} {
            set spec_file [get_str_arr $spec_file arr $i]
          }
          set the_vcu_code $arr(0)
	  set the_phsp_file  $arr(1)
          set the_vcu_input_file $arr(2)
	  if {"$the_vcu_code" != "0"} {
	      set i_MLC 1
	  } else {
	      set i_MLC 0
	  }
        }
	if {$isource==21} {
          for {set i 0} {$i<=4} {incr i} {
            set spec_file [get_str_arr $spec_file arr $i]
          }
          set the_beam_code $arr(0)
          set the_input_file $arr(1)
          set the_pegs_file $arr(2)
	  set the_vcu_code $arr(3)
          set the_vcu_input_file $arr(4)
	  if {"$the_vcu_code" != "0"} {
	      set i_MLC 1
	  } else {
	      set i_MLC 0
	  }
        }
    }

    if $enflag==3 {
	# get the latch bit filter(s)
	# first i_bit_filter(values(21)), nbit1, nbit2
	gets $fileid data
	for {set i 1} {$i<=3} {incr i} {
	    set data [get_val $data arr $i]
	}
	set values(21) $options(21,$arr(1))
	set nbit1 $arr(2)
	set nbit2 $arr(3)

	# now read latbit, up to nbit1 (arr(1) is i_bit_filter)
	gets $fileid data
	for {set i 1} {$i<=$nbit1} {incr i} {
	    set data [get_val $data latbit $i]
	}
	# from this, create inc, exc.
	for {set i 1} {$i<29} {incr i} {
	    set inc($i) 0
	    set exc($i) 0
	}
	if $arr(1)==0 {
	    if $nbit2>0 {
		# read the other half:
		gets $fileid data
		for {set i [expr $nbit1+1]} {$i<=[expr $nbit2+$nbit2]} {incr i} {
		    set data [get_val $data latbit $i]
		}
	    }
	    for {set i 1} {$i<=$nbit1} {incr i} {
		set inc($latbit($i)) 1
	    }
	    for {set i [expr $nbit1+1]} {$i<=[expr $nbit1+$nbit2]} {incr i} {
		set exc($latbit($i)) 1
	    }
	} elseif $arr(1)==1 {
	    # just exclusive
	    for {set i 1} {$i<=$nbit1} {incr i} {
		set exc($latbit($i)) 1
	    }
	} elseif $arr(1)==2 {
	    # just inclusive for regions
	    for {set i 1} {$i<=$nbit1} {incr i} {
		set inc($latbit($i)) 1
	    }
	} elseif $arr(1)==2 {
	    # just exclusive for regions
	    for {set i 1} {$i<=$nbit1} {incr i} {
		set exc($latbit($i)) 1
	    }
	}
    }

    # ncase etc:
    gets $fileid data
    for {set i 5} {$i <= 15} {incr i} {
	set data [get_val $data values $i]
	if [expr $numopts($i) > 0] {
	    set values($i) $options($i,$values($i))
	}
    }

    # 16, NRCYCL
    set data [get_val $data values $i]

    # for 17, iparallel, ncpu is the value and 17 is on/off
    set data [get_val $data arr 0]
    set ncpu $arr(0)
    if $ncpu>1 {
	set values(17) $options(17,1)
    } else {
	set values(17) $options(17,0)
    }
    # now for parnum
    set data [get_val $data arr 0]
    set parnum $arr(0)
    # and n_split, account for older input files
    if { $data != "" } {
       set data [get_val $data arr 0]
       set values(18) $arr(0)
    }
    # and ihowfarless
    set data [get_val $data arr 0]
    if {$arr(0)==1} {
       set values(ihowfarless) $options(ihowfarless,1)
    } else {
       set values(ihowfarless) $options(ihowfarless,0)
    }
    # phsp output options
    set data [get_val $data arr 0]
    set iphspout 0
    if {$arr(0)<=2} { set iphspout $arr(0) }

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

    catch {close $fileid}

    # now edit/show them:
    edit_parameters
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
