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
#  Author:          Elise Badun, 2021
#
#  Contributors:
#
###############################################################################
*/

// definitions for StandardJS formatter
/* global d3 */

// REMOVE THESE GLOBAL IMPORTS ONCE MODULES RE-IMPLEMENTED
/* global Slider */
/* global coordsToVoxel */

// import { Slider } from './sliders/slider.js'
// import { coordsToVoxel } from './voxel-coordinates.js'

/**
 * Initialize the min and max density sliders for the given volume viewer.
 *
 * @param {Object} minParentDiv The parent div of the min slider.
 * @param {Object} maxParentDiv The parent div of the max slider.
 * @param {DensityVolume} densityVol The density volume the sliders control.
 * @param {VolumeViewer} volumeViewer The volumeViewer to initialize the slider.
 */
var initializeSliceSlider = ( // eslint-disable-line no-unused-vars
  sliceParentDiv,
  axis,
  volumeViewer
) => {
  // Clear slice slider divs
  sliceParentDiv.selectAll('*').remove()

  // Make slice slider
  var onSliceChangeCallback = (sliderVal) => {
    const currPanel = volumeViewer.panels[axis]

    // Update slice of current panel
    volumeViewer.updateSlice(currPanel.axis, parseInt(sliderVal) - 1)

    // TODO: Fix this, bug after zooming/translating and changing slice
    // Update marker position, voxel information and dose profile
    const plotCoords = currPanel.markerPosition
    if (currPanel.showMarker()) {
      // ISSUE HERE: don't need to redraw other panels on slider change,
      // just marker position and voxel info
      const voxelCoords = coordsToVoxel(
        plotCoords,
        currPanel.axis,
        currPanel.densitySliceNum || currPanel.doseSliceNum,
        currPanel.volume,
        currPanel.zoomTransform
      )

      // Update voxel info
      const worldCoords = currPanel.coordsToWorld(plotCoords)
      volumeViewer.updateVoxelInfo(worldCoords)

      Object.values(volumeViewer.panels).forEach((panel) => {
        if (panel.axis !== currPanel.axis) {
          let voxelNums
          if (panel.axis === 'xy') {
            voxelNums = [voxelCoords[0], voxelCoords[1]]
          } else if (panel.axis === 'yz') {
            voxelNums = [voxelCoords[1], voxelCoords[2]]
          } else {
            voxelNums = [voxelCoords[0], voxelCoords[2]]
          }

          // Convert voxel number to pixel value for both x and y coordinates
          const xScale = panel.volume.baseSlices[panel.axis].xPixelToVoxelScale.invertExtent
          const yScale = panel.volume.baseSlices[panel.axis].yPixelToVoxelScale.invertExtent

          let coords
          if (panel.zoomTransform) {
            coords = panel.zoomTransform.apply([
              Math.ceil(xScale(voxelNums[0]).reduce((total, num) => total + num) / 2),
              Math.ceil(yScale(voxelNums[1]).reduce((total, num) => total + num) / 2)
            ])
          } else {
            coords = [
              Math.ceil(xScale(voxelNums[0]).reduce((total, num) => total + num) / 2),
              Math.ceil(yScale(voxelNums[1]).reduce((total, num) => total + num) / 2)
            ]
          }

          panel.updateMarker(coords, false)
        }
      })
    }
  }

  const sliceSliderParams = {
    id: 'slice-number-' + axis,
    label: 'Slice Number',
    format: d3.format('d'),
    startingVal: 0,
    minVal: 1,
    maxVal: 1,
    step: 1,
    margin: {
      top: 0,
      right: volumeViewer.mainViewerDimensions.margin.right,
      bottom: 0,
      left: volumeViewer.mainViewerDimensions.margin.left
    },
    style: { 'text-align': 'center' },
    onSliderChangeCallback: onSliceChangeCallback
  }

  const sliceSlider = new Slider(
    sliceParentDiv,
    sliceSliderParams
  )

  return sliceSlider
}

// export { initializeMinMaxDensitySlider }
