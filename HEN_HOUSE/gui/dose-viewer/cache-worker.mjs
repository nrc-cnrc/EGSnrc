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
  return d3.scaleSqrt().domain([minVal, maxVal]).range([0, 255])
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
      imageData[j++] = val // R value
      imageData[j++] = val // G value
      imageData[j++] = val // B value
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

  // Get the axes and slice dimensions
  const [dim1, dim2, dim3] =
    axis === 'xy'
      ? ['x', 'y', 'z']
      : axis === 'yz'
        ? ['y', 'z', 'x']
        : ['x', 'z', 'y']

  const xVoxels = data.voxelNumber[dim1]
  const yVoxels = data.voxelNumber[dim2]

  //  Find zScale to get sliceNum from slicePos
  const z = data.voxelArr[dim3]
  const totalSlices = data.voxelNumber[dim3] - 1
  const zDomain = [z[0], z[z.length - 1]]
  const zRange = [0, totalSlices]
  const zScale = (val) => (
    zRange[0] + (zRange[1] - zRange[0]) * ((val - zDomain[0]) / (zDomain[1] - zDomain[0]))
  )
  const sliceNum = Math.round(zScale(slicePos))

  // Get the slice data for the given axis and index
  // For address calculations:
  // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1999-L2034
  const sliceData = new Array(xVoxels * yVoxels)

  for (let i = 0; i < xVoxels; i++) {
    for (let j = 0; j < yVoxels; j++) {
      let address
      if (axis === 'xy') {
        address = i + xVoxels * (j + sliceNum * yVoxels)
      } else if (axis === 'yz') {
        address =
          sliceNum + data.voxelNumber.x * (i + j * xVoxels)
      } else if (axis === 'xz') {
        address =
          i + xVoxels * (sliceNum + j * data.voxelNumber.y)
      }
      const newAddress = i + xVoxels * j
      sliceData[newAddress] = data[dataName][address]
    }
  }

  const slice = {
    xVoxels: xVoxels,
    yVoxels: yVoxels,
    slicePos: slicePos,
    sliceData: sliceData,
    sliceNum: sliceNum,
    axis: axis
  }

  return slice
}
