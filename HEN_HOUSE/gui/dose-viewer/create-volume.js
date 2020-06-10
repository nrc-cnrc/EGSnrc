class DensityVolume {
  // General volume structure
  // https://github.com/aces/brainbrowser/blob/fe0ce114c6cd8e317a6bdd9b7ef97cbf1c38309d/src/brainbrowser/volume-viewer/volume-loaders/minc.js#L88-L190

  constructor(height, width) {
    this.height = height; //TODO: Figure out a better system for height and width
    this.width = width;
    this.data = {};
  }

  addData(data, position) {
    // TODO: Find max/min for this.colour without stack overflow
    // TODO: Incorporate xScale, yScale
    this.data = data;
    this.position = position || {};
    this.colour = d3.scaleSequentialSqrt(d3.interpolateYlGnBu).domain([0, 2]);
  }

  getSlice(axis, sliceNum) {
    sliceNum = sliceNum === undefined ? volume.position.sliceNum : sliceNum;
    let slice;
    // For slice structure
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1502-L1546
    if (axis === "xy") {
      slice = {
        dx: this.data.voxelSize.x,
        dy: this.data.voxelSize.y,
        xVoxels: this.data.voxelNumber.x,
        yVoxels: this.data.voxelNumber.y,
        xMin: this.data.voxelArr.x[0],
        yMin: this.data.voxelArr.y[0],
      };
    } else if (axis === "yz") {
      slice = {
        dx: this.data.voxelSize.y,
        dy: this.data.voxelSize.z,
        xVoxels: this.data.voxelNumber.y,
        yVoxels: this.data.voxelNumber.z,
        xMin: this.data.voxelArr.y[0],
        yMin: this.data.voxelArr.z[0],
      };
    } else if (axis === "xz") {
      slice = {
        dx: this.data.voxelSize.x,
        dy: this.data.voxelSize.z,
        xVoxels: this.data.voxelNumber.x,
        yVoxels: this.data.voxelNumber.z,
        xMin: this.data.voxelArr.x[0],
        yMin: this.data.voxelArr.z[0],
      };
    }

    // For address calculations:
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1999-L2034
    let sliceData = new Array(slice.xVoxels * slice.yVoxels);

    // TODO: Fix the sliceData so output is not shifted
    for (let i = 0; i < slice.xVoxels; i++) {
      for (let j = 0; j < slice.yVoxels; j++) {
        let address;
        if (axis === "xy") {
          address = i + slice.xVoxels * (j + sliceNum * slice.yVoxels);
        } else if (axis === "yz") {
          address =
            sliceNum + this.data.voxelNumber.x * (i + j * slice.xVoxels);
        } else if ((axis = "xz")) {
          address =
            i + slice.xVoxels * (sliceNum + j * this.data.voxelNumber.y);
        }
        let new_address = i + slice.xVoxels * j;
        sliceData[new_address] = this.colour(this.data.density[address]);
      }
    }

    slice["axis"] = axis;
    slice["sliceData"] = sliceData;
    slice["sliceNum"] = sliceNum;
    return slice;
  }

  getSliceImageContext(slice, canvas) {
    let context = canvas.node().getContext("2d");
    context.clearRect(0, 0, canvas.node().width, canvas.node().height);
    for (let i = 0; i < slice.xVoxels; i++) {
      for (let j = 0; j < slice.yVoxels; j++) {
        let new_address = i + slice.xVoxels * j;
        context.fillStyle = slice.sliceData[new_address];
        context.fillRect(i, j, 1, 1);
      }
    }
    canvas.id = "canvas";
    return context;
  }
}
