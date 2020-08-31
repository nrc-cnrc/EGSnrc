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
 * Enable the plot density checkbox for the dose profile plots.
 */
var enableCheckboxForDensityPlot = () => {
  let densityCheckbox = d3
    .select("input[name='density-profile-checkbox']")
    .node();
  if (densityCheckbox.disabled) densityCheckbox.disabled = false;
};

/**
 * Enable the checkbox for the dose profile plots.
 */
var enableCheckboxForDoseProfilePlot = () => {
  let showDoseProfileCheckbox = d3
    .select("input[name='show-dose-profile-checkbox']")
    .node();
  if (showDoseProfileCheckbox.disabled)
    showDoseProfileCheckbox.disabled = false;
};

/**
 * Enable the export visualization to png button.
 */
var enableExportVisualizationButton = () => {
  let exportVisualizationButton = d3.select("button#save-vis").node();
  if (exportVisualizationButton.disabled)
    exportVisualizationButton.disabled = false;
};

/**
 * Enable the checkbox to view voxel information on click.
 */
var enableCheckboxForVoxelInformation = () => {
  let showMarkerCheckbox = d3
    .select("input[name='show-marker-checkbox']")
    .node();
  if (showMarkerCheckbox.disabled) showMarkerCheckbox.disabled = false;
};

/**
 * Define the behaviour of clicking the add volume viewer button.
 */
d3.select("#add-volume-viewer").on("click", function () {
  volumeViewerList.push(
    new VolumeViewer(
      mainViewerDimensions,
      legendDimensions,
      sideDoseProfileDimensions,
      "vol-" + volumeViewerList.length
    )
  );
});

/**
 * Define the behaviour of selecting the show dose profile checkbox.
 */
d3.select("input[name='show-dose-profile-checkbox']").on("change", function () {
  // Call all panels to show/hide crosshairs
  volumeViewerList.forEach((volumeViewer) => {
    Object.values(volumeViewer.panels).forEach((panel) => {
      panel.updateCrosshairDisplay();
    });

    if (this.checked) {
      // Hide dose profile plots
      volumeViewer.doseProfileList.forEach((doseProfile) =>
        doseProfile.parentSvg.style("display", null)
      );

      // Enable saving dose profiles as csv
      d3.select("#save-dose-profile").node().disabled = false;

      // Update dose profiles
      // Only choose first panel because it will update all dose profiles
      let panel = volumeViewer.panels["xy"];
      if (panel.markerPosition) {
        updateVoxelCoords(
          panel.densityVol,
          panel.doseVol,
          panel.markerPosition,
          panel.axis,
          panel.sliceNum,
          panel.zoomTransform,
          volumeViewer.id
        );
      }
    } else {
      // Show dose profile plots
      volumeViewer.doseProfileList.forEach((doseProfile) => {
        doseProfile.parentSvg.style("display", "none");
      });

      // Disable saving dose profiles as csv
      d3.select("#save-dose-profile").node().disabled = true;
    }
  });
});

/**
 * Define the behaviour of selecting the show voxel information checkbox.
 */
d3.select("input[name='show-marker-checkbox']").on("change", function () {
  // Call all panels to show/hide circle marker
  volumeViewerList.forEach((volumeViewer) => {
    Object.values(volumeViewer.panels).forEach((panel) => {
      panel.updateCircleMarkerDisplay();
    });

    let voxelInfo = d3.selectAll("div#voxel-info-" + volumeViewer.id);
    if (this.checked) {
      // Remove hidden class
      voxelInfo.classed("hidden", false);

      // Update voxel information
      volumeViewerList.forEach((volumeViewer) => {
        let panel = volumeViewer.panels["xy"];
        if (panel.markerPosition) {
          updateVoxelCoords(
            panel.densityVol,
            panel.doseVol,
            panel.markerPosition,
            panel.axis,
            panel.sliceNum,
            panel.zoomTransform,
            panel.volumeViewerId
          );
        }
      });
    } else {
      // Add hidden class
      voxelInfo.classed("hidden", true);
    }
  });
});
