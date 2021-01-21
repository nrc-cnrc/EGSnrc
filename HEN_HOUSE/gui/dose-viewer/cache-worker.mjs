/* global addEventListener */
/* global close */
/* global d3 */
/* global importScripts */
/* global postMessage */
/* global removeEventListener */

if (typeof importScripts === 'function') {
  importScripts('https://d3js.org/d3-color.v2.min.js')
  importScripts('https://d3js.org/d3-scale.v3.min.js')
  importScripts('https://d3js.org/d3-interpolate.v2.min.js')
  importScripts('https://d3js.org/d3-scale-chromatic.v2.min.js')
  addEventListener('message', handleMessage)
}

function handleMessage (e) {
  // Process position to get centre voxel position rather than boundaries
  var position = e.data.voxArr
  position = position.map((val, i) => {
    return val + (e.data.voxArr[i + 1] - val) / 2
  })
  position.pop()

  // Get the function to get colour
  const colourFn = getColourFunction(e.data.minVal, e.data.maxVal)

  // TODO: Modify getSlice to take sliceNum instead of pos!
  // Get and cache image for each slice in axis
  const promiseImgCache = position.map((slicePos) => {
    const slice = getSlice(e.data.data, e.data.dimensions, e.data.axis, slicePos, 'density')
    return getDataArray(slice, colourFn)
  })

  Promise.all(promiseImgCache).then(imgCache => {
    postMessage(imgCache)
    removeEventListener('message', handleMessage)
    close()
  })
}

/**
 * Create the function to return the pixel colour of each pixel value
 *
 * @param {number} minVal The minimum density value
 * @param {number} maxVal The maximum density value
 * @returns {getColourFunction~colourFn}
 */
function getColourFunction (minVal, maxVal) {
  var colourScheme = d3.scaleSequentialSqrt(d3.interpolateGreys).domain([maxVal, minVal])

  var colourFn = (val) => d3.color(colourScheme(val))
  return colourFn
}

/**
 * Returns the data image of the slice
 *
 * @param {Object} slice The slice of the density data.
 * @returns {number[]}
 */
function getDataArray (slice, colourFn) {
  // Create the image data
  var imageData = new Uint8ClampedArray(slice.xVoxels * slice.yVoxels * 4)
  var j = 0
  for (let i = 0; i < slice.sliceData.length; i++) {
    const val = colourFn(slice.sliceData[i])

    if (val !== null) {
      // Modify pixel data
      imageData[j++] = val.r // R value
      imageData[j++] = val.g // G value
      imageData[j++] = val.b // B value
      imageData[j++] = 255 // A value
    }
  }

  return imageData
}

/**
 * Get a slice of data through an axis.
 *
 * @param {string} axis The axis of the slice (xy, yz, or xz).
 * @param {number} sliceNum The number of the slice.
 * @param {string} dataName The type of data, either "density" or "dose".
 * @returns {Object}
 */
function getSlice (data, dimensions, axis, slicePos, dataName) {
  // TODO: Cache previous slices
  // TODO: Only redefine slice attributes on axis change
  // For slice structure
  // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1502-L1546

  var sliceNum

  // Get the axes and slice dimensions
  const [dim1, dim2, dim3] =
    axis === 'xy'
      ? ['x', 'y', 'z']
      : axis === 'yz'
        ? ['y', 'z', 'x']
        : ['x', 'z', 'y']

  const x = data.voxelArr[dim1]
  const y = data.voxelArr[dim2]
  const z = data.voxelArr[dim3]
  const totalSlices = data.voxelNumber[dim3] - 1

  // Get the length in cm of the x and y dimensions
  var getLengthCm = (voxelArrDim) =>
    Math.abs(voxelArrDim[voxelArrDim.length - 1] - voxelArrDim[0])
  const [xLengthCm, yLengthCm] = [getLengthCm(x), getLengthCm(y)]

  // Initialize variables to make slice scales
  let xRange, yRange

  // if (args !== undefined) {
  //   xDomain = args[axis].xScale.domain()
  //   yDomain = args[axis].yScale.domain()
  //   xRange = [Math.round(args[axis].xScale(x[0])), Math.round(args[axis].xScale(x[x.length - 1]))]
  //   yRange = [Math.round(args[axis].yScale(y[0])), Math.round(args[axis].yScale(y[y.length - 1]))]
  // } else {
  if (xLengthCm > yLengthCm) {
    xRange = [0, dimensions.width]
    yRange = axis === 'xy' ? [dimensions.height * (1 - (yLengthCm / xLengthCm)), dimensions.height] : [0, dimensions.height * (yLengthCm / xLengthCm)]
  } else {
    xRange = [0, dimensions.width * (xLengthCm / yLengthCm)]
    yRange = axis === 'xy' ? [0, dimensions.height] : [dimensions.height, 0]
  }
  // }

  const zDomain = [z[0], z[z.length - 1]]
  const zRange = [0, totalSlices]
  const zScale = (val) => (
    zRange[0] + (zRange[1] - zRange[0]) * ((val - zDomain[0]) / (zDomain[1] - zDomain[0]))
  )

  sliceNum = zScale(slicePos)
  // TODO: Change scales to quantile to map exactly which pixels
  let slice = {
    dx: data.voxelSize[dim1],
    dy: data.voxelSize[dim2],
    xVoxels: data.voxelNumber[dim1],
    yVoxels: data.voxelNumber[dim2],
    x: x,
    y: y,
    totalSlices: totalSlices,
    dxDraw: xRange[0],
    dyDraw: yRange[0],
    dWidthDraw: xRange[1] - xRange[0],
    dHeightDraw: yRange[1] - yRange[0],
    slicePos: slicePos,
    dimensions: dimensions,
    axis: axis
  }

  // If current slice number is larger than the total number of slices
  // set slice number to last slice
  sliceNum =
    sliceNum >= slice.totalSlices
      ? Math.round(slice.totalSlices - 1)
      : Math.round(sliceNum)

  // Get the slice data for the given axis and index
  // For address calculations:
  // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1999-L2034
  const sliceData = new Array(slice.xVoxels * slice.yVoxels)

  for (let i = 0; i < slice.xVoxels; i++) {
    for (let j = 0; j < slice.yVoxels; j++) {
      let address
      if (axis === 'xy') {
        address = i + slice.xVoxels * (j + sliceNum * slice.yVoxels)
      } else if (axis === 'yz') {
        address =
          sliceNum + data.voxelNumber.x * (i + j * slice.xVoxels)
      } else if (axis === 'xz') {
        address =
          i + slice.xVoxels * (sliceNum + j * data.voxelNumber.y)
      }
      const newAddress = i + slice.xVoxels * j
      sliceData[newAddress] = data[dataName][address]
    }
  }

  slice = {
    ...slice,
    sliceData: sliceData,
    sliceNum: sliceNum
  }

  return slice
}
