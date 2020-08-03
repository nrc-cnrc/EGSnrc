// TODO: If marker exists, on axis change, use marker coordinates?
// https://github.com/aces/brainbrowser/blob/master/src/brainbrowser/volume-viewer.js#L411-L415

function coordsToWorld(coords, axis, sliceNum, volume, updateXY) {
  // TODO: Have a more permanent solution to the click/transform problem, make a class?
  let i, j;
  if (updateXY) {
    // Invert transformation if applicable then invert scale to get world coordinate
    i = volume.prevSlice.xScale.invert(
      zoomTransform ? invertTransform(coords[0], zoomTransform, "x") : coords[0]
    );
    j = volume.prevSlice.yScale.invert(
      zoomTransform ? invertTransform(coords[1], zoomTransform, "y") : coords[1]
    );
  } else {
    // Use previous axes coordinates
    [i, j] = [
      d3.select("#world-" + axis[0] + "-value").node().value,
      d3.select("#world-" + axis[1] + "-value").node().value,
    ];
  }

  // Add 0.5 to sliceNum in order to map values to center of voxel bondaries
  // TODO: Perhaps fix scale to get rid of the 0.5 hack
  let k = volume.prevSlice.zScale.invert(parseInt(sliceNum) + 0.5);

  let [xVal, yVal, zVal] =
    axis === "xy" ? [i, j, k] : axis === "yz" ? [k, i, j] : [i, k, j];
  return [xVal, yVal, zVal];
}

function coordsToVoxel(coords, axis, sliceNum, volume, updateXY) {
  let i, j;
  if (updateXY) {
    // Invert transformation if applicable then apply scale to get voxel coordinate
    i = volume.prevSlice.xPixelToVoxelScale(
      zoomTransform ? invertTransform(coords[0], zoomTransform, "x") : coords[0]
    );
    j = volume.prevSlice.yPixelToVoxelScale(
      zoomTransform ? invertTransform(coords[1], zoomTransform, "y") : coords[1]
    );
  } else {
    // Use previous axes coordinates
    [i, j] = [
      parseInt(d3.select("#voxel-" + axis[0] + "-value").node().value),
      parseInt(d3.select("#voxel-" + axis[1] + "-value").node().value),
    ];
  }

  let k = parseInt(sliceNum);

  let [xVal, yVal, zVal] =
    axis === "xy" ? [i, j, k] : axis === "yz" ? [k, i, j] : [i, k, j];
  return [xVal, yVal, zVal];
}

function updateWorldLabels(coords) {
  let format = d3.format(".3f");
  d3.select("#world-x-value").node().value = format(coords[0]);
  d3.select("#world-y-value").node().value = format(coords[1]);
  d3.select("#world-z-value").node().value = format(coords[2]);
}

function updateVoxelLabels(coords) {
  d3.select("#voxel-x-value").node().value = coords[0];
  d3.select("#voxel-y-value").node().value = coords[1];
  d3.select("#voxel-z-value").node().value = coords[2];
}

function invertTransform(val, transform, dir) {
  return (val - transform[dir]) / transform.k;
}

function updateVoxelCoords(coords, axis, sliceNum, updateXY = false) {
  if (!densityVol.isEmpty() || !doseVol.isEmpty()) {
    let vol = !densityVol.isEmpty() ? densityVol : doseVol;

    // Get world and voxel coordinates from pixel value
    let worldCoords = coordsToWorld(coords, axis, sliceNum, vol, updateXY);
    let voxelCoords = coordsToVoxel(coords, axis, sliceNum, vol, updateXY);

    // Update voxel info if checkbox is checked
    if (d3.select("input[name='show-marker-checkbox']").node().checked) {
      updateWorldLabels(worldCoords);
      updateVoxelLabels(voxelCoords);
      updateVoxelInfo(voxelCoords);
    }

    // Update dose profiles if checkbox is checked
    if (
      !doseVol.isEmpty() &&
      d3.select("input[name='show-dose-profile-checkbox']").node().checked
    ) {
      updateDoseProfiles(axis, voxelCoords, worldCoords);
    }
  }
}

function updateVoxelInfo(voxelCoords) {
  if (!densityVol.isEmpty()) {
    let density = densityVol.getDataAtVoxelCoords(voxelCoords);
    d3.select("#density-value").node().value =
      d3.format(".3f")(density) + " g/cm^3";

    let material = densityVol.getMaterialAtVoxelCoords(voxelCoords);
    d3.select("#material-value").node().value = material;
  }

  if (!doseVol.isEmpty()) {
    let dose = doseVol.getDataAtVoxelCoords(voxelCoords) || 0;
    let error = doseVol.getErrorAtVoxelCoords(voxelCoords) || 0;
    d3.select("#dose-value").node().value =
      d3.format(".1%")(dose) + " +/- " + d3.format(".1%")(error);
  }
}

function updateDoseProfiles(axis, voxelCoords, worldCoords) {
  var getCoords = (coords) =>
    axis === "xy"
      ? [
          [coords[1], coords[2]],
          [coords[0], coords[2]],
        ]
      : axis === "yz"
      ? [
          [coords[0], coords[2]],
          [coords[0], coords[1]],
        ]
      : [
          [coords[1], coords[2]],
          [coords[0], coords[1]],
        ];

  let [voxelCoordsX, voxelCoordsY] = getCoords(voxelCoords);
  let [worldCoordsX, worldCoordsY] = getCoords(worldCoords);

  doseProfileX.setDoseProfileData(axis[0], voxelCoordsX);
  doseProfileY.setDoseProfileData(axis[1], voxelCoordsY);

  // Plot the dose profile along the x axis
  doseProfileX.plotDoseProfile(axis, axis[0], worldCoordsX);

  // Plot the dose profile along the y axis
  doseProfileY.plotDoseProfile(axis, axis[1], worldCoordsY);
}
