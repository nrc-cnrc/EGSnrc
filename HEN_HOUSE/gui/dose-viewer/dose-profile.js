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
    this.zoomObj = null;
    this.prevAxis = null;
    this.data = null;
    this.dim = null;
  }

  set zoomTransform(val) {
    this.transform = val;
  }

  get zoomTransform() {
    return this.transform;
  }

  set plotDensity(val) {
    this.densityChecked = val;
    if (this.data !== null) this.updateAxes();
  }

  get plotDensity() {
    return this.densityChecked;
  }

  resetZoomTransform() {
    this.svg
      .select("rect.bounding-box")
      .call(this.zoomObj.transform, d3.zoomIdentity.scale(1));
  }

  setDoseProfileData(profileDim, coords) {
    let [dim1, dim2, dim3] =
      profileDim === "x"
        ? ["x", "y", "z"]
        : profileDim === "y"
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
      if (profileDim === "z") {
        address = coords[0] + xVoxels * (coords[1] + i * yVoxels);
      } else if (profileDim === "x") {
        address =
          i +
          parseInt(doseVol.data.voxelNumber.x) *
            (coords[0] + coords[1] * xVoxels);
      } else if (profileDim === "y") {
        address =
          coords[0] +
          xVoxels * (i + coords[1] * parseInt(doseVol.data.voxelNumber.y));
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
    this.data = doseProfileData;
  }

  // TODO: Don't update on slider change, only on crosshair position or axes change
  setDoseScales() {
    let [minPos, maxPos] = [
      this.data[0].position,
      this.data[this.data.length - 1].position,
    ];

    // Create x and y scale
    this.xScale = d3
      .scaleLinear()
      .domain(minPos < maxPos ? [minPos, maxPos] : [maxPos, minPos])
      .range(
        minPos < maxPos
          ? [0, this.dimensions.width]
          : [this.dimensions.width, 0]
      );

    this.yDoseScale = d3
      .scaleLinear()
      .domain([0, 1.0])
      .range([this.dimensions.height, 0]);

    if (this.densityChecked) {
      let maxDensity = Math.max(...this.data.map((v) => v.density));

      this.yDensityScale = d3
        .scaleLinear()
        .domain([0, maxDensity])
        .range([this.dimensions.height, 0]);
    }
  }

  // TODO: Don't update on slider change, only on crosshair position or axes change
  plotAxes() {
    // Clear existing axes and labels
    this.svg.selectAll(".profile-x-axis").remove();
    this.svg.selectAll(".profile-y-dose-axis").remove();

    // Create and append x and dose y axes
    let xAxis = d3
      .axisBottom()
      .scale(this.xScale)
      .tickSize(-this.dimensions.height);

    let yDoseAxis = d3
      .axisLeft()
      .scale(this.yDoseScale)
      .ticks(10)
      .tickFormat(d3.format(".0%"))
      .tickSize(-this.dimensions.width);

    this.svg
      .append("g")
      .attr("class", "profile-x-axis")
      .attr("transform", "translate(0," + this.dimensions.height + ")")
      .call(xAxis);

    this.svg.append("g").attr("class", "profile-y-dose-axis").call(yDoseAxis);

    // Label for position x axis
    this.svg
      .append("text")
      .attr("class", "profile-x-axis")
      .attr(
        "transform",
        "translate(" +
          this.dimensions.width / 2 +
          " ," +
          (this.dimensions.height + this.dimensions.margin.top + 5) +
          ")"
      )
      .style("text-anchor", "middle")
      .text(this.dim + " Position (cm)");

    // Label for dose y axis
    this.svg
      .append("text")
      .attr("class", "profile-y-dose-axis")
      .attr("transform", "rotate(-90)")
      .attr(
        "transform",
        "translate(" +
          -(this.dimensions.margin.left / 2) +
          " ," +
          this.dimensions.height / 2 +
          ") rotate(-90)"
      )
      .style("text-anchor", "middle")
      .text("Dose");

    if (this.densityChecked) {
      // Clear existing axis and label
      this.svg.selectAll(".profile-y-density-axis").remove();

      // Create and append density y axes
      let yDensityAxis = d3
        .axisRight()
        .scale(this.yDensityScale)
        .tickFormat(d3.format(".2f"))
        .tickSize(-this.dimensions.width);

      this.svg
        .append("g")
        .attr("class", "profile-y-density-axis")
        .attr("transform", "translate(" + this.dimensions.width + ",0)")
        .call(yDensityAxis);

      // Label for density y axis
      this.svg
        .append("text")
        .attr("class", "profile-y-density-axis")
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

  makeTitle(axis, coords) {
    // Clear existing title
    this.svg.select(".title").remove();

    let [dim1, dim2] =
      axis === "x" ? ["y", "z"] : axis === "y" ? ["x", "z"] : ["x", "y"];

    let format = d3.format(".2f");

    this.svg
      .append("text")
      .attr("class", "title")
      .attr("x", this.dimensions.width / 2)
      .attr("y", 0 - this.dimensions.margin.top / 2)
      .attr("text-anchor", "middle")
      .style("font-size", "14px")
      .style("text-decoration", "underline")
      .text(
        axis +
          " Axis Dose Profile at (" +
          dim1 +
          ", " +
          dim2 +
          "): (" +
          format(coords[0]) +
          " cm, " +
          format(coords[1]) +
          " cm)"
      );
  }

  plotData() {
    let data = this.data;
    let preYDoseScale = d3
      .scaleLinear()
      .domain([0, doseVol.maxDoseVar * 1.1])
      .range([0, 1.1]);

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
      .y0((d) =>
        this.yDoseScale(preYDoseScale(d.value * (1.0 - parseFloat(d.err))))
      )
      .y1((d) =>
        this.yDoseScale(preYDoseScale(d.value * (1.0 + parseFloat(d.err))))
      );

    // Create the dose line
    let line = d3
      .line()
      .x((d) => this.xScale(d.position))
      .y((d) => this.yDoseScale(preYDoseScale(d.value)));

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
      .attr("stroke-width", 1.5)
      .attr("stroke-linejoin", "round")
      .attr("stroke-linecap", "round")
      .attr("class", "lines")
      .attr("d", line);

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

    if (this.transform) {
      this.svg
        .selectAll("path.lines")
        .attr("transform", this.transform.toString());
    }
  }

  // TODO: Instead of leaving logic inside of dose profile object, just updateAxes before plotDoseProfile if need be
  updateAxes() {
    this.setDoseScales();
    this.plotAxes();
    if (this.zoomObj !== null) this.resetZoomTransform();
  }

  plotDoseProfile(axis, dim, coords) {
    this.dim = dim;
    let axisChange = axis !== this.prevAxis ? true : false;

    if (this.xScale === null || axisChange) {
      this.updateAxes();
    }

    this.makeTitle(dim, coords);
    this.plotData();
    this.prevAxis = axis;
  }
}
