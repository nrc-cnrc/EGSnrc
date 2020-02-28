################################################################################
#
#  EGSnrc mevegs post-processing Gmsh API helper functions.
#
#  Copyright (C) 2020 Mevex Corporation
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
#  Authors:          Max Orok
#
###############################################################################
#
#  Calculate various quantities using EGSnrc output data and the Gmsh API.
#
#  Requires:
#  * Python 3
#  * The Gmsh Python SDK >= v4.4
#
###############################################################################

import sys
import gmsh

# print to stderr
def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

# print error and exit
def abort(*args, **kwargs):
    eprint("\nin " + __file__ + ":")
    eprint(*args, **kwargs)
    eprint("aborting\n")
    sys.exit(1)

def get_scalar_data(viewTag):
    _, tags, data,  _, _ = gmsh.view.getModelData(viewTag, step=0)
    # returns a list of lists, extract here
    data = [x for [x] in data]
    return (tags, data)

# sum a quantity over all the volume elements for a given view
def get_volume_totals(viewTag):
    # get all the data for a view
    data_map = dict(zip(*get_scalar_data(viewTag)))
    volumes = gmsh.model.getEntities(3)
    len(volumes) > 0 or abort("no volumes in data file")
    # build list of all elements
    elts = []
    volume_sum = []

    for dim, volume in volumes:
        _, vol_elts, _ = gmsh.model.mesh.getElements(dim, volume)
        len(vol_elts) == 1 or abort("mixed element types data file")
        [vol_elts] = vol_elts # take first entry of vector of vectors
        elts.extend(vol_elts)
        vol_total = sum([data_map.get(elt, 0.0) for elt in vol_elts])
        volume_sum.extend([vol_total for _ in vol_elts])

    return (elts, volume_sum)


def add_scalar_data(viewTag, tags, data):
    models = gmsh.model.list()
    len(models) == 1 or abort("more than one model loaded")
    gmsh.view.addModelData(viewTag, 0, models[0], "ElementData", tags, [[dat] for dat in data])

def append_view(viewTag, filename):
    # don't save duplicate mesh data
    gmsh.option.setNumber("PostProcessing.SaveMesh", 0)
    gmsh.view.write(viewTag, filename, append=True)

def append_to_file(title, filename, elts, data):
    view_tag = gmsh.view.add(title)
    add_scalar_data(view_tag, elts, data)
    append_view(view_tag, filename)
