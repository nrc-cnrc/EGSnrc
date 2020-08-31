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


/**
 * Initialize the window and level sliders.
 *
 * @param {Object} levelParentDiv The parent div of the level slider.
 * @param {Object} windowParentDiv The parent div of the window slider.
 * @param {DensityVolume} densityVol The density volume the sliders control.
 * @param {Object} panels The panel for each axis.
 */
var initializeWindowAndLevelSlider = (
  levelParentDiv,
  windowParentDiv,
  densityVol,
  panels
) => {
  // Make level slider
  var levelSliderChangeCallback = (sliderVal) => {
    densityVol.setLevel(sliderVal);
    densityVol.addColourScheme();
    ["xy", "yz", "xz"].forEach((axis) =>
      densityVol.drawDensity(
        densityVol.prevSlice[axis],
        panels[axis].zoomTransform
      )
    );
  };

  let levelSliderParams = {
    id: "level",
    label: "Level",
    format: d3.format(".2f"),
    startingVal: densityVol.level,
    minVal: densityVol.window / 2,
    maxVal: densityVol.data.maxDensity - densityVol.window / 2,
    step: 0.01,
  };

  let levelSlider = new Slider(
    levelParentDiv,
    levelSliderChangeCallback,
    levelSliderParams
  );

  // Make window slider
  var windowSliderChangeCallback = (sliderVal) => {
    densityVol.setWindow(sliderVal);
    densityVol.addColourScheme();
    ["xy", "yz", "xz"].forEach((axis) =>
      densityVol.drawDensity(
        densityVol.prevSlice[axis],
        panels[axis].zoomTransform
      )
    );
    // Fix level slider min and max vals
    levelSlider.setMinValue(densityVol.window / 2);
    levelSlider.setMaxValue(densityVol.data.maxDensity - densityVol.window / 2);

    // Set level in density volume if changed
    densityVol.setLevel(levelSlider.value);
  };

  let windowSliderParams = {
    id: "window",
    label: "Window",
    format: d3.format(".2f"),
    startingVal: densityVol.window,
    minVal: 0.0,
    maxVal: densityVol.window,
    step: 0.01,
  };

  let windowSlider = new Slider(
    windowParentDiv,
    windowSliderChangeCallback,
    windowSliderParams
  );
};
