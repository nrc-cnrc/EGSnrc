// Set the dimensions and margins of the graph
let doseProfileDimensions = {
  fullWidth: 510,
  fullHeight: 440,
  margin: { top: 30, right: 60, bottom: 60, left: 80 },
  get width() {
    return this.fullWidth - this.margin.left - this.margin.right;
  },
  get height() {
    return this.fullHeight - this.margin.top - this.margin.bottom;
  },
};

var doseProfileSvg = d3
  .select("#dose-profile-svg")
  .attr("width", doseProfileDimensions.fullWidth)
  .attr("height", doseProfileDimensions.fullHeight)
  .append("g")
  .attr(
    "transform",
    "translate(" +
      doseProfileDimensions.margin.left +
      "," +
      doseProfileDimensions.margin.top +
      ")"
  );

const doseProfile = new DoseProfile(doseProfileDimensions, doseProfileSvg);

const doseProfileButtons = d3
  .selectAll("input[name='profile-axis']")
  .on("change", function () {
    profileAxis = this.value;

    if (!densityVol.isEmpty()) {
      updateCoordInputsLabels(profileAxis, densityVol.data.voxelNumber);
    } else if (!doseVol.isEmpty()) {
      updateCoordInputsLabels(profileAxis, doseVol.data.voxelNumber);
    }

    return true;
  });

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

d3.select("input[name='show-dose-profile-checkbox']").on("change", function () {
  if (this.checked) {
    // Enable zooming
    doseProfileX.svg.select("rect.bounding-box").call(doseProfileX.zoomObj);
    doseProfileY.svg.select("rect.bounding-box").call(doseProfileY.zoomObj);

    // Remove hidden class
    let doseProfilePlots = d3.selectAll("svg.dose-profile-plot");
    doseProfilePlots.classed("hidden", false);

    // Plot dose profile
    if (plotCoords) {
      updateVoxelCoords(plotCoords, getAxis(), getSliceNum(), false);
    }

    // Enable saving dose profiles as svg
    d3.select("#save-dose-profile").node().disabled = false;
  } else {
    // Disable zooming
    disableZoom(doseProfileX.svg.select("rect.bounding-box"));
    disableZoom(doseProfileY.svg.select("rect.bounding-box"));

    // Add hidden class to dose profile plots
    let doseProfilePlots = d3.selectAll("svg.dose-profile-plot");
    doseProfilePlots.classed("hidden", true);

    // Disable saving dose profiles as svg
    d3.select("#save-dose-profile").node().disabled = true;
  }
});

function updateCoordInputsLabels(profileAxis, voxelNumber) {
  // Update max value and label of coordinate inputs
  if (profileAxis === "x") {
    d3.select("#coord-1").node().max = voxelNumber.y - 1;
    d3.select("#coord-2").node().max = voxelNumber.z - 1;
    d3.select("#coord-1-label").node().textContent = "y";
    d3.select("#coord-2-label").node().textContent = "z";
  } else if (profileAxis === "y") {
    d3.select("#coord-1").node().max = voxelNumber.x - 1;
    d3.select("#coord-2").node().max = voxelNumber.z - 1;
    d3.select("#coord-1-label").node().textContent = "x";
    d3.select("#coord-2-label").node().textContent = "z";
  } else if (profileAxis === "z") {
    d3.select("#coord-1").node().max = voxelNumber.x - 1;
    d3.select("#coord-2").node().max = voxelNumber.y - 1;
    d3.select("#coord-1-label").node().textContent = "x";
    d3.select("#coord-2-label").node().textContent = "y";
  }
}

function enableCoordInputs(voxelNumber) {
  // Enable profile-axis radio buttons if disabled
  let doseProfileAxisButtons = d3
    .selectAll("input[name='profile-axis']")
    .nodes();
  doseProfileAxisButtons.forEach((radioButton) => {
    if (radioButton.disabled) radioButton.disabled = false;
  });

  // Enable dose profile button if disabled
  let doseProfileMakeButton = d3
    .select("button[id='dose-profile-button']")
    .node();
  if (doseProfileMakeButton.disabled) doseProfileMakeButton.disabled = false;

  // Enable inputs if disabled
  let coordInputs = d3.selectAll("input[name='coord-input']").nodes();
  coordInputs.forEach((coordInput) => {
    if (coordInput.disabled) coordInput.disabled = false;
  });

  // Get selected profile axis and check dimensions of uploaded file
  let profileAxis = d3.selectAll("input[name='profile-axis']:checked").node()
    .value;

  updateCoordInputsLabels(profileAxis, voxelNumber);

  if (!densityVol.isEmpty()) {
    enableCheckboxForDensityPlot();
  }
}

var voxelCoordsToWorld = (voxelCoords, profileDim, volume) =>
  profileDim === "x"
    ? [
        volume.yWorldToVoxelScale
          .invertExtent(voxelCoords[0])
          .reduce((e, arr) => e + arr) / 2,
        volume.zWorldToVoxelScale
          .invertExtent(voxelCoords[1])
          .reduce((e, arr) => e + arr) / 2,
      ]
    : profileDim === "y"
    ? [
        volume.xWorldToVoxelScale
          .invertExtent(voxelCoords[0])
          .reduce((e, arr) => e + arr) / 2,
        volume.zWorldToVoxelScale
          .invertExtent(voxelCoords[1])
          .reduce((e, arr) => e + arr) / 2,
      ]
    : [
        volume.xWorldToVoxelScale
          .invertExtent(voxelCoords[0])
          .reduce((e, arr) => e + arr) / 2,
        volume.yWorldToVoxelScale
          .invertExtent(voxelCoords[1])
          .reduce((e, arr) => e + arr) / 2,
      ];

d3.select("#dose-profile-button").on("click", function () {
  // Get 1D plot data
  let profileDim = d3.selectAll("input[name='profile-axis']:checked").node()
    .value;
  let coord1 = parseInt(d3.select("#coord-1").node().value);
  let coord2 = parseInt(d3.select("#coord-2").node().value);
  let densityChecked = d3
    .select("input[name='density-profile-checkbox']")
    .node().checked;
  let axisList =
    profileDim === "x" ? "xyz" : profileDim === "y" ? "yxz" : "zxy";

  // TODO: Add listener to density-profile-checkbox instead of setting the variable each time
  doseProfile.plotDensity = densityChecked;
  doseProfile.setDoseProfileData(axisList, [coord1, coord2]);
  doseProfile.updateAxes();

  // TODO: Add a check to see if dose and density have same coordinate system
  let axis = profileDim === "x" ? "yz" : profileDim === "y" ? "xz" : "xy";

  doseProfile.plotDoseProfile(
    axis,
    profileDim,
    voxelCoordsToWorld([coord1, coord2], profileDim, doseVol)
  );
});
