
###############################################################################
#
#  EGSnrc BEAMnrc graphical user interface: get configuration options
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
#  Author:          Iwan Kawrakow, 2004
#
#  Contributors:
#
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
#  Tcl procedures to load and query EGSnrcMP config files
#
#  get_config_options { }
#      Must be called to initialize (load) the EGSnrcMP config Returns zero on
#      success, 1 on failure. A failure occurs if the environment variable
#      EGS_CONFIG is not set, or if the file pointed to by $EGS_CONFIG does
#      not exist. or if one of the files that $EGS_CONFIG attempts to include
#      does not exist.
#
#  get_config_value { key }
#      Returns the value corresponding to key. If there is no such key in the
#      config files, then checks if the environment variable key is set and
#      returns its value in case it is. Otherwise returns an empty string.
#
# list_config_options { }
#     Print all key-value pairs to stdout.
#
# test_config_options { }
#     Is there for testing. Reads in a key from stdin and prints the
#     corresponding value until 'stop' is input.
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

proc test_config_options {} {
    while 1 {
        gets stdin key
        if {$key=="stop"} {
            break
        }
        puts [get_config_value $key]
    }
}

proc list_config_options {} {
    global egs_config_options
    foreach key [array names egs_config_options] {
        puts "$key <$egs_config_options($key)>"
    }
}


