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

// import { Slider } from './slider.js'

/**
 * Initialize the dose comparison normalization slider for the given volume viewer.
 *
 * @param {Object} doseCompNormParentDiv The parent div of the slider.
 * @param {DoseComparisonVolume} doseVol The dose volume the sliders control.
 * @param {VolumeViewer} volumeViewer The volumeViewer to initialize the slider.
 */
var initializeDoseCompNormSlider = ( // eslint-disable-line no-unused-vars
  doseCompNormParentDiv,
  doseVol,
  volumeViewer
) => {
  // Clear max div
  doseCompNormParentDiv.selectAll('*').remove()

  // Make dose comparison normalization slider
  var onMaxDoseChangeCallback = (sliderVal) => {
    volumeViewer.setDoseComparisonNormFactor(sliderVal)

    // Update voxel info
    if (volumeViewer.worldCoords) volumeViewer.updateVoxelInfo(volumeViewer.worldCoords)
  }

  const doseCompNormSliderParams = {
    id: 'dose-comp-norm',
    label: 'Dose Comparison Normalization',
    format: d3.format('.0%'),
    startingVal: 1.0,
    minVal: 0.5,
    maxVal: 1.5,
    step: 0.05,
    onSliderChangeCallback: onMaxDoseChangeCallback
  }

  const doseCompNormSlider = new Slider( // eslint-disable-line no-unused-vars
    doseCompNormParentDiv,
    doseCompNormSliderParams
  )
}

// export { initializeDoseCompNormSlider }
