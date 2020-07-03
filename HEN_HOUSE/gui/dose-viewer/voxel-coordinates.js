// TODO: If marker exists, on axis change, use marker coordinates?
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

function worldCoordsToVoxel(worldCoords, volume) {
  // Map pixel to number of voxels
  let xVal = volume.xWorldToVoxelScale(worldCoords[0]);
  let yVal = volume.yWorldToVoxelScale(worldCoords[1]);
  let zVal = volume.zWorldToVoxelScale(worldCoords[2]);

  return [xVal, yVal, zVal];
}

// TODO: Ask if should I round to nearest world coord value read in vs interpolating
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

function updateMarker(coords, svg) {
  // Remove old marker
  svg.select(".marker").remove();

  // If there is existing transformation, calculate proper x and y coordinates
  let x = zoomTransform
    ? invertTransform(coords[0], zoomTransform, "x")
    : coords[0];
  let y = zoomTransform
    ? invertTransform(coords[1], zoomTransform, "y")
    : coords[1];

  // Add new marker with modified coordinates so it can smoothly transform with other elements
  let marker = svg
    .append("g")
    .attr("class", "marker")
    .attr("transform", zoomTransform ? zoomTransform.toString() : "");

  // Create centre circle
  marker
    .append("circle")
    .attr("cx", x)
    .attr("cy", y)
    .attr("id", "crosshairCentre")
    .attr("class", "crosshair")
    .attr("r", 2);

  // Create horizontal line
  marker
    .append("line")
    .attr("id", "crosshairX")
    .attr("class", "crosshair")
    .attr("x1", x)
    .attr("y1", 0)
    .attr("x2", x)
    .attr("y2", mainViewerDimensions.height);

  // Create vertical line
  marker
    .append("line")
    .attr("id", "crosshairY")
    .attr("class", "crosshair")
    .attr("x1", 0)
    .attr("y1", y)
    .attr("x2", mainViewerDimensions.width)
    .attr("y2", y);
}

function updateVoxelCoords(coords, axis, sliceNum, updateXY = false) {
  if (!densityVol.isEmpty() || !doseVol.isEmpty()) {
    let vol = !densityVol.isEmpty() ? densityVol : doseVol;
    worldCoords = coordsToWorld(coords, axis, sliceNum, vol, updateXY);
    voxelCoords = worldCoordsToVoxel(worldCoords, vol);
    updateWorldLabels(worldCoords);
    updateVoxelLabels(voxelCoords);
    updateVoxelInfo(voxelCoords);

    if (updateXY) {
      updateMarker(coords, svgMarker);
    }

    if (!doseVol.isEmpty()) {
      updateDoseProfiles(voxelCoords);
    }
  }
}

function updateVoxelInfo(voxelCoords) {
  if (!densityVol.isEmpty()) {
    let density = densityVol.getDataAtVoxelCoords(voxelCoords);
    d3.select("#density-value").node().value =
      d3.format(".3f")(density) + " g/cm^3";
  }

  if (!doseVol.isEmpty()) {
    let dose = doseVol.getDataAtVoxelCoords(voxelCoords) || 0;
    let error = doseVol.getErrorAtVoxelCoords(voxelCoords) || 0;
    d3.select("#dose-value").node().value =
      "(" +
      d3.format(".3e")(dose) +
      " +/- " +
      d3.format(".3e")(error * dose) +
      ") Gy";
  }
}

function updateDoseProfiles(voxelCoords) {
  if (d3.select("input[name='show-dose-profile-checkbox']").node().checked) {
    let axis = getAxis();

    let [coord1X, coord2X, coord1Y, coord2Y] =
      axis === "xy"
        ? [voxelCoords[1], voxelCoords[2], voxelCoords[0], voxelCoords[2]]
        : axis === "yz"
        ? [voxelCoords[0], voxelCoords[2], voxelCoords[0], voxelCoords[1]]
        : [voxelCoords[1], voxelCoords[2], voxelCoords[0], voxelCoords[1]];

    let doseProfileXData = doseProfileX.getDoseProfileData(
      axis[0],
      coord1X,
      coord2X
    );
    let doseProfileYData = doseProfileY.getDoseProfileData(
      axis[1],
      coord1Y,
      coord2Y
    );

    // Plot the dose profile along the x axis
    doseProfileX.plotDoseProfile(
      doseProfileXData,
      axis,
      axis[0],
      coord1X,
      coord2X
    );

    // Plot the dose profile along the y axis
    doseProfileY.plotDoseProfile(
      doseProfileYData,
      axis,
      axis[1],
      coord1Y,
      coord2Y
    );
  }
}

// TODO: Update voxel info upon dose or density upload for existing marker
svgMarker.on("click", function () {
  plotCoords = d3.mouse(this);
  let axis = getAxis();
  let sliceNum = getSliceNum();

  updateVoxelCoords(plotCoords, axis, sliceNum, true);
  return true;
});
