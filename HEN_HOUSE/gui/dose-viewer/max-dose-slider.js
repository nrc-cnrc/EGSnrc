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
/* global Slider */

// import { Slider } from './slider.js'

/**
 * Initialize the min and max density sliders for the given volume viewer.
 *
 * @param {Object} maxDoseParentDiv The parent div of the max dose slider.
 * @param {DoseVolume} doseVol The dose volume the sliders control.
 * @param {VolumeViewer} volumeViewer The volumeViewer to initialize the slider.
 */
var initializeMaxDoseSlider = ( // eslint-disable-line no-unused-vars
  maxDoseParentDiv,
  doseVol,
  volumeViewer
) => {
  // Clear max div
  maxDoseParentDiv.selectAll('*').remove()

  // Make max slider
  var onMaxDoseChangeCallback = (sliderVal) =>
    volumeViewer.setMaxDose(sliderVal)

  const doseSliderParams = {
    id: 'max-dose',
    label: 'Max Dose',
    format: d3.format('.0%'),
    startingVal: 1.0,
    minVal: 0.0,
    maxVal: 1.5,
    step: 0.01,
    onSliderChangeCallback: onMaxDoseChangeCallback
  }

  const maxDoseSlider = new Slider( // eslint-disable-line no-unused-vars
    maxDoseParentDiv,
    doseSliderParams
  )
}

// export { initializeMinMaxDensitySlider }
