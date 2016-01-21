#!/bin/sh
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: main program
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
# The next line restarts using wish \
exec wish "$0" ${1+"$@"}


# THIS IS THE MAIN PROGRAM FOR A GUI FOR THE BEAMnrc CODE.
# Started in June 1998 by Joanne Treurniet

proc get_beam_user_defaults {} {
    global maxvals env mod_base egs_home omega

    set file1 [file join [pwd] beamnrc_user_macros.mortran]
    set file2 [file join $egs_home beamnrc beamnrc_user_macros.mortran]
    set file3 [file join $omega beamnrc beamnrc_user_macros.mortran]

    if [file exists $file1] {
	set userfile $file1
    } elseif [file exists $file2] {
	set userfile $file2
    } elseif [file exists $file3] {
	set userfile $file3
    } else {
	tk_dialog .error "GUI error" "I can't find the file \
		beamnrc_user_macros.mortran in $file1, $file2, or $file3.  \
		Defaults are not defined, which will lead to errors."  \
		info 0 OK
    }
    set list {MAX_CMs MXMED MXREG MXSTACK MAXBRSPLIT MAX_DOSE_ZONE MAXIT\
	    MAX_SC_PLANES MAX_SC_ZONES MAX_SC_PARAMETERS MAXPTS_SRC9\
	    MXRDIST MIN_GAP LATCH_NUMBER_OF_BITS}

    foreach i $list {
	set fid [open $userfile r]
	set maxvals($i) {}
	while { [eof $fid] != 1 } {
	    gets $fid data
	    if [string first $i $data]>-1 {
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
		    beamnrc_user_macros.mortran file." info 0 OK
	}
	close $fid
    }
}


#*****************************************************************************
#
# $Id: beamnrc_gui.tcl,v 1.25 2013/09/23 14:45:09 bwalters Exp $
#
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
       #set xsect [string trimright $xsect "_pair.data"]
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
     tk_dialog .photXsec "Missing default photon xsection files"
                         "No file of the form *_pair.data found in \
                         HEN_HOUSE/data. PEGS4 file is the only option\
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


if [catch {set egs_config $env(EGS_CONFIG)}]==1 {
    tk_dialog .error "GUI error" "The environment variable EGS_CONFIG is \
        not set but it must be for the proper operation of EGSnrc and \
        BEAMnrc." error 0 OK
    exit
}
if [catch {set egs_home $env(EGS_HOME)}]==1 {
    tk_dialog .error "GUI error" "The environment variable EGS_HOME is \
        not set but it must be for the proper operation of EGSnrc and \
        BEAMnrc." error 0 OK
    exit
}
if [catch {get_config_options}]==1 {
    tk_dialog .error "GUI error" "Failed to load the EGSnrc config \
        files." error 0 OK
    exit
}
if [catch {set hen_house [get_config_value "HEN_HOUSE"]}]==1 {
    tk_dialog .error "GUI error" "The environment variable HEN_HOUSE is \
        not set and also not defined in the config file. Can not continue \
        without HEN_HOUSE being defined" error 0 OK
    exit
}

set home $env(HOME)

set omega [file join $hen_house omega]
set GUI_DIR [file join $omega progs gui beamnrc]

wm iconname . "BEAM GUI"
#wm iconbitmap . @$GUI_DIR/beamnrc.xbm

set start_dir [pwd]
set run_dir [pwd]
if {![string match *BEAM_* $start_dir]} {
  set start_dir $egs_home
  set run_dir $egs_home
}
set pegs4filename {}
set mod_file {}
set mod_base {}
set inp_file {}
set inp_base {}
set inp_file_dir $start_dir
set num_cm 0
set nmed 0
set med_per_column 40

get_beam_user_defaults

source $GUI_DIR/beamnrc_params.tcl

get_photxsections $hen_house
get_eiixsections $hen_house
get_compxsections $hen_house

# you can set the colours using this command:
# tk_setPalette activeBackground white background PeachPuff
# Read in the .gui_defaults file, either from home or omegahome

if [file exists $home/.gui_defaults]==1 {
    option readfile $home/.gui_defaults
} elseif [file exists $GUI_DIR/.gui_defaults]==1 {
    option readfile $GUI_DIR/.gui_defaults
}


# set the colors for graphic - VACUUM is always grey -- there are 44 here,
# and the 43rd is black, the default colour for undefined media.
set default_colours {
grey          #ffff00000000 #a410b861ffff #ffffafc80000 #ffffea600000
#ffffffff0000 #a69985c6a699 #159fffff0000 #0000ffffafc8 #0000ffffffff
#0000c567ffff #00008acfffff #00000000ffff #afc80000ffff #ea600000ffff
#ffff0000c567 #ffffa410a410 #ffffdcf3a410 #ffffffffa410 #a410ffffa410
#a410ffffdcf3 #a410ffffffff #a410f554ffff #ffff75300000 #a410a410ffff
#dcf3a410ffff #ffffa410ffff #ffffa410b450 #aa0f00000000 #aa0f58ca0000
#aa0f95bd0000 #aa0faa0f0000 #0000aa0f0000 #0000aa0faa0f #0000712baa0f
#00000000aa0f #99ce0000aa0f #aa0f0000aa0f #fffff473dd5a  white
#e6e6e6e6fafa #4040e0e0d0d0 #aa0f00003848 #556b2f       #8470ff
#fa8072       #cd6839        black
}
#set default_colours {
#grey red SkyBlue LawnGreen purple pink turquoise yellow orange cyan bisque\
#lavender gold salmon coral chocolate maroon magenta plum HotPink thistle\
#sienna aquamarine burlywood1 tan2 orange3 pink1 white blue green OrangeRed\
#DarkRed LightGreen MidnightBlue DeepSkyBlue khaki peru BlueViolet plum brown\
#wheat DarkOliveGreen LightSlateBlue salmon sienna3}
# start off in colour
set colour 1
set colorlist $default_colours

set helvfont "-*-helvetica-medium-r-normal--*-120-*"
set smhelvfont "-*-helvetica-bold-r-normal--*-120-*"
set normfont "-*-times-medium-r-normal--*-120-*"
set italfont "-*-times-medium-i-normal--*-120-*"

wm title . "BEAMnrc GUI"
set width  [winfo screenwidth  .]
set height [winfo screenheight .]
set rootx  [expr ($width/2)  - 400]
set rooty  [expr ($height/2) - 300]
wm geometry . +$rootx+$rooty

font create myDefaultFont -family Helvetica -size 11 -weight bold
#option add *font myDefaultFont

#define a main menubar

frame .mbar -relief raised -bd 1 
menubutton .mbar.file -text "File" -menu .mbar.file.menu -font myDefaultFont 
menu .mbar.file.menu -font myDefaultFont
.mbar.file.menu add command -label "Load a previous accelerator" \
	-command "load_old_module"
.mbar.file.menu add command -label "Specify a new accelerator" \
	-command "specify_new_module"
.mbar.file.menu add separator
.mbar.file.menu add command -label "Change PEGS4 file" \
	-command "get_pegs4file"
.mbar.file.menu add separator
.mbar.file.menu add command -label "Load a previous input file" \
	-command "load_input"
.mbar.file.menu add command -label "Save input parameters" \
        -command "overwrite_input"
.mbar.file.menu add command -label "Save input parameters as..." \
	-command "save_input"
.mbar.file.menu add separator
.mbar.file.menu add command -label "Exit" -command "exit_prompt"

menubutton .mbar.preview -text "Preview" -menu .mbar.preview.menu -font myDefaultFont
menu .mbar.preview.menu -font myDefaultFont
.mbar.preview.menu add command -label "Preview accelerator" -command "draw_accel"
.mbar.preview.menu add command -label "Change colour scheme"\
        -command "change_color_scheme"

menubutton .mbar.run -text "Execute" -menu .mbar.run.menu -font myDefaultFont
menu .mbar.run.menu -font myDefaultFont
.mbar.run.menu add command -label "Compile" -command "compile_accel"
.mbar.run.menu add command -label "Run"\
	-command "run"

menubutton .mbar.about -text "About" -menu .mbar.about.menu -font myDefaultFont
menu .mbar.about.menu -font myDefaultFont
.mbar.about.menu add command -label "About BEAMnrc" -command "about_BEAM"
.mbar.about.menu add command -label "About the BEAMnrc GUI" \
    -command "about_BEAM_GUI"

menubutton .mbar.help -text "Help" -menu .mbar.help.menu -font myDefaultFont
menu .mbar.help.menu -font myDefaultFont
#.mbar.help.menu add command -label "BEAMnrc Help"
.mbar.help.menu add command -label "GUI Help" \
    -command {help_dialog .help "GUI Help" $GUI_help_text info 0 Close}

# Graphic stuff for top level window
frame .graphic  -background white

# create the image (NRC banner) from a gif file:
image create photo nrcbanner -file [file join $GUI_DIR graphics nrc-badge-ribbon-thin-e.gif]
label .graphic.banner -image nrcbanner  -background white
pack  .graphic.banner -anchor w -pady 2

frame .graphic.left -bd 5   -background white
frame .graphic.right -bd 5   -background white

# define the image (logo guy) from a bitmap file:
image create photo bianchi -file [file join $GUI_DIR graphics bianchi.gif]
label .graphic.right.logo -image bianchi  -background white
pack .graphic.right.logo

# message box for copyright notice
message .graphic.left.title -text "BEAMnrc Graphical User Interface 2.0"\
   -width 4i -font "-adobe-helvetica-bold-r-normal--18-*-*-*-p-*-iso8859-1"  -background white

message .graphic.left.copy -text {
Ionizing Radiation Standards Group
Institute for National Measurement Standards
National Research Council Canada

Copyright 1995-2011 National Research Council Canada

} -width 4i -font $helvfont  -background white
pack .graphic.left.title .graphic.left.copy -anchor w -pady 0

pack .graphic.left .graphic.right -side left

label .modloaded -text {} -font $smhelvfont
label .inploaded -text {} -font $smhelvfont
label .pegs4label -text {} -font $smhelvfont

pack .mbar.file .mbar.preview .mbar.run -side left -anchor w -padx 5
pack .mbar.about .mbar.help -side right -anchor e -padx 5

frame .sep -bd 4 -width 100 -height 2 -relief groove
pack .mbar .graphic .sep .modloaded .inploaded .pegs4label\
    -side top -anchor n -fill both

source $GUI_DIR/slabs.tcl
source $GUI_DIR/cons3r.tcl
source $GUI_DIR/conestak.tcl
source $GUI_DIR/flatfilt.tcl
source $GUI_DIR/jaws.tcl
source $GUI_DIR/circapp.tcl
source $GUI_DIR/applicat.tcl
source $GUI_DIR/pyramids.tcl
source $GUI_DIR/chamber.tcl
source $GUI_DIR/mirror.tcl
source $GUI_DIR/mesh.tcl
source $GUI_DIR/xtube.tcl
source $GUI_DIR/sidetube.tcl
source $GUI_DIR/mlc.tcl
source $GUI_DIR/mlcq.tcl
source $GUI_DIR/block.tcl
source $GUI_DIR/arcchm.tcl
source $GUI_DIR/varmlc.tcl
source $GUI_DIR/mlce.tcl
source $GUI_DIR/dynvmlc.tcl
source $GUI_DIR/dynjaws.tcl
source $GUI_DIR/syncjaws.tcl
source $GUI_DIR/syncmlce.tcl
source $GUI_DIR/syncvmlc.tcl
source $GUI_DIR/synchdmlc.tcl

source $GUI_DIR/run_beamnrc.tcl
source $GUI_DIR/change_color_scheme.tcl
source $GUI_DIR/draw_accel_egsnrc.tcl
source $GUI_DIR/create_accel.tcl
source $GUI_DIR/query_filename.tcl
source $GUI_DIR/browser.tcl
source $GUI_DIR/save_module_egsnrc.tcl
source $GUI_DIR/load_input2_egsnrc.tcl
source $GUI_DIR/load_module_egsnrc.tcl
source $GUI_DIR/set_main_inputs_egsnrc.tcl
source $GUI_DIR/save_input_egsnrc.tcl
source $GUI_DIR/help.tcl
source $GUI_DIR/new_create_file_egsnrc.tcl
source $GUI_DIR/misc_egsnrc.tcl
source $GUI_DIR/pegs4_egsnrc.tcl
source $GUI_DIR/pegsless_egsnrc.tcl

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
#now see if the user has supplied an input file name as an argument
#if so, then set the global inp_file to this argument
if {$argc > 0} {
   set inp_file [lindex $argv 0]
   #add .egsinp extension if not supplied
   set strlen [string length $inp_file]
   set substr [string range $inp_file [expr $strlen-7] [expr $strlen-1]]
   if {$substr!=".egsinp"} {
      set inp_file $inp_file.egsinp
   }
   #now see if the user has provided a directory tree
   #if so, assume that that this is the start directory
#   if { "$tcl_platform(platform)"=="windows" } {
#      set filesep \\
#   } else {
#      set filesep /
#   }
   #save the input file directory
#   set inp_file_dir_save $inp_file_dir
#   if {[string match *$filesep* $inp_file]} {
#      set inp_file_dir [string range $inp_file 0 [expr [string last $filesep $inp_file]-1]]
#   } else {
#      set inp_file [file join $inp_file_dir $inp_file
#   }
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


proc reset_inputs {} {
    # RESET THE INPUTS SO THAT THEY DON'T SHOW UP IN THE NEXT LOADED
    # FILE... (a work in progress...)

    for {set i 1} {$i<=5} {incr i} {
	set srcopts($i) 0
    }
}

proc about_BEAM_GUI {} {
    global helvfont
    toplevel .about_gui

    set text {
BEAMnrc GUI $Revision: 1.25 $ last edited $Date: 2013/09/23 14:45:09 $.

This GUI was created to facilitate the creation of\
	    input files for the BEAMnrc code.  \
	    By using it, one can create input files with less\
	    typing and less chance of typing errors.

The code was written by J. Treurniet in 1998 under the direction\
	    of Dave Rogers at the Institute for National Measurement Standards, \
	    Ionizing Radiation Standards, at the National Research Council.  \
           Blake Walters has provided numerous bug fixes and extensions \
           to acommodate new component modules and other options such as DBS.\
           Iwan Kawrakow and Blake Walters have adjusted the GUI to work with \
           the new EGSnrcMP environment.
}

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

proc about_BEAM {} {
    global helvfont
    toplevel .about_BEAM

    set text {
Copyright 1995-2011 by the National Research Council\
of Canada, all rights reserved.  This code is part\
of the BEAMnrc code system for Monte carlo simulation\
of radiotherapy treatment units. It was developed\
at the National Research Council of Canada as part\
of the OMEGA collaborative project with the University\
of Wisconsin.  The system is described in:

    BEAM: A Monte Carlo code to simulate radiotherapy treatment
    units, D.W.O. Rogers, B.A. Faddegon, G.X. Ding, C.-M. Ma,
    J. Wei and T.R.Mackie, Medical Physics 22 (1995) 503 -- 524.

    and in the

    BEAMnrc Users Manual
    D.W.O. Rogers, B. Walters, I. Kawrakow
    NRC Report PIRS-509A (rev K)

As well as the authors of the above papers, \
G. Zhang, D. Sheikh-Bagheri and J. Sun, all at NRC, made\
significant contributions to the code.

The BEAMnrc code system (excluding the EGSnrc portions of it)\
is copyright and subject to the BEAMnrc General License \
which is included in the distribution.  The BEAMnrc code\
system is provided as is without any warranty or\
guarantee of any kind, either expressed or implied, \
as to its accuracy or ability to perform particular\
calculations.  \

Copyright NRC 1995-2011}

    frame .about_BEAM.msg -bd 5

    message .about_BEAM.msg.text -text $text -font $helvfont -width 6i\
	-relief sunken -bd 5
    # vertical scrollbar
    pack .about_BEAM.msg.text -side left
    pack .about_BEAM.msg -side top

    frame .about_BEAM.buts -bd 4
    button .about_BEAM.buts.okb -text "OK" -command "destroy .about_BEAM" \
	    -relief groove -bd 8
    pack .about_BEAM.buts.okb -side left -padx 10
    pack .about_BEAM.buts -pady 10
}

proc exit_prompt {} {
    set result [tk_dialog .exit "Exit" "Make sure you've saved your input file!\
	        Exit the BEAMnrc GUI?" question 0 No Yes]
    if $result==1 { exit }
}

