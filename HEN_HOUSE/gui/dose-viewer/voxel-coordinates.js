/*
###############################################################################
#
#  EGSnrc online voxel and dose visualization tool
#  Copyright (C) 2020 Magdalena Bazalova-Carter and Elise Badun
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
#  Author:          Elise Badun, 2020
#
#  Contributors:
#
###############################################################################
*/

// TODO: Make voxel information a class
/**
 * Create the HTML elements to display the voxel information.
 *
 * @param {Object} parentDiv The HTML parent div.
 * @param {string} id The unique ID of the volume viewers voxel info.
 */
function buildVoxelInfoHtml (parentDiv, id) {
  // Define label texts and tags
  const labelName = [
    'World Coordinates (cm):',
    'Voxel Coordinates:',
    'Density:',
    'Material:',
    'Dose:'
  ]

  const tagList = [
    'world-coords',
    'voxel-coords',
    'density-value',
    'material-value',
    'dose-value'
  ]

  // Add container
  const voxelInfoHolder = parentDiv
    .append('div')
    .classed('voxel-info', true)
    .attr('id', 'voxel-info-' + id)
    .classed('hidden', true)

  // Iterate through tag list and add label and output for each
  tagList.forEach((tag, i) => {
    voxelInfoHolder
      .append('label')
      .attr('for', tag + '-' + id)
      .text('  ' + labelName[i] + ' ')

    voxelInfoHolder
      .append('output')
      .attr('type', 'text')
      .attr('class', 'voxel-info-output')
      .attr('id', tag + '-' + id)
  })
}

/**
 * Transform panel coordinates to world coordinates
 *
 * @param {number[]} coords The coordinates of the mouse position on the panel.
 * @param {string} axis The axis of the slice (xy, yz, or xz).
 * @param {number} sliceNum The number of the current slice displayed in the panel.
 * @param {Volume} volume The volume of the panel.
 * @param {Object} transform The zoom transform of the panel.
 * @returns {number[]}
 */
// TODO: Just pass in panel and coords?
function coordsToWorld (coords, axis, sliceNum, volume, transform) {
  // Invert transformation if applicable then invert scale to get world coordinate
  const i = volume.prevSlice[axis].xScale.invert(
    transform ? invertTransform(coords[0], transform, 'x') : coords[0]
  )
  const j = volume.prevSlice[axis].yScale.invert(
    transform ? invertTransform(coords[1], transform, 'y') : coords[1]
  )

  // Add 0.5 to sliceNum in order to map values to center of voxel bondaries
  // TODO: Perhaps fix scale to get rid of the 0.5 hack
  const k = volume.prevSlice[axis].zScale.invert(parseInt(sliceNum) + 0.5)

  const [xVal, yVal, zVal] =
    axis === 'xy' ? [i, j, k] : axis === 'yz' ? [k, i, j] : [i, k, j]
  return [xVal, yVal, zVal]
}

/**
 * Transform panel coordinates to voxel coordinates
 *
 * @param {number[]} coords The coordinates of the mouse position on the panel.
 * @param {string} axis The axis of the slice (xy, yz, or xz).
 * @param {number} sliceNum The number of the current slice displayed in the panel.
 * @param {Volume} volume The volume of the panel.
 * @param {Object} transform The zoom transform of the panel.
 * @returns {number[]}
 */
function coordsToVoxel (coords, axis, sliceNum, volume, transform) {
  // Invert transformation if applicable then apply scale to get voxel coordinate
  const i = volume.prevSlice[axis].xPixelToVoxelScale(
    transform ? invertTransform(coords[0], transform, 'x') : coords[0]
  )
  const j = volume.prevSlice[axis].yPixelToVoxelScale(
    transform ? invertTransform(coords[1], transform, 'y') : coords[1]
  )
  const k = parseInt(sliceNum)

  const [xVal, yVal, zVal] =
    axis === 'xy' ? [i, j, k] : axis === 'yz' ? [k, i, j] : [i, k, j]
  return [xVal, yVal, zVal]
}

/**
 * Update the world coordinates.
 *
 * @param {number[]} coords The worlds coordinates to show.
 * @param {string} id  The unique ID of the volume viewers voxel info.
 */
function updateWorldLabels (coords, id) {
  const format = d3.format('.2f')
  const formattedCoords = coords.map((coord) => format(coord))
  d3.select('#world-coords-' + id).node().value =
    '(' + formattedCoords.join(', ') + ')'
}

/**
 * Update the voxel coordinates.
 *
 * @param {number[]} coords The voxel coordinates to show.
 * @param {string} id The unique ID of the volume viewers voxel info.
 */
function updateVoxelLabels (coords, id) {
  d3.select('#voxel-coords-' + id).node().value = '(' + coords.join(', ') + ')'
}

/**
 * Invert the zoom transform to get the coordinates as if no transformation had
 * been applied.
 *
 * @param {number} val The value to invert.
 * @param {Object} transform The zoom transform to reverse.
 * @param {string} dir The direction of the zoom transform (x or y) to reverse.
 * @returns {number}
 */
function invertTransform (val, transform, dir) {
  return (val - transform[dir]) / transform.k
}

/**
 * Apply the zoom transform to get transformed coordinates.
 *
 * @param {number} val The value to transform.
 * @param {Object} transform The zoom transform to apply.
 * @param {string} dir The direction of the zoom transform (x or y) to apply.
 * @returns {number}
 */
function applyTransform (val, transform, dir) {
  return val * transform.k + transform[dir]
}

/**
 * Update the voxel coordinates and dose profiles.
 *
 * @param {DensityVolume} densityVol The density volume of the volume viewer.
 * @param {DoseVolume} doseVol The dose volume of the volume viewer.
 * @param {number[]} coords The coordinates of the mouse position on the panel.
 * @param {string} axis The axis of the slice (xy, yz, or xz).
 * @param {number} sliceNum The number of the current slice displayed in the panel.
 * @param {Object} transform The zoom transform of the panel.
 * @param {string} id The unique ID of the volume viewers voxel info.
 */
function updateVoxelCoords (
  densityVol,
  doseVol,
  coords,
  axis,
  sliceNum,
  transform,
  id
) {
  const vol = densityVol || doseVol
  if (vol) {
    // Get world and voxel coordinates from pixel value
    const worldCoords = coordsToWorld(coords, axis, sliceNum, vol, transform)
    const voxelCoords = coordsToVoxel(coords, axis, sliceNum, vol, transform)

    // Update voxel info if checkbox is checked
    if (d3.select("input[name='show-marker-checkbox']").node().checked) {
      updateWorldLabels(worldCoords, id)
      updateVoxelLabels(voxelCoords, id)
      updateVoxelInfo(voxelCoords, densityVol, doseVol, id)
    }

    // Update dose profiles if checkbox is checked
    if (
      doseVol &&
      d3.select("input[name='show-dose-profile-checkbox']").node().checked
    ) {
      updateDoseProfiles(voxelCoords, worldCoords)
    }
  }
}

/**
 * Update the voxel information including density, material, and dose.
 *
 * @param {number[]} voxelCoords The voxel coordinates of the data.
 * @param {DensityVolume} densityVol The density volume of the volume viewer.
 * @param {DoseVolume} doseVol The dose volume of the volume viewer.
 * @param {string} id The unique ID of the volume viewers voxel info.
 */
function updateVoxelInfo (voxelCoords, densityVol, doseVol, id) {
  if (densityVol) {
    const density = densityVol.getDataAtVoxelCoords(voxelCoords)
    d3.select('#density-value-' + id).node().value =
      d3.format('.3f')(density) + ' g/cm\u00B3'

    const material = densityVol.getMaterialAtVoxelCoords(voxelCoords)
    d3.select('#material-value-' + id).node().value = material
  }

  if (doseVol) {
    const dose = doseVol.getDataAtVoxelCoords(voxelCoords) || 0
    const error = doseVol.getErrorAtVoxelCoords(voxelCoords) || 0
    d3.select('#dose-value-' + id).node().value =
      d3.format('.1%')(dose) + ' \u00B1 ' + d3.format('.1%')(error)
  }
}

/**
 * Update the dose profiles to show the dose through the given coordinates.
 *
 * @param {number[]} voxelCoords The voxel coordinates of the data.
 * @param {number[]} worldCoords The world coordinates of the data.
 */
function updateDoseProfiles (voxelCoords, worldCoords) {
  var getCoords = (coords) => [
    [coords[0], coords[1]],
    [coords[1], coords[2]],
    [coords[0], coords[2]]
  ]

  const voxelCoordsList = getCoords(voxelCoords)
  const worldCoordsList = getCoords(worldCoords)
  const dimensionsList = ['z', 'x', 'y']

  volumeViewerList.forEach((volumeViewer) => {
    volumeViewer.doseProfileList.forEach((doseProfile, i) => {
      if (volumeViewer.doseVolume) {
        // Set the data
        doseProfile.setDoseProfileData(
          volumeViewer.doseVolume,
          volumeViewer.densityVolume,
          dimensionsList[i],
          voxelCoordsList[i]
        )

        // Plot the dose profile
        doseProfile.plotDoseProfile(worldCoordsList[i])
      }
    })
  })
}
