
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: run beamnrc
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
#  Portions of this code were derived from the Quality Assurance tool written
#  by Blake Walters
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


proc My_Reader { pipe } {

    global waitvar errtest
    if [eof $pipe] {
        fconfigure $pipe -blocking 1
        if {[catch {close $pipe}]==1} {
           set errtest 1
        }
        set waitvar 1
        return
    }
    if {[gets $pipe line] >= 0} {
        .bot.frm.dialg insert end "$line \n"
        .bot.frm.dialg see end
    }
}

proc check_and_build {} {
    global inp_file_dir mod_base omega
    if [file exists $inp_file_dir]==1 {
        set the_makefile [file join $inp_file_dir Makefile]
        if [file exists $the_makefile]==0 {
            tk_dialog .error "Warning" "There's no Makefile for the accelerator\
              $mod_base.  Will rebuild this accelerator, but you will lose\
              any changes you have made to the sources.make file." error 0 OK
        } else {
            # no need to build
            return 0
        }
    }
    # The inp_file_dir directory hasn't been created yet
    # With the new BEAMnrc for EGSnrcMP this implies that the
    # beam_build has not been run yet => run it.

    # first check if beam_build.exe exists in $HEN_HOUSE/bin/$my_machine
    set egs_bindir [get_config_value "EGS_BINDIR"]
    set beam_build [file join $egs_bindir beam_build.exe]
    #puts "$egs_bindir $beam_build"
    if [file exists $beam_build]==0 {
        tk_dialog .error "Info" "The tool beam_build.exe does not exist \
               in your main executable area. Will try to create it by \
               compiling beam_build.mortran" info 0 OK
        set currentdir [pwd]
        cd [file join $omega beamnrc tools]
        set make_prog [get_config_value "make_prog"]
        set waitvar 0
        .bot.frm.dialg insert end "\n\n\
          ****************** Compiling beam_build ***************** \n"
        .bot.frm.dialg see end
        set pipe [open "|$make_prog" "r"]
        fconfigure $pipe -blocking 0
        fileevent $pipe readable [list My_Reader $pipe]
        tkwait variable waitvar
        if [file exists $beam_build]==0 {
            # If it still doesn't exist, the make process must have failed.
            tk_dialog .error "Error" "Failed to compile the beam_build.exe\
                 tool. This is needed for creating accelerators from \
                 spec modules. Please try building beam_build.exe manually \
                 and retry." error 0 OK
             return 1
        }
        cd $currentdir
    }

    # now build the accelerator
    set waitvar 0
    .bot.frm.dialg insert end "\n\n\
    ****************** Building $mod_base **************************** \n"
    .bot.frm.dialg see end
    set pipe [open "|$beam_build $mod_base" "r"]
    fconfigure $pipe -blocking 0
    fileevent $pipe readable [list My_Reader $pipe]
    tkwait variable waitvar
    tk_dialog .error "Info" "The accelerator $mod_base has\
		been successfully built."  info 0 OK
    return 0
}

proc compile_accel {} {
    # compile the code for the saved accelerator.

    global mod_base inp_base pegs4filename inp_file_dir waitvar errtest pid
    global mortranc fortranc optlevel xall helvfont omega is_pegsless

    if [string compare $mod_base ""]==0 {
	# There's no accelerator loaded; nothing to compile!
	tk_dialog .error "Error" "There's no accelerator\
		specifed!  Load an accelerator or specify a new one\
		before compiling."  error 0 OK
	return
    }

    # make a scrolling text window
    toplevel .bot
    wm title .bot "Building & Compilation window"

    set optlevel opt

    label .bot.lab1 -text "Code to run: $mod_base" -font $helvfont
    label .bot.lab2 -text "Input file: $inp_base"  -font $helvfont
    if {$is_pegsless==1} {
      label .bot.lab3 -text "Data file: PEGSless" -font $helvfont
    } else {
      label .bot.lab3 -text "Data file: [file rootname [file tail\
	    $pegs4filename]]" -font $helvfont
    }
    pack .bot.lab1 .bot.lab2 .bot.lab3 -pady 1 -anchor w

    frame .bot.top
    label .bot.top.title -text "Make options:"
    frame .bot.top.four -relief groove -bd 4
    set w .bot.top

    radiobutton $w.four.opt1 -text "Optimization on" -variable optlevel -value opt
    radiobutton $w.four.opt2 -text "Optimization off" -variable optlevel -value noopt
    radiobutton $w.four.opt3 -text "Debug on" -variable optlevel -value debug
    radiobutton $w.four.opt4 -text "Clean-up" -variable optlevel -value clean
    pack $w.four.opt1 $w.four.opt2 $w.four.opt3 $w.four.opt4 -anchor w

    pack $w.title $w.four -side left -padx 10
    pack $w -side top -anchor n -pady 10

    frame .bot.frm
    scrollbar .bot.frm.scroll2 -command {.bot.frm.dialg yview} -orient vertical
    text .bot.frm.dialg -yscroll {.bot.frm.scroll2 set};
    pack .bot.frm.scroll2 -side left -fill both
    pack .bot.frm.dialg -side top -fill both
    .bot.frm.dialg insert end "\n Accelerator will be built prior to compilation if accelerator directory does not exist.\n"
    .bot.frm.dialg insert end "\n Select BUILD & COMPILE to begin.\n"
    pack .bot.frm

    frame .bot.bot -relief raised -bd 2
    button .bot.bot.b -text "BUILD & COMPILE" -command {
       if {[check_and_build]==1} {
           return
       }
       set currentdir [pwd]
       set adir [file join $egs_home BEAM_${mod_base}]
       cd $adir
       set waitvar 0
       set errtest 0
       set arg "$optlevel"
       set make_prog [get_config_value "make_prog"]
       .bot.frm.dialg insert end "\n\n\
        ******************* Executing $make_prog $arg ****************** \n\n"
       .bot.frm.dialg see end
       set pipe [open "|$make_prog $arg" "r"]
       fconfigure $pipe -blocking 0
       fileevent $pipe readable [list My_Reader $pipe]
       set compile_pid [pid $pipe]
       grab .bot
       tkwait variable waitvar
       grab release .bot
       if {$errtest != 0} {
           bell
           tk_dialog .compfailed "Failed" "Compilation failed!" error 0 OK
       } else {
           tk_dialog .compok "Finished" "Compilation completed successfully." \
               info 0 OK
       }
       cd $currentdir
    }

    button .bot.bot.a -text "KILL/CLOSE" -command {
        global compile_pid
        catch {exec kill -9 $compile_pid}
        grab release .bot
        destroy .bot
    }
    button .bot.bot.c -text "CLEAR DIALOGUE" -command {.bot.frm.dialg delete 1.0 end}
    pack .bot.bot.b .bot.bot.a .bot.bot.c -side left -fill x -expand no
    pack .bot.bot -side right -padx 10 -pady 5

}

proc run {} {
    global env inp_base inp_file_dir mod_base pegs4filename errtest waitvar pid
    global batch debug speed iparallel nparallel cparallel helvfont inp_file
    global optlevel is_pegsless

    if [string compare $inp_file ""]==0 {
	tk_dialog .error "Error" "There's no input file defined.  Save the\
		one you're working on now before running the simulation, \
		or load a previously made input file."  error 0 OK
	return
    }

    # make a scrolling text window
    toplevel .bot
    wm title .bot "Running window"

    label .bot.lab1 -text "Code to run: $mod_base" -font $helvfont
    label .bot.lab2 -text "Input file: $inp_base"  -font $helvfont
    if {$is_pegsless==1} {
       label .bot.lab3 -text "Data file: PEGSless" -font $helvfont
    } else {
       label .bot.lab3 -text "Data file: [file rootname [file tail\
	    $pegs4filename]]" -font $helvfont
    }
    pack .bot.lab1 .bot.lab2 .bot.lab3 -pady 1 -anchor w

    set batch 0
    set debug 0
    set speed long
    set optlevel opt
    if [catch {expr $iparallel}]==1 { set iparallel 0 }

    frame .bot.top
    label .bot.top.title -text "Run options:"
    frame .bot.top.one -relief groove -bd 4
    frame .bot.top.two -relief groove -bd 4
    frame .bot.top.three -relief groove -bd 4
    frame .bot.top.four -relief groove -bd 4
    frame .bot.top.five -relief groove -bd 4
    set w .bot.top

    radiobutton $w.five.opt1 -text "Optimization on" -variable optlevel -value opt
    radiobutton $w.five.opt2 -text "Optimization off" -variable optlevel -value noopt
    pack $w.five.opt1 $w.five.opt2 -anchor w

    radiobutton $w.three.short -text "short (20 min)" -variable speed -value short
    radiobutton $w.three.medium -text "medium (2 hr)" -variable speed -value medium
    radiobutton $w.three.long -text "long (40 days)" -variable speed -value long
    pack $w.three.short $w.three.medium $w.three.long -anchor w

    radiobutton $w.one.batchon -text "batch" -variable batch -value 1 \
	    -command "toggle_batch 1"
    radiobutton $w.one.batchoff -text "interactive" -variable batch -value 0\
	    -command "toggle_batch 0"
    pack $w.one.batchon $w.one.batchoff -anchor w

    checkbutton $w.four.check -text "run in parallel" -variable iparallel \
	    -onvalue 1 -offvalue 0 -command {
	if $iparallel==0 {
	    .bot.top.four.frm.text configure -state disabled
	    .bot.top.four.frm.label configure -fg grey
	    .bot.top.one.batchoff configure -state normal
	    .bot.top.one.batchoff configure -fg black
	} else {
	    .bot.top.four.frm.text configure -state normal
	    .bot.top.four.frm.label configure -fg black
	    .bot.top.one.batchoff configure -state disabled
	    .bot.top.one.batchoff configure -fg grey
	    # also shut off red colour for interactive
	    .bot.top.one.batchon select
	    .bot.top.one.batchoff deselect
	}
    }
    frame $w.four.frm
    label $w.four.frm.label -text "Number of jobs"
    entry $w.four.frm.text -textvariable nparallel -width 4
    pack $w.four.frm.label $w.four.frm.text -side left
    pack $w.four.check $w.four.frm -anchor w

    toggle_batch $batch
    if $iparallel==0 {
	$w.four.frm.text configure -state disabled
	$w.four.frm.label configure -fg grey
    } else {
	$w.four.frm.text configure -state normal
	$w.four.frm.label configure -fg black
    }

    pack $w.title -side left -padx 5
    pack $w.five -side left -padx 5
    pack $w.three -side left -padx 5
    pack $w.two -side left
    pack $w.one -side left -padx 5
    pack $w.four -side left -padx 5
    pack $w -side top -anchor n -pady 10

    frame .bot.frm
    scrollbar .bot.frm.scroll2 -command {.bot.frm.dialg yview} -orient vertical
    text .bot.frm.dialg -yscroll {.bot.frm.scroll2 set};
    pack .bot.frm.scroll2 -side left -fill both
    pack .bot.frm.dialg -side top -fill both
    .bot.frm.dialg insert end "Make sure you've saved any changes to the\
	    input file before starting! \n"
    .bot.frm.dialg insert end "Select EXECUTE to begin."
    pack .bot.frm

    frame .bot.bot -relief raised -bd 2
    button .bot.bot.b -text EXECUTE -command {
       global inp_base pegs4filename inp_file_dir mod_base
       global batch debug speed iparallel nparallel hen_house
       global optlevel
       set waitvar 0
       set errtest 0
       set cparallel 0
       if {"$optlevel"=="noopt"} {
         set opt "_noopt"
       } else {
         set opt ""
       }
       set pegs4base [file rootname [file tail $pegs4filename]]
       set current_dir [pwd]
       cd $inp_file_dir
       .bot.frm.dialg insert end "\n\n\n"
       if $batch==0 {
           if {$is_pegsless==1} {
             set rpipe [open "|BEAM_${mod_base}${opt} -i $inp_base" "r"]
           } else {
             set rpipe [open "|BEAM_${mod_base}${opt} -p $pegs4base -i $inp_base" "r"]
           }
       } else {
           set arg "$speed"
           if [catch {set egs_batch_system $env(EGS_BATCH_SYSTEM)}] {
                set egs_batch_system "pbs"
           }
           set arg "$arg batch=$egs_batch_system"
           if  {$iparallel==1 && $nparallel>1} {
               set arg "$arg p=$nparallel"
           }
           puts $arg
           set batch_script [file join $hen_house scripts run_user_code_batch]
           if {$is_pegsless==1} {
             set rpipe [open "|$batch_script BEAM_${mod_base}${opt} $inp_base pegsless $arg 2>&1" "r"]
           } else {
             set rpipe [open "|$batch_script BEAM_${mod_base}${opt} $inp_base $pegs4base $arg 2>&1" "r"]
           }
       }
       fconfigure $rpipe -blocking 0
       fileevent $rpipe readable [list My_Reader $rpipe]
       set run_pid [pid $rpipe]
       grab .bot
       tkwait variable waitvar
       grab release .bot
       if {$errtest != 0} {
           bell
           tk_dialog .compfailed "Failed" "The job has failed." error 0 OK
       } else {
           if $batch==1 {
           tk_dialog .subok "Finished" "The job has been successfully \
               submitted to the queue." info 0 OK
           } else {
               tk_dialog .subok "Finished" "The job is finished." info 0 OK
           }
       }
       cd $current_dir
    }

    button .bot.bot.a -text "KILL/CLOSE" -command {
        global run_pid rpipe
        catch {
            exec kill -9 $run_pid
            close $rpipe
        }
        grab release .bot; destroy .bot
    }
    button .bot.bot.c -text "CLEAR DIALOGUE" -command {
        .bot.frm.dialg delete 1.0 end
    }
    pack .bot.bot.b .bot.bot.a .bot.bot.c -side left -fill x -expand no
    pack .bot.bot -side right -padx 10 -pady 5

}

proc toggle_batch {var} {
    set w .bot.top
    if $var==1 {
	$w.three.short configure -fg black
	$w.three.medium configure -fg black
	$w.three.long configure -fg black
    } else {
	$w.three.short configure -fg grey
	$w.three.medium configure -fg grey
	$w.three.long configure -fg grey
    }
}


