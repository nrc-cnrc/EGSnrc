#!/bin/sh
###############################################################################
#
#  EGSnrc DOSXYZnrc graphical user interface: main program
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
#
#  Note: either wishx is on your system path of you must modify the following
#  line to put the explicit path to wishx. You can use the following command
#  to find wishx on your system:
#
#  cd /usr; find . -name "*wish*" -print;
#
# The next line restarts using wishx \
exec wish "$0" "$@"

#note: either wishx is on your system path of you must modify the following
#       line to put the explicit path to wishx
#       To find wishx: cd /usr;find . -name "*wish*" -print ;
# the next line restarts using wishx \
exec wish "$0" "$@"


# THIS IS THE MAIN PROGRAM FOR A GUI FOR THE DOSXYZnrc CODE.
# Started in August 1998 by Joanne Treurniet

# Iwan's changes for new EGSnrcMP:
#   - don't rely on OMEGA_HOME, use $HEN_HOUSE/omega instead.
#   - define variables egs_home (from $EGS_HOME) and hen_house (from $HEN_HOUSE)
#   - Look in $pwd, $egs_home/dosxyznrc and $hen_house/user_codes/dosxyznrc
#     for dosxyznrc_user_macros.mortran.
#   - wishx not there on my system at home => use wish instead
#     (available at home and at work)

#*****************************************************************************
# Tcl procedures to load and query EGSnrcMP config files
#
# get_config_options { }
#     Must be called to initialize (load) the EGSnrcMP config
#     Rerturns zero on success, 1 on failure.
#     A failure occures if the environment variable EGS_CONFIG is not set
#     or if the file pointed to by $EGS_CONFIG does not exist or if
#     one of the files that $EGS_CONFIG attempts to include does not
#     exist.
#
# get_config_value { key }
#     Returns the value corresponding to key.
#     If there is no such key in the config files, then checks if the
#     environment variable key is set and returns its value in case it is.
#     Otherwise returns an empty string.
#
# Written by Iwan Kawrakow, Feb 5 2004.
#
#*****************************************************************************

proc get_config_options { } {
    global env
    array set egs_config_options { }
    if [catch {set egs_config $env(EGS_CONFIG)}] {
        puts "The environment variable EGS_CONFIG is not set!"
        puts "Please set it and retry."
        return 1
    }
    set result [load_config_file $egs_config]
    return $result
}

proc load_config_file { the_file } {
    global egs_config_options
    #puts "Loading file $the_file"
    if [ file exists $the_file ]==0 {
        puts "Sorry, $the_file does not exist"
        return 1
    }
    set fid [open $the_file r]
    set the_line ""
    set result 0
    while {[gets $fid line]>-1} {
        set cindex [string first "#" $line]
        if {$cindex>=0} {
            set line [string range $line 0 [expr $cindex-1]]
        }
        set strlen [string length $line]
        if {$strlen > 0} {
            set ind [string last "\\" $line]
            set ind1 [expr $strlen-$ind]
            if $ind1==1 {
                set line [string trimright $line "\\"]
                set line [string trimright $line]
                set len1 [string length $the_line]
                if $len1==0 {
                    set the_line $line
                } else {
                    set line [string trimleft $line]
                    set the_line "$the_line $line"
                }
                continue
            } else {
                set len1 [string length $the_line]
                if $len1==0 {
                    set the_line $line
                } else {
                    set line [string trimleft $line]
                    set the_line "$the_line $line"
                }
            }
            set ind [string first "include" $the_line]
            set len1 [string length $the_line]
            if {$ind==0} {; #it is an include statement, load the file
                set fname [string range $the_line 8 [expr $len1-1]]
                set tmp_res [load_config_file [simplify_value $fname]]
                set the_line ""
                if $tmp_res==1 {
                    set result 1
                }
            } else {
                set eind [string first "=" $the_line]
                set eind1 [string first ":=" $the_line]
                if $eind1>0 {
                    set key [string range $the_line 0 [expr $eind1-2]]
                    if {$key=="DSEP"} {
                       #set val [file separator]
                       set val "\\"
                    } else {
                        set vstart [expr $eind1+3]
                        set vend [expr $len1-1]
                        if {[expr $vend >= $vstart]==1} {
                            set val [string range $the_line $vstart $vend]
                            set val [simplify_value $val]
                            if {$val=="\$(shell echo \\)"} {
                                set val "\\"
                            }
                        } else {
                            set val ""
                        }
                   }
                    array set egs_config_options [list $key $val]
                    set the_line ""
                } elseif {$eind > 0} {
                    set key [string range $the_line 0 [expr $eind-2]]
                    set vstart [expr $eind+2]
                    set vend [expr $len1-1]
                    if {[expr $vend >= $vstart]==1} {
                        set val [string range $the_line $vstart $vend]
                        if {$val=="\$(shell echo \\)"} {
                            set val "\\"
                        }
                    } else {
                        set val ""
                    }
                    array set egs_config_options [list $key $val]
                    set the_line ""
                }
            }
        }
    }
    close $fid
    return $result
}

proc simplify_value { what } {
    #puts "Simplify: $what"
    set result $what
    set aux "\[^)\]"
    while { [regexp "\\$\\(($aux*)\\)" $result match1 match2]==1 } {
        #puts "match1: <$match1> match2: <$match2>"
        set new_val [get_config_value $match2]
        set ind [string first "\$\($match2\)" $result]
        set len [string length $match2]
        set result [string replace $result $ind [expr $ind+$len+2] $new_val]
    }
    return $result
}

proc get_config_value { key } {
    global egs_config_options env
    if [catch {set value $egs_config_options($key)}] {
        #puts "Key $key does not exist"
        if [catch {set value $env($key)}] {
            #set value "__${key}__"
            set value {}
        }
        return $value
    }
    return [simplify_value $value]
}

proc get_photxsections { hen_house } {
#finds out what photon x-section files the user has available and
#adds them to the EGSnrc input menu

global numopts options

   set photxsection_files [glob -nocomplain [file join $hen_house data *_pair.data]]
   if { $photxsection_files != "" } {
     foreach xsect $photxsection_files {
       set xsect [file tail $xsect]
       #set xsect [string trimright $xsect "_pair.data"] WRONG!!!!
       set xsect [string range $xsect 0 [expr [string last "_pair.data" $xsect]-1]]
       set xsect_photo [file join $hen_house data ${xsect}_photo.data]
       set xsect_rayleigh [file join $hen_house data ${xsect}_rayleigh.data]
       set xsect_triplet [file join $hen_house data ${xsect}_triplet.data]
       if {[file exists $xsect_photo] && [file exists $xsect_rayleigh] &&
           [file exists $xsect_triplet]} {
          set numopts(photon_xsections) [expr $numopts(photon_xsections)+1]
          set options(photon_xsections,[expr $numopts(photon_xsections)-1]) $xsect
       }
     }
     # Place PEGS4 option to end of the list
     set options(photon_xsections,0) $xsect
     set options(photon_xsections,[expr $numopts(photon_xsections)-1]) "PEGS4"
   } else {
     # If for some reason no photon data available, one can still use the
     # PEGS4 file. Add this option to the end of list
     set options(photon_xsections,[expr $numopts(photon_xsections)-1]) "PEGS4"
     tk_dialog .photXsec "Missing default photon xsection files" "No file of the form *_pair.data found in \
                         HEN_HOUSE/data. PEGS4 file is the only option \
                         available!" warning 0 OK
   }
}

proc get_eiixsections { hen_house } {
#finds out what EII x-section files the user has available and
#adds them to the EGSnrc input menu

global numopts options

   set eiixsection_files [glob -nocomplain [file join $hen_house data eii_*]]
   if { $eiixsection_files != "" } {
     foreach xsect $eiixsection_files {
       set xsect [file tail $xsect]
       set xsect [string range $xsect 4 [expr [string last ".data" $xsect]-1]]
       set numopts(eii_flag) [expr $numopts(eii_flag)+1]
       set options(eii_flag,[expr $numopts(eii_flag)-1]) $xsect
     }
   }
}

proc get_compxsections { hen_house } {
#finds out what Compton x-section files the user has available and
#adds then to EGSnrc input menu.  Note that default is compton_sigma.data

global numopts options

   set compxsection_files [glob -nocomplain [file join $hen_house data *_compton.data]]
   if { $compxsection_files != "" } {
     foreach xsect $compxsection_files {
       set xsect [file tail $xsect]
       set xsect [string trimright $xsect "_compton.data"]
       set numopts(comp_xsections) [expr $numopts(comp_xsections)+1]
          set options(comp_xsections,[expr $numopts(comp_xsections)-1]) $xsect
     }
   }
}


set home $env(HOME)
#set omega $env(OMEGA_HOME)
set omega $env(HEN_HOUSE)/omega
set egs_home $env(EGS_HOME)
set hen_house $env(HEN_HOUSE)
set XYZ_DIR $omega/progs/gui/dosxyznrc
set dosxyznrc_dir [file join $egs_home dosxyznrc]
if {[file exists $dosxyznrc_dir]} {
   set start_dir $dosxyznrc_dir
   set run_dir $dosxyznrc_dir
} else {
   set start_dir $egs_home
   set run_dir $egs_home
}
set pegs4filename {}
set inp_file {}
set inp_file_dir $start_dir
set debug 0

#procedure below currently only needed to get my_machine for source 9
if [catch {get_config_options}]==1 {
    tk_dialog .error "GUI error" "Failed to load the EGSnrc config \
        files." error 0 OK
    exit
}

# Define application icon
wm iconname . "DOSXYZ GUI"
wm iconbitmap . @$XYZ_DIR/dosxyznrc.xbm

# Read in the .gui_defaults file, either from home or omegahome
if [file exists $home/.gui_defaults]==1 {
    option readfile $home/.gui_defaults
} elseif [file exists $XYZ_DIR/.gui_defaults]==1 {
    option readfile $XYZ_DIR/.gui_defaults
}

# Read in the dosxyznrc_user_macros.mortran file for default values.
    set file1 [file join [pwd] dosxyznrc_user_macros.mortran]
    #set file2 [file join $home egsnrc dosxyznrc dosxyznrc_user_macros.mortran]
    set file2 [file join $egs_home dosxyznrc dosxyznrc_user_macros.mortran]
    set file3 [file join $omega dosxyznrc dosxyznrc_user_macros.mortran]
    set file3 [file join $hen_house user_codes dosxyznrc dosxyznrc_user_macros.mortran]

    if [file exists $file1] {
        set userfile $file1
    } elseif [file exists $file2] {
        set userfile $file2
    } elseif [file exists $file3] {
        set userfile $file3
    } else {
        tk_dialog .error "GUI error" "I can't find the file \
                dosxyznrc_user_macros.mortran in $file1, $file2, or $file3.  \
                Defaults are not defined, which will lead to errors."  \
                info 0 OK
    }
    set list {MXMED MXSTACK IMAX JMAX KMAX MXANG}

    foreach i $list {
        set fid [open $userfile r]
        set maxvals($i) {}
        while { [eof $fid] != 1 } {
            gets $fid data
            if {[string first $i $data]>-1 &&\
                [string first "REPLACE" $data]>-1} {
                set start [string last "\{" $data]
                incr start
                set end [string last "\}" $data]
                incr end -1
                set maxvals($i) [string range $data $start $end]
                set maxvals($i) [expr $maxvals($i)]
                break
            }
        }
        if [catch {expr $maxvals($i)}]==1 {
            tk_dialog .error "Default not found" "I couldn't find the default\
                    value $i.  I picked up \{ $maxvals($i) \} from the\
                    dosxyznrc_user_macros.mortran file." info 0 OK
        }
        close $fid
    }

source $XYZ_DIR/xyznrc_parameters.tcl

#set helvfont "-*-helvetica-medium-r-normal--*-140-*-*-p-*-iso8859-1"
#set normfont "-*-times-medium-r-normal--*-140-*-*-p-*-iso8859-1"
#set italfont "-*-times-medium-i-normal--*-140-*-*-p-*-iso8859-1"

set helvfont "-*-helvetica-medium-r-normal--*-120-*"
set smhelvfont "-*-helvetica-bold-r-normal--*-120-*"
set normfont "-*-times-medium-r-normal--*-120-*"
set italfont "-*-times-medium-i-normal--*-120-*"


#change value below if you want to change overall height of window
#where main/egsnrc input parameters are edited
set ypad 5

wm title . "DOSXYZnrc GUI"
set width [winfo screenwidth .]
set height [winfo screenheight .]
set rootx [expr ($width/2) - 400]
set rooty [expr ($height/2) -300]
wm geometry . +$rootx+$rooty

#define a main menubar

font create myDefaultFont -family Helvetica -size 11 -weight bold
#option add *font myDefaultFont

frame .mbar -relief raised -bd 1

menubutton .mbar.file -text "File" -menu .mbar.file.menu -font myDefaultFont
menu .mbar.file.menu -font myDefaultFont
.mbar.file.menu add command -label "Start a new input file" \
	-command "reset_parameters; edit_parameters"
.mbar.file.menu add command -label "Load a previous input file" \
	-command "load_input"
.mbar.file.menu add command -label "Edit parameters"\
    -command "edit_parameters" -state disabled
.mbar.file.menu add command -label "Save input parameters" \
        -command "overwrite_input" -state disabled
.mbar.file.menu add command -label "Save input parameters as..." \
	-command "save_input"
.mbar.file.menu add separator
.mbar.file.menu add command -label "Change PEGS4 file" \
	-command "get_pegs4file"
.mbar.file.menu add separator
.mbar.file.menu add command -label "Exit" -command "exit_prompt"

menubutton .mbar.about -text "About" -menu .mbar.about.menu -font myDefaultFont
menu .mbar.about.menu -font myDefaultFont
.mbar.about.menu add command -label "About DOSXYZnrc" -command "about_XYZ"
.mbar.about.menu add command -label "About the DOSXYZnrc GUI" \
    -command "about_XYZ_GUI"

menubutton .mbar.run -text "Run" -menu .mbar.run.menu -font myDefaultFont
menu .mbar.run.menu -font myDefaultFont
.mbar.run.menu add command -label "Compile" -command "compile_accel"
.mbar.run.menu add command -label "Run"\
	-command "run"

menubutton .mbar.help -text "Help" -menu .mbar.help.menu -font myDefaultFont
menu .mbar.help.menu -font myDefaultFont
#.mbar.help.menu add command -label "DOSXYZnrc Help"
.mbar.help.menu add command -label "GUI Help" \
    -command {help_dialog .help "GUI Help" $GUI_help_text info 0 Close}

# Graphic stuff for top level window
frame .graphic  -background white

# create the image (NRC banner) from a gif file:
image create photo nrcbanner -file [file join $XYZ_DIR graphics nrc-badge-ribbon-thin-e.gif]
label .graphic.banner -image nrcbanner  -background white
pack .graphic.banner -anchor w -pady 2

frame .middle -background white
frame .middle.left -bd 5  -background white
frame .middle.right -bd 5  -background white

# define the image (logo guy) from a bitmap file:
image create photo bianchi -file [file join $XYZ_DIR graphics bianchi.gif]
label .middle.right.logo -image bianchi  -background white
pack .middle.right.logo

# message box for copyright notice
message .middle.left.name -text "DOSXYZnrc Graphical User Interface 1.1" -width 4i\
    -font "-*-helvetica-bold-r-normal--18-*-*-*-p-*-iso8859-1" -background white

message .middle.left.copy -text {
Ionizing Radiation Standards Group
Institute for National Measurement Standards
National Research Council Canada

Copyright 1999-2011 National Research Council Canada

} -width 4i -font $helvfont  -background white

pack .middle.left.name .middle.left.copy -anchor w

pack .middle.left .middle.right -side left

label .inploaded -text {} -font $smhelvfont
label .pegs4label -text {} -font $smhelvfont

pack .mbar.file .mbar.run -side left -anchor w -padx 5
pack .mbar.about .mbar.help -side right -anchor e -padx 5

frame .sep -bd 4 -width 100 -height 2 -relief groove

pack .mbar .graphic .middle .sep .inploaded .pegs4label -side top\
	 -anchor n -fill both

get_photxsections $hen_house
get_eiixsections $hen_house
get_compxsections $hen_house

source $XYZ_DIR/reset_parameters_nrc.tcl
source $XYZ_DIR/pegs4_egsnrc.tcl
source $XYZ_DIR/pegsless_egsnrc.tcl
source $XYZ_DIR/query_filename.tcl
source $XYZ_DIR/browser.tcl
source $XYZ_DIR/load_input2_nrc.tcl
source $XYZ_DIR/set_main_inputs_egsnrc.tcl
source $XYZ_DIR/save_input_nrc.tcl
source $XYZ_DIR/help.tcl
source $XYZ_DIR/create_file_nrc.tcl
source $XYZ_DIR/misc.tcl
source $XYZ_DIR/define_phantom_egsnrc.tcl
source $XYZ_DIR/set_source.tcl
source $XYZ_DIR/run_dosxyznrc.tcl

reset_parameters
init_pegsless_variables

if {$argc > 1} {
   set pegs4filename [lindex $argv 1]
   set local_pegs_area [file join $egs_home pegs4 data]
   set main_pegs_area [file join $hen_house pegs4 data]
   if {[file extension $pegs4filename]!=".pegs4dat"} {
      set pegs4filename $pegs4filename.pegs4dat
   }
   #go for local area first
   set local_pegs_file [file join $local_pegs_area $pegs4filename]
   set main_pegs_file [file join $main_pegs_area $pegs4filename]
   if {[file exists $local_pegs_file]} {
      set pegs4filename $local_pegs_file
      set pegs_from_home 1
      check_pegs4file
   } elseif {[file exists $main_pegs_file]} {
      set pegs4filename $main_pegs_file
      set pegs_from_home 0
      check_pegs4file
   } else {
     tk_dialog .pegs_file_error "PEGS file does not exist." "The pegs file $pegs4filename\
                specified on the command line does not exist in either the local or main\
                pegs4 data directory.  You will have to browse for the PEGS4 data\
                or restart with another argument." error 0 OK
      set pegs4filename {}
   }
}
if {$argc > 0} {
   set inp_file [lindex $argv 0]
   #add .egsinp extension if not supplied
   set strlen [string length $inp_file]
   set substr [string range $inp_file [expr $strlen-7] [expr $strlen-1]]
   if {$substr!=".egsinp"} {
      set inp_file $inp_file.egsinp
   }
   set full_inp_file [file join $start_dir $inp_file]
   if {![file exists $full_inp_file]} {
      tk_dialog .inp_file_error "Input file does not exist." "The input file $full_inp_file\
                specified on the command line does not exist.  Use the GUI to browse for\
                an existing input file or restart with another argument." error 0 OK
      set inp_file {}
      return
   } else {
      set_inp_file $start_dir $inp_file
   }
}

proc about_XYZ_GUI {} {
    global helvfont
    toplevel .about_gui

    set text {This GUI was created to facilitate the creation of\
	    input files for the DOSXYZnrc code.  \
	    By using it, one can create input files with less\
	    typing and less chance of typing errors.

    The code was written by Joanne Treurniet in 1998 under the direction\
	    of Dave Rogers at the Institute for National Measurement Standards, \
	    Ionizing Radiation Standards, at the National Research Council.  \
           Since then Blake Walters has provided numerous bug fixes\
           and extensions and together with Iwan Kawrakow has addapted\
           the GUI to work with the new EGSnrcMP environment.}

    frame .about_gui.msg -bd 5

    message .about_gui.msg.text -text $text -font $helvfont -width 5i\
	-relief sunken -bd 5
    # vertical scrollbar
    pack .about_gui.msg.text -side left
    pack .about_gui.msg -side top

    frame .about_gui.buts -bd 4
    button .about_gui.buts.okb -text "OK" -command "destroy .about_gui" \
	    -relief groove -bd 8
    pack .about_gui.buts.okb -side left -padx 10
    pack .about_gui.buts -pady 10

}

proc about_XYZ {} {
    global helvfont
    toplevel .about_DOSXYZ

    set text {DOSXYZnrc is a general purpose EGSnrc user code to\
do cartesian coordinate dose\
deposition studies. Every voxel (volume element) can have different\
materials and/or varying densities (for use with CT data).

The geometry is a rectilinear volume with a right-handed coordinate system: \
the X-Y plane on the page, X to the right, Y down and the\
Z-axis into the page. Voxel dimensions are completely variable in\
    all three directions. For more detail on geometry see subroutine HOWFAR\
in the BEAM User's Manual.

Origin of DOSXYZnrc
================

DOSXYZ started out as a demonstration code that Dave Rogers wrote\
to show Ralph Nelson that special purpose coding of rectilinear\
voxels was faster than\
using his more general macros.  It then got used to estimate the\
time required to do a full Monte Carlo treatment planning\
calculation and the results published 3 years later in a book\
chapter (Rogers and Bielajew, 1990).  It then became the basis for\
a Monte Carlo timing benchmark (Bielajew and Rogers, Med. Phys.\
19, (1992) 303).  \
The OMEGA project took this code over and added a variety of\
different source routines with coding contributions from\
B.A. Faddegon, C.M. Ma, G.X. Ding, A.F. Bielajew and P. Reckwerdt.  \
More recent mods have reduced the array space used by the code, \
and added beam characterization inputs (C.M. Ma), btree inputs, \
(Brian Geiser), correlated sampling hooks (Mark Holmes).  \
Blake Walters and Mark Holmes added the CT reading ability and\
Dave Rogers cleaned up code in 1997.\

The code is copyright by the OMEGA collaboration of NRCC and\
University of Wisconsin, 1995, 1996, 1997, 1998.

DOSXYZnrc was created as an EGSnrc version of DOSXYZ by Blake Walters\
in January, 2001. Blake Walters, Iwan Kawrakow and Dave Rogers added\
history-by-history statistical analysis to the code in 2003.\
DOSXYZnrc was addapted to the new EGSnrcMP anvironment by Blake\
Walters and Iwan Kawrakow in 2004.

}
    frame .about_DOSXYZ.msg -bd 5

    message .about_DOSXYZ.msg.text -text $text -font $helvfont -width 6i\
	-relief sunken -bd 5
    # vertical scrollbar
    pack .about_DOSXYZ.msg.text -side left
    pack .about_DOSXYZ.msg -side top

    frame .about_DOSXYZ.buts -bd 4
    button .about_DOSXYZ.buts.okb -text "OK" -command "destroy .about_DOSXYZ" \
	    -relief groove -bd 8
    pack .about_DOSXYZ.buts.okb -side left -padx 10
    pack .about_DOSXYZ.buts -pady 10
}

proc exit_prompt {} {
    set result [tk_dialog .exit "Exit" "Make sure you have saved your\
           input file!  Exit the DOSXYZnrc GUI?" question 0 No Yes]
    if $result==1 { exit }
}

