// TODO: Only update dose profile on slider release to speed things up
// TODO: Use curr slice for dose profile data for side plots

class DoseProfile {
  constructor(dimensions, svg) {
    this.dimensions = dimensions;
    this.svg = svg;
  }

  getDoseProfileData(profileAxis, coord1, coord2, densityChecked) {
    let [dim1, dim2, dim3] =
      profileAxis === "x"
        ? ["x", "y", "z"]
        : profileAxis === "y"
        ? ["y", "x", "z"]
        : ["z", "x", "y"];

    let totalSlices = parseInt(doseVol.data.voxelNumber[dim1]);
    let position = doseVol.data.voxelArr[dim1].slice();
    let xVoxels = parseInt(doseVol.data.voxelNumber[dim2]);
    let yVoxels = parseInt(doseVol.data.voxelNumber[dim3]);

    // Process position to get centre voxel position rather than boundaries
    position.map((val, i) => {
      return val + (position[i + 1] - val) / 2;
    });
    position.pop();

    let doseProfileData = new Array(totalSlices);

    for (let i = 0; i < totalSlices; i++) {
      let address;
      if (profileAxis === "z") {
        address = coord1 + xVoxels * (coord2 + i * yVoxels);
      } else if (profileAxis === "x") {
        address =
          i +
          parseInt(doseVol.data.voxelNumber.x) * (coord1 + coord2 * xVoxels);
      } else if (profileAxis === "y") {
        address =
          coord1 +
          xVoxels * (i + coord2 * parseInt(doseVol.data.voxelNumber.y));
      }

      if (densityChecked) {
        doseProfileData[i] = {
          position: position[i],
          value: doseVol.data.dose[address] || 0,
          err: doseVol.data.error[address] || 0,
          density: densityVol.data.density[address],
        };
      } else {
        doseProfileData[i] = {
          position: position[i],
          value: doseVol.data.dose[address] || 0,
          err: doseVol.data.error[address] || 0,
        };
      }
    }

    return doseProfileData;
  }

  plotDoseProfile(data, axis, coord1, coord2, densityChecked) {
    let profileSvg = this.svg;
    let profileDims = this.dimensions;
    // Clear all existing elements
    profileSvg.selectAll("*").remove();

    let [minPos, maxPos] = [data[0].position, data[data.length - 1].position];
    let maxDose = Math.max(...data.map((v) => v.value));

    // Create x and y scale
    let xScale = d3
      .scaleLinear()
      .domain([minPos, maxPos])
      .range([0, profileDims.width]);

    let yDoseScale = d3
      .scaleLinear()
      .domain([0, maxDose])
      .range([profileDims.height, 0]);

    // Create and append x and y axes
    let xAxis = d3.axisBottom().scale(xScale);
    let yDoseAxis = d3
      .axisLeft()
      .scale(yDoseScale)
      .tickFormat(d3.format(".2e"));

    profileSvg
      .append("g")
      .attr("class", "x-axis")
      .attr("transform", "translate(0," + profileDims.height + ")")
      .call(xAxis);

    profileSvg.append("g").attr("class", "y-dose-axis").call(yDoseAxis);

    let plotArea = profileSvg
      .append("g")
      .attr("width", profileDims.fullWidth)
      .attr("height", profileDims.fullHeight)
      .attr("fill", "none");

    // Create the dose error area
    let errorArea = d3
      .area()
      .x((d) => xScale(d.position))
      .y0((d) => yDoseScale(d.value * (1.0 - parseFloat(d.err))))
      .y1((d) => yDoseScale(d.value * (1.0 + parseFloat(d.err))));

    // Create the dose line
    let line = d3
      .line()
      .x((d) => xScale(d.position))
      .y((d) => yDoseScale(d.value));

    // Plot error
    plotArea
      .append("path")
      .datum(data)
      .attr("fill", "lightblue")
      .attr("d", errorArea);

    // Plot dose
    plotArea
      .append("path")
      .datum(data)
      .attr("fill", "none")
      .attr("stroke", "steelblue")
      .attr("stroke-width", 1.5)
      .attr("stroke-linejoin", "round")
      .attr("stroke-linecap", "round")
      .attr("d", line);

    // Label for position x axis
    profileSvg
      .append("text")
      .attr(
        "transform",
        "translate(" +
          profileDims.width / 2 +
          " ," +
          (profileDims.height + profileDims.margin.top + 5) +
          ")"
      )
      .style("text-anchor", "middle")
      .text(axis + " Position (cm)");

    // Label for dose y axis
    profileSvg
      .append("text")
      .attr("transform", "rotate(-90)")
      .attr(
        "transform",
        "translate(" +
          (15 - profileDims.margin.left) +
          " ," +
          profileDims.height / 2 +
          ") rotate(-90)"
      )
      .style("text-anchor", "middle")
      .text("Dose (Gy)");

    if (densityChecked) {
      let maxDensity = Math.max(...data.map((v) => v.density));

      let yDensityScale = d3
        .scaleLinear()
        .domain([0, maxDensity])
        .range([profileDims.height, 0]);

      let yDensityAxis = d3
        .axisRight()
        .scale(yDensityScale)
        .tickFormat(d3.format(".2f"));

      profileSvg
        .append("g")
        .attr("class", "y-density-axis")
        .attr("transform", "translate(" + profileDims.width + ",0)")
        .call(yDensityAxis);

      // Create the density line
      let densityLine = d3
        .line()
        .x((d) => xScale(d.position))
        .y((d) => yDensityScale(d.density));

      // Plot density
      plotArea
        .append("path")
        .datum(data)
        .attr("fill", "none")
        .attr("stroke", "red")
        .attr("stroke-width", 1.5)
        .attr("stroke-linejoin", "round")
        .attr("stroke-linecap", "round")
        .attr("d", densityLine);

      // Label for density y axis
      profileSvg
        .append("text")
        .attr("transform", "rotate(-90)")
        .attr(
          "transform",
          "translate(" +
            (profileDims.width + 45) +
            " ," +
            profileDims.height / 2 +
            ") rotate(90)"
        )
        .style("text-anchor", "middle")
        .text("Density (g/cm^2)");
    }

    let [dim1, dim2] =
      axis === "x" ? ["y", "z"] : axis === "y" ? ["x", "z"] : ["x", "y"];

    profileSvg
      .append("text")
      .attr("x", profileDims.width / 2)
      .attr("y", 0 - profileDims.margin.top / 2)
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
}
