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

// definitions for StandardJS formatter
/* global d3 */

// REMOVE THESE GLOBAL IMPORTS ONCE MODULES RE-IMPLEMENTED
/* global VolumeViewer */
/* global volumeViewerList */
/* global MAIN_VIEWER_DIMENSIONS */
/* global LEGEND_DIMENSIONS */
/* global DOSE_PROFILE_DIMENSIONS */
/* global updateVoxelCoords */
/* global structureSetVolumeList */

// import {
//   DOSE_PROFILE_DIMENSIONS, LEGEND_DIMENSIONS, MAIN_VIEWER_DIMENSIONS,
//   volumeViewerList, structureSetVolumeList
// } from './index.js'
// import { VolumeViewer } from './volume-viewer.js'
// import { updateVoxelCoords } from './voxel-coordinates.js'

/**
 * Enable the plot density checkbox for the dose profile plots.
 */
var enableCheckboxForDensityPlot = () => { // eslint-disable-line no-unused-vars
  const densityCheckbox = d3
    .select("input[name='density-profile-checkbox']")
    .node()
  if (densityCheckbox.disabled) densityCheckbox.disabled = false
}

/**
 * Enable a button.
 */
var enableButton = (button) => { // eslint-disable-line no-unused-vars
  if (button.node().disabled) { button.node().disabled = false }
}

/**
 * Define the behaviour of clicking the add volume viewer button.
 */
d3.select('#add-volume-viewer').on('click', function () {
  volumeViewerList.push(
    new VolumeViewer(
      MAIN_VIEWER_DIMENSIONS,
      LEGEND_DIMENSIONS,
      DOSE_PROFILE_DIMENSIONS,
      'vol-' + volumeViewerList.length
    )
  )
})

/**
 * Define the behaviour of selecting the show dose profile checkbox.
 */
var defineShowProfileCheckboxBehaviour = function (volumeViewer, checkbox) { // eslint-disable-line no-unused-vars
  // Call all panels to show/hide crosshairs
  Object.values(volumeViewer.panels).forEach((panel) => {
    panel.updateCrosshairDisplay()
  })

  if (checkbox.checked) {
    // Hide dose profile plots
    volumeViewer.doseProfileList.forEach((doseProfile) =>
      doseProfile.parentSvg.style('display', null)
    )

    // Enable saving dose profiles as csv
    volumeViewer.saveDoseProfileButton.node().disabled = false

    // Update dose profiles
    // Only choose first panel because it will update all dose profiles
    const panel = volumeViewer.panels.xy
    if (volumeViewer.worldCoords) {
      updateVoxelCoords(
        panel.densityVol,
        panel.doseVol,
        volumeViewer.worldCoords,
        volumeViewer.id,
        panel.showMarker,
        panel.showDoseProfile
      )
    }
  } else {
    // Show dose profile plots
    volumeViewer.doseProfileList.forEach((doseProfile) => {
      doseProfile.parentSvg.style('display', 'none')
    })

    // Disable saving dose profiles as csv
    volumeViewer.saveDoseProfileButton.node().disabled = true
  }
}

/**
 * Define the behaviour of selecting the show voxel information checkbox.
 */
var defineShowMarkerCheckboxBehaviour = function (volumeViewer, checkbox) { // eslint-disable-line no-unused-vars
  // Call all panels to show/hide circle marker
  Object.values(volumeViewer.panels).forEach((panel) => {
    panel.updateCircleMarkerDisplay()
  })

  if (checkbox.checked) {
    // Remove hidden class
    volumeViewer.voxelInfoDiv.classed('hidden', false)

    // Update voxel information
    const panel = volumeViewer.panels.xy
    if (volumeViewer.worldCoords) {
      updateVoxelCoords(
        panel.densityVol,
        panel.doseVol,
        volumeViewer.worldCoords,
        volumeViewer.id,
        panel.showMarker,
        panel.showDoseProfile
      )
    }
  } else {
    // Add hidden class
    volumeViewer.voxelInfoDiv.classed('hidden', true)
  }
}

/**
 * Define the behaviour of selecting the show ROI outlines checkbox.
 */
var defineShowROICheckboxBehaviour = function (volumeViewer, checkbox) { // eslint-disable-line no-unused-vars
  if (checkbox.checked) {
    // Set volume viewer structure set to the set most recently added
    volumeViewer.setStructureSetVolume(structureSetVolumeList[structureSetVolumeList.length - 1])
  } else {
    // Set volume viewer structure set to null
    volumeViewer.structureSetVolume = null

    // Update the plots
    Object.values(volumeViewer.panels).forEach((panel) => {
      panel.axisElements['plot-dose'].selectAll('g.roi-contour').remove()
    })
  }
}

/**
 * Define the behaviour of selecting the show DVH checkbox.
 */
var defineShowDVHCheckboxBehaviour = function (volumeViewer, checkbox) { // eslint-disable-line no-unused-vars
  if (checkbox.checked) {
    if (volumeViewer.structureSetVolume && volumeViewer.DVH && volumeViewer.doseVolume) {
      // Show DVH plots
      volumeViewer.DVH.parentSvg.style('display', null)
      volumeViewer.DVH.setDVHData(volumeViewer.structureSetVolume, volumeViewer.doseVolume)
      volumeViewer.DVH.plotData()

      // Enable saving DVH as csv
      volumeViewer.saveDVHButton.node().disabled = false
    }
  } else {
    volumeViewer.DVH.parentSvg.style('display', 'none')

    // Disable saving DVH as csv
    volumeViewer.saveDVHButton.node().disabled = true
  }
}

// export {
//   defineShowMarkerCheckboxBehaviour, defineShowProfileCheckboxBehaviour,
//   enableCheckboxForDensityPlot, defineShowROICheckboxBehaviour, enableButton,
//   defineShowDVHCheckboxBehaviour
// }
