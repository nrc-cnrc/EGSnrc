var enableCheckboxForDensityPlot = () => {
  let densityCheckbox = d3
    .select("input[name='density-profile-checkbox']")
    .node();
  if (densityCheckbox.disabled) densityCheckbox.disabled = false;
};

var enableCheckboxForDoseProfilePlot = () => {
  let showDoseProfileCheckbox = d3
    .select("input[name='show-dose-profile-checkbox']")
    .node();
  if (showDoseProfileCheckbox.disabled)
    showDoseProfileCheckbox.disabled = false;
};

var enableExportVisualizationButton = () => {
  let exportVisualizationButton = d3.select("button#save-vis").node();
  if (exportVisualizationButton.disabled)
    exportVisualizationButton.disabled = false;
};

var enableCheckboxForVoxelInformation = () => {
  let showMarkerCheckbox = d3
    .select("input[name='show-marker-checkbox']")
    .node();
  if (showMarkerCheckbox.disabled) showMarkerCheckbox.disabled = false;
};

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
          volumeViewer.id,
          true
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
            panel.volumeViewerId,
            true
          );
        }
      });
    } else {
      // Add hidden class
      voxelInfo.classed("hidden", true);
    }
  });
});
