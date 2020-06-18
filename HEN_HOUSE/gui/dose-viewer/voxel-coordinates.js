function coordsToWorld(coords, axis, sliceNum, volume) {
  // Invert transformation if applicable then invert scale to get world coordinate
  let i = volume.prevSlice.xScale.invert(
    zoomTransform ? invertTransform(coords[0], zoomTransform, "x") : coords[0]
  );
  let j = volume.prevSlice.yScale.invert(
    zoomTransform ? invertTransform(coords[1], zoomTransform, "y") : coords[1]
  );

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

  // Add new marker with modified coordinates so it can smoothly transform with other elements
  svg
    .append("g")
    .attr("class", "marker")
    .attr("transform", zoomTransform ? zoomTransform.toString() : "")
    .append("circle")
    .attr(
      "cx",
      zoomTransform ? invertTransform(coords[0], zoomTransform, "x") : coords[0]
    )
    .attr(
      "cy",
      zoomTransform ? invertTransform(coords[1], zoomTransform, "y") : coords[1]
    )
    .attr("r", 2)
    .style("stroke", "red")
    .style("stroke-width", "1.5")
    .style("fill", "none");
}

function updateVoxelCoords(coords, axis, sliceNum, svg) {
  if (!densityVol.isEmpty() || !doseVol.isEmpty()) {
    let vol = !densityVol.isEmpty() ? densityVol : doseVol;
    worldCoords = coordsToWorld(coords, axis, sliceNum, vol);
    voxelCoords = worldCoordsToVoxel(worldCoords, vol);
    updateWorldLabels(worldCoords);
    updateVoxelLabels(voxelCoords);

    if (!densityVol.isEmpty()) {
      let density = densityVol.getDataAtVoxelCoords(voxelCoords);
      d3.select("#density-value").node().value = d3.format(".3f")(density);
    }

    if (!doseVol.isEmpty()) {
      let dose = doseVol.getDataAtVoxelCoords(voxelCoords) || 0;
      d3.select("#dose-value").node().value = d3.format(".3e")(dose);
    }
  }
}

// TODO: Update voxel info upon dose or density upload for existing marker
svgMarker.on("click", function () {
  plotCoords = d3.mouse(this);
  let sliceNum = d3.select("#slider-value").node().value;
  updateVoxelCoords(plotCoords, axis, sliceNum, svgDose);
  updateMarker(plotCoords, svgMarker);
  return true;
});
