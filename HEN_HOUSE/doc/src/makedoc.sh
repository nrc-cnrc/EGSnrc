#!/bin/bash
###############################################################################
#
#  EGSnrc script to build documentation
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
#  Author:          Frederic Tessier, 2015
#
#  Contributors:
#
###############################################################################


### check that there is at least one argument
if [ $# -lt 1 ]
then
    echo "Usage: `basename $0` {directory name}"
    exit
fi


### recursively call for each file, if there are many files
if [ $# -gt 1 ]
then
    # loop over all files
    for arg in $@
    do
        # self-call for each file
        echo; echo Building $arg...
        ./`basename $0` $arg
    done
    exit
fi


### get base name of document, make sure there is no slash
doc=${1%/*}


### change directory, if the directory exists
if [ ! -d $doc ]
then
    echo "Can't find directory $doc"
    exit
else
    set -x
    cd $doc
    { set +x; } 2>/dev/null
fi


### generate html with doxygen if there is a Doxyfile
if [ -f Doxyfile ]
then
    dochtml=${doc%-*}-html
    set -x
    doxygen >doxygen.log
    \mv html $dochtml
    \zip -qr $dochtml.zip $dochtml
    \rm -r $dochtml
    \rm doxygen.log
    \mv $dochtml.zip ..
    { set +x; } 2>/dev/null
    exit
fi


### compile the tex file to generate a pdf
if [ ! -f $doc.tex ]
then
    echo "Can't find regular file $doc.tex"
else
    set -x
    pdflatex -interaction=nonstopmode $doc.tex >/dev/null 2>&1
    pdflatex -interaction=nonstopmode $doc.tex >/dev/null 2>&1
    bibtex $doc >/dev/null 2>&1
    makeindex $doc.idx >/dev/null 2>&1
    { set +x; } 2>/dev/null
    if [ -f $doc.lof ]
    then
        set -x
        sed -i "/contin/d" $doc.lof
        pdflatex $doc.tex >/dev/null 2>&1
        makeindex $doc.idx >/dev/null 2>&1
        sed -i "/contin/d" $doc.lof
        { set +x; } 2>/dev/null
    fi
    set -x
    pdflatex $doc.tex >/dev/null 2>&1
    pdflatex $doc.tex | grep Warning
    { set +x; } 2>/dev/null

    ### remove tex auxiliary files
    for extension in aux log lof lot toc idx ind ilg blg bbl biblio out; do
        if [ -f $doc.$extension ]
        then
            set -x
            \rm $doc.$extension
            { set +x; } 2>/dev/null
        fi
    done

    ### move pdf to parent directory
    if [ -f $doc.pdf ]
    then
        set -x
        \mv $doc.pdf ..
        { set +x; } 2>/dev/null
    fi
    exit
fi


