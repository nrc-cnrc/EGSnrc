
###############################################################################
#
#  EGSnrc BEAMnrc beamdp graphical user interface: utility functions
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


proc beamdp1 { w file } {
    global phspfile srcfilename newsrcfilename xmgrfile opt0vals fpar isub plotq
    global types waitvar errvar fields fieldtype start_dir
    global nfielde Re estype Rscoree Rtreate
    toplevel .opt0 -bd 5
    wm title .opt0 "Multiple source model"

    set text {}
    if $isub==0 {
        if {$file!="undefined"} {
          set text "Input new sub-source specifiers and analyze ph-sp data.  \
                 Reference source model is: $file"
        } else {
          reset_beamdp1_parameters
          set text "Input new sub-source specifiers and analyze ph-sp data"
        }
        set srcfilename $file
    } elseif $isub==1 {
	set text "Change sub-source specifiers in an existing source model.  \
		Source model is: $file"
    } elseif $isub==2 {
	set text "Analyze ph-sp data using sub-source specifiers from an existing model.  \
		Source model is: $file"
    } elseif $isub==3 {
	set text "Plot characteristics of a source model.  \
		Execution will call xmgr to display the result. \
		Source model is: $file"
	for {set i 1} {$i<=$opt0vals(nsrc)} {incr i} {
	    set plotq($i) 0
	}
    }

    message .opt0.label -text $text -width 7i
    pack .opt0.label -pady 10

    if $isub!=3 {
	frame .opt0.tfrm
	button .opt0.tfrm.help -text ? -command \
		"help_dialog .opt0.tfrm.help.help {Help} {The\
		title should hold detailed information\
		about the beam, including information about the machine, \
		beam energy, field size, etc.  This will serve as a reminder\
		for later use.} question 0 OK"
	textfield .opt0.tfrm.title "Title (<70 chars)" opt0vals(title)
	pack .opt0.tfrm.help -side left -padx 10
	pack .opt0.tfrm.title -side right -expand 1 -anchor e
	pack .opt0.tfrm -fill x

	frame .opt0.nsfrm
	button .opt0.nsfrm.help -text ? -command \
		"help_dialog .opt0.nsfrm.help.help {Help} {The maximum\
		number of subsources is [get_1_default NS].  This number\
		is set as a macro value at the top of beamdp.mortran and\
		can be changed.  \
		In general, one type of particle from a CM should be\
		considered to be from a sub-source.  However, scattering\
		foils, mirror and monitoring chamber can be worked out as a\
		virtual point source.  If the relative source intensity of a \
		sub-source is < 0.1% this sub-source will automatically \
		be ignored in later use.\
                Note that if you are using an existing source model as\
                a reference for your own sub-source specifier input, changing\
                specifiers in an existing source model, or analyzing ph-sp\
                data using sub-source specifiers in an existing source model,\
                you cannot change this number.} question 0 OK"
        if {$isub==2} {
             textandbutton .opt0.nsfrm.numsrc "Number of subsources" opt0vals(nsrc) \
"View subsources..." {if  $opt0vals(nsrc)<4 {define_subsources} else {define_lots_of_subsources}}
        } else {
	     textandbutton .opt0.nsfrm.numsrc "Number of subsources" opt0vals(nsrc) \
               "Define subsources..." {if  $opt0vals(nsrc)<4 {define_subsources} else {define_lots_of_subsources}}
        }
	pack .opt0.nsfrm.help -side left -padx 10
	pack .opt0.nsfrm.numsrc -side right -expand 1 -anchor e
	pack .opt0.nsfrm -fill x

	if { $isub==1 || $isub==2 } {
	    # The user isn't allowed to change the number of subsources
	    .opt0.nsfrm.numsrc.inp configure -state disabled
	    .opt0.nsfrm.numsrc.inp configure -bg grey
	} elseif { $isub==0 && $file!="undefined" } {
            # The user isn't allowed to change the number of subsources
            .opt0.nsfrm.numsrc.inp configure -state disabled
            .opt0.nsfrm.numsrc.inp configure -bg grey
        }

	frame .opt0.sep1 -bd 4 -width 100 -height 2 -relief groove
	pack .opt0.sep1 -fill x -pady 10

	label .opt0.enlab -text "Energy spectrum"
	pack .opt0.enlab -pady 5

	frame .opt0.en
	button .opt0.en.help -text ? -command \
		"help_dialog .opt0.en.help.help {Help} \
		{Energy Spectrum

	BEAMDP analyzes the BEAM phase-space data and generates energy spectra.\
		There are two options if you are using a circular field with\
                circular ring bins or a square field with square ring bins\
                (see 'Define subsources' button):
                1) Generate one energy spectrum for particles inside the\
                treatment field and another for particles outside the treatment\
                field (but inside the scoring field).  In this case, you must\
                specify the radii or half-widths (if you are using a square\
                field) of the treatment and scoring fields.
                2) Generate spectra in rings with user-specified outer radii or\
                half-widths (if using the square field).\
                You must specify the number of radii/half-widths and\
                then define the outer radius/half-width of each ring.  You must\
                also supply the radius/half-width of the scoring field.  The\
                outer radius/half-width of the outermost ring must be =\
                the radius/half-width of the scoring field.

        Option 1) is simpler, but should be adequate for beams in which mean\
                particle energy varies significantly around the treatment edge\
                but remains fairly constant well-inside or well-outside the\
                treatment field (away from the field edges).

                Note that the\
                radii/half-widths of the treatment and scoring fields\
                specified here need not be equal to those specified for\
                fluence (i.e. when you press the\
                'Define subsources' button).

        If you are using a rectangular field\
                with rectangular bins (see 'Define subsources' button) then\
                the options are disabled, and two energy spectra are\
                determined: one inside the treatment field and one outside\
                the treatment field.  The dimensions of the treatment and\
                scoring fields in this case are equal to those specified\
                for fluence.

	The maximum number of energy bins allowed for spectra 200 but this can be changed\
		by modifying the beamdp.mortran program (variable NB).  \
		Since electron depth-dose curves are sensitive to the electron\
		incident energy, especially to the energies of the direct\
		electrons, which usually have a very narrow peak, it is\
		suggested that 0.1 MeV bin width be used for an energy\
		distribution.

        Note that you cannot change the energy spectrum of your source model\
               if you are analyzing ph-sp data using sub-source specifiers from\
               an existing model.} question 0 OK"
	frame .opt0.en.right
	textfield .opt0.en.right.nbin "Number of bins for an energy distribution" opt0vals(nbin)
	textfield .opt0.en.right.emin "Minimum kinetic energy of phase-space particles in file (MeV)" opt0vals(emin)
	textfield .opt0.en.right.emax "Maximum kinetic energy of phase-space particles in file (MeV)" opt0vals(emax)
        if $isub==2 {
          .opt0.en.right.nbin.inp configure -state disabled -bg grey
          .opt0.en.right.emin.inp configure -state disabled -bg grey
          .opt0.en.right.emax.inp configure -state disabled -bg grey
        }

        label .opt0.en.right.etype -text "Determine spectrum:"

        frame .opt0.en.right.espec
        frame .opt0.en.right.espec.estype0
        radiobutton .opt0.en.right.espec.estype0.choice -text "Inside/Outside treatment field" -variable estype -value 0 -command { \
            .opt0.en.right.espec.estype0.rinner.lab configure -fg black
            .opt0.en.right.espec.estype0.rinner.inp configure -state normal
            .opt0.en.right.espec.estype0.router.lab configure -fg black
            .opt0.en.right.espec.estype0.router.inp configure -state normal
            .opt0.en.right.espec.estype1.nfield.lab configure -fg grey
            .opt0.en.right.espec.estype1.nfield.inp configure -state disabled
            .opt0.en.right.espec.estype1.rfielddef configure -state disabled
            .opt0.en.right.espec.estype1.router.lab configure -fg grey
            .opt0.en.right.espec.estype1.router.inp configure -state disabled
        }

        frame .opt0.en.right.espec.estype0.rinner
        label .opt0.en.right.espec.estype0.rinner.lab -text "R treatment field"
        entry .opt0.en.right.espec.estype0.rinner.inp -textvariable Rtreate
        pack .opt0.en.right.espec.estype0.rinner.lab .opt0.en.right.espec.estype0.rinner.inp -side left
        frame .opt0.en.right.espec.estype0.router
        label .opt0.en.right.espec.estype0.router.lab -text "R scoring field"
        entry .opt0.en.right.espec.estype0.router.inp -textvariable Rscoree
        if {$estype==1} {
            .opt0.en.right.espec.estype0.rinner.lab configure -fg grey
            .opt0.en.right.espec.estype0.rinner.inp configure -state disabled
            .opt0.en.right.espec.estype0.router.lab configure -fg grey
            .opt0.en.right.espec.estype0.router.inp configure -state disabled
        }
        pack .opt0.en.right.espec.estype0.router.lab .opt0.en.right.espec.estype0.router.inp -side left
        pack .opt0.en.right.espec.estype0.choice .opt0.en.right.espec.estype0.rinner .opt0.en.right.espec.estype0.router -side left -padx 10

        frame .opt0.en.right.espec.estype1
        radiobutton .opt0.en.right.espec.estype1.choice -text "At specified radii" -variable estype -value 1 -command { \
            .opt0.en.right.espec.estype0.rinner.lab configure -fg grey
            .opt0.en.right.espec.estype0.rinner.inp configure -state disabled
            .opt0.en.right.espec.estype0.router.lab configure -fg grey
            .opt0.en.right.espec.estype0.router.inp configure -state disabled
            .opt0.en.right.espec.estype1.nfield.lab configure -fg black
            .opt0.en.right.espec.estype1.nfield.inp configure -state normal
            .opt0.en.right.espec.estype1.rfielddef configure -state normal
            .opt0.en.right.espec.estype1.router.lab configure -fg black
            .opt0.en.right.espec.estype1.router.inp configure -state normal
        }
        frame .opt0.en.right.espec.estype1.nfield
        label .opt0.en.right.espec.estype1.nfield.lab -text "No. of radii"
        entry .opt0.en.right.espec.estype1.nfield.inp -textvariable nfielde
        pack .opt0.en.right.espec.estype1.nfield.lab .opt0.en.right.espec.estype1.nfield.inp -side left
        frame .opt0.en.right.espec.estype1.router
        label .opt0.en.right.espec.estype1.router.lab -text "R scoring field"
        entry .opt0.en.right.espec.estype1.router.inp -textvariable Rscoree
        pack .opt0.en.right.espec.estype1.router.lab .opt0.en.right.espec.estype1.router.inp -side left
        button .opt0.en.right.espec.estype1.rfielddef -text "Define Radii" \
             -command "rfielde_def"
        if {$estype==0} {
            .opt0.en.right.espec.estype1.nfield.lab configure -fg grey
            .opt0.en.right.espec.estype1.nfield.inp configure -state disabled
            .opt0.en.right.espec.estype1.rfielddef configure -state disabled
            .opt0.en.right.espec.estype1.router.lab configure -fg grey
            .opt0.en.right.espec.estype1.router.inp configure -state disabled
        }

        if {$isub==2} {
        #disable energy spectra field inputs
            .opt0.en.right.espec.estype0.rinner.lab configure -fg grey
            .opt0.en.right.espec.estype0.rinner.inp configure -state disabled
            .opt0.en.right.espec.estype0.router.lab configure -fg grey
            .opt0.en.right.espec.estype0.router.inp configure -state disabled
            .opt0.en.right.espec.estype1.nfield.lab configure -fg grey
            .opt0.en.right.espec.estype1.nfield.inp configure -state disabled
            .opt0.en.right.espec.estype1.rfielddef configure -state disabled
            .opt0.en.right.espec.estype1.router.lab configure -fg grey
            .opt0.en.right.espec.estype1.router.inp configure -state disabled
            .opt0.en.right.espec.estype0.choice configure -state disabled
            .opt0.en.right.espec.estype1.choice configure -state disabled
        }

        pack .opt0.en.right.espec.estype1.choice .opt0.en.right.espec.estype1.nfield .opt0.en.right.espec.estype1.router .opt0.en.right.espec.estype1.rfielddef -side left -padx 10

        pack .opt0.en.right.espec.estype0 .opt0.en.right.espec.estype1 -side top -anchor w
        pack .opt0.en.right.etype .opt0.en.right.espec -side left -padx 10

	pack .opt0.en.help -side left -anchor w -padx 10
	pack .opt0.en.right -side right -anchor e -fill x
	pack .opt0.en

	frame .opt0.sep2 -bd 4 -width 100 -height 2 -relief groove
	pack .opt0.sep2 -fill x -pady 10

	textandbutton .opt0.phsp "Phase space filename" phspfile "Browse" \
		{browse "$start_dir" set_phspfile 0 {Select the phase space file:}}
        if { $isub==0 && $file=="undefined" } {
            set newsrcfilename ""
            textandbutton .opt0.srcf "Save new source model as:" newsrcfilename\
                   "Browse" {browse "$start_dir" set_newsrcfilename 1 {Save the source information as:}}
	} else {
	    set w .opt0.srcf
	    frame $w
	    label $w.label -text "Existing source model:"
	    entry $w.inp -textvariable srcfilename -state disabled -bg grey
	    pack $w.label -anchor w -side left
	    pack $w.inp -anchor e -side right -fill x -expand true
	    pack $w -pady 5 -fill x -padx 10

	    textandbutton .opt0.srcnf "Save new source model as:" newsrcfilename\
		    "Browse" {browse "$start_dir" set_newsrcfilename 1 {Enter the source info filename to write the new information to.}}
        }
    } else {
	# For each subsource, allow the user to select whether to plot it
	frame .opt0.grid
	set w .opt0.grid

	for {set i 0} {$i<=3} {incr i} {
	    if [string compare $fields($fieldtype) $fields($i)]==0 {
		set fldidx $i
		break
	    }
	}
	set plots(0) "No"
	set plots(1) "E - inside treatment field"
	set plots(2) "E - outside treatment field"
	set plots(3) "E - in annulus"
	set plots(4) "Fluence"

	label $w.labty -text "Type"
	grid config $w.labty -row 0 -column 1
	label $w.labch -text "Charge"
	grid config $w.labch -row 0 -column 2
	label $w.labpl -text "Plot"
	grid config $w.labpl -row 0 -column 3
	for {set i 1} {$i<=$opt0vals(nsrc)} {incr i} {
	    label $w.lab$i -text "Subsource $i"
	    grid config $w.lab$i -row $i -column 0
	    label $w.type$i -text $types($opt0vals(type,$i))
	    grid config $w.type$i -row $i -column 1
	    label $w.charge$i -text $opt0vals(charge,$i)
	    grid config $w.charge$i -row $i -column 2
	    menubutton $w.plot$i -text $plots($plotq($i)) -menu $w.plot$i.m\
		    -relief raised -width 25 -bd 1
	    menu $w.plot$i.m
	    if $fldidx==3 {
		foreach j {0 3 4} {
		    $w.plot$i.m add command -label $plots($j) -command \
	    "set plotq($i) $j; set_plot_radii $j $i; $w.plot$i configure -text\
			    {$plots($j)}"
		}
	    } else {
		foreach j {0 1 2 4} {
		    $w.plot$i.m add command -label $plots($j) -command \
			    "set plotq($i) $j; $w.plot$i configure -text\
			    {$plots($j)}"
		}
	    }
	    grid config $w.plot$i -row $i -column 3 -padx 5 -pady 5
	}

	pack $w -pady 10
	textandbutton .opt0.srcf "Save xmgr file as:" xmgrfile "Browse" \
		{browse "$start_dir" set_xmgrfile 1 {Set the xmgr filename:}}
    }
    # 3 buttons, exec, save, load, last 2 get a browser.
    frame .opt0.bfrm -bd 1 -relief raised
    button .opt0.bfrm.exec -text "Execute beamdp" -command {
	set returnval [create_beamdp1_script]
	if $returnval==1 {
	    toplevel .run -bd 5
	    wm title .run "beamdp Output"
	    frame .run.frm
	    scrollbar .run.frm.scroll2 -command {.run.frm.dialg yview} \
		    -orient vertical
	    text .run.frm.dialg -yscroll {.run.frm.scroll2 set}
	    pack .run.frm.dialg .run.frm.scroll2 -side left -fill y
	    button .run.buts -text "Close" -command "destroy .run"
	    pack .run.frm .run.buts -side top -pady 10
	    .run.frm.dialg insert end "\n Running beamdp... \n\n"
	    set waitvar 0
	    set errvar 0
	    startjob
	    displaydialg "temp"
	    grab .run
	    tkwait variable waitvar
	    grab release .run
	    if $errvar==1 {
		tk_dialog .error "Error" "Beamdp has aborted for some \
			reason.  Please examine the output to determine\
			the cause. " error 0 OK
	    }
            if {$isub!=3} {
             if {[file exists $newsrcfilename.log]==1} {
               file delete $newsrcfilename.log
             }
             file copy temp $newsrcfilename.log
            } else {
             if {[file exists $xmgrfile.log]==1} {
               file delete $xmgrfile.log
             }
             file copy temp $xmgrfile.log
            }
	}
    }
    button .opt0.bfrm.can -text "Cancel" -command "destroy .opt0"
    pack .opt0.bfrm.exec .opt0.bfrm.can -side left -padx 5 -pady 5
    pack .opt0.bfrm -pady 5

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
    if [catch {set val [expr $val]}]==1 {
	set val {}
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

proc load_source_info {} {
    global srcfilename opt0vals fpar phspfile types nvar fieldtype fields
    global nfielde Re Rscoree Rtreate estype

    set id [open $srcfilename r]
    gets $id opt0vals(title)
    gets $id opt0vals(nsrc)
    for {set i 1} {$i<=$opt0vals(nsrc)} {incr i} {
	gets $id data
	set data [get_val $data opt0vals type,$i]
	set data [get_val $data opt0vals charge,$i]
	if $opt0vals(charge,$i)==-1 {
	    set opt0vals(charge,$i) electrons
	} elseif $opt0vals(charge,$i)==0 {
	    set opt0vals(charge,$i) photons
	} else {
	    set opt0vals(charge,$i) positrons
	}
	set data [get_val $data opt0vals latch,$i]

	gets $id data
	for {set j 1} {$j<=$nvar($opt0vals(type,$i))} {incr j} {
	    set data [get_val $data opt0vals zmin,$i,$j]
	    if [string compare $opt0vals(zmin,$i,$j) {}]==0 {
		#we're out of numbers on this line, get the next line and redo
		gets $id data
		set data [get_val $data opt0vals zmin,$i,$j]
	    }
	}
    }
    gets $id data
    set data [get_val $data opt0vals nbin]
    set data [get_val $data opt0vals emin]
    set data [get_val $data opt0vals emax]
    gets $id estype
    set estype [string trimleft $estype]
    set estype [string trimright $estype]
    gets $id data
    for {set i 1} {$i<=2} {incr i} {
       set data [get_val $data Re $i]
    }
    if {$estype==1} {
       set nfielde $Re(1)
       set Rscoree $Re(2)
       gets $id data
       for {set i 1} {$i<=$nfielde} {incr i} {
         set data [get_val $data Re $i]
       }
    } else {
       set Rtreate $Re(1)
       set Rscoree $Re(2)
    }
    gets $id fieldtype
    set fieldtype [string trimleft $fieldtype]
    set fieldtype [string trimright $fieldtype]
    if {$fieldtype==2} {
	set pmax 5
    } else {
	set pmax 4
    }
    gets $id data
    for { set j 1 } {$j<=$pmax} {incr j} {
	set data [get_val $data fpar $j]
    }
    if {$fieldtype==2} {
      #4 more parameters to get
       gets $id data
       for { set j 6 } {$j<=9} {incr j} {
          set data [get_val $data fpar $j]
       }
    }

    gets $id phspfile
    set phspfile [string trimright $phspfile]

    # If we're doing choice 3, have to input more stuff
    # 3- plot graphs using data from a source parameter file.
    # Fill in later...
}

proc textfield { w label var } {
    frame $w
    label $w.label -text $label
    entry $w.inp -textvariable $var -width 10
    pack $w.label -anchor w -side left
    pack $w.inp -anchor e -side right -fill x -expand true
    pack $w -pady 5 -fill x -padx 10
}

proc textandbutton { w label1 var label2 command } {
    frame $w
    label $w.label -text $label1
    entry $w.inp -textvariable $var
    button $w.but -text $label2 -command $command
    pack $w.label -anchor w -side left
    pack $w.inp -side left -fill x -expand true
    pack $w.but -anchor e -side right
    pack $w -pady 5 -fill x -padx 10

}

proc define_subsources {} {
    # FOR LESS THAN 4 SUBSOURCES DO THIS ONE
    # For each subsource, define type, charge, latch
    # Type menu leads to type options
    # also put in a field type menu which leads to options.
    global opt0vals fieldtype fields types fluence_help_text subsource_help_text
    global isub

    set top .opt0

    catch { destroy $top.buts }
    toplevel $top.buts -bd 10
    set w $top.buts
    wm title $w "Intermediate window"

    # FOR EACH SUBSOURCE PUT ON 3 MENUS TO CHOOSE CHARGE, TYPE, LATCH.
    # Type connects to define_subs_window

    message $w.lab -text "Define each subsource.  A window should appear to \
	    define the subsource model when one is selected.  If the type\
	    appears in red, it has not been completely defined." -width 7i
    pack $w.lab -pady 10

    frame $w.grid

    label $w.grid.clab -text "Charge"
    grid config $w.grid.clab -row 1 -column 0 -sticky e
    label $w.grid.llab -text "LATCH bit"
    grid config $w.grid.llab -row 2 -column 0 -sticky e
    label $w.grid.tlab -text "Type"
    grid config $w.grid.tlab -row 3 -column 0 -sticky e

    for {set i 1} {$i<=$opt0vals(nsrc)} {incr i} {
	label $w.grid.lab$i -text "Subsource $i"
	grid config $w.grid.lab$i -row 0 -column $i

        if {$opt0vals(charge,$i)==""} {
           menubutton $w.grid.chargemb$i -text "Select charge" \
                -menu $w.grid.chargemb$i.m -bd 1 -relief raised -indicatoron 1
        } else {
	   menubutton $w.grid.chargemb$i -text $opt0vals(charge,$i) \
		-menu $w.grid.chargemb$i.m -bd 1 -relief raised -indicatoron 1
        }
	menu $w.grid.chargemb$i.m -tearoff no
	foreach ctype {electrons photons positrons} {
	    $w.grid.chargemb$i.m add command -label $ctype -command \
		    "set opt0vals(charge,$i) $ctype; \
		    $w.grid.chargemb$i configure -text $ctype"
	}
	grid config $w.grid.chargemb$i -row 1 -column $i -sticky ew

	entry $w.grid.latch$i -textvariable opt0vals(latch,$i)
	grid config $w.grid.latch$i -row 2 -column $i -sticky ew
        if $isub==2 {
           $w.grid.latch$i configure -state disabled -bg grey
        }

	menubutton $w.grid.typemb$i -text $types($opt0vals(type,$i)) -menu \
		$w.grid.typemb$i.m -bd 1 -relief raised -indicatoron 1
	menu $w.grid.typemb$i.m -tearoff no
	foreach j {1 2 3 4 5 11} {
	    $w.grid.typemb$i.m add command -label $types($j) -command \
		    "set opt0vals(type,$i) $j; \
		    $w.grid.typemb$i configure -text {$types($j)};\
		    define_subs_window $w $i"
	}
	grid config $w.grid.typemb$i -row 3 -column $i -sticky ew

        if $isub==2 {
           $w.grid.chargemb$i configure -state disabled
           $w.grid.latch$i configure -state disabled -bg grey
           $w.grid.typemb$i configure -state disabled -bg grey
        }

	set_button_color $i $w
    }
    pack $w.grid -pady 5

    # PUT ON THE FIELD TYPE OPTION
    button $w.helpb -text ? -command "help_dialog $w.helpb.help {Help} {$subsource_help_text} question 0 OK" -relief groove -bd 8
    pack $w.helpb -anchor e

    frame $w.sep7 -bd 4 -width 100 -height 2 -relief groove
    pack $w.sep7 -fill x -pady 10

    label $w.fieldlabel -text "Set the type of field to use:"
    pack $w.fieldlabel -pady 5

    frame $w.ffrm
    button $w.ffrm.help -text ? -command "help_dialog $w.ffrm.help.help {Help} {$fluence_help_text} question 0 OK"

    if {$fieldtype==""} {
        menubutton $w.ffrm.fieldm -text "set field type" -menu \
            $w.ffrm.fieldm.m -bd 1 -relief raised -indicatoron 1
    } else {
        menubutton $w.ffrm.fieldm -text $fields($fieldtype) -menu \
            $w.ffrm.fieldm.m -bd 1 -relief raised -indicatoron 1
    }
    menu $w.ffrm.fieldm.m -tearoff no
    for {set i 0} {$i <= 2} {incr i} {
	$w.ffrm.fieldm.m add command -label $fields($i)\
		-command "get_field_beamdp1_parameters $i $w"
    }
    pack $w.ffrm.help -side left -anchor w -padx 10
    pack $w.ffrm.fieldm -fill x -expand 1 -padx 10
    pack $w.ffrm -fill x

    if {$fieldtype!=""} {

      for {set i 0} {$i<=2} {incr i} {
        if [string compare $fields($fieldtype) $fields($i)]==0 {
            get_field_beamdp1_parameters $i $w
            break
        }
      }
    }

    if $isub==2 {
       $w.ffrm.fieldm configure -state disabled
    }

    frame $w.buts
    button $w.buts.closeb -text "Close" -command "destroy $w"\
	    -relief groove -bd 8
    #    button $w.buts.helpb -text "Help" -command "help_dialog $w.buts.helpb.help {Help} {$subsource_help_text} question 0 OK" -relief groove -bd 8

    pack $w.buts.closeb -side left -padx 10
    pack $w.buts -side bottom -pady 5 -anchor se
}

proc define_lots_of_subsources {} {
    # FOR MORE THAN 3 SUBSOURCES, DO THIS ONE
    # On this window, put on buttons for each subsource, each leading to
    # another window to define it.  Then put on field type stuff.
    global opt0vals fieldtype fields types fluence_help_text isub

    # First check that they haven't input more than allowed
    set mxss [get_1_default NS]
    if $opt0vals(nsrc)>$mxss {
	tk_dialog .failure "Error" "The maximum number of subsources allowed\
		is at this time $mxss." error 0 OK
	return
    }

    set top .opt0

    catch { destroy $top.buts }
    toplevel $top.buts -bd 10
    set w $top.buts
    wm title $w "Intermediate window"

    set nrow [expr $opt0vals(nsrc)/2]
    if [expr $opt0vals(nsrc)%2]!=0 { incr nrow }

    frame $w.grid

    set isrc 1
    for {set i 1} {$i<=2} {incr i} {
	for {set j 1} {$j<=$nrow} {incr j} {
	    button $w.grid.but$isrc -text "Subsource $isrc" \
		    -command "define_ss $isrc"
	    grid config $w.grid.but$isrc -row $j -column $i
	    incr isrc
	    if $isrc>$opt0vals(nsrc) break
	}
	if $isrc>$opt0vals(nsrc) break
    }
    pack $w.grid

    # PUT ON THE FIELD TYPE OPTION

    frame $w.sep7 -bd 4 -width 100 -height 2 -relief groove
    pack $w.sep7 -fill x -pady 10

    label $w.fieldlabel -text "Set the type of field to use:"
    pack $w.fieldlabel -pady 5

    frame $w.ffrm
    button $w.ffrm.help -text ? -command "help_dialog $w.ffrm.help.help {Help} {$fluence_help_text} question 0 OK"

    if {$fieldtype==""} {
        menubutton $w.ffrm.fieldm -text "set field type" -menu \
            $w.ffrm.fieldm.m -bd 1 -relief raised -indicatoron 1
    } else {
        menubutton $w.ffrm.fieldm -text $fields($fieldtype) -menu \
	    $w.ffrm.fieldm.m -bd 1 -relief raised -indicatoron 1
    }
    menu $w.ffrm.fieldm.m -tearoff no
    for {set i 0} {$i <= 2} {incr i} {
	$w.ffrm.fieldm.m add command -label $fields($i)\
		-command "get_field_beamdp1_parameters $i $w"
    }
    pack $w.ffrm.help -side left -anchor w -padx 10
    pack $w.ffrm.fieldm -fill x -expand 1 -padx 10
    pack $w.ffrm -fill x

    if {$fieldtype!=""} {

      for {set i 0} {$i<=2} {incr i} {
	if [string compare $fields($fieldtype) $fields($i)]==0 {
	    get_field_beamdp1_parameters $i $w
	    break
	}
      }
    }

    if $isub==2 {
       $w.ffrm.fieldm configure -state disabled
    }

    button $w.closeb -text "Close" -command "destroy $w"\
	    -relief groove -bd 8
    pack $w.closeb -side bottom -pady 5 -anchor se
}


proc define_ss { i } {
    global opt0vals types subsource_help_text isub
    # FOR EACH SUBSOURCE PUT ON 3 MENUS TO CHOOSE CHARGE, TYPE, LATCH.
    # Type connects to define_subs_window

    toplevel .opt0.buts.ss$i
    set w .opt0.buts.ss$i
    wm title $w "Subsource $i"

    message $w.lab -text "Define subsource $i.  A window should appear to \
	    define the subsource model when one is selected.  If the type\
	    appears in red, it has not been completely defined." -width 7i
    pack $w.lab -pady 10

    frame $w.grid

    label $w.grid.clab -text "Charge"
    grid config $w.grid.clab -row 1 -column 0 -sticky e
    label $w.grid.llab -text "LATCH bit"
    grid config $w.grid.llab -row 2 -column 0 -sticky e
    label $w.grid.tlab -text "Type"
    grid config $w.grid.tlab -row 3 -column 0 -sticky e

    label $w.grid.lab$i -text "Subsource $i"
    grid config $w.grid.lab$i -row 0 -column $i

    if {$opt0vals(charge,$i)==""} {
         menubutton $w.grid.chargemb$i -text "select charge" \
            -menu $w.grid.chargemb$i.m -bd 1 -relief raised -indicatoron 1
    } else {
         menubutton $w.grid.chargemb$i -text $opt0vals(charge,$i) \
	    -menu $w.grid.chargemb$i.m -bd 1 -relief raised -indicatoron 1
    }
    menu $w.grid.chargemb$i.m -tearoff no
    foreach ctype {electrons photons positrons} {
	$w.grid.chargemb$i.m add command -label $ctype -command \
		"set opt0vals(charge,$i) $ctype; \
		$w.grid.chargemb$i configure -text $ctype"
    }
    grid config $w.grid.chargemb$i -row 1 -column $i -sticky ew

    entry $w.grid.latch$i -textvariable opt0vals(latch,$i)
    grid config $w.grid.latch$i -row 2 -column $i -sticky ew

    if {$opt0vals(type,$i)==""} {
        menubutton $w.grid.typemb$i -text "select subsource type" -menu \
            $w.grid.typemb$i.m -bd 1 -relief raised -indicatoron 1
    } else {
        menubutton $w.grid.typemb$i -text $types($opt0vals(type,$i)) -menu \
	    $w.grid.typemb$i.m -bd 1 -relief raised -indicatoron 1
    }
    menu $w.grid.typemb$i.m -tearoff no
    foreach j {1 2 3 4 5 11} {
	$w.grid.typemb$i.m add command -label $types($j) -command \
		"set opt0vals(type,$i) $j; \
		$w.grid.typemb$i configure -text {$types($j)};\
		define_subs_window $w $i"
    }
    grid config $w.grid.typemb$i -row 3 -column $i -sticky ew

    if $isub==2 {
        $w.grid.chargemb$i configure -state disabled
        $w.grid.latch$i configure -state disabled -bg grey
        $w.grid.typemb$i configure -state disabled
    }

    set_button_color $i $w

    pack $w.grid -pady 5

    frame $w.buts
    button $w.buts.closeb -text "Close" -command "destroy $w"\
	    -relief groove -bd 8
    button $w.buts.helpb -text "Help" -command "help_dialog $w.buts.helpb.help {Help} {$subsource_help_text} question 0 OK" -relief groove -bd 8

    pack $w.buts.helpb $w.buts.closeb -side left -padx 10
    pack $w.buts -side bottom -pady 5 -anchor se
}

proc get_field_beamdp1_parameters { i par } {
    global fields fpar fieldtype isub estype

    $par.ffrm.fieldm configure -text $fields($i)

    catch { destroy $par.fieldp }

    frame $par.fieldp -bd 4

    set fieldtype $i

    if $i==0 {
	set ftext(1) "No. bins for planar fluence distribution inside treatment field"
	set ftext(2) "Radius of the treatment field (cm)"
	set ftext(3) "No. bins for planar fluence distribution outside treatment field"
        set ftext(4) "Radius of scoring field (cm)"
	set pmax 4
        #enable energy spectr field inputs
        .opt0.en.right.espec.estype0.choice configure -state normal
        .opt0.en.right.espec.estype1.choice configure -state normal
        if {$estype==1} {
            .opt0.en.right.espec.estype0.rinner.lab configure -fg grey
            .opt0.en.right.espec.estype0.rinner.inp configure -state disabled
            .opt0.en.right.espec.estype0.router.lab configure -fg grey
            .opt0.en.right.espec.estype0.router.inp configure -state disabled
            .opt0.en.right.espec.estype1.nfield.lab configure -fg black
            .opt0.en.right.espec.estype1.nfield.inp configure -state normal
            .opt0.en.right.espec.estype1.rfielddef configure -state normal
            .opt0.en.right.espec.estype1.router.lab configure -fg black
            .opt0.en.right.espec.estype1.router.inp configure -state normal
        } elseif {$estype==0} {
            .opt0.en.right.espec.estype0.rinner.lab configure -fg black
            .opt0.en.right.espec.estype0.rinner.inp configure -state normal
            .opt0.en.right.espec.estype0.router.lab configure -fg black
            .opt0.en.right.espec.estype0.router.inp configure -state normal
            .opt0.en.right.espec.estype1.nfield.lab configure -fg grey
            .opt0.en.right.espec.estype1.nfield.inp configure -state disabled
            .opt0.en.right.espec.estype1.rfielddef configure -state disabled
            .opt0.en.right.espec.estype1.router.lab configure -fg grey
            .opt0.en.right.espec.estype1.router.inp configure -state disabled
        }
    } elseif $i==1 {
	set ftext(1) "No. bins for planar fluence distribution inside treatment field"
	set ftext(2) "Half-width of the treatment field (cm)"
        set ftext(3) "No. bins for planar fluence distribution outside treatment field"
        set ftext(4) "Half-width of the scoring field (cm)"
 	set pmax 4
        #enable energy spectr field inputs
        .opt0.en.right.espec.estype0.choice configure -state normal
        .opt0.en.right.espec.estype1.choice configure -state normal
        if {$estype==1} {
            .opt0.en.right.espec.estype0.rinner.lab configure -fg grey
            .opt0.en.right.espec.estype0.rinner.inp configure -state disabled
            .opt0.en.right.espec.estype0.router.lab configure -fg grey
            .opt0.en.right.espec.estype0.router.inp configure -state disabled
            .opt0.en.right.espec.estype1.nfield.lab configure -fg black
            .opt0.en.right.espec.estype1.nfield.inp configure -state normal
            .opt0.en.right.espec.estype1.rfielddef configure -state normal
            .opt0.en.right.espec.estype1.router.lab configure -fg black
            .opt0.en.right.espec.estype1.router.inp configure -state normal
        } elseif {$estype==0} {
            .opt0.en.right.espec.estype0.rinner.lab configure -fg black
            .opt0.en.right.espec.estype0.rinner.inp configure -state normal
            .opt0.en.right.espec.estype0.router.lab configure -fg black
            .opt0.en.right.espec.estype0.router.inp configure -state normal
            .opt0.en.right.espec.estype1.nfield.lab configure -fg grey
            .opt0.en.right.espec.estype1.nfield.inp configure -state disabled
            .opt0.en.right.espec.estype1.rfielddef configure -state disabled
            .opt0.en.right.espec.estype1.router.lab configure -fg grey
            .opt0.en.right.espec.estype1.router.inp configure -state disabled
        }
    } elseif $i==2 {
	set ftext(1) "Number of bins for a planar fluence distribution (same for x and y)"
	set ftext(2) "Minimum x coordinate for the scoring field"
	set ftext(3) "Maximum x coordinate for the scoring field"
	set ftext(4) "Minimum y coordinate for the scoring field"
	set ftext(5) "Maximum y coordinate for the scoring field"
	set ftext(6) "Minimum x coordinate for the treatment field"
	set ftext(7) "Maximum x coordinate for the treatment field"
	set ftext(8) "Minimum y coordinate for the treatment field"
	set ftext(9) "Maximum y coordinate for the treatment field"
	set pmax 9
        #disable energy spectra field inputs
        .opt0.en.right.espec.estype0.rinner.lab configure -fg grey
        .opt0.en.right.espec.estype0.rinner.inp configure -state disabled
        .opt0.en.right.espec.estype0.router.lab configure -fg grey
        .opt0.en.right.espec.estype0.router.inp configure -state disabled
        .opt0.en.right.espec.estype1.nfield.lab configure -fg grey
        .opt0.en.right.espec.estype1.nfield.inp configure -state disabled
        .opt0.en.right.espec.estype1.rfielddef configure -state disabled
        .opt0.en.right.espec.estype1.router.lab configure -fg grey
        .opt0.en.right.espec.estype1.router.inp configure -state disabled
        .opt0.en.right.espec.estype0.choice configure -state disabled
        .opt0.en.right.espec.estype1.choice configure -state disabled
    }

    for {set j 1} {$j<=$pmax} {incr j} {
	textfield $par.fieldp.inp$j $ftext($j) fpar($j)
        if $isub==2 {
           $par.fieldp.inp$j.inp configure -state disabled -bg grey
        }
    }
    pack $par.fieldp
}

proc set_button_color { i w } {
    global opt0vals nvar
    # i is the subsource to be checked.

    if $opt0vals(type,$i)==0 {
	# Not set yet.  Red.
	$w.grid.typemb$i configure -fg red
	return
    }
    for {set k 1} {$k<=$nvar($opt0vals(type,$i))} {incr k} {
	if [catch {expr $opt0vals(zmin,$i,$k)}]==1 {
	    $w.grid.typemb$i configure -fg red
	    return
	}
    }
    $w.grid.typemb$i configure -fg black
}

proc define_subs_window { parent i } {
    global opt0vals nvar ssddefined

    catch { destroy $parent.grid.baby$i }
    toplevel $parent.grid.baby$i -bd 6
    set top $parent.grid.baby$i
    wm title $top "Define subsource $i"

    frame $top.frm

    # put a grid on the frame with 9 rows
    if $opt0vals(type,$i)==1 {
	set srctext(1) "Distance from sub-source to scoring plane (cm)"
	set srctext(2) "Minimum x-coordinate for the opening of the applicator (cm)"
	set srctext(3) "Maximum x-coordinate for the opening of the applicator (cm)"
	set srctext(4) "Minimum y-coordinate for the opening of the applicator (cm)"
	set srctext(5) "Maximum y-coordinate for the opening of the applicator (cm)"
	set srctext(6) "Maximum absolute x-coordinate for the applicator (outer dimension) (cm)"
	set srctext(7) "Maximum absolute y-coordinate for the applicator (outer dimension) (cm)"
    } elseif $opt0vals(type,$i)==11 {
	set srctext(1) "Distance from bottom of sub-source to scoring plane (cm)"
	set srctext(2) "Distance from top of sub-source to scoring plane (cm)"
	set srctext(3) "Minimum x-coordinate for the opening of the applicator (cm)"
	set srctext(4) "Maximum x-coordinate for the opening of the applicator (cm)"
	set srctext(5) "Minimum y-coordinate for the opening of the applicator (cm)"
	set srctext(6) "Maximum y-coordinate for the opening of the applicator (cm)"
	set srctext(7) "Maximum absolute x-coordinate for the applicator (outer dimension) (cm)"
	set srctext(8) "Maximum absolute y-coordinate for the applicator (outer dimension) (cm)"
    } elseif $opt0vals(type,$i)==2 {
	set srctext(1) "Distance from sub-source to scoring plane (cm)"
	set srctext(2) "Minimum x-coordinate for the opening of the applicator (cm)"
	set srctext(3) "Maximum x-coordinate for the opening of the applicator (cm)"
	set srctext(4) "Minimum y-coordinate for the opening of the applicator (cm)"
	set srctext(5) "Maximum y-coordinate for the opening of the applicator (cm)"
	set srctext(6) "Maximum absolute x-coordinate for the applicator (outer dimension) (cm)"
	set srctext(7) "Maximum absolute y-coordinate for the applicator (outer dimension) (cm)"
	set srctext(8) "Jaw orientation (0-bars along x-axis, 1-along y-axis)"
    } elseif $opt0vals(type,$i)==3 {
	set srctext(1) "Distance from sub-source to scoring plane (cm)"
	set srctext(2) "Radius of inner opening of the ring or cone (=0 for point source) (cm)"
	set srctext(3) "Outer radius of the ring or cone (=0 for point source) (cm)"
	set srctext(4) "Outer radius of a ring for virtual SSD analysis (only for a point source--define only for first point source specified) (cm)"
    } elseif $opt0vals(type,$i)==4 {
	set srctext(1) "Distance from sub-source to scoring plane (cm)"
	set srctext(2) "Minimum x-coordinate of the planar source (cm)"
	set srctext(3) "Maximum x-coordinate of the planar source (cm)"
	set srctext(4) "Minimum y-coordinate of the planar source (cm)"
	set srctext(5) "Maximum y-coordinate of the planar source (cm)"
    } else {
	set srctext(1) "Distance from sub-source to scoring plane (cm)"
	set srctext(2) "Radius of the planar source"
    }

    for {set ii 1} {$ii<=$nvar($opt0vals(type,$i))} {incr ii} {
	label $top.frm.lab$ii -text $srctext($ii)
	grid config $top.frm.lab$ii -row $ii -column 0 -sticky e
	entry $top.frm.ent$ii -textvariable opt0vals(zmin,$i,$ii)
	grid config $top.frm.ent$ii -row $ii -column 1
    }
    pack $top.frm -pady 5
    button $top.okb -text "OK" -command "destroy $parent.grid.baby$i;\
	    set_button_color $i $parent" -relief groove -bd 8
    pack $top.okb -pady 10
}


proc create_beamdp1_script { } {
    global fieldtype fields fpar env
    global phspfile newsrcfilename
    global isub srcfilename opt0vals plotq xmgrfile nvar
    global min_plotr max_plotr
    global nfielde Re estype Rscoree Rtreate

    # Writes the script to run beamdp for the Multisource option.
    set id [open beamdpguitemp.script w]

    puts $id n
    puts $id 0
    puts $id $isub

    if {$isub==0 && $srcfilename=="undefined"} {
      # don't use an existing source model as a reference
	puts $id n
	if [string compare $newsrcfilename {}]==0 {
	    tk_dialog .error "Error" "The source filename has not been \
		    specified." error 0 OK
	    return 0
	}
	puts $id $newsrcfilename
	puts $id $opt0vals(title)
	if [string compare $opt0vals(title) {}]==0 {
	    tk_dialog .error "Error" "The title has not been input."\
		    error 0 OK
	    return 0
	}
	puts $id $opt0vals(nsrc)
	if [catch {expr $opt0vals(nsrc)}]==1 {
	    tk_dialog .error "Error" "The number of subsources has not been\
		    set." error 0 OK
	    return 0
	}
        set myflag 0
	for {set i 1} {$i<=$opt0vals(nsrc)} {incr i} {
	    if [string compare $opt0vals(charge,$i) {}]==0 {
		tk_dialog .error "Error" "The charge for subsource $i has\
			not been input." error 0 OK
		return 0
	    }
	    if [string compare $opt0vals(type,$i) {}]==0 {
		tk_dialog .error "Error" "The subsource type $i has\
			not been defined." error 0 OK
		return 0
	    }
	    if [catch {expr $opt0vals(latch,$i)}]==1 {
		tk_dialog .error "Error" "The latch bit for subsource $i has\
			not been set." error 0 OK
		return 0
	    }
	    if { $opt0vals(latch,$i)<1|$opt0vals(latch,$i)>23 } {
		tk_dialog .error "Error" "LATCH must be between 1 and 23, \
			inclusive, as in BEAM."  error 0 OK
		return 0
	    }
	    if [string compare $opt0vals(charge,$i) electrons]==0 {
		puts $id "$opt0vals(type,$i), -1, $opt0vals(latch,$i)"
	    } elseif [string compare $opt0vals(charge,$i) photons]==0 {
		puts $id "$opt0vals(type,$i), 0, $opt0vals(latch,$i)"
	    } else {
		puts $id "$opt0vals(type,$i), 1, $opt0vals(latch,$i)"
	    }
	    if [string compare $opt0vals(zmin,$i,1) {}]==0 {
		tk_dialog .error "Error" "The first of the parameters for\
			subsource $i has not been input." error 0 OK
		return 0
	    }
	    set str $opt0vals(zmin,$i,1)
	    for {set j 2} {$j<=$nvar($opt0vals(type,$i))} {incr j} {
		if [string compare $opt0vals(zmin,$i,$j) {}]==0 {
		    tk_dialog .error "Error" "One of the parameters for\
			    subsource $i has not been input." error 0 OK
		    return 0
		}
		set str "$str, $opt0vals(zmin,$i,$j)"
	    }
	    puts $id $str
            if { $opt0vals(type,$i)==3 & $myflag==0 } {
                if {$opt0vals(zmin,$i,2)<1.0 & $opt0vals(zmin,$i,1)>50.0} {
                    puts $id $opt0vals(zmin,$i,4)
                    set myflag 1
                }
            }
	}

	set opt0vals(nbin) [expr int($opt0vals(nbin))]

	foreach j {nbin emin emax} {
	    if [catch {expr $opt0vals($j)}]==1 {
		tk_dialog .error "Error" "One of the parameters \
			has not been input." error 0 OK
		return 0
	    }
	}
	puts $id "$opt0vals(nbin), $opt0vals(emin), $opt0vals(emax)"

        if [catch {expr $estype}]==1 {
            tk_dialog .error "Error" "The energy spectrum type has not been set."\
                    error 0 OK
            return 0
        }
        puts $id "$estype"
        if {$estype==1} {
            if {[catch {expr $nfielde}]==1 || [catch {expr $Rscoree}]==1} {
                 tk_dialog .error "Error" "No. of radii or R scoring field \
                                   has not been set."\
                    error 0 OK
            return 0
            }
            puts $id "$nfielde, $Rscoree"
            set str {}
            for {set i 1} {$i<=$nfielde} {incr i} {
               if {[catch {expr $Re($i)}]==1} {
                    tk_dialog .error "Error" "r($i) is undefined."\
                    error 0 OK
                    return 0
               }
               set str "$str$Re($i), "
            }
            puts $id $str
        } else {
            if {[catch {expr $Rscoree}]==1 || [catch {expr $Rtreate}]==1} {
                    tk_dialog .error "Error" "R treatment field or R scoring\
                    field has not been set."\
                    error 0 OK
                    return 0
            }
            puts $id "$Rtreate, $Rscoree"
        }

	# Put msmfxy, circ ring, square ring, rectangular
	for {set i 0} {$i<=3} {incr i} {
	    if [string compare $fields($fieldtype) $fields($i)]==0 {
		puts $id $i
		set msmfxy $i
		break
	    }
	}
	if [catch {expr $msmfxy}]==1 {
	    tk_dialog .error "Error" "The field type has not been set."\
		    error 0 OK
	    return 0
	}
	if $msmfxy==2 {
	    set pmax 5
	} else {
	    set pmax 4
	}
	set str {}
	for {set i 1} {$i<=$pmax} {incr i} {
	    if [catch {expr $fpar($i)}]==1 {
		tk_dialog .error "Error" "One of the field type parameters \
			has not been set. " error 0 OK
		return 0
	    }
	    set str "$str$fpar($i), "
	}
	puts $id $str
	if $msmfxy==2 {
	    for {set i 6} {$i<=9} {incr i} {
		if [catch {expr $fpar($i)}]==1 {
		    tk_dialog .error "Error" "One of the field type parameters \
			    has not been set. " error 0 OK
		    return 0
		}
	    }
	    puts $id "$fpar(6), $fpar(7), $fpar(8), $fpar(9)"
	}
	if [file exists $phspfile]==0 {
	    tk_dialog .failure "Failure" "The file $phspfile doesn't\
		    exist!" error 0 OK
	    return 0
	}
	puts $id $phspfile
	puts $id 0
    } elseif {$isub==1 || $isub==0} {
        #isub==0 case now uses a reference source model
        if {$isub==0} {
	   puts $id y
        }
	if [file exists $srcfilename]==0 {
	    tk_dialog .failure "Failure" "The file $srcfilename doesn't\
		    exist!" error 0 OK
	    return 0
	}
	puts $id $srcfilename
	if [string compare $newsrcfilename {}]==0 {
	    tk_dialog .error "Error" "The new source filename has not been \
		    specified." error 0 OK
	    return 0
	}
	puts $id $newsrcfilename
	if [string compare $opt0vals(title) {}]==0 {
	    tk_dialog .error "Error" "The title has not been input."\
		    error 0 OK
	    return 0
	}
	puts $id $opt0vals(title)

	set myflag 0
	for {set i 1} {$i<=$opt0vals(nsrc)} {incr i} {
	    puts $id y
	    if [string compare $opt0vals(charge,$i) {}]==0 {
		tk_dialog .error "Error" "The charge for subsource $i has\
			not been input." error 0 OK
		return 0
	    }
	    if [string compare $opt0vals(type,$i) {}]==0 {
		tk_dialog .error "Error" "The subsource type $i has\
			not been defined." error 0 OK
		return 0
	    }
	    if [catch {expr $opt0vals(latch,$i)}]==1 {
		tk_dialog .error "Error" "The latch bit for subsource $i has\
			not been set." error 0 OK
		return 0
	    }
	    if { $opt0vals(latch,$i)<1|$opt0vals(latch,$i)>23 } {
		tk_dialog .error "Error" "LATCH must be between 1 and 23, \
			inclusive, as in BEAM."  error 0 OK
		return 0
	    }
	    if [string compare $opt0vals(charge,$i) electrons]==0 {
		puts $id "$opt0vals(type,$i), -1, $opt0vals(latch,$i)"
	    } elseif [string compare $opt0vals(charge,$i) photons]==0 {
		puts $id "$opt0vals(type,$i), 0, $opt0vals(latch,$i)"
	    } else {
		puts $id "$opt0vals(type,$i), 1, $opt0vals(latch,$i)"
	    }
	    if [string compare $opt0vals(zmin,$i,1) {}]==0 {
		tk_dialog .error "Error" "The first of the parameters for\
			subsource $i has not been input." error 0 OK
		return 0
	    }
	    set str $opt0vals(zmin,$i,1)
	    for {set j 2} {$j<=$nvar($opt0vals(type,$i))} {incr j} {
		if [string compare $opt0vals(zmin,$i,1) {}]==0 {
		    tk_dialog .error "Error" "One of the parameters for\
			    subsource $i has not been input." error 0 OK
		    return 0
		}
                set str "$str, $opt0vals(zmin,$i,$j)"
	    }
	    puts $id y
	    puts $id $str
	    if { $opt0vals(type,$i)==3 & $myflag==0 } {
		if {$opt0vals(zmin,$i,2)<1.0 & $opt0vals(zmin,$i,1)>50.0} {
		    puts $id y
                    puts $id $opt0vals(zmin,$i,4)
		    set myflag 1
		}
	    }
	}
	puts $id y
	foreach j {nbin emin emax} {
	    if [catch {expr $opt0vals($j)}]==1 {
		tk_dialog .error "Error" "One of the parameters \
			has not been input." error 0 OK
		return 0
	    }
	}
	puts $id "$opt0vals(nbin), $opt0vals(emin), $opt0vals(emax)"

        if [catch {expr $estype}]==1 {
            tk_dialog .error "Error" "The energy spectrum type has not been set."\
                    error 0 OK
            return 0
        }
        puts $id y
        puts $id "$estype"
        if {$estype==1} {
            if {[catch {expr $nfielde}]==1 || [catch {expr $Rscoree}]==1} {
                 tk_dialog .error "Error" "No. of radii or R scoring field \
                                   has not been set."\
                    error 0 OK
            return 0
            }
            puts $id y
            puts $id "$nfielde, $Rscoree"
            set str {}
            for {set i 1} {$i<=$nfielde} {incr i} {
               if {[catch {expr $Re($i)}]==1} {
                    tk_dialog .error "Error" "r($i) is undefined."\
                    error 0 OK
                    return 0
               }
               set str "$str$Re($i), "
            }
            puts $id y
            puts $id $str
        } else {
            if {[catch {expr $Rtreate}]==1 || [catch {expr $Rscoree}]==1} {
                    tk_dialog .error "Error" "R treatment field or R scoring\
                    field has not been set."\
                    error 0 OK
                    return 0
            }
            puts $id y
            puts $id "$Rtreate, $Rscoree"
        }

	puts $id y
	# Put msmfxy, circ ring, square ring, rectangular
	for {set i 0} {$i<=2} {incr i} {
	    if [string compare $fields($fieldtype) $fields($i)]==0 {
		puts $id $i
		set msmfxy $i
		break
	    }
	}
	if [catch {expr $msmfxy}]==1 {
	    tk_dialog .error "Error" "The field type has not been set."\
		    error 0 OK
	    return 0
	}
	if $msmfxy==2 {
	    set pmax 5
	} else {
	    set pmax 4
	}
	puts $id y
	set str {}
	for {set i 1} {$i<=$pmax} {incr i} {
	    if [catch {expr $fpar($i)}]==1 {
		tk_dialog .error "Error" "One of the field type parameters \
			has not been set. " error 0 OK
		return 0
	    }
	    set str "$str$fpar($i), "
	}
	puts $id $str
	if $msmfxy==2 {
	    for {set i 6} {$i<=9} {incr i} {
		if [catch {expr $fpar($i)}]==1 {
		    tk_dialog .error "Error" "One of the field type parameters \
			    has not been set. " error 0 OK
		    return 0
		}
	    }
	    puts $id "$fpar(6), $fpar(7), $fpar(8), $fpar(9)"
	}
        puts $id y
	if [file exists $phspfile]==0 {
	    tk_dialog .failure "Failure" "The file $phspfile doesn't\
		    exist!" error 0 OK
	    return 0
	}
	puts $id $phspfile
	puts $id 0
    } elseif {$isub==2} {
	if [file exists $srcfilename]==0 {
	    tk_dialog .failure "Failure" "The file $srcfilename doesn't\
		    exist!" error 0 OK
	    return 0
	}
	puts $id $srcfilename
	if [string compare $newsrcfilename {}]==0 {
	    tk_dialog .failure "Failure" "The new source filename has not
		    been specified." error 0 OK
	    return 0
	}
	puts $id $newsrcfilename
	if [string compare $opt0vals(title) {}]==0 {
	    tk_dialog .error "Error" "The title has not been input."\
		    error 0 OK
	    return 0
	}
	puts $id $opt0vals(title)
	puts $id y
	if [file exists $phspfile]==0 {
	    tk_dialog .failure "Failure" "The file $phspfile doesn't\
		    exist!" error 0 OK
	    return 0
	}
	puts $id $phspfile
	puts $id 0
    } elseif {$isub==3} {
	if [file exists $srcfilename]==0 {
	    tk_dialog .failure "Failure" "The file $srcfilename doesn't\
		    exist!" error 0 OK
	    return 0
	}
	puts $id $srcfilename
	if [string compare $xmgrfile {}]==0 {
	    tk_dialog .failure "Failure" "The xmgr data output file has not
		    been specified." error 0 OK
	    return 0
	}
	puts $id $xmgrfile
	for {set i 1} {$i<=$opt0vals(nsrc)} {incr i} {
	    if [catch {expr $plotq($i)}]==1 {
		tk_dialog .error "Error" "One of the plot types\
			has not been set. " error 0 OK
		return 0
	    }
	    puts $id $plotq($i)
            if {$plotq($i)==3} {
               puts $id "$min_plotr($i), $max_plotr($i)"
            }
	}
	puts $id 0
    }
    # add a blank line at the end, just for kicks
    puts $id ""
    close $id
    return 1
}

proc set_plot_radii { opt i } {
#procedure to allow the user to select Rmin and Rmax of the annulus to plot
#energy in--only for field type 3
    global min_plotr max_plotr

    if $opt==3 {
      catch { destroy .opt0.eplot }
      toplevel .opt0.rplot -bd 10
      set w .opt0.rplot
      wm title $w "Define annulus for energy plot"
      message $w.lab -text "Define the Rmin and Rmax of the annulus\
                                   that you want to plot the energy spectrum\
                                   in.  May span multiple radial bins or\
                                   contain partial bins from the original\
                                   field used to create the source model." \
                                   -width 15c
      pack $w.lab
      frame $w.minr
      label $w.minr.label -text "min. radius (cm)"
      entry $w.minr.inp -textvariable min_plotr($i) -width 10
      pack $w.minr.label -anchor w -side left
      pack $w.minr.inp -anchor e -side right -fill x -expand true
      pack $w.minr -pady 5 -fill x -padx 10
      frame $w.maxr
      label $w.maxr.label -text "max. radius (cm)"
      entry $w.maxr.inp -textvariable max_plotr($i) -width 10
      pack $w.maxr.label -anchor w -side left
      pack $w.maxr.inp -anchor e -side right -fill x -expand true
      pack $w.maxr -pady 5 -fill x -padx 10

      button $w.doneb -text "Done" -command "destroy $w"
      pack $w.doneb
    }
}

proc reset_beamdp1_parameters {} {
#procedure to reset sub-source parameters--really only necessary before
#choosing the option to input your own sub-source specifiers without a
#reference source model, just in case a source model had been loaded
#previously
    global srcfilename opt0vals fpar phspfile fieldtype

    set fieldtype {}

    set opt0vals(title) {}
    set opt0vals(nsrc) {}
    for {set i 1} {$i<=200} {incr i} {
      set opt0vals(type,$i) 0
      set opt0vals(charge,$i) {}
      set opt0vals(latch,$i) {}
      for {set j 1} {$j<=10} {incr j} {
        set opt0vals(zmin,$i,$j) {}
      }
    }
    set opt0vals(nbin) {}
    set opt0vals(emin) {}
    set opt0vals(emax) {}
    for {set i 1} {$i<=10} {incr i} {
      set fpar($i) {}
    }
    set phspfile {}
}

proc rfielde_def { } {
#opens a window to define radii for energy spectra
    global nfielde Re Rscoree Rtreate

    catch { destroy .opt0.rfielde }
    toplevel .opt0.rfielde
    wm title .opt0.rfielde "Define Radii"

    frame .opt0.rfielde.lab

    label .opt0.rfielde.lab.lab1 -text "Input radii from smallest to largest (cm)"
    label .opt0.rfielde.lab.lab2 -text "(last radius must be = R scoring field)"
    pack .opt0.rfielde.lab.lab1 -side top
    pack .opt0.rfielde.lab.lab2 -side top
    pack .opt0.rfielde.lab -side top -padx 5 -pady 5

    frame .opt0.rfielde.grid

    if {$nfielde != "" && $Rscoree != ""} {
        set Re($nfielde) $Rscoree
    }

    for {set i 1} {$i<=$nfielde} {incr i} {
        label .opt0.rfielde.grid.l$i -text "r($i)"
        grid config .opt0.rfielde.grid.l$i -column 0 -row $i
        entry .opt0.rfielde.grid.e$i -textvariable Re($i) -width 10
        grid config .opt0.rfielde.grid.e$i -column 1 -row $i
    }

    pack .opt0.rfielde.grid -side top -padx 60

    button .opt0.rfielde.okb -text "OK" -command "destroy \
            .opt0.rfielde" -relief groove -bd 8
    pack .opt0.rfielde.okb -pady 10
}

