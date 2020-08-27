// TODO: If marker exists, on axis change, use marker coordinates?
// // https://github.com/aces/brainbrowser/blob/master/src/brainbrowser/volume-viewer.js#L411-L415
// class VoxelInfo {
//   constructor() {

//   }
// }

function buildVoxelInfoHtml(parentDiv, id) {
  // Define label texts and tags
  let labelName = [
    "World Coordinates (cm):",
    "Voxel Coordinates:",
    "Density:",
    "Material:",
    "Dose:",
  ];

  let tagList = [
    "world-coords",
    "voxel-coords",
    "density-value",
    "material-value",
    "dose-value",
  ];

  // Add container
  let voxelInfoHolder = parentDiv
    .append("div")
    .classed("voxel-info", true)
    .attr("id", "voxel-info-" + id)
    .classed("hidden", true);

  // Iterate through tag list and add label and output for each
  tagList.forEach((tag, i) => {
    voxelInfoHolder
      .append("label")
      .attr("for", tag + "-" + id)
      .text("  " + labelName[i] + " ");

    voxelInfoHolder
      .append("output")
      .attr("type", "text")
      .attr("class", "voxel-info-output")
      .attr("id", tag + "-" + id);
  });
}

function coordsToWorld(coords, axis, sliceNum, volume, transform, updateXY) {
  // TODO: Have a more permanent solution to the click/transform problem, make a class?

  // Invert transformation if applicable then invert scale to get world coordinate
  let i = volume.prevSlice[axis].xScale.invert(
    transform ? invertTransform(coords[0], transform, "x") : coords[0]
  );
  let j = volume.prevSlice[axis].yScale.invert(
    transform ? invertTransform(coords[1], transform, "y") : coords[1]
  );

  // Add 0.5 to sliceNum in order to map values to center of voxel bondaries
  // TODO: Perhaps fix scale to get rid of the 0.5 hack
  let k = volume.prevSlice[axis].zScale.invert(parseInt(sliceNum) + 0.5);

  let [xVal, yVal, zVal] =
    axis === "xy" ? [i, j, k] : axis === "yz" ? [k, i, j] : [i, k, j];
  return [xVal, yVal, zVal];
}

function coordsToVoxel(coords, axis, sliceNum, volume, transform, updateXY) {
  // Invert transformation if applicable then apply scale to get voxel coordinate
  let i = volume.prevSlice[axis].xPixelToVoxelScale(
    transform ? invertTransform(coords[0], transform, "x") : coords[0]
  );
  let j = volume.prevSlice[axis].yPixelToVoxelScale(
    transform ? invertTransform(coords[1], transform, "y") : coords[1]
  );
  let k = parseInt(sliceNum);

  let [xVal, yVal, zVal] =
    axis === "xy" ? [i, j, k] : axis === "yz" ? [k, i, j] : [i, k, j];
  return [xVal, yVal, zVal];
}

function updateWorldLabels(coords, id) {
  let format = d3.format(".2f");
  let formattedCoords = coords.map((coord) => format(coord));
  d3.select("#world-coords-" + id).node().value =
    "(" + formattedCoords.join(", ") + ")";
}

function updateVoxelLabels(coords, id) {
  d3.select("#voxel-coords-" + id).node().value = "(" + coords.join(", ") + ")";
}

function invertTransform(val, transform, dir) {
  return (val - transform[dir]) / transform.k;
}

function applyTransform(val, transform, dir) {
  return val * transform.k + transform[dir];
}

function updateVoxelCoords(
  densityVol,
  doseVol,
  coords,
  axis,
  sliceNum,
  transform,
  id,
  updateXY = false
) {
  let vol = densityVol || doseVol;
  if (vol) {
    // Get world and voxel coordinates from pixel value
    let worldCoords = coordsToWorld(
      coords,
      axis,
      sliceNum,
      vol,
      transform,
      updateXY
    );
    let voxelCoords = coordsToVoxel(
      coords,
      axis,
      sliceNum,
      vol,
      transform,
      updateXY
    );

    // Update voxel info if checkbox is checked
    if (d3.select("input[name='show-marker-checkbox']").node().checked) {
      updateWorldLabels(worldCoords, id);
      updateVoxelLabels(voxelCoords, id);
      updateVoxelInfo(voxelCoords, densityVol, doseVol, id);
    }

    // TODO: Use dispatch event instead of this
    // Update dose profiles if checkbox is checked
    if (
      doseVol &&
      d3.select("input[name='show-dose-profile-checkbox']").node().checked
    ) {
      updateDoseProfiles(voxelCoords, worldCoords);
    }
  }
}

function updateVoxelInfo(voxelCoords, densityVol, doseVol, id) {
  if (densityVol) {
    let density = densityVol.getDataAtVoxelCoords(voxelCoords);
    d3.select("#density-value-" + id).node().value =
      d3.format(".3f")(density) + " g/cm\u00B3";

    let material = densityVol.getMaterialAtVoxelCoords(voxelCoords);
    d3.select("#material-value-" + id).node().value = material;
  }

  if (doseVol) {
    let dose = doseVol.getDataAtVoxelCoords(voxelCoords) || 0;
    let error = doseVol.getErrorAtVoxelCoords(voxelCoords) || 0;
    d3.select("#dose-value-" + id).node().value =
      d3.format(".1%")(dose) + " \u00B1 " + d3.format(".1%")(error);
  }
}

function updateDoseProfiles(voxelCoords, worldCoords) {
  var getCoords = (coords) => [
    [coords[0], coords[1]],
    [coords[1], coords[2]],
    [coords[0], coords[2]],
  ];

  let voxelCoordsList = getCoords(voxelCoords);
  let worldCoordsList = getCoords(worldCoords);
  let dimensionsList = ["z", "x", "y"];
  // let axesList = ["yz", "xz", "xy"];

  volumeViewerList.forEach((volumeViewer) => {
    volumeViewer.doseProfileList.forEach((doseProfile, i) => {
      if (volumeViewer.doseVolume) {
        // Set the data
        doseProfile.setDoseProfileData(
          volumeViewer.doseVolume,
          volumeViewer.densityVolume,
          dimensionsList[i],
          voxelCoordsList[i]
        );

        // Plot the dose profile
        doseProfile.plotDoseProfile(worldCoordsList[i]);
      }
    });
  });
}
