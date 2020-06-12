// TODO: Make general volume structure that includes dose
class DoseVolume {}

class DensityVolume {
  // General volume structure
  // https://github.com/aces/brainbrowser/blob/fe0ce114c6cd8e317a6bdd9b7ef97cbf1c38309d/src/brainbrowser/volume-viewer/volume-loaders/minc.js#L88-L190

  constructor(height, width) {
    //TODO: Figure out a better system for height and width
    this.height = height;
    this.width = width;
    this.data = {};
    this.prevAxis = "";
  }
  //TODO: Add function to check if data has been added

  addDensityData(data) {
    // TODO: Find max/min for this.colour without stack overflow
    this.data = data;
    this.colour = d3
      .scaleSequentialSqrt(d3.interpolateGreys)
      .domain([0, data.maxDensity]);
  }

  addDoseData(data) {
    // TODO: Find max/min for this.doseColour without stack overflow
    this.doseData = data;
    this.doseColour = d3
      .scaleSequentialSqrt(d3.interpolateYlOrRd)
      .domain([0, data.maxDose]);
  }

  getSlice(axis, sliceNum) {
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

    let slice = {
      dx: this.data.voxelSize[dim1],
      dy: this.data.voxelSize[dim2],
      xVoxels: this.data.voxelNumber[dim1],
      yVoxels: this.data.voxelNumber[dim2],
      x: x,
      y: y,
      totalSlices: this.data.voxelNumber[dim3],
      xScale: d3
        .scaleLinear()
        .domain([x[0], x[x.length - 1]])
        .range([0, this.width]), // unit: pixels
      yScale: d3
        .scaleLinear()
        .domain([y[0], y[y.length - 1]])
        .range([0, this.height]), // unit: pixels
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
    let doseData = new Array(slice.xVoxels * slice.yVoxels);

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
        sliceData[new_address] = this.data.density[address];
        doseData[new_address] = this.doseData.dose[address];
      }
    }

    return {
      ...slice,
      axis: axis,
      sliceData: sliceData,
      doseData: doseData,
      sliceNum: sliceNum,
    };
  }

  getSliceImageContext(slice, canvas) {
    // TODO: Clear svgAxis <g> only if axis changed
    // TODO: Allow zooming and translating
    // TODO: Make two new functions: change slicenum and change axes

    // For axis structure
    // https://bl.ocks.org/ejb/e2da5a23e9a09d494bd532803d8db61c
    let context = canvas.node().getContext("2d");

    // Clear canvas context and svg plot
    context.clearRect(0, 0, canvas.node().width, canvas.node().height);
    svgPlot.selectAll("g").remove();

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

    // Draw contours
    let thresholds = d3
      .range(0, 1.1, 0.1)
      .map((i) => i * this.doseData.maxDose);

    var contours = d3
      .contours()
      .size([slice.xVoxels, slice.yVoxels])
      .thresholds(thresholds)(slice.doseData);

    var scaledContours = contours.map(({ type, value, coordinates }) => ({
      type,
      value,
      coordinates: coordinates.map((rings) =>
        rings.map((points) =>
          points.map(([i, j]) => [
            i * (this.width / slice.xVoxels),
            j * (this.height / slice.yVoxels),
          ])
        )
      ),
    }));

    svgPlot
      .append("g")
      .attr("width", this.width)
      .attr("height", this.height)
      .attr("fill", "none")
      .attr("stroke", "#fff")
      .attr("stroke-opacity", 0.5)
      .selectAll("path")
      .data(scaledContours)
      .join("path")
      .attr("fill", "none")
      .attr("stroke-width", 5.0)
      .attr("stroke", (d) => this.doseColour(d.value))
      .attr("stroke-linejoin", "round")
      .attr("d", d3.geoPath());

    // Calcuate display pixel dimensions
    let dxScaled = this.width / slice.xVoxels;
    let dyScaled = this.height / slice.yVoxels;

    // TODO: Turn this into a mapping/forEach?
    for (let i = 0; i < slice.xVoxels; i++) {
      for (let j = 0; j < slice.yVoxels; j++) {
        let new_address = i + slice.xVoxels * j;
        context.fillStyle = this.colour(slice.sliceData[new_address]);
        context.fillRect(
          slice.xScale(slice.x[i]),
          slice.yScale(slice.y[j]),
          dxScaled,
          dyScaled
        );
      }
    }
    canvas.id = "canvas";
    this.prevAxis = axis;
    return context;
  }
}
