// TODO: Allow for other axis views
class DensityVolume {
  constructor(height, width, data, position) {
    this.height = data.numVoxY;
    this.width = data.numVoxX;
    this.data = data;
    this.position = position || {};
    this.xScale = d3
      .scaleLinear()
      .domain(x[0] < x[1] ? [x[0], x[-1]] : [x[-1], x[0]]) // unit: mm
      .range([0, 400]); // unit: pixels
    this.yScale = d3
      .scaleLinear()
      .domain(y[0] < y[1] ? [y[0], y[-1]] : [y[-1], y[0]]) // unit: mm
      .range([0, 400]); // unit: pixels
    this.yScale = d3
      .scaleLinear()
      .domain(z[0] < z[1] ? [z[0], z[-1]] : [z[-1], z[0]]) // unit: mm
      .range([0, 400]); // unit: pixels

    // TODO: Find max/min without stack overflow
    this.colour = d3.scaleSequentialSqrt(d3.interpolateYlGnBu).domain([0, 2]);
  }

  getSlice(axis, sliceNum) {
    sliceNum = sliceNum === undefined ? volume.position[axis] : sliceNum;

    let size;

    if (axis === "xy") {
      size = this.data.numVoxX * this.data.numVoxY;
    } else if (axis === "yz") {
      size = this.data.numVoxY * this.data.numVoxZ;
    } else if (axis === "xz") {
      size = this.data.numVoxX * this.data.numVoxZ;
    }

    //https://github.com/aces/brainbrowser/blob/fe0ce114c6cd8e317a6bdd9b7ef97cbf1c38309d/src/brainbrowser/volume-viewer/volume-loaders/minc.js#L161
    let sliceData = new Array(size);

    let i = 0;
    let z = sliceNum;

    // Order 'F' indexing: First index changing fastest, last index changing slowest
    for (let x = 0; x < this.data.numVoxX; x++) {
      for (let y = 0; y < this.data.numVoxY; y++) {
        sliceData[i++] = this.colour(
          this.data.density[
            z * (this.data.numVoxX * this.data.numVoxY) +
              y * this.data.numVoxX +
              x
          ]
        );
      }
    }

    return {
      axis: axis,
      sliceData: sliceData,
      width: this.data.numVoxX,
      height: this.data.numVoxY,
    };
  }

  getSliceImageContext(slice) {
    let canvas = d3
      .select("#chartholder")
      .append("canvas")
      .attr("width", this.width)
      .attr("height", this.height)
      .attr("class", "volume-plot");

    let context = canvas.node().getContext("2d");

    let i = 0;
    for (let x = 0; x < this.data.numVoxX; x++) {
      for (let y = 0; y < this.data.numVoxY; y++) {
        context.fillStyle = slice.sliceData[i++];
        context.fillRect(x, y, 2, 2);
      }
    }
    canvas.id = "canvas";
    return context;
  }
}
