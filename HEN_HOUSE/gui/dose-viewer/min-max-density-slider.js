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

import { Slider } from './slider.js'

/**
 * Initialize the min and max density sliders.
 *
 * @param {Object} minParentDiv The parent div of the min slider.
 * @param {Object} maxParentDiv The parent div of the max slider.
 * @param {DensityVolume} densityVol The density volume the sliders control.
 * @param {Object} panels The panel for each axis.
 */
var initializeMinMaxDensitySlider = (
  minParentDiv,
  maxParentDiv,
  densityVol,
  panels
) => {
  // Clear min and max divs
  minParentDiv.selectAll('*').remove()
  maxParentDiv.selectAll('*').remove()

  // Make min slider
  var minSliderChangeCallback = (sliderVal) => {
    if (parseFloat(sliderVal) > parseFloat(maxSlider.value)) {
      minSlider.setCurrentValue(maxSlider.value)
    }
  }

  var minSliderReleaseCallback = (sliderVal) => {
    densityVol.setMinDensityVar(sliderVal, panels)
  }

  const minSliderParams = {
    id: 'min',
    label: 'Min',
    format: densityVol.densityFormat,
    startingVal: densityVol.minDensityVar,
    minVal: densityVol.data.minDensity,
    maxVal: densityVol.maxDensityVar,
    step: densityVol.densityStep,
    onSliderChangeCallback: minSliderChangeCallback,
    onSliderReleaseCallback: minSliderReleaseCallback
  }

  const minSlider = new Slider( // eslint-disable-line no-unused-vars
    minParentDiv,
    minSliderParams
  )

  // Make max slider
  var maxSliderChangeCallback = (sliderVal) => {
    if (parseFloat(sliderVal) < parseFloat(minSlider.value)) {
      maxSlider.setCurrentValue(minSlider.value)
    }
  }

  var maxSliderReleaseCallback = (sliderVal) => {
    densityVol.setMaxDensityVar(sliderVal, panels)
  }

  const maxSliderParams = {
    id: 'max',
    label: 'Max',
    format: densityVol.densityFormat,
    startingVal: densityVol.maxDensityVar,
    minVal: densityVol.minDensityVar,
    maxVal: densityVol.data.maxDensity,
    step: densityVol.densityStep,
    onSliderChangeCallback: maxSliderChangeCallback,
    onSliderReleaseCallback: maxSliderReleaseCallback
  }

  const maxSlider = new Slider( // eslint-disable-line no-unused-vars
    maxParentDiv,
    maxSliderParams
  )
}

export { initializeMinMaxDensitySlider }
