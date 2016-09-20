
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: pegsless functions
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
#  Author:          Blake Walters, 2013
#
#  Contributors:
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


array set element_names {
  1  H
  2  He
  3  Li
   4 Be
  5  B
  6  C
  7  N
  8  O
  9  F
  10  Ne
  11  Na
  12  Mg
  13  Al
  14  Si
  15  P
  16  S
  17  Cl
  18  Ar
  19  K
  20  Ca
  21  Sc
  22  Ti
  23  V
  24  Cr
  25  Mn
   26 Fe
   27 Co
   28 Ni
   29 Cu
   30 Zn
   31 Ga
   32 Ge
   33 As
   34 Se
   35 Br
   36 Kr
   37 Rb
   38 Sr
   39 Y
   40 Zr
   41 Nb
   42 Mo
   43 Tc
   44 Ru
   45 Rh
   46 Pd
   47 Ag
   48 Cd
   49 In
   50 Sn
   51 Sb
   52 Te
   53 I
   54 Xe
   55 Cs
   56 Ba
   57 La
   58 Ce
   59 Pr
   60 Nd
   61 Nd
   62 Pm
   63 Sm
   64 Eu
   65 Gd
   66 Tb
   67 Dy
   68 Ho
   69 Er
   70 Tm
   71 Yb
   72 Lu
   73 Hf
   74 Ta
   75 W
   76 Re
    77 Os
  78  Ir
  79  Pt
  80  Au
  81  Hg
  82  Tl
  83  Pb
  84  Bi
  85  Po
  86  At
  87  Rn
  88  Fr
  89  Ra
  90  Ac
  91  Th
  92  Pa
  93  U
  94  Np
  95  Pu
  96  Am
  97  Cm
  98  Bk
  99  Cf
  100  Es
  101  Fm
}

set med_type(0) "Compound"
set med_type(1) "Mixture"
set med_type(2) "Element"

set rho_units(0) "g/cm^3" 
set rho_units(1) "kg/m^3"

set df_searchopt(0) "EGS_HOME"
set df_searchopt(1) "HEN_HOUSE"

set label_pz_or_rhoz(0) "Stoichiometric index"
set label_pz_or_rhoz(1) "Fraction by weight"
set label_pz_or_rhoz(2) ""

#below only used by pegsless stuff--probably should be used elsewhere
set med_per_column 40

proc define_pegsless_media { } {
    global matfilename helvfont env egs_home media ae ap ue up ninpmed

    catch {destroy .pegsless}

    toplevel .pegsless -bd 5
    wm title .pegsless "PEGSless Inputs"

#probably do not want to restrict the size of this
#    set width [win/hfo screenwidth .]
#    set height [winfo screenheight .]
#    set x [expr ($width/2) - 450]
#    set y [expr ($height/2) -150]
#    wm geometry .pegsless +$x+$y

    frame .pegsless.elimits
    label .pegsless.elimits.text -text "Energy limits (MeV) of cross-sections" -font $helvfont
    pack .pegsless.elimits.text -side top
    button .pegsless.elimits.help -text "?" -command "help ae"
    frame .pegsless.elimits.right
    frame .pegsless.elimits.right.e
    label .pegsless.elimits.right.e.aetxt -text "AE"
    entry .pegsless.elimits.right.e.aeval -textvariable ae
    label .pegsless.elimits.right.e.uetxt -text "UE"
    entry .pegsless.elimits.right.e.ueval -textvariable ue
    pack .pegsless.elimits.right.e.aetxt .pegsless.elimits.right.e.aeval .pegsless.elimits.right.e.uetxt .pegsless.elimits.right.e.ueval -side left
    frame .pegsless.elimits.right.p
    label .pegsless.elimits.right.p.aptxt -text "AP"
    entry .pegsless.elimits.right.p.apval -textvariable ap
    label .pegsless.elimits.right.p.uptxt -text "UP"
    entry .pegsless.elimits.right.p.upval -textvariable up
    pack .pegsless.elimits.right.p.aptxt .pegsless.elimits.right.p.apval .pegsless.elimits.right.p.uptxt .pegsless.elimits.right.p.upval -side left
    pack .pegsless.elimits.right.e .pegsless.elimits.right.p -side top
    pack .pegsless.elimits.help .pegsless.elimits.right -side left
    pack .pegsless.elimits

    # Add a separator
    frame .pegsless.sep1 -bd 4 -width 100 -height 2 -relief groove
    pack .pegsless.sep1 -fill x

    #assume material data file is in local pegs area
    set local_pegs_area [file join $egs_home pegs4 data]

    frame .pegsless.matfile
    label .pegsless.matfile.text -text "Enter name of material data file" -font $helvfont
    pack .pegsless.matfile.text
    frame .pegsless.matfile.top
    button .pegsless.matfile.top.help -text "?" -command "help matfile"
    entry .pegsless.matfile.top.filename -textvariable matfilename
    bind .pegsless.matfile.top.filename <Return> {read_matfile}
    button .pegsless.matfile.top.browse -text "Browse" -command\
     "query_filename set_matfilename $local_pegs_area *"
    pack .pegsless.matfile.top.help .pegsless.matfile.top.filename .pegsless.matfile.top.browse -side left
    button .pegsless.matfile.list -text "List media in file" -command "list_matfile_media"
    pack .pegsless.matfile.top .pegsless.matfile.list -side top
    pack .pegsless.matfile

    # Add a separator
    frame .pegsless.sep2 -bd 4 -width 100 -height 2 -relief groove
    pack .pegsless.sep2 -fill x

    frame .pegsless.def
    label .pegsless.def.text -text "Add/Modify media in .egsinp file" -font $helvfont
    pack .pegsless.def.text
    frame .pegsless.def.grid -bd 4
    label .pegsless.def.grid.l0 -text "medium name:"
    grid configure .pegsless.def.grid.l0 -row 0 -column 0
    for {set i 1} {$i<=$ninpmed} {incr i} {
       entry .pegsless.def.grid.name$i -textvariable inpmedium($i)
       button .pegsless.def.grid.edit$i -text "Edit" -command "edit_medium $i"

       grid configure .pegsless.def.grid.name$i -row $i -column 0
       grid configure .pegsless.def.grid.edit$i -row $i -column 1
    }

    pack .pegsless.def.grid
    frame .pegsless.def.buts
    button .pegsless.def.buts.edit -text "Add medium" -command "add_modify_med"
    button .pegsless.def.buts.del -text "Delete last medium" -command "subtract_med"
    pack .pegsless.def.buts.edit .pegsless.def.buts.del -side left
    pack .pegsless.def.buts
    pack .pegsless.def

    button .pegsless.okb -text "Done" -command "add_inp_media; destroy .pegsless"
    pack .pegsless.okb
}

proc set_matfilename { tree filename } {
    global matfilename env egs_home hen_house
    set matfilename [file join $tree $filename]
    if [file exists $matfilename]==0 {
	tk_dialog .error "Error" "$matfilename doesn't exist!  Please try\
		again." error 0 OK
	return
    }

    read_matfile
}

proc read_matfile {} {
    global matfilename env nmed medium helvfont nmatmed
    global smhelvfont egs_home hen_house pegs
    global density_file rho elements pz_or_rhoz iaprim iunrst gasp def_by_pz

# put on a small window with a message letting the user know it's
# loading the material data file.
#	toplevel .pegslessmessage -bd 0

#	set width [winfo screenwidth .]
#	set height [winfo screenheight .]
#	set x [expr ($width/2) - 200]
#	set y [expr ($height/2) -100]
#	wm geometry .pegslessmessage +$x+$y
#	wm transient .pegslessmessage .

#	label .pegslessmessage.m -text "Loading media from material data file..." -font $helvfont
#	pack .pegslessmessage.m -padx 20 -pady 20
#	update

	# Now read in the materials from the file:
        set nmed 0
        set nmatmed 0
        set pegs(0) "VACUUM"
	if [file readable $matfilename] {
	    set file [open $matfilename r]
	    set i 1
	    while { [eof $file] != 1 } {
              gets $file data
                #now see if the word medium appears on the line
              set eindex [string first "=" $data]
              set data1 [string range $data 0 [expr $eindex-1]]
              set data1 [string tolower $data1]
              set mindex [string first "medium" $data1]
              set rindex [string first "rho" $data1]
              set elemindex [string first "elements" $data1]
              set atomindex [string first "number of atoms" $data1]
              set rhozindex [string first "mass fractions" $data1]
              set iaprimindex [string first "bremsstrahlung correction" $data1]
              set iunrstindex [string first "stopping powers" $data1]
              set gaspindex [string first "gas pressure" $data1]
              set dfindex [string first "density correction file" $data1]
              set cindex [string first "#" $data1]
              if {$cindex==-1 } {
                set strlen [string length $data]
                set data [string range $data [expr $eindex+1] [expr $strlen-1]]
                set data [string trimleft $data]
                set data [string trimright $data]
                if {$data!=""} {
                  if { $mindex>-1 } {
		    set pegs($i) $data
		    incr i
                  }
                }
              }
            }
            close $file
            set nmed [expr $i-1]
            set nmatmed $nmed
        }
}

# keep this around in case we decide to list more info for each medium
#                  } else if {$rindex > -1} {
#                    set rho($i) $data
#                  } else if {$elemindex > -1} {
#                    set nelements($i) 0
#                    while {$data!=""} {
#                      set data [get_str_arr $data arr $nelements($i)]
#                      if {$data!=""} {
#                        set elements($i,$nelements($i)) arr($nelements($i))
#                        incr nelements($i)
#                      }
#                    }
#                  } else if {$atomindex > -1} {
#                    set j 0
#                    while {$data!=""} {
#                      set data [get_str_arr $data arr $j]
#                      if {$data!=""} {
#                        set elements($i,$j) arr($j)
#                        incr j
#                      }
#                    }
#                  }



proc list_matfile_media {} {

global medium nmed matfilename helvfont pegs

   toplevel .matfilemed  -bd 5
   wm title .matfilemed "Media defined in $matfilename"

   frame .matfilemed.grid

   read_matfile

   if {$nmed>0} {
   set mednum 0
   for {set i 0} {$i<[expr ($nmed-1)/5+1]} {incr i} {
    for {set j 0} {$j<5} {incr j} {
       incr mednum
       if { $mednum <= $nmed } {
         label .matfilemed.grid.$i$j -text $pegs($mednum) -bd 5
         grid configure .matfilemed.grid.$i$j -row $i -column $j
       }
    }
  }
  } else {
   label .matfilemed.grid.err -text "No media defined in material data file!"
   pack .matfilemed.grid.err
  }

  pack .matfilemed.grid
}


proc add_modify_med {} {

global inpmedium ninpmed elements ipz pz_or_rhoz iaprim iunrst density_file rho gasp nelem
global rho_scale is_gas dcf_specified element_names

   set w .pegsless.def.grid

   incr ninpmed

   #set up necessary defaults
   set iaprim($ninpmed) 0
   set iunrst($ninpmed) 0
   set gasp($ninpmed) {}
   set density_file($ninpmed) {}
   set rho($ninpmed) {}
   set nelem($ninpmed) 1
   set elements($ninpmed,1) $element_names(1) 
   set ipz($ninpmed) 0
   set rho_scale($ninpmed) 0
   set is_gas($ninpmed) 0
   set dcf_specified($ninpmed) 0

   entry $w.name$ninpmed -textvariable inpmedium($ninpmed)
   button $w.edit$ninpmed -text "Edit" -command "edit_medium $ninpmed"

   grid configure $w.name$ninpmed -row $ninpmed -column 0
   grid configure $w.edit$ninpmed -row $ninpmed -column 1

}

proc subtract_med { } {

global ninpmed nmed

   set w .pegsless.def.grid

   destroy $w.name$ninpmed
   destroy $w.edit$ninpmed

   incr ninpmed -1
   incr nmed -1
}

proc edit_medium { mednum } {

   global elements ipz pz_or_rhoz iaprim iunrst density_file rho gasp nelem
   global hen_house envi dfile_temp inpmedium sterncid element_names 
   global med_per_column med_type is_gas rho_scale dcf_specified
   global df_searchopt label_pz_or_rhoz rho_units egs_home
   global df_area

   toplevel .define$mednum -bd 5
   wm title .define$mednum "Define $inpmedium($mednum)"

   label .define$mednum.title -text "Define $inpmedium($mednum)"

   pack .define$mednum.title

   frame .define$mednum.u

   frame .define$mednum.u.l

   label .define$mednum.u.l.title -text "Medium composition:" -pady 5
   pack .define$mednum.u.l.title -anchor w

   frame .define$mednum.u.l.grid

   label .define$mednum.u.l.grid.l1 -text "Elements"

   #first set up the column headers

   label .define$mednum.u.l.grid.l2 -text $label_pz_or_rhoz($ipz($mednum)) 

   grid configure .define$mednum.u.l.grid.l1 -row 0 -column 1
   grid configure .define$mednum.u.l.grid.l2 -row 0 -column 2


   for {set i 1} {$i<=$nelem($mednum)} {incr i} {

      label .define$mednum.u.l.grid.ind$i -text $i

      menubutton .define$mednum.u.l.grid.elem$i -text $elements($mednum,$i) -menu\
                 .define$mednum.u.l.grid.elem$i.m -bd 1 -relief raised -indicatoron 1 -width 10
      menu .define$mednum.u.l.grid.elem$i.m -tearoff no
      for {set j 1} {$j <= [array size element_names]} {incr j} {
          .define$mednum.u.l.grid.elem$i.m add command -label $element_names($j) -columnbreak [expr $j % $med_per_column == 0] -command ".define$mednum.u.l.grid.elem$i configure -text $element_names($j); set elements($mednum,$i) $element_names($j)"
      }
      entry .define$mednum.u.l.grid.pzorrhoz$i -textvariable pz_or_rhoz($mednum,$i)

      grid configure .define$mednum.u.l.grid.ind$i -row $i -column 0 
      grid configure .define$mednum.u.l.grid.elem$i -row $i -column 1
      grid configure .define$mednum.u.l.grid.pzorrhoz$i -row $i -column 2
   }

   pack .define$mednum.u.l.grid

   frame .define$mednum.u.l.b

   button .define$mednum.u.l.b.adelem -text "Add element" -command "add_element .define$mednum.u.l.grid $mednum"
   button .define$mednum.u.l.b.delelem -text "Remove element" -command "subtract_element .define$mednum.u.l.grid $mednum"
   pack .define$mednum.u.l.b.adelem .define$mednum.u.l.b.delelem -side left

   pack .define$mednum.u.l.b
  
   #now set up a right frame with the other media options

   frame .define$mednum.u.r

   # medium type and rho
   frame .define$mednum.u.r.top

   #medium type menu
   frame .define$mednum.u.r.top.type
   frame .define$mednum.u.r.top.type.top
   label .define$mednum.u.r.top.type.top.title -text "Medium type:"
   button .define$mednum.u.r.top.type.top.help -text "?" -command "help med_type"
   pack .define$mednum.u.r.top.type.top.help .define$mednum.u.r.top.type.top.title -side left -anchor w
   frame .define$mednum.u.r.top.type.bot
   menubutton .define$mednum.u.r.top.type.bot.mb -text $med_type($ipz($mednum))\
     -menu .define$mednum.u.r.top.type.bot.mb.m -bd 1 -relief raised -indicatoron 1
   menu .define$mednum.u.r.top.type.bot.mb.m -tearoff no
   for {set j 0} {$j < 3} {incr j} {
          .define$mednum.u.r.top.type.bot.mb.m add command -label $med_type($j) -command "set_medium_type $mednum $j"
   }
   pack .define$mednum.u.r.top.type.bot.mb
   pack .define$mednum.u.r.top.type.top .define$mednum.u.r.top.type.bot -side top -anchor w 

   #rho
   frame .define$mednum.u.r.top.rho
   frame .define$mednum.u.r.top.rho.top
   label .define$mednum.u.r.top.rho.top.title -text "Mass density:"
   button .define$mednum.u.r.top.rho.top.help -text "?" -command "help rho"
   pack .define$mednum.u.r.top.rho.top.help .define$mednum.u.r.top.rho.top.title -side left -anchor w
   frame .define$mednum.u.r.top.rho.bot
   entry .define$mednum.u.r.top.rho.bot.val -textvariable rho($mednum)
   menubutton .define$mednum.u.r.top.rho.bot.scale -text $rho_units($rho_scale($mednum))\
      -menu .define$mednum.u.r.top.rho.bot.scale.m -relief raised -indicatoron 1
   menu .define$mednum.u.r.top.rho.bot.scale.m -tearoff no
   for {set j 0} {$j < 2} {incr j} {
      .define$mednum.u.r.top.rho.bot.scale.m add command -label $rho_units($j)\
         -command "set rho_scale($mednum) $j; .define$mednum.u.r.top.rho.bot.scale configure -text $rho_units($j)"
   }
   pack .define$mednum.u.r.top.rho.bot.val .define$mednum.u.r.top.rho.bot.scale -side left -anchor w
   pack .define$mednum.u.r.top.rho.top .define$mednum.u.r.top.rho.bot -side top -anchor w

   pack .define$mednum.u.r.top.type .define$mednum.u.r.top.rho -side left -padx 5

   #other medium options
   frame .define$mednum.u.r.bot
   label .define$mednum.u.r.bot.title -text "Options:"
   pack .define$mednum.u.r.bot.title -pady 5 -anchor w

   #option to input density correction file
   frame .define$mednum.u.r.bot.dcf
   button .define$mednum.u.r.bot.dcf.help -text "?" -command "help icru_density"
   checkbutton .define$mednum.u.r.bot.dcf.chk -variable dcf_specified($mednum) -command " use_dcf $mednum"
   label .define$mednum.u.r.bot.dcf.lab -text "ICRU density correction"
   pack .define$mednum.u.r.bot.dcf.help .define$mednum.u.r.bot.dcf.chk .define$mednum.u.r.bot.dcf.lab -side left -anchor w

   #iaprim
   frame .define$mednum.u.r.bot.iaprim
   button .define$mednum.u.r.bot.iaprim.help -text "?" -command "help iaprim"
   checkbutton .define$mednum.u.r.bot.iaprim.chk -variable iaprim($mednum)
   label .define$mednum.u.r.bot.iaprim.lab -text "ICRU radiative stopping power"
   pack .define$mednum.u.r.bot.iaprim.help .define$mednum.u.r.bot.iaprim.chk .define$mednum.u.r.bot.iaprim.lab -side left -anchor w

   #gasp
   frame .define$mednum.u.r.bot.gasp 
   frame .define$mednum.u.r.bot.gasp.l
   button .define$mednum.u.r.bot.gasp.l.help -text "?" -command "help gasp"
   checkbutton .define$mednum.u.r.bot.gasp.l.chk -variable is_gas($mednum) \
     -command "enable_disable_gasp_input $mednum"
   label .define$mednum.u.r.bot.gasp.l.lab -text "Medium is a gas"
   pack .define$mednum.u.r.bot.gasp.l.help .define$mednum.u.r.bot.gasp.l.chk .define$mednum.u.r.bot.gasp.l.lab -side left
   frame .define$mednum.u.r.bot.gasp.r
   label .define$mednum.u.r.bot.gasp.r.lab -text "    gas pressure (atm.)"
   entry .define$mednum.u.r.bot.gasp.r.val -textvariable gasp($mednum)
   pack .define$mednum.u.r.bot.gasp.r.lab .define$mednum.u.r.bot.gasp.r.val -side left
   pack .define$mednum.u.r.bot.gasp.l .define$mednum.u.r.bot.gasp.r -side left -anchor w

   pack .define$mednum.u.r.bot.dcf .define$mednum.u.r.bot.iaprim .define$mednum.u.r.bot.gasp -side top -anchor w

   #okay, now pack the upper part of this window
   pack .define$mednum.u.r.top .define$mednum.u.r.bot -side top -pady 5 -anchor w
   pack .define$mednum.u.l .define$mednum.u.r -side left -padx 5 -anchor w 

   #density correction file input
   set_df_area 0
   frame .define$mednum.b
   frame .define$mednum.b.top
   label .define$mednum.b.top.title -text "Density correction file:"
   button .define$mednum.b.top.help -text "?" -command "help density_file"
   pack .define$mednum.b.top.help .define$mednum.b.top.title -side left -anchor w -pady 5
   frame .define$mednum.b.bot
   label .define$mednum.b.bot.lab -text "Look in"
   menubutton .define$mednum.b.bot.searchdir -text $df_searchopt(0)\
      -menu .define$mednum.b.bot.searchdir.m -relief raised -indicatoron 1
   menu .define$mednum.b.bot.searchdir.m -tearoff no 
   for {set j 0} {$j < 2} {incr j} {
      .define$mednum.b.bot.searchdir.m add command -label $df_searchopt($j)\
         -command "set_df_area $j; .define$mednum.b.bot.searchdir configure -text $df_searchopt($j)"
   }
   #default to users pegs4 area
   entry .define$mednum.b.bot.fname -textvariable density_file($mednum) -width 25
   bind .define$mednum.b.bot.fname <Return> "read_dcf $mednum 0"
   button .define$mednum.b.bot.browse -text "Browse" -command\
     "call_query_filename $mednum"
   pack .define$mednum.b.bot.lab .define$mednum.b.bot.searchdir .define$mednum.b.bot.fname .define$mednum.b.bot.browse -side left
   pack .define$mednum.b.top .define$mednum.b.bot -side top -anchor w

   pack .define$mednum.u .define$mednum.b -side top 

   #an okay button

   button .define$mednum.okb -text "Done" -command "dcf_final_check $mednum; destroy .define$mednum"
   pack .define$mednum.okb -side top

   #now update configuration based on paramters read in

   #density correction file
   if {$dcf_specified($mednum)==1} {
     #attempt to read medium comp. and rho from dcf
     set dcf_specified($mednum) [read_dcf $mednum 1]
   }
   use_dcf $mednum

   #gas pressure
   enable_disable_gasp_input $mednum 

}

proc set_df_area { searchind } {
   global df_area egs_home hen_house

   if {$searchind==0} {
      set df_area [file join $egs_home pegs4 density_corrections]
   } else {
      set df_area [file join $hen_house pegs4 density_corrections]
   }

}

proc dcf_final_check { mednum } {
   global dcf_specified

   if {$dcf_specified($mednum)==1} {
      set dcf_specified($mednum) [read_dcf $mednum 2]
   }
}

proc set_medium_type { mednum type } {
   global arr label_pz_or_rhoz nelem med_type ipz

   #actually set the global medium type
   set ipz($mednum) $type

   #the rest is all about configuring the display
   #update the text on the menu button
   .define$mednum.u.r.top.type.bot.mb configure -text $med_type($type)

   #update column header in medium comp. table 
   .define$mednum.u.l.grid.l2 configure -text $label_pz_or_rhoz($type)
   
   #if the user has chosen an element, delete all but the first row
   #and disable the add and delete element button s
   if {$ipz($mednum)==2} {
     #first find out how many elements are in the med. comp. table
     set data [grid size .define$mednum.u.l.grid]
     for {set i 0} {$i<2} {incr i} {
       set data [get_val $data arr $i]
     }
     set nrows $arr(1)
     for {set i 2} {$i<=$nrows} {incr i} {
        destroy .define$mednum.u.l.grid.ind$i 
        destroy .define$mednum.u.l.grid.elem$i
        destroy .define$mednum.u.l.grid.pzorrhoz$i
     }
     .define$mednum.u.l.b.delelem configure -state disabled 
     .define$mednum.u.l.b.adelem configure -state disabled 
     #also disable entry window for stoichiometric index/mass fraction
     .define$mednum.u.l.grid.pzorrhoz1 configure -state disabled
     #and set the no. of elements to one
     set nelem($mednum) 1
   } else {
     #enable add element button and entry window for first element
     .define$mednum.u.l.b.adelem configure -state normal
     .define$mednum.u.l.grid.pzorrhoz1 configure -state normal
   }
}

proc use_dcf { mednum } {
   #enable or disable medium composition table, medium type, rho inputs
   #density file input
   global nelem arr dcf_specified ipz

   #first find out how many elements are in the med. comp. table
   set data [grid size .define$mednum.u.l.grid]
   for {set i 0} {$i<2} {incr i} {
       set data [get_val $data arr $i]
   }
   set nrows $arr(1)

   if {$dcf_specified($mednum)==0} {
    #not using a density correction file
    .define$mednum.u.l.grid.l1 configure -state normal
    .define$mednum.u.l.grid.l2 configure -state normal
    for {set i 1} {$i<$nrows} {incr i} {
      .define$mednum.u.l.grid.ind$i configure -state normal
      .define$mednum.u.l.grid.elem$i configure -state normal
      .define$mednum.u.l.grid.pzorrhoz$i configure -state normal
    }
    .define$mednum.u.l.b.delelem configure -state normal
    .define$mednum.u.l.b.adelem configure -state normal
    .define$mednum.u.r.top.type.bot.mb configure -state normal
    .define$mednum.u.r.top.rho.bot.val configure -state normal 
    .define$mednum.u.r.top.rho.bot.scale configure -state normal
    .define$mednum.b.bot.lab configure -state disabled 
    .define$mednum.b.bot.searchdir configure -state disabled 
    .define$mednum.b.bot.fname configure -state disabled
    .define$mednum.b.bot.browse configure -state disabled
    #disable delete element button if there is only one element
    if {$nelem($mednum)==1} {
       .define$mednum.u.l.b.delelem configure -state disabled
       if {$ipz($mednum)==2} {
         .define$mednum.u.l.grid.pzorrhoz1 configure -state disabled
         .define$mednum.u.l.b.adelem configure -state disabled
       }
    }
   } elseif {$dcf_specified($mednum)==1} {
    .define$mednum.u.l.grid.l1 configure -state disabled 
    .define$mednum.u.l.grid.l2 configure -state disabled 
    for {set i 1} {$i<$nrows} {incr i} {
      .define$mednum.u.l.grid.ind$i configure -state disabled 
      .define$mednum.u.l.grid.elem$i configure -state disabled 
      .define$mednum.u.l.grid.pzorrhoz$i configure -state disabled 
    }
    .define$mednum.u.l.b.delelem configure -state disabled 
    .define$mednum.u.l.b.adelem configure -state disabled 
    .define$mednum.u.r.top.type.bot.mb configure -state disabled 
    .define$mednum.u.r.top.rho.bot.val configure -state disabled 
    .define$mednum.u.r.top.rho.bot.scale configure -state disabled 
    .define$mednum.b.bot.lab configure -state normal 
    .define$mednum.b.bot.searchdir configure -state normal 
    .define$mednum.b.bot.fname configure -state normal 
    .define$mednum.b.bot.browse configure -state normal 
   }
}

proc enable_disable_gasp_input { mednum } {
   #enable/disable gas pressure input
   global is_gas
  
   if {$is_gas($mednum)==0} {
    .define$mednum.u.r.bot.gasp.r.lab configure -state disabled
    .define$mednum.u.r.bot.gasp.r.val configure -state disabled
   } elseif {$is_gas($mednum)==1} {
    .define$mednum.u.r.bot.gasp.r.lab configure -state normal
    .define$mednum.u.r.bot.gasp.r.val configure -state normal 
   }
}
    
proc add_element { w mednum }  {
   global nelem elements pz_or_rhoz element_names med_per_column

   incr nelem($mednum)

   #default to H for now
   set elements($mednum,$nelem($mednum)) $element_names(1)

   label $w.ind$nelem($mednum) -text $nelem($mednum)

   menubutton $w.elem$nelem($mednum) -text $element_names(1) -menu\
                 $w.elem$nelem($mednum).m -bd 1 -relief raised -indicatoron 1\
                 -width 10
    menu $w.elem$nelem($mednum).m -tearoff no
      for {set j 1} {$j <= [array size element_names]} {incr j} {
          $w.elem$nelem($mednum).m add command -label $element_names($j) -columnbreak [expr $j % $med_per_column == 0] -command "$w.elem$nelem($mednum) configure -text $element_names($j); set elements($mednum,$nelem($mednum)) $element_names($j)"
      }
   entry $w.pzorrhoz$nelem($mednum) -textvariable pz_or_rhoz($mednum,$nelem($mednum))

   grid configure $w.ind$nelem($mednum) -row $nelem($mednum) -column 0
   grid configure $w.elem$nelem($mednum) -row $nelem($mednum) -column 1
   grid configure $w.pzorrhoz$nelem($mednum) -row $nelem($mednum) -column 2

   .define$mednum.u.l.b.delelem configure -state normal

}

proc subtract_element { w mednum } {
   global nelem

   destroy $w.ind$nelem($mednum) 
   destroy $w.elem$nelem($mednum)
   destroy $w.pzorrhoz$nelem($mednum)

   incr nelem($mednum) -1

   if {$nelem($mednum)==1} {
      .define$mednum.u.l.b.delelem configure -state disabled
   }
}

proc call_query_filename { mednum } {
# just a procedure to call query_filename so we can use different
# search trees
    global density_file df_area
   
    query_filename set_dfname $df_area * $mednum
} 

proc set_dfname { tree filename mednum } {
    global density_file
    #always use full directory path to dcf
    set density_file($mednum) [file join $tree $filename]
    if [file exists $density_file($mednum)]==0 {
        tk_dialog .error "Error" "$tempdf doesn't exist!  Please try\
                again." error 0 OK
        return
    }
    #now open and read the medium composition and rho from the dcf
    read_dcf $mednum 0
}

proc read_dcf { mednum mode } {
    #opens the dcf for mednum and reads the composition and density
    #supposed to be used to return value of dcf_specified
    global density_file nelem rho elements pz_or_rhoz ipz 
    global arr element_names rho_units egs_home hen_house

    #first check to see if the file exists
    if {[file exists $density_file($mednum)]==0} {
       #if the file name does not include a directory separator
       #assume the .density extension is also not included and begin
       #searching for the file
       set sep [file separator]
       if {[string first sep $density_file($mednum)] < 0} {
           set density_file($mednum) "$density_file($mednum).density"
           #first look in EGS_HOME/pegs4/density_corrections
           set tmp_str [file join $egs_home pegs4 density_corrections $density_file($mednum)]
           if {[file exists $tmp_str]==0} {
             set tmp_str [file join $egs_home pegs4 density_corrections elements $density_file($mednum)]
               if {[file exists $tmp_str]==0} {
                  set tmp_str [file join $egs_home pegs4 density_corrections compounds $density_file($mednum)]
                  if {[file exists $tmp_str]==0} {
                     #check in old density subdirectory if still there
                      set tmp_str [file join $egs_home pegs4 density $density_file($mednum)]
                    if {[file exists $tmp_str]==0} {
                       #now look through HEN_HOUSE/pegs4/density_corrections
                       set tmp_str [file join $hen_house pegs4 density_corrections elements $density_file($mednum)]
                       if {[file exists $tmp_str]==0} {
                          set tmp_str [file join $hen_house pegs4 density_corrections compounds $density_file($mednum)]
                          if {[file exists $tmp_str]==0} {
                             tk_dialog .error "Error" "The density correction file specified for this medium, $density_file($mednum), cannot be found and will not be included in media specs unless it is changed." error 0 OK
                             return 0
                          }
                       }
                    }
                 }
               }
           }
           set density_file($mednum) $tmp_str
        }
    }

    if {[file readable $density_file($mednum)]} {
      set dcf_id [open $density_file($mednum) "r"]
      #now read the composition and density 
      #get title line
      gets $dcf_id data
      gets $dcf_id data
      for {set i 0} {$i < 4} {incr i} {
        set data [get_val $data arr $i]
      }
      set rho($mednum) $arr(2)
      set nelem($mednum) $arr(3)
      gets $dcf_id data
      for {set i 1} {$i<=$nelem($mednum)} {incr i} {
        set data [get_val $data arr 0]
        set data [get_val $data arr 1]
        set elements($mednum,$i) $element_names($arr(0))
        set pz_or_rhoz($mednum,$i) $arr(1)
      }
      if {$nelem($mednum)>1} {
        set ipz($mednum) 1
        #mixture
      } else {
        set ipz($mednum) 2
        #element
      }
      #rho entry gets updated on its own but need to update scaling menu
      .define$mednum.u.r.top.rho.bot.scale configure -text $rho_units(0)
      #and now...update the composition table (even though it's disabled)
      update_comp_table $mednum
    } else {
      if {$mode==0 || $mode==1} {
     #output a warning and let user keep trying
        tk_dialog .error "Error" "$density_file($mednum) is not readable.  Please try another file or define medium using the table." error 0 OK
      } elseif {$mode==2} {
        tk_dialog .error "Error" "The density correction file specified for this medium, $density_file($mednum), is not readable and will not be included in media specs unless it is changed." error 0 OK
      }
      return 0 
    }
    return 1
} 

proc update_comp_table { mednum } {
#procedure to update the contents of the composition table
#assumes grid has already been defined and will be "packed" after call
#to this
    global label_pz_or_rhoz ipz nelem elements pz_or_rhoz arr 
    global med_per_column element_names

    #title of column 2
    .define$mednum.u.l.grid.l2 configure -text $label_pz_or_rhoz($ipz($mednum)) 

    #delete current elements in table
    #do fiddly bit below to find out current no. of rows in grid
    set data [grid size .define$mednum.u.l.grid]
    for {set i 0} {$i<2} {incr i} {
       set data [get_val $data arr $i]
    }
    set nrows $arr(1) 
    #do not delete first row
    for {set i 1} {$i<$nrows} {incr i} {
      destroy .define$mednum.u.l.grid.ind$i
      destroy .define$mednum.u.l.grid.elem$i
      destroy .define$mednum.u.l.grid.pzorrhoz$i 
    }
 
    #now go through and display elements and quantities of each
    for {set i 1} {$i<=$nelem($mednum)} {incr i} {

      label .define$mednum.u.l.grid.ind$i -text $i

      menubutton .define$mednum.u.l.grid.elem$i -text $elements($mednum,$i) -menu\
                 .define$mednum.u.l.grid.elem$i.m -bd 1 -relief raised -indicatoron 1 -width 10
      menu .define$mednum.u.l.grid.elem$i.m -tearoff no
      for {set j 1} {$j <= [array size element_names]} {incr j} {
          .define$mednum.u.l.grid.elem$i.m add command -label $element_names($j) -columnbreak [expr $j % $med_per_column == 0] -command ".define$mednum.u.l.grid.elem$i configure -text $element_names($j); set elements($mednum,$i) $element_names($j)"
      }
      entry .define$mednum.u.l.grid.pzorrhoz$i -textvariable pz_or_rhoz($mednum,$i)

      grid configure .define$mednum.u.l.grid.ind$i -row $i -column 0
      grid configure .define$mednum.u.l.grid.elem$i -row $i -column 1
      grid configure .define$mednum.u.l.grid.pzorrhoz$i -row $i -column 2

      #and disable these
      .define$mednum.u.l.grid.ind$i configure -state disabled 
      .define$mednum.u.l.grid.elem$i configure -state disabled 
      .define$mednum.u.l.grid.pzorrhoz$i configure -state disabled 
   }
}
       

proc write_pegsless_data { fileid } {
#called from new_create_file_nrc
    global pegsless_on_inp is_pegsless matfilename ae ap ue up ninpmed inpmedium
    global nelem elements pz_or_rhoz ipz rho iunrst iaprim gasp density_file sterncid
    global is_gas dcf_specified rho_units rho_scale

    #don't output anything if the user is not calling for pegsless run and
    #no pegsless data was read in
    if {$pegsless_on_inp==1 || $is_pegsless==1} {
       puts $fileid " :start media definition:"
       puts $fileid " "
       if {$ae!=""} { puts $fileid " ae=$ae" }
       if {$ue!=""} { puts $fileid " ue=$ue" }
       if {$ap!=""} { puts $fileid " ap=$ap" }
       if {$up!=""} { puts $fileid " up=$up" }
       if {$matfilename!=""} {
          puts $fileid " "
          puts $fileid " material data file=$matfilename"
       }
       if {$ninpmed>0} {
         for {set i 1} {$i<=$ninpmed} {incr i} {
             puts $fileid " "
             puts $fileid " :start $inpmedium($i):"
             if {$dcf_specified($i)==1 && $density_file($i)!=""} {
                #composition and rho specified by density file
                puts $fileid " density correction file= $density_file($i)"
             } else {
               #specify composition and rho explicitly
             if {$nelem($i)>0} {
               set str " elements="
               for {set j 1} {$j<=$nelem($i)} {incr j} {
                  set str "$str $elements($i,$j)"
                  if {$j<$nelem($i)} { set str "$str," }
               }
               puts $fileid $str
               if {$ipz($i)!=2} {
               if {$ipz($i)==0} {
                 set str " number of atoms="
               } elseif {$ipz($i)==1} {
                 set str " mass fractions="
               }
               for {set j 1} {$j<=$nelem($i)} {incr j} {
                  set str "$str $pz_or_rhoz($i,$j)"
                  if {$j<$nelem($i)} { set str "$str," }
               }
               puts $fileid $str
               }
             }
             if {$rho($i)!=""} {
               set rho_mult 1
               if { $rho_scale($i)==1} { set rho_mult 0.001 }
               puts $fileid " rho= [expr $rho_mult*$rho($i)]"
             }
             }
             if {$iaprim($i)!=""} {
               if {$iaprim($i)==0} {
                 puts $fileid " bremsstrahlung correction= KM"
               } elseif {$iaprim($i)==1} {
                 puts $fileid " bremsstrahlung correction= NRC"
               } 
             }
             if {$is_gas($i)==1} {
                if {$gasp($i)>0.0} {
                   puts $fileid " gas pressure= $gasp($i)"
                } else {
                   #default to 1 atm
                   puts $fileid " gas pressure= 1.0"
                }
             }
             puts $fileid " :stop $inpmedium($i):"
         }
       }
       puts $fileid " "
       puts $fileid " :stop media definition:"
       puts $fileid " ########################"
    }
}

proc get_pegsless_inputs { fileid } {

global pegsless_on_inp ae ue ap up matfilename ninpmed inpmedium nelem elements
global ipz pz_or_rhoz rho iunrst iaprim gasp density_file
global inp_file arr is_pegsless sterncid is_gas rho_scale dcf_specified

    #start from the top of the file
    catch {close $fileid}
    set fileid [open $inp_file r]

    #just a parameter
    set max_elem 20

    #initialize variables
    set pegsless_on_inp 0
    set ninpmed 0
    set matfilename ""
    set ae ""
    set ue ""
    set ap ""
    set up ""

    set nmainval 5
    set mainval(0) "ae"
    set mainval(1) "ap"
    set mainval(2) "ue"
    set mainval(3) "up"
    set mainval(4) "material data file"

    set nmedval 9
    set medval(0) "elements"
    set medval(1) "number of atoms"
    set medval(2) "mass fractions"
    set medval(3) "rho"
    set medval(4) "stopping powers"
    set medval(5) "bremsstrahlung correction"
    set medval(6) "gas pressure"
    set medval(7) "density correction file"
    set medval(8) "sterncid"

    while {[gets $fileid data]>-1} {
      set data [string tolower $data]
      set sindex [string first ":start media definition:" $data]
      if {$sindex>-1} {
       #away we go, enter a mode where we look for all the data between this
       #and the stop delimiter
       set begin_def [tell $fileid]
       set endindex -1
       while {$endindex<=-1 && [gets $fileid data]>-1} {
          set data1 [string tolower $data]
          set endindex [string first ":stop media definition:" $data1]
          if {$endindex<=-1} {
            set eindex [string first "=" $data1]
            set cindex [string first "#" $data1]
            if { $cindex<=-1 } {
              #not a comment see if it's one of the main values
              for {set i 0} {$i<$nmainval} {incr i} {
                set valindex [string first $mainval($i) $data1]
                if {$valindex>-1 && $eindex>$valindex } {
                    set data [string range $data [expr $eindex+1] [string length $data]]
                    set data [string trimleft $data]
                    set data [string trimright $data]
                    if {$i==0} {
                      set ae $data
                    } elseif {$i==1} {
                      set ap $data
                    } elseif {$i==2} {
                      set ue $data
                    } elseif {$i==3} {
                      set up $data
                    } elseif {$i==4} {
                      set matfilename $data
                    }
                    break
                }
              }
              if {$i==$nmainval && $valindex<=-1} {
                #see if it's a medium definition
                set mindex [string first ":start" $data1]
                if {$mindex>-1} {
                  incr ninpmed
                  #set up necessary defaults
                  set iaprim($ninpmed) 0
                  set iunrst($ninpmed) 0
                  set gasp($ninpmed) {}
                  set density_file($ninpmed) {}
                  set rho($ninpmed) {}
                  set sterncid($ninpmed) {}
                  set nelem($ninpmed) 0
                  set ipz($ninpmed) 0
                  set rho_scale($ninpmed) 0
                  set is_gas($ninpmed) 0
                  set dcf_specified($ninpmed) 0
                  #find medium name
                  set data [string range $data [expr $mindex+6] [expr [string last ":" $data]-1]]
                  set data [string trimleft $data]
                  set data [string trimright $data]
                  set inpmedium($ninpmed) $data
                  set mendindex -1
                  #now loop through and see what is defined for this medium
                  while {$mendindex<=-1 && [gets $fileid data]>-1} {
                     set data1 [string tolower $data]
                     set mendindex [string first ":stop" $data1]
                     if {$mendindex<=-1} {
                       set eindex [string first "=" $data1]
                       set cindex [string first "#" $data1]
                       if {$cindex<=-1} {
                         #not a comment, see what's defined
                         for {set j 0} {$j<$nmedval} {incr j} {
                            set mvalindex [string first $medval($j) $data1]
                            if {$mvalindex>-1 && $eindex>$mvalindex} {
                               set data [string range $data [expr $eindex+1] [string length $data]]
                               set data [string trimleft $data]
                               set data [string trimright $data]
                               if {$j==0} {
                                #elements
                                while {$nelem($ninpmed) <= $max_elem} {
                                   incr nelem($ninpmed)
                                   set data [get_str_arr $data arr 0]
                                   set elements($ninpmed,$nelem($ninpmed)) $arr(0)
                                   if {$data==""} break
                                }
                                if {$nelem($ninpmed)==1} { set ipz($ninpmed) 2}
                               } elseif {$j==1} {
                                #no of atoms
                                if {$nelem($ninpmed)>1} {set ipz($ninpmed) 0}
                                set k 0
                                while {$k <= $max_elem} {
                                   incr k
                                   set data [get_str_arr $data arr 0]
                                   set pz_or_rhoz($ninpmed,$k) $arr(0)
                                   if {$data==""}  break
                                }
                               } elseif {$j==2} {
                                #rhoz
                                if {$nelem($ninpmed)>1} { set ipz($ninpmed) 1}
                                set k 0
                                while {$k <= $max_elem} {
                                   incr k
                                   set data [get_str_arr $data arr 0]
                                   set pz_or_rhoz($ninpmed,$k) $arr(0)
                                   if {$data==""}  break
                                }
                               } elseif {$j==3} {
                                #rho
                                set rho($ninpmed) $data
                               } elseif {$j==4} {
                                #stopping power type
                                set data [string tolower $data]
                                if {[string first "restricted total" $data]>-1} {
                                   set iunrst($ninpmed) 0
                                } elseif {[string first "unrestricted collision" $data]>-1} {
                                   set iunrst($ninpmed) 1
                                } elseif {[string first "unrestricted collision and radiative" $data]>-1} {
                                   set iunrst($ninpmed) 2
                                } elseif {[string first "unrestricted collision and restricted radiative" $data]>-1} {
                                   set iunrst($ninpmed) 3
                                } elseif {[string first "restricted collision and unrestricted radiative" $data]>-1} {
                                   set iunrst($ninpmed) 4
                                } elseif {[string first "unrestricted radiative" $data]>-1} {
                                   set iunrst($ninpmed) 5
                                }
                               } elseif {$j==5} {
                                #bremsstrahlung correction
                                set data [string tolower $data]
                                if {[string first "km" $data]>-1} {
                                   set iaprim($ninpmed) 0
                                } elseif {[string first "nrc" $data]>-1} {
                                   set iaprim($ninpmed) 1
                                } elseif {[string first "none" $data]>-1} {
                                   #gui will treat this as km 
                                   set iaprim($ninpmed) 0
                                }
                               } elseif {$j==6} {
                                #gasp
                                set gasp($ninpmed) $data
                                if {$gasp($ninpmed)>0} { set is_gas($ninpmed) 1}
                               } elseif {$j==7} {
                                #density correction file
                                set density_file($ninpmed) $data
                                if { $density_file($ninpmed) != ""} {
                                   set dcf_specified($ninpmed) 1
                                }
                               } elseif {$j==8} {
                                #sterncid
                                set sterncid($ninpmed) $data
                               }
                            }
                         }
                      }
                   }
                }
              }
            }
           }
         }
       }
       set pegsless_on_inp 1
     }
   }

   if {$is_pegsless==1} {
        #user requested pegsless run, load the media read here into the medium array
        if {$matfilename!=""} read_matfile
        if {$ninpmed>0} add_inp_media
   }

   #close and reopen the file so that the next read routine can start again at the beginning
   catch {close $fileid}
   set fileid [open $inp_file r]
}

proc add_inp_media {} {
global ninpmed inpmedium nmatmed medium nmed pegs

  for {set i 1} {$i<=$ninpmed} {incr i} {
     incr nmed
     set pegs($nmed) $inpmedium($i)
  }

}

proc init_pegsless_variables {} {
global is_pegsless matfilename nmatmed ninpmed
global pegsless_on_inp ae ue ap up
#just initialize a couple of required global variables

   set is_pegsless 0
   set pegsless_on_inp 0
   set ae ""
   set ue ""
   set ap ""
   set up ""
   set nmatmed 0
   set ninpmed 0
   set matfilename {}

}

set help_text(ae) {
AE,UE,AP,UP

AE,UE: define the lower and upper bounds of the energy (total) range over which electron cross-sections are generated.
AP,UP: define the lower and upper bounds of the energy range over which photon cross-sections are generated.

Energies are specified in MeV.

Default values are:
AE=ECUT
AP=PCUT
UE=50.511 MeV
UP=50.0 MeV
}

set help_text(matfile) {
The full name of a material data file.  The material data file contains a listing of media names.  Each name\
is followed by any of:
1. The elements comprising the medium and
1a. The stoichiometric coefficients of each element (PZ) or
1b. The mass fraction of each element (RHOZ)
2. The density (rho) of the medium in g/cm^3
3. The type of electron stopping power to be calculated [IUNRST].  The default is "restricted total" (IUNRST=0).
4. The type of correction applied to bremsstrahlung cross-sections [IAPRIM].  The default uses Koch and Motz (KM) empirical corrections (IAPRIM=0)
5. The gas pressure of the medium [GASP].  This is only used if the medium is a gas.  Defaults to 0.0.
6. A density correction file.
7. A Sternheimer-Seltzer-Berger ID (STERNCID).  This name is compared to a lookup table at run time and, if a match
is found, the SSB density effect parameters are applied.  Default is the medium name itself.

The only essential items required for defining a medium are 1 (defining the elements and their quantities) and\
2 (defining the medium density).  If these are not specified in the material data file, then they must either
be specified in\
the .egsinp file or a density correction file, from which these two items can be read, must be
specified.  Note that parameters for a medium specified in the .egsinp are always given priority over those\
specified in the material data file.  Thus, the .egsinp file can be used to change any of the parameters from\
their default values.

A material data file specifying the media that are also defined in 700icru.pegs4dat and 521icru.pegs4dat is included\
with the EGSnrc distribution.  Note that the parameters specified in the material data file should yield cross-sections\
identical to those in the PEGS data (for the same AE,UE,AP,UP).

Note that the browser button starts in your $EGS_HOME/pegs4/data directory.  It is assumed that if you create\
your own material data file, then it will be stored there.
}

set help_text(med_type) {
Use this menu to select how the medium will be specified.  If the medium is specified\
using the stoichiometric coefficient of each constituent element, select 'Compound.'\
To specify the medium using the fraction by weight of each element, select 'Mixture.'\
The 'Element' setting allows specification of a medium consisting of a single element.\
If you switch to the 'Element' setting after 'Mixture' or 'Compound,' any\
(disabled) stoichiometric coefficient or fraction by weight associated with the\
first element becomes meaningless.

Note that if a valid ICRU density correction file is used, the medium composition\
is read from the file, and the option to specify medium type is disabled.  It will,\
however, indicate whether the medium read in from a density correction file is an\
element or a mixture (density correction files do not specify composition by\
stoichiometric coefficient, so 'Compound' will never be indicated in this case).
} 

set help_text(rho) {
This is the density of the medium in g/cm^3 or kg/m^3.  If a valid density correction\
file is used, then this entry box will be disabled and display the density as\
read from the density correction file.
}

set help_text(iunrst) {
IUNRST

The user can choose which type of electron stopping power to output:
1. "restricted total" (IUNRST=0) is the default.  This will result in a normal simulation with\
interactions above AE treated discretely.
2. "unrestricted collision" (IUNRST=1).  This will result in collision interactions (Moeller, Bhaba) being\
treated in a CSDA manner (i.e. not as discrete events) and radiative events (bremsstrahlung, annihilation) not\
simulated at all.
3. "unrestricted collision and radiative" (IUNRST=2).  This will produce a CSDA simulation, where none of the\
electron interactions are treated in a discrete manner, and all energy from these interactions is\
deposited locally.
4. "unrestricted collision and restricted radiative" (IUNRST=3).  Only radiative events (bremsstrahlung, annihilation)\
are treated discretely.
5. "restricted collision and unrestricted radiative" (IUNRST=4).  Only collision events (Moeller, Bhaba) are\
treated discretely.
6. "unrestricted radiative" (IUNRST=5). A complement to 2 above.  Radiative events are treated in a CSDA manner\
and collision events are not simulated.
}

set help_text(iaprim) {
Check this box to apply NRC corrections (based on NIST/ICRU) to calculated\
bremsstrahlung cross-sections.  Otherwise, Koch and Motz (KM) empirical\
corrections are applied.
}

set help_text(gasp) {
Check this box if the medium is a gas.  This will enable you to specify\
a gas pressure for the medium (atm.).  If this is selected and the gas pressure\
specified is blank or <=0, then a default gas pressure of 1 atm. is used.
}

set help_text(density_file) {
This panel is enabled if you have selected the 'ICRU density correction' checkbox.\
You then have the option of entering the name of a density correction file containing density effects which,\
when applied to the calculated collision stopping powers, results in agreement with the collision stopping\
powers published in ICRU37.  This is recommended for precise dosimetry work.

You have the option of browsing for the density correction file in either your\
$EGS_HOME/pegs4/density_corrections directory (default) or in the $HEN_HOUSE/pegs4/density_corrections directory\
or explicitly entering the density correction file (with full path name).

Once a valid density correction file has been entered, the medium composition\
and density will be read from this file.  Composition and density will be displayed\
in the disabled 'Medium composition' grid and 'Mass density' entry box.

If a medium is saved with an invalid density correction file (i.e. one that cannot\
be read) then a warning will be issued and, unless another, valid density correction\
file is specified, the 'ICRU density correction' option will be unchecked and\
whatever you have entered in the 'Medium composition' table and 'Mass density' entry box\
will be used to specify medium composition and density.

}

set help_text(icru_density) {
Check this box if you want to enable the option to enter the name of a file containing\
density correction effects which, when applied to the calculated collision stopping powers,\
results in agreement with the collision stopping powers published in ICRU37.\
This is recommended for precise dosimetry work. 

Checking this box will disable the 'Medium composition' table and 'Mass density'\
entry box since the medium composition will be read from the specified density\
correction file (see below).
}

set help_text(sterncid) {
STERNCID

This is a Sternheimer-Seltzer-Berger (SSB) ID for the medium.  The STERNCID is compared to a lookup table and\
if the medium is found, then pre-calculated density effect parameters are applied to collision stopping powers.\
Otherwise, a general Sternheimer-Peierls formula is used to calculate density effect parameters.  The default\
STERNCID is the same as the medium name.

Note that if a density correction file is specified, then the density effect corrections read from the file\
are used instead of the values determined based on the STERNCID.
}
