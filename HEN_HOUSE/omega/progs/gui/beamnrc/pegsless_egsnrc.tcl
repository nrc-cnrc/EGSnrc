
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
    label .pegsless.def.text -text "Add/Modify media" -font $helvfont
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
    global matfilename num_cm env nmed medium helvfont nmatmed warnnum2
    global colorlist smhelvfont colornum egs_home hen_house default_colours
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

	# ACTIVATE THE EDIT BUTTONS if necessary
	if $num_cm>0 {
	    .cm_selected.f1.mainb configure -state normal
	    for {set i 1} {$i<=$num_cm} {incr i} {
		.cm_selected.f1.cm($i).editb configure -state normal
	    }
	}
	# Now read in the materials from the file:
        set nmed 0
        set nmatmed 0
        set medium(0) "VACUUM"
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
		    set medium($i) $data
		    incr i
                  }
                }
              }
            }
            close $file
            set nmed [expr $i-1]
            set nmatmed $nmed
            set colornum [expr [llength $colorlist]-1]
            if {$nmed>=$colornum} {
              incr warnnum2
              if {$warnnum2==1} {
                 #we do not have enough colours to display all media
                 toplevel .pegslessnoofcolors -bd 0 -class Dialog
                 wm title .pegslessnoofcolors "WARNING: Not enough colors"
                 wm transient .pegslessnoofcolors .
                 label .pegslessnoofcolors.topmsg -text "There are not enough colours to display all of the media in \
the material data file.
More than one medium will be associated with each colour.  To avoid this, add new colours
to the default colour list defined in \$OMEGA_HOME/progs/gui/beamnrc/beamnrc_gui (before `black').
Use `showrgb' for a list of colours available on your system." -font $helvfont -bd 5
                 pack .pegslessnoofcolors.topmsg
                 button .pegslessnoofcolors.okb -text "OK" -command \
                        "destroy .pegslessnoofcolors" -relief groove -bd 8
                 pack .pegslessnoofcolors.okb
                 tkwait window .pegslessnoofcolors
              }
              set colorlist [lrange $default_colours 0 [expr $colornum-1]]
              for {set i 1} {$i<=[expr $nmed/$colornum]} {incr i} {
                  set colorist [concat $colorlist [lrange $default_colours 1 [expr $colornum-1]]]
              }
              #now add black back
              lappend colorist "black"
              set colornum [expr [llength $colorlist]-1]
            }
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

global medium nmed matfilename helvfont

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
         label .matfilemed.grid.$i$j -text $medium($mednum) -bd 5
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

   set w .pegsless.def.grid

   incr ninpmed

   #set up necessary defaults
   set iaprim($ninpmed) 0
   set iunrst($ninpmed) 0
   set gasp($ninpmed) {}
   set density_file($ninpmed) {}
   set rho($ninpmed) {}
   set nelem($ninpmed) 1
   set elements($ninpmed,1) {}
   set ipz($ninpmed) 0

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
   global hen_house envi dfile_temp inpmedium sterncid

   toplevel .define$mednum -bd 5
   wm title .define$mednum "Define $inpmedium($mednum)"

   frame .define$mednum.u

   frame .define$mednum.u.l
   frame .define$mednum.u.l.grid

   label .define$mednum.u.l.grid.l0 -text "Elements"

   set label_pz_or_rhoz(0) "No. of atoms"
   set label_pz_or_rhoz(1) "Mass fractions"

   #first set up the column headers

   menubutton .define$mednum.u.l.grid.l1 -text $label_pz_or_rhoz($ipz($mednum)) -menu .define$mednum.u.l.grid.l1.m \
    -bd 1 -relief raised -indicatoron 1
   menu .define$mednum.u.l.grid.l1.m
   for {set i 0} {$i<2} {incr i} {
      .define$mednum.u.l.grid.l1.m add command -label $label_pz_or_rhoz($i) -command\
       "set ipz($mednum) $i; .define$mednum.u.l.grid.l1 configure -text {$label_pz_or_rhoz($i)}"
   }

   grid configure .define$mednum.u.l.grid.l0 -row 0 -column 0
   grid configure .define$mednum.u.l.grid.l1 -row 0 -column 1


   for {set i 1} {$i<=$nelem($mednum)} {incr i} {
      entry .define$mednum.u.l.grid.elem$i -textvariable elements($mednum,$i)
      entry .define$mednum.u.l.grid.pzorrhoz$i -textvariable pz_or_rhoz($mednum,$i)

      grid configure .define$mednum.u.l.grid.elem$i -row $i -column 0
      grid configure .define$mednum.u.l.grid.pzorrhoz$i -row $i -column 1
   }

   pack .define$mednum.u.l.grid

   frame .define$mednum.u.l.b

   button .define$mednum.u.l.b.adelem -text "Add element" -command "add_element .define$mednum.u.l.grid $mednum"
   button .define$mednum.u.l.b.delelem -text "Remove element" -command "subtract_element .define$mednum.u.l.grid $mednum"
   pack .define$mednum.u.l.b.adelem .define$mednum.u.l.b.delelem -side left

   pack .define$mednum.u.l.b

   #now set up a right frame with the other media options

   frame .define$mednum.u.r

   #rho
   frame .define$mednum.u.r.rho -bd 4
   button .define$mednum.u.r.rho.help -text "?" -command "help rho"
   label .define$mednum.u.r.rho.lab -text "rho (g/cm^3)"
   entry .define$mednum.u.r.rho.val -textvariable rho($mednum)
   pack .define$mednum.u.r.rho.help .define$mednum.u.r.rho.lab .define$mednum.u.r.rho.val -side left

   #iunrst
   set label_sp(0) "restricted total"
   set label_sp(1) "unrestricted collision"
   set label_sp(2) "unrestricted collision and radiative"
   set label_sp(3) "unrestricted collision and restricted radiative"
   set label_sp(4) "restricted collision and unrestricted radiative"
   set label_sp(5) "unrestricted radiative"

   frame .define$mednum.u.r.sp -bd 4
   button .define$mednum.u.r.sp.help -text "?" -command "help iunrst"
   label .define$mednum.u.r.sp.lab -text "stopping power type:"
   menubutton .define$mednum.u.r.sp.opt -text $label_sp($iunrst($mednum)) -menu .define$mednum.u.r.sp.opt.m \
     -bd 1 -relief raised -indicatoron 1
   menu .define$mednum.u.r.sp.opt.m
   for {set i 0} {$i<6} {incr i} {
      .define$mednum.u.r.sp.opt.m add command -label $label_sp($i) -command\
       "set iunrst($mednum) $i; .define$mednum.u.r.sp.opt configure -text {$label_sp($i)}"
   }
   pack .define$mednum.u.r.sp.help .define$mednum.u.r.sp.lab .define$mednum.u.r.sp.opt -side left

   #iaprim
   set label_iaprim(0) "KM"
   set label_iaprim(1) "NRC"
   set label_iaprim(2) "none"

   frame .define$mednum.u.r.iaprim -bd 4
   button .define$mednum.u.r.iaprim.help -text "?" -command "help iaprim"
   label .define$mednum.u.r.iaprim.lab -text "bremsstrahlung correction:"
   menubutton .define$mednum.u.r.iaprim.opt -text $label_iaprim($iaprim($mednum)) -menu .define$mednum.u.r.iaprim.opt.m \
     -bd 1 -relief raised -indicatoron 1
   menu .define$mednum.u.r.iaprim.opt.m
   for {set i 0} {$i<3} {incr i} {
      .define$mednum.u.r.iaprim.opt.m add command -label $label_iaprim($i) -command\
       "set iaprim($mednum) $i; .define$mednum.u.r.iaprim.opt configure -text {$label_iaprim($i)}"
   }
   pack .define$mednum.u.r.iaprim.help .define$mednum.u.r.iaprim.lab .define$mednum.u.r.iaprim.opt -side left

   #gasp
   frame .define$mednum.u.r.gasp -bd 4
   button .define$mednum.u.r.gasp.help -text "?" -command "help gasp"
   label .define$mednum.u.r.gasp.lab -text "gas pressure (atm.)"
   entry .define$mednum.u.r.gasp.val -textvariable gasp($mednum)
   pack .define$mednum.u.r.gasp.help .define$mednum.u.r.gasp.lab .define$mednum.u.r.gasp.val -side left

   #density correction file
   set df_area [file join $hen_house pegs4 density_corrections]
   frame .define$mednum.u.r.df -bd 4

   label .define$mednum.u.r.df.title -text "Density correction file:"

   frame  .define$mednum.u.r.df.select
   button .define$mednum.u.r.df.select.help -text "?" -command "help density_file"
   entry .define$mednum.u.r.df.select.fname -textvariable density_file($mednum)
   button .define$mednum.u.r.df.select.browse -text "Browse" -command\
     "query_filename set_dfname $df_area * $mednum"
   pack .define$mednum.u.r.df.select.help .define$mednum.u.r.df.select.fname .define$mednum.u.r.df.select.browse -side left
   pack .define$mednum.u.r.df.title .define$mednum.u.r.df.select -side top

   #sterncid
   frame .define$mednum.u.r.sterncid -bd 4
   label .define$mednum.u.r.sterncid.lab -text "sterncid"
   button .define$mednum.u.r.sterncid.help -text "?" -command "help sterncid"
   entry .define$mednum.u.r.sterncid.val -textvariable sterncid($mednum)
   pack .define$mednum.u.r.sterncid.help .define$mednum.u.r.sterncid.lab .define$mednum.u.r.sterncid.val -side left

   pack .define$mednum.u.r.rho .define$mednum.u.r.sp .define$mednum.u.r.iaprim .define$mednum.u.r.gasp .define$mednum.u.r.df .define$mednum.u.r.sterncid -side top -anchor w

   pack .define$mednum.u.l .define$mednum.u.r -side left

   #an okay button

   button .define$mednum.okb -text "Done" -command "destroy .define$mednum"
   pack .define$mednum.u .define$mednum.okb -side top
}

proc add_element { w mednum }  {
   global nelem elements pz_or_rhoz

   incr nelem($mednum)

   entry $w.elem$nelem($mednum) -textvariable elements($mednum,$nelem($mednum))
   entry $w.pzorrhoz$nelem($mednum) -textvariable pz_or_rhoz($mednum,$nelem($mednum))

   grid configure $w.elem$nelem($mednum) -row $nelem($mednum) -column 0
   grid configure $w.pzorrhoz$nelem($mednum) -row $nelem($mednum) -column 1

}

proc subtract_element { w mednum } {
   global nelem

   destroy $w.elem$nelem($mednum)
   destroy $w.pzorrhoz$nelem($mednum)

   incr nelem($mednum) -1
}

proc set_dfname { tree filename mednum } {
    global density_file
    set tempdf [file join $tree $filename]
    if [file exists $tempdf]==0 {
        tk_dialog .error "Error" "$tempdf doesn't exist!  Please try\
                again." error 0 OK
        return
    }

    #if the user has selected a file outside of the $HEN_HOUSE/density_corrections/compounds or
    #$HEN_HOUSE/density_corrections/elements directories, then keep the entire name
    set dfdir1 [file join $hen_house pegs4 density_corrections compounds]
    set dfdir2 [file join $hen_house pegs4 density_corrections elements]
    if { [string first $dfdir1 $tempdf]<=-1 && [string first $dfdir2 $tempdf]<=-1 } {
       set density_file($mednum) $tempdf
    } else {
       set sindex [string last ".density" $filename]
       set density_file($mednum) [string range $filename 0 [expr $sindex-1]]
    }
}

proc write_pegsless_data { fileid } {
#called from new_create_file_nrc
    global pegsless_on_inp is_pegsless matfilename ae ap ue up ninpmed inpmedium
    global nelem elements pz_or_rhoz ipz rho iunrst iaprim gasp density_file sterncid

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
             if {$nelem($i)>0} {
               set str " elements="
               for {set j 1} {$j<=$nelem($i)} {incr j} {
                  set str "$str $elements($i,$j)"
                  if {$j<$nelem($i)} { set str "$str," }
               }
               puts $fileid $str
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
             if {$rho($i)!=""} {
               puts $fileid " rho= $rho($i)"
             }
             if {$iunrst($i)!=""} {
               if {$iunrst($i)==0} {
                 puts $fileid " stopping powers= restricted total"
               } elseif {$iunrst($i)==1} {
                 puts $fileid " stopping powers= unrestricted collision"
               } elseif {$iunrst($i)==2} {
                 puts $fileid " stopping powers= unrestricted collision and radiative"
               } elseif {$iunrst($i)==3} {
                 puts $fileid " stopping powers= unrestricted collision and restricted radiative"
               } elseif {$iunrst($i)==4} {
                 puts $fileid " stopping powers= restricted collision and unrestricted radiative"
               } elseif {$iunrst($i)==5} {
                 puts $fileid " stopping powers= unrestricted radiative"
               }
             }
             if {$iaprim($i)!=""} {
               if {$iaprim($i)==0} {
                 puts $fileid " bremsstrahlung correction= KM"
               } elseif {$iaprim($i)==1} {
                 puts $fileid " bremsstrahlung correction= NRC"
               } elseif {$iaprim($i)==2} {
                 puts $fileid " bremsstrahlung correction= none"
               }
             }
             if {$gasp($i)!=""} {
               puts $fileid " gas pressure= $gasp($i)"
             }
             if {$density_file($i)!=""} {
               puts $fileid " density correction file= $density_file($i)"
             }
             if {$sterncid($i)!=""} {
               puts $fileid " sterncid= $sterncid($i)"
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
global inp_file arr is_pegsless sterncid

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
                               } elseif {$j==1} {
                                #no of atoms
                                set ipz($ninpmed) 0
                                set k 0
                                while {$k <= $max_elem} {
                                   incr k
                                   set data [get_str_arr $data arr 0]
                                   set pz_or_rhoz($ninpmed,$k) $arr(0)
                                   if {$data==""}  break
                                }
                               } elseif {$j==2} {
                                #rhoz
                                set ipz($ninpmed) 1
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
                                   set iaprim($ninpmed) 2
                                }
                               } elseif {$j==6} {
                                #gasp
                                set gasp($ninpmed) $data
                               } elseif {$j==7} {
                                #density correction file
                                set density_file($ninpmed) $data
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
global ninpmed inpmedium nmatmed medium nmed colornum colorlist default_colours helvfont warnnum2

# don't know what this did
#  set nmed_save $nmed
#  for {set i 1} {$i<= [expr $nmatmed+$ninpmed-$nmed_save]} {incr i} {
#     incr nmed
#     set medium($nmed) $inpmedium($ninpmed)
#  }
  for {set i 1} {$i<=$ninpmed} {incr i} {
     incr nmed
     set medium($nmed) $inpmedium($i)
  }

  set colornum [expr [llength $colorlist]-1]
  if {$nmed>=$colornum} {
      incr warnnum2
      if {$warnnum2==1} {
                 #we do not have enough colours to display all media
                 toplevel .pegslessnoofcolors -bd 0 -class Dialog
                 wm title .pegslessnoofcolors "WARNING: Not enough colors"
                 wm transient .pegslessnoofcolors .
                 label .pegslessnoofcolors.topmsg -text "There are not enough colours to display all of the media in \
the .egsinp file.
More than one medium will be associated with each colour.  To avoid this, add new colours
to the default colour list defined in \$OMEGA_HOME/progs/gui/beamnrc/beamnrc_gui (before `black').
Use `showrgb' for a list of colours available on your system." -font $helvfont -bd 5
                 pack .pegslessnoofcolors.topmsg
                 button .pegslessnoofcolors.okb -text "OK" -command \
                        "destroy .pegslessnoofcolors" -relief groove -bd 8
                 pack .pegslessnoofcolors.okb
                 tkwait window .pegslessnoofcolors
       }
       set colorlist [lrange $default_colours 0 [expr $colornum-1]]
       for {set i 1} {$i<=[expr $nmed/$colornum]} {incr i} {
                    set colorist [concat $colorlist [lrange default_colours 1 [expr $colornum-1]]]
       }
       #now add black back
       lappend colorist "black"
       set colornum [expr [llength $colorlist]-1]
  }
}

proc init_pegsless_variables {} {
global is_pegsless matfilename nmatmed ninpmed warnnum2 
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

   set warnnum2 0

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

set help_text(rho) {
RHO is the density of the medium in g/cm^3.  It is essential for specification of a medium, and if it is not\
found in the .egsinp file or in the material data file, then it will be read from the density correction\
file, if specified.
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
IAPRIM

The user can select which type of correction to apply to calculated bremsstrahlung cross-sections:
1. Koch and Motz (KM) empirical corrections (IAPRIM=0, the default).
2. NRC corrections based on NIST/ICRU (IAPRIM=1).  These corrections are read from the file\
$HEN_HOUSE/pegs4/aprime.data
3. None (IAPRIM=2)
}

set help_text(gasp) {
GASP

This is the gas pressure (in atm.) of the media.  Only used if the medium is a gas, otherwise it defaults to 0.0.
}

set help_text(density_file) {
The user has the option of entering the name of a density correction file containing density effects which,\
when applied to the calculated collision stopping powers, results in agreement with the collision stopping\
powers published in ICRU37.  This is recommended for precise dosimetry work.

If neither the directory path nor the the ".density" extension is specified, then it is assumed that\
the density correction file exists in either the $HEN_HOUSE/pegs4/density_corrections/compounds directory\
or the $HEN_HOUSE/pegs4/density_corrections/elements directory.\
Note that the browser starts in the directory $HEN_HOUSE/pegs4/density_corrections, from\
which you can enter either the "elements" or "compounds" directory to search the density correction files\
supplied with the distribution.

If you have not specified the essential composition of your medium (elements, fraction of each element, and\
medium density), then it can be inferred from the density correction file, if supplied.
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
