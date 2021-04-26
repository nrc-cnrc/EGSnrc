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
    // Given the voxel array, return the range of values for each dimension
    const getRange = (voxArr) => {
      var range = {}
      Object.entries(voxArr).forEach(([dim, vals]) => {
        range[dim] = [vals[0], vals[vals.length - 1]]
      })
      return range
    }

    // Returns true if ROIOutlines overlap with voxelArr
    const overlap = (ROIOutlines, voxelArr) => {
      // First get ROI range
      const ROIRange = getRange(ROIOutlines[0].voxelArr)
      ROIOutlines.forEach((ROIOutline) => {
        const range = getRange(ROIOutline.voxelArr)
        Object.entries(range).forEach(([dim, vals]) => {
          if (range[dim][0] < ROIRange[dim][0]) ROIRange[dim][0] = range[dim][0]
          if (range[dim][1] > ROIRange[dim][1]) ROIRange[dim][1] = range[dim][1]
        })
      })

      // Get density or dose range
      const voxelRange = getRange(voxelArr)

      // Determine if the areas overlap
      const overlap = Object.entries(ROIRange).reduce((acc, [dim, range]) => {
        return (acc && (voxelRange[dim][0] <= range[1]) && (range[0] <= voxelRange[dim][1]))
      }, 1)

      return overlap
    }

    // Set volume viewer structure set to the set that overlaps with the current
    // files displayed
    var structureSetVol
    for (let i = 0; i < structureSetVolumeList.length; i++) {
      structureSetVol = structureSetVolumeList[i]
      // Check if studyInstanceUID matches or areas overlap
      if ((volumeViewer.densityVolume &&
           ((structureSetVol.data.studyInstanceUID === volumeViewer.densityVolume.data.studyInstanceUID) ||
           overlap(structureSetVol.ROIoutlines, volumeViewer.densityVolume.data.voxelArr))) ||
          (volumeViewer.doseVolume &&
            ((structureSetVol.data.studyInstanceUID === volumeViewer.doseVolume.data.studyInstanceUID) ||
            overlap(structureSetVol.ROIoutlines, volumeViewer.doseVolume.data.voxelArr)))) {
        volumeViewer.setStructureSetVolume(structureSetVol)
      } else {
        console.log('No structure set matches current dose/density files')
        checkbox.checked = false
      }
    }

    if (volumeViewer.isDVHAllowed()) volumeViewer.enableCheckbox(volumeViewer.showDVHCheckbox)
  } else {
    // Set volume viewer structure set to null
    volumeViewer.structureSetVolume = null

    // Remove ROI legend
    volumeViewer.ROILegendSvg.selectAll('*').remove()

    // Uncheck show DVH checkbox and disable
    volumeViewer.showDVHCheckbox.node().checked = false
    volumeViewer.DVH.parentSvg.style('display', 'none')
    volumeViewer.disableCheckbox(volumeViewer.showDVHCheckbox)

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
      // Show DVH plot
      volumeViewer.DVH.parentSvg.style('display', null)
      volumeViewer.DVH.setDVHData(volumeViewer.structureSetVolume, volumeViewer.doseVolume)
      volumeViewer.DVH.createPlots()
      volumeViewer.DVH.plotDVH(volumeViewer.doseNorm)

      // Enable saving DVH as csv
      volumeViewer.saveDVHButton.node().disabled = false
    }
  } else {
    // Reset the zoom and hide the plot
    volumeViewer.DVH.resetZoomTransform()
    volumeViewer.DVH.parentSvg.style('display', 'none')

    // Disable saving DVH as csv
    volumeViewer.saveDVHButton.node().disabled = true
  }
}

/**
 * Define the behaviour of selecting the normalize DICOM checkbox.
 */
var defineNormalizeDoseCheckboxBehaviour = function (volumeViewer, checkbox) { // eslint-disable-line no-unused-vars
  if (checkbox.checked) {
    // Enable the input boxes
    volumeViewer.doseNormValInput.select('input').attr('disabled', null)
    volumeViewer.doseNormPercentInput.select('input').attr('disabled', null)

    // If existing values in boxes, update doseNorm
    const doseNormVal = parseFloat(volumeViewer.doseNormValInput.select('input').node().value) * 100 // Convert from cGy to Gy
    const doseNormPercent = parseFloat(volumeViewer.doseNormPercentInput.select('input').node().value) / 100 // Convert from percent to decimal
    if (doseNormVal && doseNormPercent) volumeViewer.normalizeDoseValues(doseNormVal, doseNormPercent)
  } else {
    // Disable the input boxes
    volumeViewer.doseNormValInput.select('input').attr('disabled', 'disabled')
    volumeViewer.doseNormPercentInput.select('input').attr('disabled', 'disabled')

    // Reset doseNorm
    volumeViewer.normalizeDoseValues(volumeViewer.doseVolume.data.maxDose, 1)

    if (!volumeViewer.isDVHAllowed()) {
      // If dose is relative, uncheck and disable show DVH checkbox
      volumeViewer.showDVHCheckbox.node().checked = false
      volumeViewer.DVH.parentSvg.style('display', 'none')
      volumeViewer.disableCheckbox(volumeViewer.showDVHCheckbox)
    }
  }
  // Reset the zoom on the DVH
  if (volumeViewer.showDVHCheckbox.node().checked) {
    volumeViewer.DVH.resetZoomTransform()
  }
}

// export {
//   defineShowMarkerCheckboxBehaviour, defineShowProfileCheckboxBehaviour,
//   enableCheckboxForDensityPlot, defineShowROICheckboxBehaviour, enableButton,
//   defineShowDVHCheckboxBehaviour, defineNormalizeDoseCheckboxBehaviour
// }
