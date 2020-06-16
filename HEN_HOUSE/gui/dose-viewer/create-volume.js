// TODO: Make a plotting object that takes list of volumes, plots them
// Each plotting object has radio buttons, slider, axes, canvas, and svg
// TODO: Make a dataName variable to reduce code
class Volume {
  // General volume structure
  // https://github.com/aces/brainbrowser/blob/fe0ce114c6cd8e317a6bdd9b7ef97cbf1c38309d/src/brainbrowser/volume-viewer/volume-loaders/minc.js#L88-L190

  constructor(height, width) {
    //TODO: Figure out a better system for height and width
    this.height = height;
    this.width = width;
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

  addColourScheme(colourScheme, maxVal) {
    this.colour = d3.scaleSequentialSqrt(colourScheme).domain([0, maxVal]);
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
        .domain([x[0], x[x.length - 1]])
        .range([0, this.width]), // unit: pixels
      yScale: d3
        .scaleLinear()
        .domain([y[0], y[y.length - 1]])
        .range([0, this.height]), // unit: pixels
      zScale: d3
        .scaleLinear()
        .domain([z[0], z[z.length - 1]])
        .range([0, totalSlices]), // unit: pixels
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

  initializeLegend(legendSvg, legendClass, format, cells, title) {
    // Note: Cells can either be a list of the value for each label or the total number of labels

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
      .attr("x", legendWidth / 2)
      .attr("y", legendMargin.top / 2)
      .attr("text-anchor", "middle")
      .style("font-size", "14px")
      .text(title);

    // Create legend
    var legend = d3
      .legendColor()
      .labelFormat(format)
      .shapeWidth(10)
      .cells(cells)
      .ascending(true)
      .orient("vertical")
      .scale(this.colour);

    legendSvg.select("." + legendClass).call(legend);
  }

  isEmpty() {
    return Object.keys(this.data).length === 0;
  }

  getDataAtVoxelCoords(voxelCoords, dataName) {
    let [x, y, z] = voxelCoords; //.map((v) => parseInt(v));
    let address =
      z * (this.data.voxelNumber.x * this.data.voxelNumber.y) +
      y * this.data.voxelNumber.x +
      x;

    return this.data[dataName][address];
  }
}

class DoseVolume extends Volume {
  constructor(height, width) {
    super(height, width); // call the super class constructor
  }

  addData(data) {
    super.addData(data);
    super.addColourScheme(d3.interpolateTurbo, this.data.maxDose);
    // Calculate the contour thresholds
    this.thresholds = d3.range(0, 1.1, 0.1).map((i) => i * this.data.maxDose);
  }

  getSlice(axis, sliceNum) {
    return super.getSlice(axis, sliceNum, "dose");
  }

  getSliceImageContext(slice, svgPlot) {
    //TODO: Don't rely on plugin for legend/colour scale
    // https://observablehq.com/@d3/color-legend

    // Clear dose plot
    svgPlot.selectAll("g").remove();

    //TODO: Try out different thresholds
    // Draw contours
    var contours = d3
      .contours()
      .size([slice.xVoxels, slice.yVoxels])
      .thresholds(this.thresholds)(slice.sliceData);

    svgPlot
      .append("g")
      .attr("width", this.width)
      .attr("height", this.height)
      .attr("fill", "none")
      .attr("stroke", "#fff")
      .attr("stroke-opacity", 0.5)
      .attr("stroke-width", 0.5)
      .selectAll("path")
      .data(
        super.scaleContour(
          contours,
          this.width / slice.xVoxels,
          this.height / slice.yVoxels
        )
      )
      .join("path")
      .attr("fill", (d) => this.colour(d.value))
      .attr("fill-opacity", 0.5)
      .attr("d", d3.geoPath());
  }

  initializeLegend() {
    super.initializeLegend(
      doseLegendSvg,
      "doseLegend",
      d3.format(".2e"),
      this.thresholds,
      "Dose"
    );
  }

  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "dose");
  }
}

class DensityVolume extends Volume {
  constructor(height, width) {
    super(height, width); // call the super class constructor
  }

  addData(data) {
    super.addData(data);
    super.addColourScheme(d3.interpolateViridis, this.data.maxDensity);
  }

  getSlice(axis, sliceNum) {
    return super.getSlice(axis, sliceNum, "density");
  }

  getSliceImageContext(slice, canvas) {
    // TODO: Leave axes outside of either volume
    // TODO: Clear svgAxis <g> only if axis changed
    // TODO: Allow zooming and translating
    // TODO: Make two new functions: change slicenum and change axes

    // For axis structure
    // https://bl.ocks.org/ejb/e2da5a23e9a09d494bd532803d8db61c
    let context = canvas.node().getContext("2d");

    // Clear canvas context
    context.clearRect(0, 0, canvas.node().width, canvas.node().height);

    // Clear and redraw axes upon change
    let axisChange = axis !== this.prevAxis ? true : false;
    if (axisChange) {
      svgAxis.selectAll("g").remove();
      var xAxis = d3.axisBottom().scale(slice.xScale);
      var yAxis = d3.axisLeft().scale(slice.yScale);
      svgAxis
        .append("g")
        .attr("class", "x-axis")
        .attr("transform", "translate(0," + height + ")")
        .call(xAxis);
      svgAxis.append("g").attr("class", "y-axis").call(yAxis);
    }

    // Calcuate display pixel dimensions
    let dxScaled = this.width / slice.xVoxels;
    let dyScaled = this.height / slice.yVoxels;

    // TODO: Turn this into a mapping/forEach?
    // TODO: Could save canvas as URL then scale up easily?
    for (let i = 0; i < slice.xVoxels; i++) {
      for (let j = 0; j < slice.yVoxels; j++) {
        let new_address = i + slice.xVoxels * j;
        context.fillStyle = this.colour(slice.sliceData[new_address]);
        context.fillRect(
          Math.ceil(slice.xScale(slice.x[i])),
          Math.ceil(slice.yScale(slice.y[j])),
          Math.ceil(dxScaled),
          Math.ceil(dyScaled)
        );
      }
    }
    canvas.id = "canvas";
    this.prevAxis = axis;
    return context;
  }

  initializeLegend() {
    super.initializeLegend(
      densityLegendSvg,
      "densityLegend",
      d3.format(".2f"),
      5,
      "Density"
    );
  }

  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "density");
  }
}
