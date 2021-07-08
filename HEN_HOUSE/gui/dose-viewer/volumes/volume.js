/*
###############################################################################
#
#  EGSnrc online voxel and dose visualization tool
#  Copyright (C) 2020 Magdalena Bazalova-Carter and Elise Badun
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Elise Badun, 2020
#
#  Contributors:
#
###############################################################################
*/

// definitions for StandardJS formatter
/* global d3 */

/** @class Volume represents a dose or density file and includes classes the
 * get slices of data.  */
class Volume { // eslint-disable-line no-unused-vars
  // General volume structure
  // https://github.com/aces/brainbrowser/blob/fe0ce114c6cd8e317a6bdd9b7ef97cbf1c38309d/src/brainbrowser/volume-viewer/volume-loaders/minc.js#L88-L190

  /**
   * Creates an instance of a Volume.
   *
   * @constructor
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   */
  constructor (fileName, dimensions, legendDimensions, args) {
    this.fileName = fileName
    this.dimensions = dimensions
    this.legendDimensions = legendDimensions
    this.args = args
    this.baseSlices = { xy: {}, yz: {}, xz: {} }
    this.sliceCache = { xy: {}, yz: {}, xz: {} }
    this.imageCache = {}
  }

  /**
   * Sets the HTML elements used in plotting as properties of the volume.
   *
   * @param {Object} htmlElementObj Stores the HTML element of each axis (xy,
   * yz, and xz).
   * @param {Object} legendHolder The parent element of the legend.
   * @param {Object} legendSvg The child svg element of the legend.
   */
  // TODO: Remove html elements as properties and just pass them in
  setHtmlObjects (htmlElementObj, legendHolder, legendSvg) {
    // TODO: Remove DOM references in volume
    this.htmlElementObj = htmlElementObj
    this.legendHolder = legendHolder
    this.legendSvg = legendSvg
  }

  /**
   * Add the colour scheme used for the plots.
   *
   * @param {function} colourScheme The d3 colour scheme.
   * @param {number} maxVal The maximum value mapped to the colour scheme.
   * @param {number} maxVal The maximum value mapped to the colour scheme.
   * @param {boolean} invertScheme Whether to map from max to min or min to max.
   */
  addColourScheme (colourScheme, maxVal, minVal, invertScheme) {
    const domain = invertScheme ? [maxVal, minVal] : [minVal, maxVal]
    this.colour = d3.scaleSequentialSqrt(colourScheme).domain(domain)
  }

  /**
   * Create the scales for the x, y and z dimensions in each axis.
   *
   * @param {Object} data The data from parsing the file.
   */
  createBaseSlices (data, type) {
    // TODO: depending on the type, leave some data out
    // TODO: Change scales to quantile to map exactly which pixels
    const AXES = ['xy', 'yz', 'xz']

    // Create scales to map world coordinates to voxel
    this.buildWorldToVoxelScales(data)

    AXES.forEach((axis) => {
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
      const xVoxels = data.voxelNumber[dim1]
      const yVoxels = data.voxelNumber[dim2]

      // Get the length in cm of the x and y dimensions
      var getLengthCm = (voxelArrDim) =>
        Math.abs(voxelArrDim[voxelArrDim.length - 1] - voxelArrDim[0])
      const [xLengthCm, yLengthCm] = [getLengthCm(x), getLengthCm(y)]

      // Initialize variables to make slice scales
      let xDomain,
        yDomain,
        xRange,
        yRange

      if (xLengthCm > yLengthCm) {
        xDomain = [x[0], x[x.length - 1]]
        yDomain = axis === 'xy' ? [y[y.length - 1], y[y.length - 1] - xLengthCm] : [y[y.length - 1] - xLengthCm, y[y.length - 1]]
        xRange = [0, this.dimensions.width]
        yRange = axis === 'xy' ? [this.dimensions.height * (1 - (yLengthCm / xLengthCm)), this.dimensions.height] : [0, this.dimensions.height * (yLengthCm / xLengthCm)]
      } else {
        xDomain = [x[0], x[0] + yLengthCm]
        yDomain = axis === 'xy' ? [y[y.length - 1], y[0]] : [y[0], y[y.length - 1]]
        xRange = [0, this.dimensions.width * (xLengthCm / yLengthCm)]
        yRange = axis === 'xy' ? [0, this.dimensions.height] : [this.dimensions.height, 0]
      }

      // Define screen pixel to real length mapping
      const xScale = d3
        .scaleLinear()
        .domain(xDomain)
        .range([0, this.dimensions.width])
      const yScale = d3
        .scaleLinear()
        .domain(yDomain)
        .range([this.dimensions.height, 0])
      const zScale = d3
        .scaleLinear()
        .domain([z[0], z[z.length - 1]])
        .range([0, totalSlices])

      if (type === 'dose') {
        xRange = [Math.round(xScale(x[0])), Math.round(xScale(x[x.length - 1]))]
        yRange = [Math.round(yScale(y[0])), Math.round(yScale(y[y.length - 1]))]
      }

      // Define the screen pixel to volume voxel mapping
      const xPixelToVoxelScale = d3
        .scaleQuantile()
        .domain(xRange)
        .range(d3.range(0, xVoxels, 1))
      const yPixelToVoxelScale = d3
        .scaleQuantile()
        .domain(yRange)
        .range(axis === 'xy' ? d3.range(0, yVoxels, 1) : d3.range(yVoxels - 1, -1, -1))

      // Define the voxel to screen mapping for dose contours
      const contourXScale = d3
        .scaleLinear()
        .domain([0, xVoxels])
        .range(xRange)
      const contourYScale = d3
        .scaleLinear()
        .domain(axis === 'xy' ? [yVoxels, 0] : [0, yVoxels])
        .range(axis === 'xy' ? yRange.reverse() : yRange)

      this.baseSlices[axis] = {
        dx: data.voxelSize[dim1],
        dy: data.voxelSize[dim2],
        xVoxels: xVoxels,
        yVoxels: yVoxels,
        x: x,
        y: y,
        totalSlices: totalSlices,
        dxDraw: xRange[0],
        dyDraw: yRange[0],
        dWidthDraw: xRange[1] - xRange[0],
        dHeightDraw: yRange[1] - yRange[0],
        xScale: xScale,
        yScale: yScale,
        zScale: zScale,
        axis: axis,
        xPixelToVoxelScale: xPixelToVoxelScale,
        yPixelToVoxelScale: yPixelToVoxelScale,
        contourTransform: ({ type, value, coordinates }) => ({
          type,
          value,
          coordinates: coordinates.map((rings) =>
            rings.map((points) =>
              points.map(([i, j]) => [contourXScale(i), contourYScale(j)])
            )
          )
        })
      }
    })
  }

  /**
   * Create the scales for the x, y and z dimensions to map world coordinates to
   * voxel coordinates.
   *
   * @param {Object} data The data from parsing the file.
   */
  buildWorldToVoxelScales (data) {
    this.worldToVoxel = {
      x: d3.scaleQuantile().domain(data.voxelArr.x).range(d3.range(0, data.voxelNumber.x, 1)),
      y: d3.scaleQuantile().domain(data.voxelArr.y).range(d3.range(0, data.voxelNumber.y, 1)),
      z: d3.scaleQuantile().domain(data.voxelArr.z).range(d3.range(0, data.voxelNumber.z, 1))
    }
  }

  /**
   * Get a slice of data through an axis.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} sliceNum The number of the slice.
   * @param {string} dataName The type of data, either "density" or "dose".
   * @returns {Object}
   */
  getSlice (axis, slicePos, dataName) {
    // TODO: Cache previous slices
    // TODO: Only redefine slice attributes on axis change
    // For slice structure
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1502-L1546

    const baseSlice = this.baseSlices[axis]
    const sliceNum = Math.round(baseSlice.zScale(slicePos))

    // If slice is cached, return it
    if ((this.sliceCache[axis] !== undefined) && (this.sliceCache[axis][sliceNum] !== undefined)) {
      return this.sliceCache[axis][sliceNum]
    }

    // Get the slice data for the given axis and index
    // For address calculations:
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1999-L2034
    const sliceData = new Array(baseSlice.xVoxels * baseSlice.yVoxels)

    for (let i = 0; i < baseSlice.xVoxels; i++) {
      for (let j = 0; j < baseSlice.yVoxels; j++) {
        let address
        if (axis === 'xy') {
          address = i + baseSlice.xVoxels * (j + sliceNum * baseSlice.yVoxels)
        } else if (axis === 'yz') {
          address =
            sliceNum + this.data.voxelNumber.x * (i + j * baseSlice.xVoxels)
        } else if (axis === 'xz') {
          address =
            i + baseSlice.xVoxels * (sliceNum + j * this.data.voxelNumber.y)
        }
        const newAddress = i + baseSlice.xVoxels * j
        sliceData[newAddress] = this.data[dataName][address]
      }
    }

    const slice = {
      slicePos: slicePos,
      sliceData: sliceData,
      sliceNum: sliceNum,
      axis: axis
    }

    this.sliceCache[axis][sliceNum] = slice
    return slice
  }

  /**
   * Get the data value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @param {string} dataName The type of data, either "density" or "dose".
   * @returns {number}
   */
  getDataAtVoxelCoords (voxelCoords, dataName) {
    const [x, y, z] = voxelCoords
    const address =
      z * (this.data.voxelNumber.x * this.data.voxelNumber.y) +
      y * this.data.voxelNumber.x +
      x

    return this.data[dataName][address]
  }

  /**
   * Convert world coords to voxel coords of volume.
   *
   * @param {number[]} worldCoords The world position of the data.
   * @returns {number[]}
   */
  worldToVoxelCoords (worldCoords) {
    return [this.worldToVoxel.x(worldCoords[0]), this.worldToVoxel.y(worldCoords[1]), this.worldToVoxel.z(worldCoords[2])]
  }

  /**
   * Get the dose value at the given world coordinates.
   *
   * @param {number[]} coords The world position of the data.
   * @param {string} dataName The type of data, either "density" or "dose".
   * @returns {number}
   */
  getDataAtPosition (coords, dataName) {
    const voxelCoords = this.worldToVoxelCoords(coords)
    return this.getDataAtVoxelCoords(voxelCoords, dataName)
  }
}

// export { Volume }
