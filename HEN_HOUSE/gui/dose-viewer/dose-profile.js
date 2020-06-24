// TODO: Only update dose profile on slider release to speed things up
// TODO: Use curr slice for dose profile data for side plots

class DoseProfile {
  constructor(dimensions, svg) {
    this.dimensions = dimensions;
    this.svg = svg;
    this.xScale = null;
    this.yDoseScale = null;
    this.yDensityScale = null;
    this.densityChecked = false;
    this.transform = null;
    this.prevAxis = null;
    this.prevCoords = [-1, -1];
  }

  set zoomTransform(val) {
    this.transform = val;
  }

  get zoomTransform() {
    return this.transform;
  }

  set plotDensity(val) {
    this.densityChecked = val;
  }

  get plotDensity() {
    return this.densityChecked;
  }

  resetTransform() {
    this.transform = null;
  }

  getDoseProfileData(profileAxis, coord1, coord2) {
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

      if (this.densityChecked) {
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

  // TODO: Don't update on slider change, only on crosshair position or axes change
  setDoseScales(data) {
    let [minPos, maxPos] = [data[0].position, data[data.length - 1].position];
    let maxDose = Math.max(
      ...data.map((v) => v.value * (1.0 + parseFloat(v.err)))
    );

    // Create x and y scale
    this.xScale = d3
      .scaleLinear()
      .domain([minPos, maxPos])
      .range([0, this.dimensions.width]);

    this.yDoseScale = d3
      .scaleLinear()
      .domain([0, maxDose])
      .range([this.dimensions.height, 0]);

    if (this.densityChecked) {
      let maxDensity = Math.max(...data.map((v) => v.density));

      this.yDensityScale = d3
        .scaleLinear()
        .domain([0, maxDensity])
        .range([this.dimensions.height, 0]);
    }
  }

  // TODO: Don't update on slider change, only on crosshair position or axes change
  plotAxes(axis) {
    // Clear existing axes and labels
    this.svg.selectAll(".x-axis").remove();
    this.svg.selectAll(".y-dose-axis").remove();

    // Create and append x and dose y axes
    let xAxis = d3.axisBottom().scale(this.xScale);
    let yDoseAxis = d3
      .axisLeft()
      .scale(this.yDoseScale)
      .tickFormat(d3.format(".2e"));

    this.svg
      .append("g")
      .attr("class", "x-axis")
      .attr("transform", "translate(0," + this.dimensions.height + ")")
      .call(xAxis);

    this.svg.append("g").attr("class", "y-dose-axis").call(yDoseAxis);

    // Label for position x axis
    this.svg
      .append("text")
      .attr("class", "x-axis")
      .attr(
        "transform",
        "translate(" +
          this.dimensions.width / 2 +
          " ," +
          (this.dimensions.height + this.dimensions.margin.top + 5) +
          ")"
      )
      .style("text-anchor", "middle")
      .text(axis + " Position (cm)");

    // Label for dose y axis
    this.svg
      .append("text")
      .attr("class", "y-dose-axis")
      .attr("transform", "rotate(-90)")
      .attr(
        "transform",
        "translate(" +
          (15 - this.dimensions.margin.left) +
          " ," +
          this.dimensions.height / 2 +
          ") rotate(-90)"
      )
      .style("text-anchor", "middle")
      .text("Dose (Gy)");

    if (this.densityChecked) {
      // Clear existing axis and label
      this.svg.selectAll(".y-density-axis").remove();

      // Create and append density y axes
      let yDensityAxis = d3
        .axisRight()
        .scale(this.yDensityScale)
        .tickFormat(d3.format(".2f"));

      this.svg
        .append("g")
        .attr("class", "y-density-axis")
        .attr("transform", "translate(" + this.dimensions.width + ",0)")
        .call(yDensityAxis);

      // Label for density y axis
      this.svg
        .append("text")
        .attr("class", "y-density-axis")
        .attr("transform", "rotate(-90)")
        .attr(
          "transform",
          "translate(" +
            (this.dimensions.width + 45) +
            " ," +
            this.dimensions.height / 2 +
            ") rotate(90)"
        )
        .style("text-anchor", "middle")
        .text("Density (g/cm^2)");
    }
  }

  makeTitle(axis, coord1, coord2) {
    // Clear existing title
    this.svg.select(".title").remove();

    let [dim1, dim2] =
      axis === "x" ? ["y", "z"] : axis === "y" ? ["x", "z"] : ["x", "y"];

    this.svg
      .append("text")
      .attr("class", "title")
      .attr("x", this.dimensions.width / 2)
      .attr("y", 0 - this.dimensions.margin.top / 2)
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

  plotData(data) {
    // Clear all existing elements
    this.svg.selectAll(".plotting-area").remove();

    let plotArea = this.svg
      .append("g")
      .attr("class", "plotting-area")
      .attr(
        "clip-path",
        this.svg.select("clipPath").node()
          ? "url(#" + this.svg.select("clipPath").node().id + ")"
          : ""
      )
      .attr("fill", "none")
      .attr("width", this.dimensions.width)
      .attr("height", this.dimensions.height);

    // Create the dose error area
    let errorArea = d3
      .area()
      .x((d) => this.xScale(d.position))
      .y0((d) => this.yDoseScale(d.value * (1.0 - parseFloat(d.err))))
      .y1((d) => this.yDoseScale(d.value * (1.0 + parseFloat(d.err))));

    // Create the dose line
    let line = d3
      .line()
      .x((d) => this.xScale(d.position))
      .y((d) => this.yDoseScale(d.value));

    // Plot error
    plotArea
      .append("path")
      .datum(data)
      .attr("fill", "lightblue")
      .attr("class", "lines")
      .attr("d", errorArea);

    // Plot dose
    plotArea
      .append("path")
      .datum(data)
      .attr("fill", "none")
      .attr("stroke", "steelblue")
      .attr("stroke-width", 1.0)
      .attr("stroke-linejoin", "round")
      .attr("stroke-linecap", "round")
      .attr("class", "lines")
      .attr("d", line);

    if (this.transform) {
      this.svg
        .selectAll("path.lines")
        .attr("transform", this.transform.toString());
    }

    if (this.densityChecked) {
      // Create the density line
      let densityLine = d3
        .line()
        .x((d) => this.xScale(d.position))
        .y((d) => this.yDensityScale(d.density));

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
    }
  }

  async plotDoseProfile(data, axis, coord1, coord2) {
    let axisChange = axis !== this.prevAxis ? true : false;
    let coordsChange =
      coord1 !== this.prevCoords[0] || coord2 !== this.prevCoords[1]
        ? true
        : false;
    if (this.xScale === null || axisChange || coordsChange) {
      this.setDoseScales(data);
      this.plotAxes(axis);
    }
    this.makeTitle(axis, coord1, coord2);
    this.plotData(data);

    this.prevAxis = axis;
    this.prevCoords = [coord1, coord2];
  }
}
