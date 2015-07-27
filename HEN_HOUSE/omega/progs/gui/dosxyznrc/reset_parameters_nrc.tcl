
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: reset parameters
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


proc reset_parameters { } {
    global values inp_file numopts options srcopts Ein CTdataflag medium medsur
    global spec_file ecutin pcutin estepm smax imax ivox gvox dsurround
    global inc exc names mvox izvox PhantFileName latbit nbit1 nbit2
    global grouping nrow izrow enflag numsrcopts izopts maxvals
    global dflag dflagopt iphspout imuphspout
    global numthphi angfixed ang1 ang2 nang pang ivary thphidef
    global numsets iso1 iso2 iso3 ang1 ang2 ang3 dsource muI
    global iphant level the_beam_code the_input_file the_pegs_file
    global num_rayl_custom rayl_cust_med rayl_cust_file
    global the_vcu_code the_vcu_input_file

    # Reset the paramters to start a new input file.

    set inp_file {}
    .inploaded configure -text $inp_file

    set values(1) {}
    set values(2) 1

    set nmed {}

    set CTdataflag 0

    set medium(0) "VACUUM"
    for {set i 1} {$i<$maxvals(MXMED)} {incr i} {
	set medium($i) {}
    }
    for {set i 0} {$i<3} {incr i} {
	set imax($i) {}
    }
    set izvox(1) {}
    set iphant 0

    for {set i 0} {$i<$maxvals(IMAX)} {incr i} {
	set ivox(0,$i) {}
	set gvox(0,$i,1) {}
	set gvox(0,$i,2) {}
    }
    for {set i 0} {$i<$maxvals(JMAX)} {incr i} {
	set ivox(1,$i) {}
	set gvox(1,$i,1) {}
	set gvox(1,$i,2) {}
    }
    for {set i 0} {$i<$maxvals(KMAX)} {incr i} {
	set ivox(2,$i) {}
	set gvox(2,$i,1) {}
	set gvox(2,$i,2) {}
    }
    # I've set the max. izrow and nrow to 20.  May need to be changed.  JT
    for {set j 0} {$j<3} {incr j} {
	set gvox($j,0) {}
	for {set i 0} {$i<20} {incr i} {
	    set mvox($i,$j,0) {}
	    set mvox($i,$j,1) {}
	    set izvox($i,$j,0) 0
	    set izvox($i,$j,1) 0
	}
    }
    for {set i 0} {$i<20} {incr i} {
	set mvox($i,0) {}
	set mvox($i,1) {}
	set izvox($i) {}
    }
    set PhantFileName {}
    set izrow {}
    set nrow {}
    set grouping(0) 0
    set grouping(1) 0
    set grouping(2) 0

    for {set i 25} {$i<=27} {incr i} {
	set values($i) $options($i,0)
    }

    set values(3) $options(3,0)
    set values(4) {}
    for {set i 0} {$i<8} {incr i} {
	set srcopts($i) {}
    }

    set thphidef {}
    set numthphi 0
    set numsets 0

    for {set i 0} {$i<$maxvals(MXANG)} {incr i} {
       set angfixed($i) {}
	set iso1($i) {}
	set iso2($i) {}
	set iso3($i) {}
	set ang1($i) {}
	set ang2($i) {}
	set ang3($i) {}
	set dsource($i) {}
	set muI($i) {}
       set ang1($i) {}
       set ang2($i) {}
       set nang($i) {}
       set pang($i) {}
       set ivary($i) {}
    }

    set enflag 0
    set values(20) $options(20,0)
    set medsur VACUUM
    for {set i 1} {$i<=4} {incr i} {
	set dsurround($i) 0
    }
    set dflag $dflagopt(0)
    set iphspout 0
    set imuphspout 0

    set Ein {}
    set spec_file {}
    set the_beam_code {}
    set the_input_file {}
    set the_pegs_file {}

    set values(21) {}
    set nbit1 {}
    set nbit2 {}
    for {set i 0} {$i<=29} {incr i} {
	set inc($i) 0
	set exc($i) 0
	set latbit($i) {}
    }

    set values(5) 100
    set values(6) $options(6,0)
    set values(7) 0.99
    set values(8) 33
    set values(9) 97
    set values(10) 100.0
    set values(11) $options(11,0)
    set values(12) $options(12,0)
    set values(13) $options(13,0)
    set values(14) $options(14,0)
    for {set i 15} {$i<=16} {incr i} {
	set values($i) {}
    }
    set values(17) $options(17,0)
    set values(18) 1

    #EGSnrc default values

    set values(ecut) {}
    set values(pcut) {}
    set values(smaxir) 5
    set values(estepe) 0.25
    set values(ximax) 0.5
    set values(skindepth_for_bca) 0
    set values(bca_algorithm) $options(bca_algorithm,1)
    set values(transport_algorithm) $options(transport_algorithm,0)
    set values(spin_effects) $options(spin_effects,1)
    set values(ibrdst) $options(ibrdst,0)
    set values(ibr_nist) $options(ibr_nist,0)
    set values(ibcmp) $options(ibcmp,0)
    set values(iprdst) $options(iprdst,1)
    set values(iphter) $options(iphter,0)
    set values(iraylr) $options(iraylr,0)
    set values(iedgfl) $options(iedgfl,0)
    set level(ibcmp) 0
    set level(iphter) 0
    set level(iraylr) 0
    set level(iedgfl) 0
    set num_rayl_custom 0
    for {set i 1} {$i <= 200} {incr i} {
      set rayl_cust_med($i) {}
      set rayl_cust_file($i) {}
    }
}
