
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: create file
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


proc create_file { file } {
    global values inp_file numopts options srcopts Ein CTdataflag medium medsur
    global spec_file imax ivox gvox dsurround
    global inc exc names mvox izvox PhantFileName latbit nbit1 nbit2 dflagopt
    global grouping nrow izrow enflag numsrcopts izopts ncpu dflag parnum
    global numthphi angfixed ang1 ang2 nang pang ivary thphidef
    global numsets iso1 iso2 iso3 ang1 ang2 ang3 dsource muI
    global iphant iphspout imuphspout
    global level ibcmp_min ibcmp_max iphter_min iphter_max
    global iraylr_min iraylr_max iedgfl_min iedgfl_max
    global i_dbs r_dbs ssd_dbs z_dbs the_beam_code the_input_file the_pegs_file
    global esplit
    global the_vcu_code the_phsp_file the_vcu_input_file i_MLC
    global num_rayl_custom rayl_cust_med rayl_cust_file

    # OUTPUT AN INPUT FILE

    # INCLUDES A CHECKER TO MAKE SURE THAT ALL VARIABLES HAVE BEEN SET
    # WHEN REQUIRED.

    set inp_file [string trimleft $file]

    set file [open $inp_file w]

    if { [string compare $values(1) {}]==0 } {
	error_flag "the title"
#	return
    }
    set data $values(1)
    set len [string length $data]
    if $len<80 {
	for {set i $len} {$i<=80} {incr i} {
	    set data "$data "
	}
    } else {
	set data [string range $data 0 80]
    }
    # now append #!GUI1.0 to the end
    set data "$data#!GUI1.0"
    puts $file $data

    if $CTdataflag==0 {
	# non-CT data
	if [catch {set val [expr $values(2)]}]==1 {
	    error_flag "the number of media"
#	    return
	}
	puts $file $values(2)

	# record 3
	for {set i 1} {$i<=$values(2)} {incr i} {
	    puts $file $medium($i)
	}
	# record 4
	set str "$values(ecut), $values(pcut),"
        #put zeroes for the rest of the inputs
	for {set i 1} {$i<=$values(2)} {incr i} {
	    set str "$str 0,"
	}
	puts $file "$str 0"

	# record 5
	for {set j 0} {$j<3} {incr j} {
	    if $grouping($j)==1 {
		set x$j -$imax($j)
	    } else {
		set x$j $imax($j)
	    }
	}
	puts $file "$x0, $x1, $x2, $iphant"

	# record 6 etc...
	for {set j 0} {$j<3} {incr j} {
	    if $grouping($j)==1 {
		# output gvox
		puts $file $gvox($j,0)
		for {set i 1} {$i<=$imax($j)} {incr i} {
		    puts $file "$gvox($j,$i,1), $gvox($j,$i,2)"
		}
	    } else {
		# output ivox
		for {set i 0} {$i<=$imax($j)} {incr i} {
		    puts $file $ivox($j,$i)
		}
	    }
	}
	# record 7, medium and density
	for {set i 1} {$i<=$nrow} {incr i} {
	    set str {}
	    set str "$str$mvox($i,0,0), $mvox($i,0,1), $mvox($i,1,0), "
	    set str "$str$mvox($i,1,1), $mvox($i,2,0), $mvox($i,2,1), "
            set mednum {}
            for {set j 0} {$j<=$values(2)} {incr j} {
                if [string compare $medium($j) $mvox($i,0)]==0 {
                    set mednum $j
                    break
                }
            }
	    set str "$str$mednum, $mvox($i,1)"
	    puts $file $str
	}
	# end with a blank entry
	puts $file "0, 0, 0, 0, 0, 0, 0, 0"

	# record 8, blank entry
	puts $file "0, 0, 0, 0, 0, 0, 0, 0"

	# record 9, izscan, max20
	if [string compare $values(27) $options(27,1)]==0 {
	    set max20val 1
	} else {
	    set max20val 0
	}
	set zeros 1
	for {set i 1} {$i<=$izrow} {incr i} {
	    set str {}
	    set str "$str$izvox($i,0,0), $izvox($i,0,1), $izvox($i,1,0), "
	    set str "$str$izvox($i,1,1), $izvox($i,2,0), $izvox($i,2,1), "
	    if [string compare $izvox($i) $izopts(0)]==0 {
		set izscan 0
	    } elseif [string compare $izvox($i) $izopts(1)]==0 {
		set izscan 1
	    } else {
	        set izscan 2
            }
	    set str "$str$izscan, $max20val"
	    puts $file $str
	    if {$izvox($i,0,0)==0 & $izvox($i,0,1)==0 & $izvox($i,1,0)==0 & \
		    $izvox($i,1,1)==0 & $izvox($i,2,0)==0 & $izvox($i,2,1)==0} {
		#This is the last entry, do not write a line of zeros
		set zeros 0; break;
	    }
	}
	# end with a blank entry
	if $zeros==1 {
	    puts $file "0, 0, 0, 0, 0, 0, 0, $max20val"
	}
    } else {
	# CT data input
	# number of media is zero for this option
	puts $file 0

	puts $file $PhantFileName
	puts $file "$values(ecut), $values(pcut), 0"

	set str {}
	for {set i 25} {$i<=27} {incr i} {
	    for {set j 0} {$j<$numopts($i)} {incr j} {
		if { [string compare $values($i) $options($i,$j)]==0 } {
		    set val $j
		}
	    }
	    if { [catch {expr $val}]==1 } {
		# AN ERROR HAS OCCURRED => STRING, NOT NUMBER
		error_flag "$names($i)"
#		return
	    }
	    set str "$str$val, "
	}
	puts $file $str
    }

    # source and source options
    # charge
    for {set j -1} {$j<=2} {incr j} {
	if { [string compare $values(3) $options(3,$j)]==0 } {
	    set str "$j, "
	}
    }
    # source number
    foreach j {0 1 2 3 4 6 7 8 9 10 20 21} {
	if { [string compare $values(4) $options(4,$j)]==0 } {
	    set str "$str$j, "
	    set isource $j
	}
    }
    if {$isource==2} {
      for {set j 0} {$j<7} {incr j} {
          set str "$str$srcopts($j), "
      }
      set str "$str$i_dbs, $r_dbs, $ssd_dbs, $z_dbs, $esplit"
      puts $file $str
    } elseif {$isource<7} {
      # just output the source options
      for {set j 0} {$j<8} {incr j} {
	set str "$str$srcopts($j), "
      }
      puts $file $str
    } elseif {$isource==7 || $isource==8 || $isource==10} {
      #output the coordinates of the isocenter
      for {set j 0} {$j<3} {incr j} {
        set str "$str$srcopts($j), "
      }
      #now output number of theta-phi pairs or groups of pairs
      if {$thphidef==0} {
        set str "$str$numthphi, "
      } elseif {$thphidef==1} {
        set str "$str-$numthphi, "
      }
      #output the rest of the source options
      if {$isource==7} {
        for {set j 3} {$j<8} {incr j} {
          set str "$str$srcopts($j), "
        }
      } elseif {$isource==8} {
        for {set j 3} {$j<5} {incr j} {
          set str "$str$srcopts($j), "
        }
        set str "$str$i_dbs, $r_dbs, $ssd_dbs, $z_dbs, $esplit"
      } elseif {$isource==10} {
        for {set j 3} {$j<5} {incr j} {
          set str "$str$srcopts($j), "
        }
        set str "$str$i_dbs, $esplit"
      }
      puts $file $str
      #now output the actual theta-phi pairs or groups of pairs
      for {set i 1} {$i<=$numthphi} {incr i} {
        if {$thphidef==0} {
          set str "$ang1($i), $ang2($i), $pang($i)"
        } elseif {$thphidef==1} {
set str "$ivary($i), $angfixed($i), $ang1($i), $ang2($i), $nang($i), $pang($i)"
        }
        puts $file $str
      }
    } elseif {$isource==9} {
      for {set j 0} {$j<7} {incr j} {
          set str "$str$srcopts($j), "
      }
      set str "$str$i_dbs, $esplit"
      puts $file $str
    } elseif {$isource==20} {
	#first output number of settings
 	set str "$str$numsets, "
	#for {set j 0} {$j<1} {incr j} {
	#    set str "$str$srcopts($j), "
	#}
	set str "$str$i_dbs, $r_dbs, $ssd_dbs, $z_dbs, $esplit, $imuphspout"
      puts $file $str
	#set str "$str$i_dbs, $esplit"
	#puts $file $str
     #now output the actual settings
	for {set i 1} {$i<=$numsets} {incr i} {
	    set str "$iso1($i), $iso2($i), $iso3($i), $ang1($i), $ang2($i), $ang3($i), $dsource($i), $muI($i)"
	    puts $file $str
      }
    } elseif {$isource==21} {
	#first output number of settings
 	set str "$str$numsets, "
	#for {set j 0} {$j<1} {incr j} {
	#    set str "$str$srcopts($j), "
	#}
	set str "$str$i_dbs, $esplit, $imuphspout"
	puts $file $str
     #now output the actual settings
	for {set i 1} {$i<=$numsets} {incr i} {
	    set str "$iso1($i), $iso2($i), $iso3($i), $ang1($i), $ang2($i), $ang3($i), $dsource($i), $muI($i)"
	    puts $file $str
      }
    }

    # enflag, mode, medsur, dsurround(1), dflag, dsurround(2,3,4)

    for {set i 1} {$i<=4} {incr i} {
        if [catch {expr $dsurround($i)}]==1 { set dsurround($i) {} }
    }

    set str "$enflag, "
    foreach j {0 2} {
	if { [string compare $values(20) $options(20,$j)]==0 } {
	    set str "$str$j, "
	}
    }
    set val 0
    for {set j 1} {$j<=$values(2)} {incr j} {
	if { [string compare $medsur $medium($j)]==0 } {
	    set val $j
	}
    }
    set str "$str$val, $dsurround(1), "
    if { [string compare $dflag $dflagopt(0)]==0 } {
	set val 0
    } else {
	set val 1
    }
    set str "$str$val, $dsurround(2), $dsurround(3), $dsurround(4)"

    puts $file $str

    if $enflag==0 {
	puts $file $Ein
    } else {
        if {$isource==9 || $isource==10} {
          set spec_file "$the_beam_code,$the_input_file,$the_pegs_file"
        }
	if {$isource==20} {
	    if {$i_MLC==1} {
		set spec_file "$the_vcu_code,$the_phsp_file,$the_vcu_input_file"
	    } else {
		set spec_file "0,$the_phsp_file,0"
	    }
        }
	if {$isource==21} {
	    if {$i_MLC==1} {
		set spec_file "$the_beam_code,$the_input_file,$the_pegs_file,$the_vcu_code,$the_vcu_input_file"
	    } else {
		set spec_file "$the_beam_code,$the_input_file,$the_pegs_file,0,0"
	    }
	}
	puts $file $spec_file
    }


    if $enflag==3 {
	set str {}
	for {set j 0} {$j<$numopts(21)} {incr j} {
	    if { [string compare $values(21) $options(21,$j)]==0 } {
		set str "$str$j, "
		set i_bit_filter $j
	    }
	}
	set str "$str$nbit1, $nbit2"
	puts $file $str
	set str {}
	for {set i 1} {$i<=$nbit1} {incr i} {
	    set str "$str$latbit($i), "
	}
	puts $file $str
	if { $i_bit_filter==0 && $nbit2>0 } {
	    set str {}
	    for {set i [expr $nbit1+1]} {$i<=[expr $nbit1+$nbit2]} {incr i} {
		set str "$str$latbit($i), "
	    }
	    puts $file $str
	}
    }

    # ncase, iwatch, ETC...

    set str {}
    for {set i 5} {$i<=16} {incr i} {
	if $numopts($i)>0 {
	    if $i==6 {
		foreach j {0 1 2 4} {
		    if { [string compare $values($i) $options($i,$j)]==0 } {
			set val $j
		    }
		}
	    } else {
		for {set j 0} {$j<$numopts($i)} {incr j} {
		    if { [string compare $values($i) $options($i,$j)]==0 } {
			set val $j
		    }
		}
	    }
	} else {
            set val $values($i)
	}
        if { $i==16 } {
          if { $isource==2 || $isource==8 } {
            if { [catch {expr $val}]==1 } {
                # AN ERROR HAS OCCURRED => STRING, NOT NUMBER
                error_flag "$names($i)"
            }
          }
        } elseif { $i!=15 } {
	    if { [catch {expr $val}]==1 } {
		# AN ERROR HAS OCCURRED => STRING, NOT NUMBER
		error_flag "$names($i)"
	    }
	}
	set str "$str$val, "
    }
    if { [catch {expr $ncpu}]==1 } {
	set ncpu 0
    }
    if { [catch {expr $parnum}]==1 } {
	set parnum 0
    }
    set str "$str$ncpu, $parnum, $values(18), "
    if { [string compare $values(ihowfarless) $options(ihowfarless,1)]==0 } {
      set val 1
    } else {
      set val 0
    }
    set str "$str$val, $iphspout"

    puts $file $str

    # EGSnrc inputs
    puts $file " #########################"
    puts $file " :Start MC Transport Parameter:"
    puts $file " "
    puts $file " Global ECUT= $values(ecut)"
    puts $file " Global PCUT= $values(pcut)"
    puts $file " Global SMAX= $values(smaxir)"
    puts $file " ESTEPE= $values(estepe)"
    puts $file " XIMAX= $values(ximax)"
    puts $file " Boundary crossing algorithm= $values(bca_algorithm)"
    puts $file " Skin depth for BCA= $values(skindepth_for_bca)"
    puts $file " Electron-step algorithm= $values(transport_algorithm)"
    puts $file " Spin effects= $values(spin_effects)"
    puts $file " Brems angular sampling= $values(ibrdst)"
    puts $file " Brems cross sections= $values(ibr_nist)"
    puts $file " Bound Compton scattering= $values(ibcmp)"
    if {$values(ibcmp)==$options(ibcmp,2) || $values(ibcmp)==$options(ibcmp,3)} {
        #on or off in regions, output the start regions
       set putintro 0
       set str ""
       for {set i 1} {$i<=$level(ibcmp)} {incr i} {
         set str "$str $ibcmp_min($i),"
         if {[string length $str]>=50 || $i==$level(ibcmp)} {
           if {$i==$level(ibcmp)} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " Bound Compton start region=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
       #output the stop regions
       set putintro 0
       set str ""
       for {set i 1} {$i<=$level(ibcmp)} {incr i} {
         set str "$str $ibcmp_max($i),"
         if {[string length $str]>=50 || $i==$level(ibcmp)} {
           if {$i==$level(ibcmp)} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " Bound Compton stop region=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
    }
    puts $file " Compton cross sections= $values(comp_xsections)"
    puts $file " Pair angular sampling= $values(iprdst)"
    puts $file " Pair cross sections= $values(pair_nrc)"
    puts $file " Photoelectron angular sampling= $values(iphter)"
    if {$values(iphter)==$options(iphter,2) || $values(iphter)==$options(iphter,3)} {
       #on or off in regions, output the start regions
       set putintro 0
       set str ""
       for {set i 1} {$i<=$level(iphter)} {incr i} {
         set str "$str $iphter_min($i),"
         if {[string length $str]>=50 || $i==$level(iphter)} {
           if {$i==$level(iphter)} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " PE sampling start region=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
       #output the stop regions
       set str ""
       set putintro 0
       for {set i 1} {$i<=$level(iphter)} {incr i} {
         set str "$str $iphter_max($i),"
         if {[string length $str]>=50 || $i==$level(iphter)} {
           if {$i==$level(iphter)} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " PE sampling stop region=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
    }
    puts $file " Rayleigh scattering= $values(iraylr)"
    if {$values(iraylr)==$options(iraylr,2) || $values(iraylr)==$options(iraylr,3)} {
       #on or off in regions, output the start regions
       set str ""
       set putintro 0
       for {set i 1} {$i<=$level(iraylr)} {incr i} {
         set str "$str $iraylr_min($i),"
         if {[string length $str]>=50 || $i==$level(iraylr)} {
           if {$i==$level(iraylr)} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " Rayleigh start region=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
       #output the stop regions
       set str ""
       set putintro 0
       for {set i 1} {$i<=$level(iraylr)} {incr i} {
         set str "$str $iraylr_max($i),"
         if {[string length $str]>=50 || $i==$level(iraylr)} {
           if {$i==$level(iraylr)} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " Rayleigh stop region=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
    } elseif {$values(iraylr)==$options(iraylr,4)} {
       #put media and associated files of custom Rayleigh cs data
       set str ""
       set putintro 0
       for {set i 1} {$i<=$num_rayl_custom} {incr i} {
         set str "$str $rayl_cust_med($i),"
         if {[string length $str]>=50 || $i==$num_rayl_custom} {
           if {$i==$num_rayl_custom} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " ff media names=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
       set str ""
       set putintro 0
       for {set i 1} {$i<=$num_rayl_custom} {incr i} {
         set str "$str $rayl_cust_file($i),"
         if {[string length $str]>=50 || $i==$num_rayl_custom} {
           if {$i==$num_rayl_custom} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " ff file names=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
    }
    puts $file " Atomic relaxations= $values(iedgfl)"
    if {$values(iedgfl)==$options(iedgfl,2) || $values(iedgfl)==$options(iedgfl,3)} {
       #on or off in regions, output the start regions
       set str ""
       set putintro 0
       for {set i 1} {$i<=$level(iedgfl)} {incr i} {
         set str "$str $iedgfl_min($i),"
         if {[string length $str]>=50 || $i==$level(iedgfl)} {
           if {$i==$level(iedgfl)} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " Relaxations start region=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
       #output the stop regions
       set str ""
       set putintro 0
       for {set i 1} {$i<=$level(iedgfl)} {incr i} {
         set str "$str $iedgfl_max($i),"
         if {[string length $str]>=50 || $i==$level(iedgfl)} {
           if {$i==$level(iedgfl)} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " Relaxations stop region=$str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
    }
    puts $file " Electron impact ionization= $values(eii_flag)"
    #if { $values(photon_xsections)!=$options(photon_xsections,0) } {
      puts $file " Photon cross sections= $values(photon_xsections)"
    #}
    puts $file " Photon cross-sections output= $values(xsec_out)"
    puts $file " "
    puts $file " :Stop MC Transport Parameter:"
    puts $file " #########################"

    write_pegsless_data $file

    close $file
}


proc error_flag { text } {
    tk_dialog .edit "Bad inputs" "You haven't defined $text.  Please\
	    fill in all options required before saving." warning 0 OK
}


