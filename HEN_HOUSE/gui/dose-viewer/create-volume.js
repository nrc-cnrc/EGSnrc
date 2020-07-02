// TODO: Make a plotting object that takes list of volumes, plots them
// Each plotting object has radio buttons, slider, axes, canvas, and svg
// TODO: Make a dataName variable to reduce code

var drawAxes = (svgAxis, slice) => {
  svgAxis.selectAll("g").remove();

  // If there is existing transformation, apply it
  let xScale = zoomTransform
    ? zoomTransform.rescaleX(slice.xScale)
    : slice.xScale;
  let yScale = zoomTransform
    ? zoomTransform.rescaleY(slice.yScale)
    : slice.yScale;

  var xAxis = d3.axisBottom().scale(xScale).tickSize(-slice.dimensions.height);
  var yAxis = d3.axisLeft().scale(yScale).tickSize(-slice.dimensions.width);

  svgAxis
    .append("g")
    .attr("class", "x-axis")
    .attr("transform", "translate(0," + slice.dimensions.height + ")")
    .call(xAxis);
  svgAxis.append("g").attr("class", "y-axis").call(yAxis);
};

class Volume {
  // General volume structure
  // https://github.com/aces/brainbrowser/blob/fe0ce114c6cd8e317a6bdd9b7ef97cbf1c38309d/src/brainbrowser/volume-viewer/volume-loaders/minc.js#L88-L190

  constructor(dimensions, legendDimensions) {
    this.dimensions = dimensions;
    this.legendDimensions = legendDimensions;
    this.data = {};
    this.prevSlice = {};
    this.prevAxis = "";
  }

  addData(data) {
    this.data = data;
    this.xWorldToVoxelScale = d3
      .scaleQuantile()
      .domain([data.voxelArr.x[0], data.voxelArr.x[data.voxelArr.x.length - 1]])
      .range(d3.range(0, data.voxelNumber.x, 1));
    this.yWorldToVoxelScale = d3
      .scaleQuantile()
      .domain([data.voxelArr.y[0], data.voxelArr.y[data.voxelArr.y.length - 1]])
      .range(d3.range(0, data.voxelNumber.y, 1));
    this.zWorldToVoxelScale = d3
      .scaleQuantile()
      .domain([data.voxelArr.z[0], data.voxelArr.z[data.voxelArr.z.length - 1]])
      .range(d3.range(0, data.voxelNumber.z, 1));
  }

  addColourScheme(colourScheme, maxVal, invertScheme) {
    let domain = invertScheme ? [maxVal, 0] : [0, maxVal];
    this.colour = d3.scaleSequentialSqrt(colourScheme).domain(domain);
  }

  // dataName : density or dose
  getSlice(axis, sliceNum, dataName) {
    // TODO: Cache previous slices
    // TODO: Only redefine slice attributes on axis change
    // For slice structure
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1502-L1546

    let [dim1, dim2, dim3] =
      axis === "xy"
        ? ["x", "y", "z"]
        : axis === "yz"
        ? ["y", "z", "x"]
        : ["x", "z", "y"];

    let x = this.data.voxelArr[dim1];
    let y = this.data.voxelArr[dim2];
    let z = this.data.voxelArr[dim3];
    let totalSlices = this.data.voxelNumber[dim3];

    var getLengthCm = (voxelArrDim) =>
      Math.abs(voxelArrDim[voxelArrDim.length - 1] - voxelArrDim[0]);
    let [xLengthCm, yLengthCm] = [getLengthCm(x), getLengthCm(y)];
    let xDomain, yDomain, contourXScale, contourYScale;
    if (xLengthCm > yLengthCm) {
      xDomain = [x[0], x[x.length - 1]];
      yDomain = [y[0], y[0] + xLengthCm];
      contourXScale = this.dimensions.width / this.data.voxelNumber[dim1];
      contourYScale =
        (this.dimensions.height * (yLengthCm / xLengthCm)) /
        this.data.voxelNumber[dim2];
    } else {
      xDomain = [x[0], x[0] + yLengthCm];
      yDomain = [y[0], y[y.length - 1]];
      contourXScale =
        (this.dimensions.width * (xLengthCm / yLengthCm)) /
        this.data.voxelNumber[dim1];
      contourYScale = this.dimensions.height / this.data.voxelNumber[dim2];
    }
    // TODO: Change scales to quantile to map exactly which pixels
    let slice = {
      dx: this.data.voxelSize[dim1],
      dy: this.data.voxelSize[dim2],
      xVoxels: this.data.voxelNumber[dim1],
      yVoxels: this.data.voxelNumber[dim2],
      x: x,
      y: y,
      totalSlices: totalSlices,
      xScale: d3
        .scaleLinear()
        .domain(xDomain)
        .range([0, this.dimensions.width]),
      yScale: d3
        .scaleLinear()
        .domain(yDomain)
        .range([0, this.dimensions.height]),
      zScale: d3
        .scaleLinear()
        .domain([z[0], z[z.length - 1]])
        .range([0, totalSlices]), // unit: pixels
      contourXScale: contourXScale,
      contourYScale: contourYScale,
      dimensions: this.dimensions,
    };

    // If current slice number is larger than the total number of slices
    // set slice number to last slice
    sliceNum =
      sliceNum >= slice.totalSlices
        ? parseInt(slice.totalSlices - 1)
        : parseInt(sliceNum);

    // For address calculations:
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1999-L2034
    let sliceData = new Array(slice.xVoxels * slice.yVoxels);

    for (let i = 0; i < slice.xVoxels; i++) {
      for (let j = 0; j < slice.yVoxels; j++) {
        let address;
        if (axis === "xy") {
          address = i + slice.xVoxels * (j + sliceNum * slice.yVoxels);
        } else if (axis === "yz") {
          address =
            sliceNum + this.data.voxelNumber.x * (i + j * slice.xVoxels);
        } else if (axis === "xz") {
          address =
            i + slice.xVoxels * (sliceNum + j * this.data.voxelNumber.y);
        }
        let new_address = i + slice.xVoxels * j;
        sliceData[new_address] = this.data[dataName][address];
      }
    }

    slice = {
      ...slice,
      axis: axis,
      sliceData: sliceData,
      sliceNum: sliceNum,
    };

    this.prevSlice = slice;
    return slice;
  }

  scaleContour(contours, xScale, yScale) {
    return contours.map(({ type, value, coordinates }) => ({
      type,
      value,
      coordinates: coordinates.map((rings) =>
        rings.map((points) => points.map(([i, j]) => [i * xScale, j * yScale]))
      ),
    }));
  }

  initializeLegend(legendSvg, legendClass, title, parameters) {
    // Clear and redraw current legend
    legendSvg.select("." + legendClass).remove();

    // Make space for legend title
    legendSvg
      .append("g")
      .attr("class", legendClass)
      .style("transform", "translate(0px," + 20 + "px)");

    // Append title
    legendSvg
      .append("text")
      .attr("x", this.legendDimensions.width / 2)
      .attr("y", this.legendDimensions.margin.top / 2)
      .attr("text-anchor", "middle")
      .style("font-size", "14px")
      .text(title);

    // Create legend
    var legend = d3
      .legendColor()
      .shapeWidth(10)
      .ascending(true)
      .orient("vertical")
      .scale(this.colour);

    // Apply all the parameters
    Object.entries(parameters).forEach(([name, val]) => {
      legend[name](val);
    });

    legendSvg.select("." + legendClass).call(legend);

    // Set the height of the svg so the div can scroll if need be
    let height =
      legendSvg
        .select("." + legendClass)
        .node()
        .getBoundingClientRect().height + 20;
    legendSvg.attr("height", height);
  }

  isEmpty() {
    return Object.keys(this.data).length === 0;
  }

  getDataAtVoxelCoords(voxelCoords, dataName) {
    let [x, y, z] = voxelCoords;
    let address =
      z * (this.data.voxelNumber.x * this.data.voxelNumber.y) +
      y * this.data.voxelNumber.x +
      x;

    return this.data[dataName][address];
  }
}

class DoseVolume extends Volume {
  constructor(dimensions, legendDimensions) {
    super(dimensions, legendDimensions); // call the super class constructor
  }

  addData(data) {
    // TODO: Want user to be able to choose their own space between contours
    super.addData(data);
    super.addColourScheme(d3.interpolateViridis, this.data.maxDose);
    // Calculate the contour thresholds
    this.contourInt = parseFloat(
      d3.select("#contour-line-select").node().value
    );
    this.updateThresholds();
  }

  updateContourInterval(val) {
    this.contourInt = val;
    this.updateThresholds();
    this.initializeLegend();
    this.drawDose(this.prevSlice, svgDose);
  }

  updateThresholds() {
    this.thresholds = d3
      .range(0, 1.0 + this.contourInt, this.contourInt)
      .map((i) => i * this.data.maxDose);
  }

  getSlice(axis, sliceNum) {
    return super.getSlice(axis, sliceNum, "dose");
  }

  drawDose(slice, svg) {
    //TODO: Don't rely on plugin for legend/colour scale
    // https://observablehq.com/@d3/color-legend

    // Clear dose plot
    svg.selectAll("g").remove();

    // Draw contours
    var contours = d3
      .contours()
      .size([slice.xVoxels, slice.yVoxels])
      .thresholds(this.thresholds)(slice.sliceData);

    let doseContour = svg
      .append("g")
      .attr("class", "dose-contour")
      .attr("width", this.dimensions.width)
      .attr("height", this.dimensions.height)
      .attr("fill", "none")
      .attr("stroke", "#fff")
      .attr("stroke-opacity", 0.5)
      .attr("stroke-width", 0.5)
      .selectAll("path")
      .data(
        super.scaleContour(contours, slice.contourXScale, slice.contourYScale)
      )
      .join("path")
      .attr("fill", (d) => this.colour(d.value))
      .attr("fill-opacity", 0.5)
      .attr("d", d3.geoPath());

    if (zoomTransform) {
      svg.select("g.dose-contour").attr("transform", zoomTransform.toString());
    }
  }

  initializeLegend() {
    super.initializeLegend(doseLegendSvg, "doseLegend", "Dose", {
      labels: this.thresholds.map((e) =>
        d3.format(".0%")(e / this.data.maxDose)
      ),
      cells: this.thresholds,
    });
  }

  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "dose");
  }

  getErrorAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "error");
  }
}

class DensityVolume extends Volume {
  constructor(dimensions, legendDimensions) {
    super(dimensions, legendDimensions); // call the super class constructor
  }

  addData(data) {
    super.addData(data);
    super.addColourScheme(d3.interpolateGreys, this.data.maxDensity, true);
    // Calculate the contour thresholds
    this.thresholds = this.getThresholds(data);
  }

  getThresholds(data) {
    let thresholds = Array.from(new Set(data.density));
    if (thresholds.length < 10) {
      return thresholds.sort();
    }
    let maxThresh = Math.ceil(data.maxDensity * 10) / 10;
    return d3.range(0, maxThresh, 0.1);
  }

  getSlice(axis, sliceNum) {
    return super.getSlice(axis, sliceNum, "density");
  }

  drawDensity(slice, svg) {
    // TODO: Make two new functions: change slicenum and change axes

    // Clear density plot
    svg.selectAll(".density-contour").remove();

    // Draw contours
    var contours = d3
      .contours()
      .size([slice.xVoxels, slice.yVoxels])
      .smooth(false)
      .thresholds(this.thresholds)(slice.sliceData);

    svg
      .append("g")
      .attr("class", "density-contour")
      .attr("width", this.dimensions.width)
      .attr("height", this.dimensions.height)
      .attr("fill", "none")
      .selectAll("path")
      .data(
        super.scaleContour(contours, slice.contourXScale, slice.contourYScale)
      )
      .join("path")
      .attr("fill", (d) => this.colour(d.value))
      .attr("fill-opacity", 1.0)
      .attr("d", d3.geoPath());

    if (zoomTransform) {
      svg
        .select("g.density-contour")
        .attr("transform", zoomTransform.toString());
    }

    this.prevAxis = axis;
  }

  initializeLegend() {
    super.initializeLegend(densityLegendSvg, "densityLegend", "Density", {
      labelFormat: d3.format(".2f"),
      cells: this.thresholds.length > 10 ? 10 : this.thresholds,
    });
  }

  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "density");
  }
}
