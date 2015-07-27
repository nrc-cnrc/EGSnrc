
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: create file
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
    global values inp_file numopts options monoen ifluor_opts srcopts Ein_val
    global ioutsp ioutsp_opts l_n_exc l_n_inc spec_file spcnam15 force_bdd
    global inc exc names special_N s1opt num_cm cm_names cm_type
    global spcnam21 spcnam31 cm_ident s9vals src19input s3opt
    global level ibcmp_min ibcmp_max iphter_min iphter_max
    global iraylr_min iraylr_max iedgfl_min iedgfl_max dbrem
    global isrc_dbs rsrc_dbs ssdsrc_dbs zsrc_dbs
    global nuser_inputs user_inputs
    global num_rayl_custom rayl_cust_med rayl_cust_file
    global num_bcse_meds bcse_meds bcse_constant bcse_rr bcse_power
    global the_beam_code the_input_file the_pegs_file
    global alpha24 beta24 dist24 i_dsb splitcm_dsb dsb_delta

    # OUTPUT AN INPUT FILE ** in the OLD FORMAT **

    set inp_file [string trimleft $file]

    set file [open $inp_file w]

    set data $values(1)
    set len [string length $data]
    if [string compare $values(1) {}]==0 {
	tk_dialog .save "Warning" "You haven't input a title.  \
		This is an important identifier." warning 0 OK
    }
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

    puts $file "$values(2)"

    # IWATCH ETC...
    set str {}
    set i 3
    set val {}
    foreach j {0 1 2 4 5} {
	if { [string compare $values(3) $options(3,$j)]==0 } {
	    set val $j
	}
    }
    if $val==5 { set val "-$special_N" }
    set str "$str$val, "

    for {set i 4} {$i<=9} {incr i} {
	set val {}
	if $i==8 {
	    # LATCH OPTION (1,2,3)
	    for {set j 1} {$j<=$numopts($i)} {incr j} {
		if { [string compare $values($i) $options($i,$j)]==0 } {
		    set val $j
		}
	    }
	} elseif $i==5 {
	    # IRESTART option (0,1,3,4)
	    foreach j {0 1 3 4} {
		if { [string compare $values($i) $options($i,$j)]==0 } {
		    set val $j
		}
	    }
	} elseif $i==4 {
	    # ISTORE option (-1,0,1)
	    for {set j -1} {$j<=1} {incr j} {
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
	set str "$str$val, "
    }
    set str "$str IWATCH ETC."
    puts $file $str

    # NCASE ETC...
    set str {}
    foreach i {10 11 12 13 14 15 16 40} {
	set val {}
	if {$numopts($i)>0} {
	    for {set j 0} {$j<$numopts($i)} {incr j} {
		if { [string compare $values($i) $options($i,$j)]==0 } {
		    set val $j
		}
	    }
	} else {
	    set val $values($i)
	}
	if $i==14 {
            set dirbrem 0
	    if $val==2 {
		set dirbrem 1
	    }
	} elseif $i==16 {
            if $val==1 {
                set val 2
            }
        }
	set str "$str$val, "
    }
    set str "$str NCASE ETC."
    puts $file $str

    if {$dirbrem==1} {
        #output DBS options
        set str {}
        for {set i 1} {$i<=6} {incr i} {
            set str "$str$dbrem($i), "
        }
        puts $file "$str DIRECTIONAL BREM OPTIONS"
    }

    if [string compare $values(40) "none"]!=0 {
	puts $file "$values(40,phot), $values(40,elec), NSPLIT"
    }

    # SOURCE AND SOURCE OPTIONS
    set str {}
    # E,PHOTON,POSITRON:
    set val {}
    for {set j -1} {$j<=2} {incr j} {
	if { [string compare $values(17) $options(17,$j)]==0 } {
	    set val $j
	}
    }
    if [string compare $val {}]==0 {
	tk_dialog .save "Warning" "You haven't input the charge of the\
		source.  BEAMnrc will not run without this." warning 0 OK
    }

    catch { if $val==2 { set val 9 } }
    set str "$str$val, "

    # ISOURCE
    set isource {}
    for {set j 0} {$j<$numopts(18)} {incr j} {
	if { [string compare $values(18) $options(18,$j)]==0 } {
	    set isource [string range $options(18,$j) 0 1]
	}
    }
    if [string compare $isource {}]==0 {
	tk_dialog .save "Warning" "You haven't input a source type.  \
		BEAMnrc will not run without this." warning 0 OK
    }
    # isource is the manual's numbering, not the GUI's (not chronological)
    catch { set isource [expr $isource] }

    set str "$str$isource, "

    # SOURCE OPTIONS
    if { $isource==19 & [string compare "fwhm" $src19input]==0 } {
	set str "$str-$srcopts(1), "
    } else {
	set str "$str$srcopts(1), "
    }
    if { $isource==1 & [string compare "rectangular" $s1opt]==0 } {
        if {$srcopts(2)>0} {
	   set str "$str-$srcopts(2), "
        } elseif {$srcopts(2)==0} {
           set str "$str-1, "
        } else {
           set str "$str$srcopts(2), "
        }
    } elseif { $isource==3 & [string compare "horizontal" $s3opt]==0 } {
	set str "$str-$srcopts(2), "
    } elseif { $isource==23 } {
        set str "$str$isrc_dbs, "
        set srcopts(3) $alpha24
        set srcopts(4) $beta24
        set srcopts(5) $dist24
    } else {
	set str "$str$srcopts(2), "
    }
    set str "$str$srcopts(3), "
    set str "$str$srcopts(4), "
    if {$isource==21 | $isource==24} {
      set str "$str$isrc_dbs, $rsrc_dbs, $ssdsrc_dbs, $zsrc_dbs, "
    } elseif {$isource==1} {
      set str "$str$srcopts(5), $srcopts(6), $srcopts(7), ,"
    } elseif {$isource==3} {
      set str "$str$i_dsb, $splitcm_dsb, $dsb_delta, ,"
    } elseif {$isource==19} {
      if {[string compare "fwhm" $src19input]==0 } {
         set str "$str$srcopts(5), -$srcopts(6), 0.0, 0.0, "
      } else {
         set str "$str$srcopts(5), $srcopts(6), 0.0, 0.0, "
      }
    } elseif {$isource==23} {
      set str "$str$srcopts(5), 0.0, 0.0, 0.0, "
    } else {
      set str "$str 0.0, 0.0, 0.0, 0.0, "
    }
    set str "$str IQIN, ISOURCE + OPTIONS"
    puts $file $str
    if { $isource==9 } {
	for {set j 1} {$j<=$srcopts(2)} {incr j} {
	    set str {}
	    for {set i 1} {$i<=3} {incr i} {
		set str "$str$s9vals($i,$j), "
	    }
	    puts $file "$str SOURCE 9 OPTIONS"
	}
    }
    if { $isource==15 } {
	puts $file $spcnam15
    }
    if { $isource==21 | $isource==24} {
	puts $file $spcnam21
    }
    if { $isource==23 } {
        puts $file "$the_beam_code, $the_input_file, $the_pegs_file"
    }
    if { $isource==24 } {
        puts $file "$alpha24, $beta24, $dist24, ALPHA24, BETA24, DIST24"
    }
    if { $isource==31 } {
	puts $file $spcnam31
    }
    if { $isource<21} {
	if [string compare $monoen "spectrum"]==0 {
	    puts $file "1, SPECTRUM"
	} else {
	    puts $file "0, MONOENERGETIC"
	}
	if [string compare $monoen "spectrum"]==0 {
	    puts $file $spec_file
	    if [string compare $ioutsp "no spectrum data in output summary"]==0 {
		puts $file 0
	    } else {
		puts $file 1
	    }
	} else {
	    puts $file $Ein_val
	}
    }

    # range rejection inputs
    set str {}
    set val {}
    for {set i 19} {$i<=20} {incr i} {
        set val {}
        if {$numopts($i)>0} {
            for {set j 0} {$j<$numopts($i)} {incr j} {
                if { [string compare $values($i) $options($i,$j)]==0 } {
                    set val $j
                }
            }
        } else {
            set val $values($i)
        }
        if {$i==19 && $dbrem(7)==1 && $val > 0} {
         #user wants augmented range rejection with DBS
            set val -$val
        }
        set str "$str$val, "
    }
    #repeat ecut,pcut in this input as well--should match ecut and pcut
    #in EGSnrc input section
    set str "0, 0, $values(ecut), $values(pcut), 0, $str 0 "

    set str "$str, ECUT,PCUT,IREJCT,ESAVE"
    puts $file $str

    # PHOTON FORCING
    set str {}
    set val {}
    for {set j 0} {$j<$numopts(21)} {incr j} {
	if { [string compare $values(21) $options(21,$j)]==0 } {
	    set val $j
	}
    }
    set str "$str$val, "
    for {set j 1} {$j<=4} {incr j} {
	set str "$str$force_bdd($j), "
    }
    set str "$str PHOTON FORCING"
    puts $file $str

    # SCORING INPUT
    set str "$values(22), "
    for {set i 1} {$i<=$values(22)} {incr i} {
	set str "$str$values(22,1,$i), "
    }
    set str "$str SCORING INPUT"
    puts $file $str

    for {set i 1} {$i<=$values(22)} {incr i} {
        if { [string compare $values(22,3,$i) "grid"]==0 } {
            set val 2
	} elseif { [string compare $values(22,3,$i) "square"]==0 } {
	    set val 1
	} else {
	    set val 0
	}
        if { [string compare $values(22,3,$i) "grid"]==0 } {
            puts $file "6,$val"
            set str {}
            for {set j 1} {$j<=6} {incr j} {
              set str "$str$values(22,4,$i,$j), "
            }
            puts $file $str
        } else {
         if {[string compare $values(22,2,$i) ""]==0 || $values(22,2,$i)<=0} {
            puts $file "0,$val"
         } elseif {$values(22,2,$i)>0} {
            set str "$values(22,2,$i), "
            set str "$str$val"
	    puts $file $str
	    # RSCORE_ZONE, 10 PER LINE, ONLY IF NSC_ZONES>0
	    if $values(22,2,$i)<11 {
		set str {}
		for {set j 1} {$j<=$values(22,2,$i)} {incr j} {
		    set str "$str$values(22,4,$i,$j), "
		}
	    } else {
		set count 0
		set str {}
		for {set j 1} {$j<=$values(22,2,$i)} {incr j} {
		    incr count
		    if $count>10 {
			puts $file $str
			set str {}
			set count 0
		    }
		    set str "$str$values(22,4,$i,$j), "
		}
	    }
	    puts $file $str
	 }
        }
    }

    # DOSE COMPONENTS
    set str {}
    set val {}
    for {set j 0} {$j<$numopts(23)} {incr j} {
	if { [string compare $values(23) $options(23,$j)]==0 } {
	    set val $j
	}
    }
    set str "$str$val, "
    set str "$str DOSE COMPONENTS"
    puts $file $str
    # IF IT_DOSE_ON=1, GET LATCH BIT INFO
    if [string compare $values(23) $options(23,1)]==0 {
	# ICM_CONTAM, IQ_CONTAM (2I5)
	set str {}
	set str "$str$values(23,1), "
	set val {}
	for {set j 0} {$j<=1} {incr j} {
	    if { [string compare $values(23,2) $options(23,2,$j)]==0 } {
		set val $j
	    }
	}
	set str $str$val
	puts $file $str
	# LNEXC (I5)
        if {[string compare $values(23,3) ""]==0 || $values(23,3)<=0} {
             puts $file "0"
        } elseif {$values(23,3)>0} {
	    puts $file $values(23,3)
	    # L_N_EXC
	    for {set i 1} {$i<=$values(23,3)} {incr i} {
		set str {}
		for {set j 1} {$j<=31} {incr j} {
		    if { $exc($i,$j)==1 } {
			set str "$str$j, "
		    }
		}
		puts $file $str
	    }
	}
	# LNINC (I5)
        if {[string compare $values(23,4) ""]==0 || $values(23,4)<=0} {
             puts $file "0"
        } elseif {$values(23,4)>0} {
	    puts $file $values(23,4)
	    # L_N_INC
	    for {set i 1} {$i<=$values(23,4)} {incr i} {
		set str1 {}
		set str2 {}
		for {set j 1} {$j<=31} {incr j} {
		    if { $inc($i,1,$j)==1 } {
			set str1 "$str1$j, "
		    }
		    if { $inc($i,2,$j)==1 } {
			set str2 "$str2$j, "
		    }
		}
		if [string compare $str2 ""]==0 {
		    set str $str1
		} else {
		    set str "${str1}0, $str2"
		}
		puts $file $str
	    }
	}
    }

    # Z TO FRONT
    puts $file "$values(24), Z TO FRONT FACE"

    for {set id 1} {$id<=$num_cm} {incr id} {
	set index $cm_type($id)
	# DUMMY LINE
	puts $file "*********** start of CM $cm_names($index) with identifier $cm_ident($id) ***********"
	write_$cm_names($index) $file $id
    }
    puts $file "*********************end of all CMs*****************************"
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
    if {$dbrem(use_rejpln)==1 || $dbrem(z_rejpln)!=""} {
       puts $file " :Start DBS rejection plane:"
       puts $file " "
       if {$dbrem(use_rejpln)==1} {
         puts $file " Use a rejection plane= On"
       } else {
         puts $file " Use a rejection plane= Off"
       }
       puts $file " Z(cm) from zero reference plane= $dbrem(z_rejpln)"
       puts $file " "
       puts $file " :Stop DBS rejection plane:"
       puts $file " #########################"
    }
    if {$values(41)==$options(41,1) || $num_bcse_meds>0 ||\
        $bcse_constant!="" || $bcse_power!=""} {
       puts $file " :Start BCSE:"
       puts $file " "
       if {$values(41)==$options(41,1)} {
         puts $file " Use BCSE= On"
       } else {
         puts $file " Use BCSE= Off"
       }
       set str ""
       set putintro 0
       for {set i 1} {$i<=$num_bcse_meds} {incr i} {
         set str "$str $bcse_meds($i),"
         if {[string length $str]>=50 || $i==$num_bcse_meds} {
           if {$i==$num_bcse_meds} {
              set index [string last "," $str]
              set str [string range $str 0 [expr $index-1]]
           }
           if {$putintro==0} {
              puts $file " Media to enhance= $str"
              incr putintro
           } else {
              puts $file "$str"
           }
           set str ""
         }
       }
       puts $file " Enhancement constant= $bcse_constant"
       puts $file " Enhancement power= $bcse_power"
       puts $file " Russian Roulette= $bcse_rr"
       puts $file " "
       puts $file " :Stop BCSE:"
       puts $file " #########################"
    }
    for {set i 1} {$i<=$nuser_inputs} {incr i} {
       if {$i==1} {
         puts $file " :Start user inputs:"
       }
       puts $file "$user_inputs($i)"
       if {$i==$nuser_inputs} {
         puts $file " :Stop user inputs:"
         puts $file " #########################"
       }
    }

    write_pegsless_data $file

    close $file
}


