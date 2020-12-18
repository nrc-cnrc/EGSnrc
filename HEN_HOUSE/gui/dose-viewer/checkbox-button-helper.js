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

import {
  DOSE_PROFILE_DIMENSIONS, LEGEND_DIMENSIONS, MAIN_VIEWER_DIMENSIONS,
  volumeViewerList
} from './index.js'
import { VolumeViewer } from './volume-viewer.js'
import { updateVoxelCoords } from './voxel-coordinates.js'

/**
 * Enable the plot density checkbox for the dose profile plots.
 */
var enableCheckboxForDensityPlot = () => {
  const densityCheckbox = d3
    .select("input[name='density-profile-checkbox']")
    .node()
  if (densityCheckbox.disabled) densityCheckbox.disabled = false
}

/**
 * Enable the checkbox for the dose profile plots.
 */
var enableCheckboxForDoseProfilePlot = () => {
  const showDoseProfileCheckbox = d3
    .select("input[name='show-dose-profile-checkbox']")
    .node()
  if (showDoseProfileCheckbox.disabled) { showDoseProfileCheckbox.disabled = false }
}

/**
 * Enable the export visualization to png button.
 */
var enableExportVisualizationButton = () => {
  const exportVisualizationButton = d3.select('button#save-vis').node()
  if (exportVisualizationButton.disabled) { exportVisualizationButton.disabled = false }
}

/**
 * Enable the checkbox to view voxel information on click.
 */
var enableCheckboxForVoxelInformation = () => {
  const showMarkerCheckbox = d3
    .select("input[name='show-marker-checkbox']")
    .node()
  if (showMarkerCheckbox.disabled) showMarkerCheckbox.disabled = false
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
d3.select("input[name='show-dose-profile-checkbox']").on('change', function () {
  // Call all panels to show/hide crosshairs
  volumeViewerList.forEach((volumeViewer) => {
    Object.values(volumeViewer.panels).forEach((panel) => {
      panel.updateCrosshairDisplay()
    })

    if (this.checked) {
      // Hide dose profile plots
      volumeViewer.doseProfileList.forEach((doseProfile) =>
        doseProfile.parentSvg.style('display', null)
      )

      // Enable saving dose profiles as csv
      d3.select('#save-dose-profile').node().disabled = false

      // Update dose profiles
      // Only choose first panel because it will update all dose profiles
      const panel = volumeViewer.panels.xy
      if (panel.markerPosition) {
        const worldCoords = panel.coordsToWorld(panel.markerPosition)
        updateVoxelCoords(
          panel.densityVol,
          panel.doseVol,
          worldCoords,
          volumeViewer.id
        )
      }
    } else {
      // Show dose profile plots
      volumeViewer.doseProfileList.forEach((doseProfile) => {
        doseProfile.parentSvg.style('display', 'none')
      })

      // Disable saving dose profiles as csv
      d3.select('#save-dose-profile').node().disabled = true
    }
  })
})

/**
 * Define the behaviour of selecting the show voxel information checkbox.
 */
d3.select("input[name='show-marker-checkbox']").on('change', function () {
  // Call all panels to show/hide circle marker
  volumeViewerList.forEach((volumeViewer) => {
    Object.values(volumeViewer.panels).forEach((panel) => {
      panel.updateCircleMarkerDisplay()
    })

    const voxelInfo = d3.selectAll('div#voxel-info-' + volumeViewer.id)
    if (this.checked) {
      // Remove hidden class
      voxelInfo.classed('hidden', false)

      // Update voxel information
      volumeViewerList.forEach((volumeViewer) => {
        const panel = volumeViewer.panels.xy
        const worldCoords = panel.coordsToWorld(panel.markerPosition)
        if (panel.markerPosition) {
          updateVoxelCoords(
            panel.densityVol,
            panel.doseVol,
            worldCoords,
            panel.volumeViewerId
          )
        }
      })
    } else {
      // Add hidden class
      voxelInfo.classed('hidden', true)
    }
  })
})

export {
  enableCheckboxForDensityPlot, enableCheckboxForDoseProfilePlot,
  enableCheckboxForVoxelInformation, enableExportVisualizationButton
}
