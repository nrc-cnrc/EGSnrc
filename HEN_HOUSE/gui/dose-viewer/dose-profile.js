const doseProfileButtons = d3
  .selectAll("input[name='profile-axis']")
  .on("change", function () {
    profileAxis = this.value;

    if (Object.keys(densityVol.data).length > 0) {
      updateCoordInputsLabels(profileAxis, densityVol.data.voxelNumber);
    } else if (Object.keys(doseVol.data).length > 0) {
      updateCoordInputsLabels(profileAxis, doseVol.data.voxelNumber);
    }

    return true;
  });

// Set the dimensions and margins of the graph
let doseProfileFullWidth = 460;
let doseProfileFullHeight = 400;
var doseProfileMargin = { top: 30, right: 30, bottom: 30, left: 60 },
  doseProfileWidth =
    doseProfileFullWidth - doseProfileMargin.left - doseProfileMargin.right,
  doseProfileHeight =
    doseProfileFullHeight - doseProfileMargin.top - doseProfileMargin.bottom;

// var doseProfileViewer = d3.select("#dose-profile-viewer");
var doseProfileSvg = d3
  .select("#dose-profile-svg")
  .attr("width", doseProfileFullWidth)
  .attr("height", doseProfileFullHeight)
  .append("g")
  .attr(
    "transform",
    "translate(" + doseProfileMargin.left + "," + doseProfileMargin.top + ")"
  );

function plotDoseProfile(data, axis, coord1, coord2) {
  //TODO: Add title, labels
  //TODO: Clear old plot when button is repressed
  // Clear all existing elements
  doseProfileSvg.selectAll("*").remove();

  let [minPos, maxPos] = [data[0].position, data[data.length - 1].position];
  let maxDose = Math.max(...data.map((v) => v.value));

  // Create x and y scale
  let xScale = d3
    .scaleLinear()
    .domain([minPos, maxPos])
    .range([0, doseProfileWidth]);

  let yScale = d3
    .scaleLinear()
    .domain([0, maxDose])
    .range([doseProfileHeight, 0]);

  // Create and append x and y axes
  let xAxis = d3.axisBottom().scale(xScale);
  let yAxis = d3.axisLeft().scale(yScale).tickFormat(d3.format(".2e"));

  doseProfileSvg
    .append("g")
    .attr("class", "x-axis")
    .attr("transform", "translate(0," + doseProfileHeight + ")")
    .call(xAxis);

  doseProfileSvg.append("g").attr("class", "y-axis").call(yAxis);

  // Plot the data
  let line = d3
    .line()
    .x((d) => xScale(d.position))
    .y((d) => yScale(d.value));

  doseProfileSvg
    .append("g")
    .attr("width", doseProfileFullWidth)
    .attr("height", doseProfileFullHeight)
    .attr("fill", "none")
    .append("path")
    .datum(data)
    .attr("fill", "none")
    .attr("stroke", "steelblue")
    .attr("stroke-width", 1.5)
    .attr("stroke-linejoin", "round")
    .attr("stroke-linecap", "round")
    .attr("d", line);

  let [dim1, dim2] =
    axis === "x" ? ["y", "z"] : axis === "y" ? ["x", "z"] : ["x", "y"];

  doseProfileSvg
    .append("text")
    .attr("x", doseProfileWidth / 2)
    .attr("y", 0 - doseProfileMargin.top / 2)
    .attr("text-anchor", "middle")
    .style("font-size", "16px")
    .style("text-decoration", "underline")
    .text(
      axis +
        " Axis Dose Profile at (" +
        dim1 +
        ", " +
        dim2 +
        "): (" +
        coord1 +
        ", " +
        coord2 +
        ")"
    );
}

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
  let coordInputs = d3.selectAll("input[name='coord-input']").nodes();

  // Enable inputs if disabled
  coordInputs.forEach((coordInput) => {
    if (coordInput.disabled) coordInput.disabled = false;
  });

  // Get selected profile axis and check dimensions of uploaded file
  let profileAxis = d3.selectAll("input[name='profile-axis']:checked").node()
    .value;

  updateCoordInputsLabels(profileAxis, voxelNumber);
}

d3.select("#dose-profile-button").on("click", function () {
  // Get 1D plot data
  let profileAxis = d3.selectAll("input[name='profile-axis']:checked").node()
    .value;
  let coord1 = parseInt(d3.select("#coord-1").node().value);
  let coord2 = parseInt(d3.select("#coord-2").node().value);

  // TODO: Add checkboxes to optionally plot density if given
  let data = doseVol.data;

  let totalSlices, xVoxels, yVoxels;

  if (profileAxis === "x") {
    totalSlices = parseInt(data.voxelNumber.x);
    position = data.voxelArr.x.slice();
    xVoxels = parseInt(data.voxelNumber.y);
    yVoxels = parseInt(data.voxelNumber.z);
  } else if (profileAxis === "y") {
    totalSlices = parseInt(data.voxelNumber.y);
    position = data.voxelArr.y.slice();
    xVoxels = parseInt(data.voxelNumber.x);
    yVoxels = parseInt(data.voxelNumber.z);
  } else if (profileAxis === "z") {
    totalSlices = parseInt(data.voxelNumber.z);
    position = data.voxelArr.z.slice();
    xVoxels = parseInt(data.voxelNumber.x);
    yVoxels = parseInt(data.voxelNumber.y);
  }

  // Process position to get centre voxel position rather than boundaries
  position.map((val, i) => {
    return val + (position[i + 1] - val) / 2;
  });
  position.pop();

  let profileDose = new Array(totalSlices);

  for (let i = 0; i < totalSlices; i++) {
    let address;
    if (profileAxis === "z") {
      address = coord1 + xVoxels * (coord2 + i * yVoxels);
    } else if (profileAxis === "x") {
      address = i + parseInt(data.voxelNumber.x) * (coord1 + coord2 * xVoxels);
    } else if (profileAxis === "y") {
      address = coord1 + xVoxels * (i + coord2 * parseInt(data.voxelNumber.y));
    }
    profileDose[i] = data.dose[address] || 0;
  }

  var doseProfileData = profileDose.map((dose, i) => ({
    position: position[i],
    value: dose,
  }));

  plotDoseProfile(doseProfileData, profileAxis, coord1, coord2);
});
