#!/usr/bin/env fish
###############################################################################
#
#  EGSnrc script to configure a Fortran compiler
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
#  Author:          Frederic Tessier, 2022
#
###############################################################################


### default commit range is from master to current commit
set commit_range "master..HEAD"

### set the git commit range to first argument if provided
if test (count $argv) -gt 0
    set commit_range $argv[1]
end

### set default return code (success)
set return_code 0

### FUNCTION
### get the list of changed files, non-binary, that contain an author tag
function get_files
    for f in (git diff --name-only $commit_range)
        if grep -EIqs "^[\"#]  Author" $f
            echo $f
        end
    end
end

### FUNCTION
### grab all the authors and contributors listed in a file
function get_authors_file
    cat $argv   | sed 's/^C/#/;s/^%/#/;s/^"/#/;s/"$//' \
                | sed -n '/^#.*Author/,/^# *$/p;/^#.*Contributor/,/^# *$/p' \
                | sed '/# *$/d' \
                | sed 's/Ernesto Mainegra/Ernesto Mainegra-Hing/'
end

### FUNCTION
### grab the authors of the changed files, according to commits, filter name variations
function get_authors_commit
    git shortlog -s $commit_range $argv | sed "s/^[[:space:][:digit:]]*//" \
                | sed 's/Ernesto$/Ernesto Mainegra-Hing/' \
                | sed 's/Ernesto Mainegra/Ernesto Mainegra-Hing/' \
                | sed 's/Hing-Hing/Hing/' \
                | sed 's/blakewalters/Blake Walters/' \
                | sed 's/M. Stoeckl/Manuel Stoeckl/' \
                | sed 's/M$/Manuel Stoeckl/' \
                | sed 's/victorMalkov/Victor Malkov/' \
                | sed 's/crcrewso/Cody Crewson/' \
                | sed 's/jantolak/John Antolak/' \
                | sed 's/Marc-Andr. Renaud/Marc-Andre Renaud/' \
                | sed 's/MartinMartinov/Martin Martinov/' \
                | sed 's/blakewalters/Blake Walters/' \
                | sed 's/Townson/Reid Townson/' \
                | sed 's/Reid Reid/Reid/' \
                | sed 's/mpayrits/Matjaz Payrits/' \
                | sed 's/ftessier/Frederic Tessier/' \
                | sed 's/Fr.d.ric Tessier/Frederic Tessier/' \
                | sort | uniq
end

### echo commit range
echo
echo "COMMITS:"
echo "   $commit_range"

### list changed files to check in range
set files_to_check (get_files)
echo
echo "FILES $commit_range:"
for f in $files_to_check
    echo "   $f"
end

### list all commit authors in range
set authors_commit (get_authors_commit)
echo
echo "AUTHORS $commit_range:"
for a in $authors_commit
    echo "   $a"
end

### report missing authors and contributors
for f in $files_to_check

    ### get authors from file and commits, for this file
    set authors_file (get_authors_file $f)
    set authors_commit (get_authors_commit $f)

    ### report missing names
    set missing 0
    for a in $authors_commit
        echo $authors_file | grep -q $a;
        if test $status -eq "1"
            if test $missing -eq 0
                set return_code 1
                set missing 1
                echo
                echo '============================================================================================='
                echo $f
                #set_color fc3
                echo "MISSING:"
            end
            echo "#                   $a"
        end
    end
    #set_color normal
end

### report files without author tag
echo
echo "FILES WITHOUT AUTHOR $commit_range:"
echo
echo "   (skipping .pdf .png .gif .jpg .jpeg .fig .density .ramp .egsphant Doxyfile HEN_HOUSE/pieces/help_message)"
echo
set files_to_check (git diff --name-only $commit_range | grep -Ev '.pdf$|.png$|.gif$|.jpg$|.jpeg$|.fig$|.density$|.ramp$|.egsphant$|Doxyfile|HEN_HOUSE/pieces/help_message')
for f in $files_to_check
    set found (grep -EIL "^[\"#%C]  Author" $f)
    if not test "$found" = ""
        echo "   $found"
    end
end
echo

### return status
exit $return_code
