#!/bin/sh
###############################################################################
#
#  EGSnrc update script for component module documentation
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
#  Author:          Blake Walters, 2004
#
#  Contributors:
#
###############################################################################
#
#  Automates the update of input descriptions for all component modules (CMs).
#
###############################################################################


if [ `uname` = "Linux" ]
        then
        ECHO="echo -e"
else
        ECHO=echo
fi

OMEGA_HOME="$HEN_HOUSE/omega"
BEAM_DIR="$OMEGA_HOME/beamnrc"
CMs="$OMEGA_HOME/beamnrc/CMs"
OUTDIR="$HEN_HOUSE/doc/pirs509a-beamnrc/inputformats"

$ECHO "Update APPLICAT input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/APPLICAT_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > APPLICAT.inp
    $ECHO "Done updating APPLICAT input description."
fi


$ECHO "Update ARCCHM input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/ARCCHM_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > ARCCHM.inp
    $ECHO "Done updating ARCCHM input description."
fi

$ECHO "Update BLOCK input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/BLOCK_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > BLOCK.inp
    $ECHO "Done updating BLOCK input description."
fi

$ECHO "Update CIRCAPP input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/CIRCAPP_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > CIRCAPP.inp
    $ECHO "Done updating CIRCAPP input description."
fi

$ECHO "Update CHAMBER input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/CHAMBER_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > CHAMBER.inp
    $ECHO "Done updating CHAMBER input description."
fi

$ECHO "Update CONS3R input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/CONS3R_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > CONS3R.inp
    $ECHO "Done updating CONS3R input description."
fi

$ECHO "Update CONESTAK input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/CONESTAK_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > CONESTAK.inp
    $ECHO "Done updating CONESTAK input description."
fi

$ECHO "Update FLATFILT input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/FLATFILT_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > FLATFILT.inp
    $ECHO "Done updating FLATFILT input description."
fi

$ECHO "Update JAWS input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/JAWS_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > JAWS.inp
    $ECHO "Done updating JAWS input description."
fi

$ECHO "Update DYNJAWS input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/DYNJAWS_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > DYNJAWS.inp
    $ECHO "Done updating DYNJAWS input description."
fi

$ECHO "Update MESH input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/MESH_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > MESH.inp
    $ECHO "Done updating MESH input description."
fi


$ECHO "Update MIRROR input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/MIRROR_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > MIRROR.inp
    $ECHO "Done updating MIRROR input description."
fi

$ECHO "Update MLC input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/MLC_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > MLC.inp
    $ECHO "Done updating MLC input description."
fi

$ECHO "Update MLCQ input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/MLCQ_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > MLCQ.inp
    $ECHO "Done updating MLCQ input description."
fi

$ECHO "Update PYRAMIDS input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/PYRAMIDS_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > PYRAMIDS.inp
    $ECHO "Done updating PYRAMIDS input description."
fi

$ECHO "Update SIDETUBE input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/SIDETUBE_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > SIDETUBE.inp
    $ECHO "Done updating SIDETUBE input description."
fi

$ECHO "Update SLABS input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/SLABS_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > SLABS.inp
    $ECHO "Done updating SLABS input description."
fi

$ECHO "Update VARMLC input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/VARMLC_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > VARMLC.inp
    $ECHO "Done updating VARMLC input description."
fi

$ECHO "Update MLCE input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/MLCE_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > MLCE.inp
    $ECHO "Done updating MLCE input description."
fi

$ECHO "Update DYNVMLC input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/DYNVMLC_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > DYNVMLC.inp
    $ECHO "Done updating DYNVMLC input description."
fi

$ECHO "Update XTUBE input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/XTUBE_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > XTUBE.inp
    $ECHO "Done updating XTUBE input description."
fi

$ECHO "Update SYNCMLCE input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/SYNCMLCE_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > SYNCMLCE.inp
    $ECHO "Done updating SYNCMLCE input description."
fi

$ECHO "Update SYNCHDMLC input description? [n] \c"
read response
if test "$response" = "y"
    then
    fgrep '"I>' $CMs/SYNCHDMLC_cm.mortran | sed -e '1,$s/"I>/ /' | sed -e '/CARDS/,$!d' | sed -f $OUTDIR/add > SYNCHDMLC.inp
    $ECHO "Done updating SYNCHDMLC input description."
fi


###########################################################################
$ECHO "Update main BEAMnrc input description? [n] \c"
read response
if test "$response" = "y"
    then
egrep '"I>|"%A00' $BEAM_DIR/beamnrc.mortran | sed -e '1,/"%A00/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A00/d' | sed -f $OUTDIR/add |sed -e 's/""toc://' > BEAM0.inp
egrep '"I>|"%A00|"%A01' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A00/,/"%A01/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A01/d' | sed -e '/"%A00/d' | sed -f $OUTDIR/add  > BEAM1.inp
egrep '"I>|"%A01|"%A02' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A01/,/"%A02/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A02/d' | sed -e '/"%A01/d' | sed -f $OUTDIR/add > BEAM2.inp
egrep '"I>|"%A02|"%A03' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A02/,/"%A03/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A03/d' | sed -e '/"%A02/d' | sed -f $OUTDIR/add > BEAM3.inp
egrep '"I>|"%A03|"%A04' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A03/,/"%A04/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A04/d' | sed -e '/"%A03/d' | sed -f $OUTDIR/add > BEAM4.inp
egrep '"I>|"%A04|"%A05' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A04/,/"%A05/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A05/d' | sed -e '/"%A04/d' | sed -f $OUTDIR/add > BEAM5.inp
egrep '"I>|"%A05|"%A06' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A05/,/"%A06/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A06/d' | sed -e '/"%A05/d' | sed -f $OUTDIR/add > BEAM6.inp
egrep '"I>|"%A06|"%A07' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A06/,/"%A07/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A07/d' | sed -e '/"%A06/d' | sed -f $OUTDIR/add > BEAM7.inp
egrep '"I>|"%A07|"%A08' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A07/,/"%A08/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A08/d' | sed -e '/"%A07/d' | sed -f $OUTDIR/add > BEAM8.inp
egrep '"I>|"%A08|"%A09' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A08/,/"%A09/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A09/d' | sed -e '/"%A08/d' | sed -f $OUTDIR/add > BEAM9.inp
egrep '"I>|"%A09|"%A10' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A09/,/"%A10/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A09/d' | sed -e '/"%A10/d' | sed -f $OUTDIR/add > BEAM10.inp
egrep '"I>|"%A10|"%A11' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A10/,/"%A11/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A10/d' | sed -e '/"%A11/d' | sed -f $OUTDIR/add > BEAM11.inp
egrep '"I>|"%A11|"%A20' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A11/,/"%A20/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A11/d' | sed -e '/"%A20/d' | sed -f $OUTDIR/add > BEAM12.inp
egrep '"I>|"%A20|"%A21' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A20/,/"%A21/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A20/d' | sed -e '/"%A21/d' | sed -f $OUTDIR/add > BEAM21.inp
egrep '"I>|"%A21|"%A22' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A21/,/"%A22/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A21/d' | sed -e '/"%A22/d' | sed -f $OUTDIR/add > BEAM22.inp
egrep '"I>|"%A22|"%A12' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A22/,/"%A12/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A22/d' | sed -e '/"%A12/d' | sed -f $OUTDIR/add > BEAM23.inp
egrep '"I>|"%A12|"%A32' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A12/,/"%A32/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A12/d' | sed -e '/"%A32/d' | sed -f $OUTDIR/add > BEAM13.inp
egrep '"I>|"%A32|"%A33' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A32/,/"%A33/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A32/d' | sed -e '/"%A33/d' | sed -f $OUTDIR/add > BEAM33.inp
egrep '"I>|"%A33|"%A13' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A33/,/"%A13/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A33/d' | sed -e '/"%A13/d' | sed -f $OUTDIR/add > BEAM34.inp
egrep '"I>|"%A13|"%A14' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A13/,/"%A14/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A13/d' | sed -e '/"%A14/d' | sed -f $OUTDIR/add > BEAM14.inp
egrep '"I>|"%A14|"%A15' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A14/,/"%A15/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A14/d' | sed -e '/"%A15/d' | sed -f $OUTDIR/add > BEAM15.inp
egrep '"I>|"%A15|"%A16' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A15/,/"%A16/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A15/d' | sed -e '/"%A16/d' | sed -f $OUTDIR/add > BEAM16.inp
egrep '"I>|"%A16|"%A17' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A16/,/"%A17/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A16/d' | sed -e '/"%A17/d' | sed -f $OUTDIR/add > BEAM17.inp
egrep '"I>|"%A17|"%A18' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A17/,/"%A18/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A17/d' | sed -e '/"%A18/d' | sed -f $OUTDIR/add > BEAM18.inp
egrep '"I>|"%A18|"%A19' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A18/,/"%A19/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A18/d' | sed -e '/"%A19/d' | sed -f $OUTDIR/add > BEAM19.inp
egrep '"I>|"%A19|"%A23' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A19/,/"%A23/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A19/d' | sed -e '/"%A23/d' | sed -f $OUTDIR/add > BEAM20.inp
egrep '"I>|"%A23|"%A24' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A23/,/"%A24/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A23/d' | sed -e '/"%A24/d' | sed -f $OUTDIR/add > BEAM24.inp
egrep '"I>|"%A24|"%A25' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A24/,/"%A25/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A24/d' | sed -e '/"%A25/d' | sed -f $OUTDIR/add > BEAM25.inp
egrep '"I>|"%A25|"%A26' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A25/,/"%A26/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A25/d' | sed -e '/"%A26/d' | sed -f $OUTDIR/add > BEAM26.inp
egrep '"I>|"%A26|"%A27' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A26/,/"%A27/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A26/d' | sed -e '/"%A27/d' | sed -f $OUTDIR/add > BEAM27.inp
egrep '"I>|"%A27|"%A28' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A27/,/"%A28/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A27/d' | sed -e '/"%A28/d' | sed -f $OUTDIR/add > BEAM28.inp
egrep '"I>|"%A28|"%A29' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A28/,/"%A29/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A28/d' | sed -e '/"%A29/d' | sed -f $OUTDIR/add > BEAM29.inp
egrep '"I>|"%A29|"%A30' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A29/,/"%A30/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A29/d' | sed -e '/"%A30/d' | sed -f $OUTDIR/add > BEAM30.inp
egrep '"I>|"%A30|"%A31' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A30/,/"%A31/!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A30/d' | sed -e '/"%A31/d' | sed -f $OUTDIR/add > BEAM31.inp
egrep '"I>|"%A31' $BEAM_DIR/beamnrc.mortran | sed -e '/"%A31/,$!d' | sed -e '1,$s/"I>/ /' | sed -e '/"%A31/d' | sed -f $OUTDIR/add > BEAM32.inp
    $ECHO "Done updating BEAMnrc input description."
fi

# chmod g+w *.inp
