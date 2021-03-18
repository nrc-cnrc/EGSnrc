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
/* global Image */
/* global Worker */

// import { volumeViewerList } from './index.js'
// import { Slider } from './slider.js'

/** @class Volume represents a dose or density file and includes classes the
 * get slices of data.  */
class Volume {
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
    const voxelArr = this.data.voxelArr
    const voxelNum = this.data.voxelNumber

    const xWorldToVoxel = d3.scaleQuantile().domain(voxelArr.x).range(d3.range(0, voxelNum.x, 1))
    const yWorldToVoxel = d3.scaleQuantile().domain(voxelArr.y).range(d3.range(0, voxelNum.y, 1))
    const zWorldToVoxel = d3.scaleQuantile().domain(voxelArr.z).range(d3.range(0, voxelNum.z, 1))

    const i = xWorldToVoxel(worldCoords[0])
    const j = yWorldToVoxel(worldCoords[1])
    const k = zWorldToVoxel(worldCoords[2])

    return [i, j, k]
  }
}

class StructureSetVolume extends Volume { // eslint-disable-line no-unused-vars
  /**
   * Creates an instance of a StructureSetVolume.
   *
   * @constructor
   * @extends Volume
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   * @param {Object} data The data from parsing the file.
   */
  constructor (fileName, dimensions, legendDimensions, data, args) {
    // TODO: Remove args?
    // Call the super class constructor
    super(fileName, dimensions, legendDimensions, args)
    this.addData(data)
  }

  /**
   * Adds data to the StructureSetVolume object.
   *
   * @param {Object} data The data from parsing the file.
   */
  addData (data) {
    this.data = data
    this.ROIoutlines = this.makeOutlines(data)
  }

  /**
   * Turns raw ROI data into useable outlines.
   *
   * @param {Object} data The data from parsing the file.
   */
  makeOutlines (data) {
    const ROIoutlines = []
    var ROI

    // If val is smaller than range min or larger than range max, update range
    const setExtrema = (val, range) => {
      if (range[0] === undefined || val < range[0]) {
        range[0] = val
      } else if (range[1] === undefined || val > range[1]) {
        range[1] = val
      }
    }

    // Get the min and max values and update rangeVals accordingly
    const updateRange = (vals, rangeVals) => {
      setExtrema(vals[0], rangeVals.x)
      setExtrema(vals[1], rangeVals.y)
      setExtrema(vals[2], rangeVals.z)
    }

    const toCm = (val) => parseFloat(val) / 10.0

    // For each region of ROI
    for (let i = 0; i < data.ROIs.length; i++) {
      ROI = data.ROIs[i]

      // If the ROI has contour data
      if (ROI.ContourSequence !== undefined) {
        const contourData = []
        const rangeVals = { x: [], y: [], z: [] }
        const contourGeometricType = new Set()

        // For each slice of the contour data
        ROI.ContourSequence.forEach((sequence) => {
          var values = sequence.ContourData.split('\\')
          var sliceContourData = []

          // The z value should be constant for each slice
          const z = toCm(values[2])

          // For each coordinate in the slice
          for (let i = 0; i < values.length; i += 3) {
            updateRange(values.slice(i, i + 3).map((val) => toCm(val)), rangeVals)
            sliceContourData.push({ x: toCm(values[i]), y: toCm(values[i + 1]) })
          }
          contourData.push({ z: z, vals: sliceContourData })
          contourGeometricType.add(sequence.ContourGeometricType)
        })

        // Find the voxel positions of the grid to overlay the ROI data on
        const increments = 0.2
        const divider = 1 / increments
        const voxArr = Object.values(rangeVals).map((range) => {
          const min = Math.floor(range[0] * divider) / divider
          const max = Math.ceil(range[1] * divider) / divider
          const items = Math.round((max - min) / increments)
          return [...Array(items + 1)].map((x, y) => min + increments * y)
        })

        const [xVoxels, yVoxels, zVoxels] = voxArr.map((arr) => arr.length)
        const [xPosToVox, yPosToVox, zPosToVox] = voxArr.map((arr) => d3.scaleQuantize().domain([arr[0] - increments / 2, arr[arr.length - 1] + increments / 2]).range(d3.range(0, arr.length, 1)))
        const ROIarray = new Array(xVoxels * yVoxels * zVoxels)

        // Build the array that represents the ROI polygons as a matrix mask
        for (let k = 0; k < contourData.length; k++) {
          const polygon = contourData[k].vals.map((val) => [val.x, val.y])
          for (let i = 0; i < xVoxels; i++) {
            for (let j = 0; j < yVoxels; j++) {
              if (d3.polygonContains(polygon, [voxArr[0][i], voxArr[1][j]])) {
                const address = i + xVoxels * (j + k * yVoxels)
                ROIarray[address] = 1
              }
            }
          }
        }

        ROIoutlines.push({
          label: ROI.ROIName || ROI.ROIObservationLabel,
          colour: 'rgb(' + ROI.ROIDisplayColor.replaceAll('\\', ', ') + ')',
          contourData: contourData,
          contourGeometricType: contourGeometricType,
          voxelNumber: { x: xVoxels, y: yVoxels, z: zVoxels },
          scales: { x: xPosToVox, y: yPosToVox, z: zPosToVox },
          ROIarray: ROIarray,
          rangeVals: rangeVals
        })
      }
    }
    return ROIoutlines
  }

  /**
   * Get all ROI data slices through an axis.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} slicePos The position of the slice.
   * @returns {Object[]}
   */
  getSlices (axis, slicePos) {
    // Collect all ROIs that contain a point at slicePos
    var insideRange = (range, val) => (val > range[0] && val < range[1]) || (val < range[0] && val > range[1])
    const ROIOutlinesInRange = this.ROIoutlines.filter((ROIOutline) => ((axis === 'xy' && insideRange(ROIOutline.scales.z.domain(), slicePos)) ||
    (axis === 'yz' && insideRange(ROIOutline.scales.x.domain(), slicePos)) ||
    (axis === 'xz' && insideRange(ROIOutline.scales.y.domain(), slicePos)))
    )

    // For each ROI in the range, calculate the data slice that intersects slicePos
    const slices = ROIOutlinesInRange.map((ROIOutline) => {
      const sliceNum = axis === 'xy' ? ROIOutline.scales.z(slicePos) : axis === 'yz' ? ROIOutline.scales.x(slicePos) : ROIOutline.scales.y(slicePos)
      const [xVoxels, yVoxels] = axis === 'xy' ? [ROIOutline.voxelNumber.x, ROIOutline.voxelNumber.y]
        : axis === 'yz' ? [ROIOutline.voxelNumber.y, ROIOutline.voxelNumber.z]
          : [ROIOutline.voxelNumber.x, ROIOutline.voxelNumber.z]

      const [xRange, yRange] = axis === 'xy' ? [ROIOutline.rangeVals.x, ROIOutline.rangeVals.y] : axis === 'yz' ? [ROIOutline.rangeVals.y, ROIOutline.rangeVals.z] : [ROIOutline.rangeVals.x, ROIOutline.rangeVals.z]

      // Get the slice data for the given axis and index
      const sliceData = new Array(xVoxels * yVoxels)

      for (let i = 0; i < xVoxels; i++) {
        for (let j = 0; j < yVoxels; j++) {
          let address
          if (axis === 'xy') {
            address = i + xVoxels * (j + sliceNum * yVoxels)
          } else if (axis === 'yz') {
            address =
            sliceNum + ROIOutline.voxelNumber.x * (i + j * xVoxels)
          } else if (axis === 'xz') {
            address =
            i + xVoxels * (sliceNum + j * ROIOutline.voxelNumber.y)
          }
          const newAddress = i + xVoxels * j
          sliceData[newAddress] = ROIOutline.ROIarray[address] || 0
        }
      }

      return {
        slicePos: slicePos,
        sliceData: sliceData,
        sliceNum: sliceNum,
        axis: axis,
        xVoxels: xVoxels,
        yVoxels: yVoxels,
        colour: d3.color(ROIOutline.colour),
        xRange: xRange,
        yRange: yRange,
        label: ROIOutline.label
      }
    })

    // TODO: Cache slices
    // // If slice is cached, return it
    // if ((this.sliceCache[axis] !== undefined) && (this.sliceCache[axis][sliceNum] !== undefined)) {
    //   return this.sliceCache[axis][sliceNum]
    // }

    // this.sliceCache[axis][sliceNum] = slice
    return slices
  }

  /**
   * Plot the ROI outlines of the current position.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} slicePos The position of the slice.
   * @param {Object} svg The svg plot element.
   * @param {Volume} volume The correponding Volume object.
   * @param {Object} zoomTransform Holds information about the current transform
   * of the slice.
   */
  plotStructureSet (axis, slicePos, svg, volume, zoomTransform, hiddenClassList) {
    var toCSSClass = (className) => className.replace(/[|~ ! @ $ % ^ & * ( ) + = , . / ' ; : " ? > < \[ \] \ \{ \} | ]/g, '')

    const slices = this.getSlices(axis, slicePos)
    const baseSlice = volume.baseSlices[axis]
    const contourPathsList = []

    // Clear plot
    svg.selectAll('g.roi-contour').remove()

    slices.forEach((slice) => {
      // Create contour transform
      const yRange = [baseSlice.yScale(slice.yRange[0]), baseSlice.yScale(slice.yRange[1])]
      const xScale = d3
        .scaleLinear()
        .domain([0, slice.xVoxels])
        .range([baseSlice.xScale(slice.xRange[0]), baseSlice.xScale(slice.xRange[1])])
      const yScale = d3
        .scaleLinear()
        .domain(axis === 'xy' ? [slice.yVoxels, 0] : [0, slice.yVoxels])
        .range(axis === 'xy' ? yRange.reverse() : yRange)

      const contourTransform = ({ type, value, coordinates }) => ({
        type,
        value,
        coordinates: coordinates.map((rings) =>
          rings.map((points) =>
            points.map(([i, j]) => [xScale(i), yScale(j)])
          )
        )
      })

      // Draw contours
      var contours = d3
        .contours()
        .size([slice.xVoxels, slice.yVoxels])
        .thresholds([0.5])
        .smooth(true)(slice.sliceData)
        .map(contourTransform)

      const contourPaths = svg
        .append('g')
        .attr('class', 'roi-contour')
        .attr('width', svg.node().clientWidth)
        .attr('height', svg.node().clientHeight)
        .attr('fill', 'none')
        .attr('stroke', slice.colour)
        .attr('stroke-opacity', 1.0)
        .attr('stroke-width', 1.0)
        .selectAll('path')
        .data(contours)
        .join('path')
        .classed('roi-outline', true)
        .attr('class', (d, i) => 'roi-outline' + ' ' + toCSSClass(slice.label))
        .attr('d', d3.geoPath())

      contourPathsList.push(contourPaths)
    })

    if (hiddenClassList.length > 0) {
      // Apply hidden class to hidden contours
      contourPathsList.forEach((contourPaths) => [
        contourPaths.filter(hiddenClassList.join(','))
          .classed('hidden', true)
      ])
    }

    if (zoomTransform) {
      svg.selectAll('g.roi-contour').attr('transform', zoomTransform.toString())
    }
  }
}

/** @class Volume represents a .3ddose file.  */
class DoseVolume extends Volume { // eslint-disable-line no-unused-vars
  /**
   * Creates an instance of a DoseVolume.
   *
   * @constructor
   * @extends Volume
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   * @param {Object} data The data from parsing the file.
   */
  constructor (fileName, dimensions, legendDimensions, data, args) {
    // TODO: Remove args?
    // Call the super class constructor
    super(fileName, dimensions, legendDimensions, args)
    this.addData(data)
  }

  /**
   * Adds data to the DoseVolume object.
   *
   * @param {Object} data The data from parsing the file.
   */
  addData (data) {
    this.data = data
    // Max dose used for dose contour plot
    super.addColourScheme(d3.interpolateViridis, data.maxDose, 0)

    this.createBaseSlices(data, 'dose')
  }

  /**
   * Get a slice of dose data for a given axis and slice index.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} sliceNum The number of the slice.
   * @returns {Object}
   */
  getSlice (axis, slicePos, args) {
    return super.getSlice(axis, slicePos, 'dose', args)
  }

  /**
   * Make a dose contour plot of the given slice.
   *
   * @param {Object} slice The slice of the dose data.
   * @param {Object} transform The zoom transform of the plot.
   * @param {Object} svg The svg plot element.
   * @param {number[]} thresholds The contour thresholds.
   * @param {function} classNameFcn Returns the DOM class name.
   * @param {number} minDose The minimum dose to scale the dose profiles with.
   * @param {number} maxDose The maximum dose to scale the dose profiles with.
   */
  drawDose (slice, transform, svg, thresholds, classNameFcn, minDose, maxDose) {
    const colourFcn =
      d3.scaleSequentialSqrt(d3.interpolateViridis).domain([minDose, maxDose])
    const baseSlice = this.baseSlices[slice.axis]

    // Clear dose plot
    svg.selectAll('g').remove()

    // Draw contours
    var contours = d3
      .contours()
      .size([baseSlice.xVoxels, baseSlice.yVoxels])
      .smooth(false)
      .thresholds(thresholds)(slice.sliceData)
      .map(baseSlice.contourTransform)

    const contourPaths = svg
      .append('g')
      .attr('class', 'dose-contour')
      .attr('width', this.dimensions.width)
      .attr('height', this.dimensions.height)
      .attr('fill', 'none')
      .attr('stroke', '#fff')
      .attr('stroke-opacity', 1.0)
      .attr('stroke-width', 0.3)
      .selectAll('path')
      .data(contours)
      .join('path')
      .classed('contour-path', true)
      .attr('class', (d, i) => 'contour-path' + ' ' + classNameFcn(i))
      .attr('fill', (d) => colourFcn(d.value))
      .attr('fill-opacity', 1.0)
      .attr('d', d3.geoPath())

    // Get list of class names of hidden contours
    const hiddenContourClassList = this.getHiddenContourClassList()

    if (hiddenContourClassList.length > 0) {
      // Apply hidden class to hidden contours
      contourPaths
        .filter(hiddenContourClassList.join(','))
        .classed('hidden', true)
    }

    if (transform) {
      svg.select('g.dose-contour').attr('transform', transform.toString())
    }
  }

  /**
   * Get a class list of all the hidden dose contours.
   *
   * @returns {string[]}
   */
  getHiddenContourClassList () {
    const hiddenContourClassList = []
    this.legendSvg.selectAll('g.cell.hidden').each(function (d, i) {
      hiddenContourClassList[i] =
        '.' + d3.select(this).attr('class').split(' ')[1]
    })

    return hiddenContourClassList
  }

  /**
   * Get the dose value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getDataAtVoxelCoords (voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, 'dose') / this.data.maxDose
  }

  /**
   * Get the dose error value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getErrorAtVoxelCoords (voxelCoords) {
    return this.data.error ? super.getDataAtVoxelCoords(voxelCoords, 'error') : 0
  }
}

/** @class Volume represents the difference between two .3ddose files.  */
class DoseComparisonVolume extends Volume { // eslint-disable-line no-unused-vars
  /**
   * Creates an instance of a DoseComparisonVolume.
   *
   * @constructor
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   * @param {DoseVolume} doseVol1 The first dose volume to compare.
   * @param {DoseVolume} doseVol2 The second dose volume to compare.
   */
  // TODO: Extend DoseVolume instead of Volume to avoid duplicating functions
  constructor (fileName, dimensions, legendDimensions, doseVol1, doseVol2) {
    // Call the super class constructor
    super(fileName, dimensions, legendDimensions)
    this.addData(doseVol1, doseVol2)
  }

  /**
   * Adds data to the DoseComparisonVolume object.
   *
   * @param {DoseVolume} doseVol1 The first dose volume to compare.
   * @param {DoseVolume} doseVol2 The second dose volume to compare.
   */
  addData (doseVol1, doseVol2) {
    // Check if dimensions are compatible
    var compatibleDims = (voxArr1, voxArr2) => (
      Object.keys(voxArr1).every((axis) => {
        const len1 = voxArr1[axis].length
        const len2 = voxArr2[axis].length
        return (len1 === len2) && (voxArr1[axis][0] === voxArr2[axis][0]) && (voxArr1[axis][len1 - 1] === voxArr2[axis][len2 - 1])
      })
    )

    // First normalize the dose data to turn into a percentage
    const doseArr1 = doseVol1.data.dose.map(
      (doseVal) => doseVal / doseVol1.data.maxDose
    )
    const doseArr2 = doseVol2.data.dose.map(
      (doseVal) => doseVal / doseVol2.data.maxDose
    )

    // Initialize dose difference and error arrays
    var doseDiff = new Array(doseArr1.length)
    const error = new Array(doseArr1.length)

    // If both dose volumes have same dimensions
    if (compatibleDims(doseVol1.data.voxelArr, doseVol2.data.voxelArr)) {
      // Take the difference
      for (let i = 0; i < doseArr1.length; i++) {
        if (doseArr1[i] || doseArr2[i]) {
          doseDiff[i] = (doseArr1[i] || 0) - (doseArr2[i] || 0)
        }

        // Calculate the error for each
        error[i] = Math.sqrt(Math.pow(doseVol1.data.error[i], 2) * Math.pow(doseVol2.data.error[i], 2))
      }
    } else {
      var grid = { x: null, y: null, z: null }
      var voxelArr = { x: null, y: null, z: null }
      Object.keys(doseVol1.data.voxelArr).forEach((dim) => { grid[dim] = getVoxelCenter(doseVol1.data.voxelArr[dim]) })
      Object.keys(doseVol2.data.voxelArr).forEach((dim) => { voxelArr[dim] = getVoxelCenter(doseVol2.data.voxelArr[dim]) })

      const interpolatedDose = trilinearInterpolation(voxelArr, doseArr2, grid)
      doseDiff = interpolatedDose.map((interpDose, i) => (doseArr1[i] - interpDose))
    }

    // Set base slices
    this.baseSlices = doseVol1.baseSlices

    // Make new volume
    this.data = {
      ...doseVol1.data, // For voxelArr, voxelNumber, and voxelSize
      dose: doseDiff,
      error: error,
      maxDose: 1.0
    }

    // Max dose used for dose contour plot
    super.addColourScheme(d3.interpolateViridis, 1.0, -1.0)
  }

  /**
   * Get the dose difference value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getDataAtVoxelCoords (voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, 'dose')
  }

  /**
   * Get a slice of dose data for a given axis and slice index.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} sliceNum The number of the slice.
   * @param {Array} args Any arguments to be passed into getSlice.
   * @returns {Object}
   */
  getSlice (axis, slicePos, args) {
    return super.getSlice(axis, slicePos, 'dose', args)
  }

  /**
   * Make a dose contour plot of the given slice.
   *
   * @param {Object} slice The slice of the dose data.
   * @param {Object} transform The zoom transform of the plot.
   * @param {Object} svg The svg plot element.
   * @param {number[]} thresholds The contour thresholds.
   * @param {function} classNameFcn Returns the DOM class name.
   * @param {number} minDose The minimum dose to scale the dose profiles with.
   * @param {number} maxDose The maximum dose to scale the dose profiles with.
   */
  drawDose (slice, transform, svg, thresholds, classNameFcn, minDose, maxDose) {
    const colourFcn =
      d3.scaleSequentialSqrt(d3.interpolateViridis).domain([minDose, maxDose])
    const baseSlice = this.baseSlices[slice.axis]

    // Clear dose plot
    svg.selectAll('g').remove()

    // Draw contours
    var contours = d3
      .contours()
      .size([baseSlice.xVoxels, baseSlice.yVoxels])
      .smooth(false)
      .thresholds(thresholds)(slice.sliceData)
      .map(baseSlice.contourTransform)

    const contourPaths = svg
      .append('g')
      .attr('class', 'dose-contour')
      .attr('width', this.dimensions.width)
      .attr('height', this.dimensions.height)
      .attr('fill', 'none')
      .attr('stroke', '#fff')
      .attr('stroke-opacity', 1.0)
      .attr('stroke-width', 0.3)
      .selectAll('path')
      .data(contours)
      .join('path')
      .classed('contour-path', true)
      .attr('class', (d, i) => 'contour-path' + ' ' + classNameFcn(i))
      .attr('fill', (d) => colourFcn(d.value))
      .attr('fill-opacity', 1.0)
      .attr('d', d3.geoPath())

    // Get list of class names of hidden contours
    const hiddenContourClassList = this.getHiddenContourClassList()

    if (hiddenContourClassList.length > 0) {
      // Apply hidden class to hidden contours
      contourPaths
        .filter(hiddenContourClassList.join(','))
        .classed('hidden', true)
    }

    if (transform) {
      svg.select('g.dose-contour').attr('transform', transform.toString())
    }
  }

  /**
   * Get a class list of all the hidden dose contours.
   *
   * @returns {string[]}
   */
  getHiddenContourClassList () {
    const hiddenContourClassList = []
    this.legendSvg.selectAll('g.cell.hidden').each(function (d, i) {
      hiddenContourClassList[i] =
        '.' + d3.select(this).attr('class').split(' ')[1]
    })

    return hiddenContourClassList
  }

  /**
   * Get the dose error value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getErrorAtVoxelCoords (voxelCoords) {
    return this.data.error ? super.getDataAtVoxelCoords(voxelCoords, 'error') : 0
  }
}

/** @class Volume represents a .egsphant file.  */
class DensityVolume extends Volume { // eslint-disable-line no-unused-vars
  /**
   * Creates an instance of a DoseVolume.
   *
   * @constructor
   * @extends Volume
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   * @param {Object} data The data from parsing the file.
   */
  constructor (fileName, dimensions, legendDimensions, data, args) {
    super(fileName, dimensions, legendDimensions, args) // call the super class constructor
    this.addData(data, args)
  }

  /**
   * Adds data to the DensityVolume object.
   *
   * @param {Object} data The data from parsing the file.
   */
  addData (data, args) {
    this.data = data
    this.densityFormat = (args !== undefined) && (args.isDicom) ? d3.format('d') : d3.format('.2f')
    this.densityStep = (args !== undefined) && (args.isDicom) ? 1.0 : 0.01
    this.addColourScheme(this.data.maxDensity, this.data.minDensity)
    this.imageCache = { xy: new Array(data.voxelNumber.z), yz: new Array(data.voxelNumber.x), xz: new Array(data.voxelNumber.y) }
    this.createBaseSlices(data, 'density')
    this.cacheAllImages(data)
  }

  addColourScheme (maxVal, minVal) {
    this.colour = d3.scaleSqrt().domain([minVal, maxVal]).range([0, 255])
  }

  /**
   * Iterate through all density slices and cache them.
   *
   * @param {Object} data The data from parsing the file.
   */
  cacheAllImages (data) {
    // Get all data slices
    const dims = ['x', 'y', 'z']
    const axes = ['yz', 'xz', 'xy']
    const vol = this

    if (window.Worker) {
      // If web workers are supported, cache images in the background
      try {
        axes.forEach((axis, i) => {
          var cacheWorker = new Worker('cache-worker.mjs')

          const e = {
            axis: axis,
            data: this.data,
            dimensions: this.dimensions,
            voxArr: data.voxelArr[dims[i]].slice(),
            maxVal: this.data.maxDensity,
            minVal: this.data.minDensity
          }

          function handleMessage (e) {
            console.log('Message recieved from worker')
            vol.imageCache[axis] = e.data

            // Remove event listener
            cacheWorker.removeEventListener('message', handleMessage)
            cacheWorker.terminate()
          }

          cacheWorker.addEventListener('message', handleMessage)
          cacheWorker.postMessage(e)
        })
      } catch (err) {
        console.log('Web workers not available.')
      }
    } else {
      console.log('Your browser doesn\'t support web workers.')
    }
  }

  /**
   * Get a slice of density data for a given axis and slice index.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} sliceNum The number of the slice.
   * @returns {Object}
   */
  getSlice (axis, slicePos) {
    return super.getSlice(axis, slicePos, 'density')
  }

  /**
   * Make a density plot of the given slice.
   *
   * @param {Object} slice The slice of the density data.
   * @param {Object} [transform] The zoom transform of the plot.
   * @param {Object} svg The svg plot element.
   * @param {number} minDensityVar The minimum density to scale the image with.
   * @param {number} maxDensityVar The maximum density to scale the image with.
   * @returns {Object}
   */
  drawDensity (slice, transform, svg, minDensityVar, maxDensityVar) {
    // Get the canvas and context in the webpage
    const baseSlice = this.baseSlices[slice.axis]
    const imgCanvas = svg.node()
    const imgContext = imgCanvas.getContext('2d', { alpha: false })
    const imageData = this.getImageData(slice)

    // If the min and max density have been changed, apply colour map
    if ((minDensityVar !== this.data.minDensity) || (maxDensityVar !== this.data.maxDensity)) {
      const colourMap = d3.scaleSqrt().domain([minDensityVar, maxDensityVar]).range([0, 255])
      const imageArray = imageData.data
      let val

      // Change colour mapping
      for (var i = 0; i < imageArray.length; i += 4) {
        val = colourMap(this.colour.invert(imageArray[i]))

        imageArray[i] = val
        imageArray[i + 1] = val
        imageArray[i + 2] = val
        // Alpha channel is ignored
      }
    }

    // Create the voxel canvas to draw the slice onto
    const canvas = document.createElement('canvas')
    canvas.width = baseSlice.xVoxels
    canvas.height = baseSlice.yVoxels
    const context = canvas.getContext('2d')

    // Draw the image data onto the voxel canvas
    context.putImageData(imageData, 0, 0)

    // Draw the voxel canvas onto the image canvas and zoom if needed
    imgContext.clearRect(0, 0, this.dimensions.width, this.dimensions.height)
    imgContext.save()
    if (transform) {
      imgContext.translate(transform.x, transform.y)
      imgContext.scale(transform.k, transform.k)
    }

    imgContext.drawImage(canvas, 0, 0, baseSlice.xVoxels, baseSlice.yVoxels, baseSlice.dxDraw, baseSlice.dyDraw, baseSlice.dWidthDraw, baseSlice.dHeightDraw)
    imgContext.restore()

    // TODO: Add event listener?
    var image = new Image()
    image.src = canvas.toDataURL()
    return image
  }

  /**
   * Create the data image of the slice and return the image URL
   *
   * @param {Object} slice The slice of the density data.
   * @returns {String}
   */
  getImageData (slice) {
    // Create new canvas element and set the dimensions
    var canvas = document.createElement('canvas')
    const xVoxels = this.baseSlices[slice.axis].xVoxels
    const yVoxels = this.baseSlices[slice.axis].yVoxels
    canvas.width = xVoxels
    canvas.height = yVoxels

    // Get the canvas context
    const context = canvas.getContext('2d')

    // Create the image data
    var imageData = context.createImageData(xVoxels, yVoxels)

    if (this.imageCache[slice.axis] !== undefined && this.imageCache[slice.axis][slice.sliceNum] !== undefined) {
      imageData.data.set(this.imageCache[slice.axis][slice.sliceNum])
    } else {
      var j = 0
      for (let i = 0; i < slice.sliceData.length; i++) {
        const val = this.colour(slice.sliceData[i])

        if (val !== null) {
          // Modify pixel data
          imageData.data[j++] = val // R value
          imageData.data[j++] = val // G value
          imageData.data[j++] = val // B value
          imageData.data[j++] = 255 // A value
        }
      }

      this.imageCache[slice.axis][slice.sliceNum] = imageData.data
    }

    // Draw the image data onto the voxel canvas and scale it
    context.putImageData(imageData, 0, 0)
    if (slice.axis !== 'xy') {
      context.scale(1, -1)
      context.drawImage(canvas, 0, -1 * yVoxels)
    }
    imageData = context.getImageData(0, 0, xVoxels, yVoxels)

    return imageData
  }

  /**
   * Get the density value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getDataAtVoxelCoords (voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, 'density')
  }

  /**
   * Get the material at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {string}
   */
  getMaterialAtVoxelCoords (voxelCoords) {
    if (this.data.material) {
      const [x, y, z] = voxelCoords
      const address =
        z * (this.data.voxelNumber.x * this.data.voxelNumber.y) +
        y * this.data.voxelNumber.x +
        x

      const divisor = this.data.voxelNumber.x
      const materialNumber = parseInt(
        this.data.material[Math.floor(address / divisor)][address % divisor]
      )
      return this.data.materialList[materialNumber - 1]
    }
    return ''
  }
}

/**
 * Interpolate from a source grid and data to a target grid.
 *
 * @param {number[]} voxelArr The voxel center positions of the source data
 * @param {number[]} data  The source data values
 * @param {number[]} grid2 The positions of the data to interpolate to
 * @returns {number[]}
 */
// TODO: Simplify
function trilinearInterpolation (voxelArr, data, grid) {
  const [xScale, yScale, zScale] = ['x', 'y', 'z'].map((dim) => {
    const domain = [voxelArr[dim][0], voxelArr[dim][voxelArr[dim].length - 1]]
    return d3
      .scaleQuantize()
      .domain(domain)
      .range(d3.range(0, voxelArr[dim].length - 1, 1))
  })

  let address
  const xVoxels = voxelArr.x.length
  const yVoxels = voxelArr.y.length

  // Initialize the interpolation array
  const c = new Array(grid.x.length * grid.y.length * grid.z.length)

  for (let xi = 0; xi < grid.x.length; xi++) {
    for (let yi = 0; yi < grid.y.length; yi++) {
      for (let zi = 0; zi < grid.z.length; zi++) {
        // Get the nearest voxel indices of the target location
        const [i, j, k] = [xScale(grid.x[xi]), yScale(grid.y[yi]), zScale(grid.z[zi])]

        // Get the space between the source grid and target location
        const [xd, yd, zd] = [(grid.x[xi] - voxelArr.x[i]) / (voxelArr.x[i + 1] - voxelArr.x[i]),
          (grid.y[yi] - voxelArr.y[j]) / (voxelArr.y[j + 1] - voxelArr.y[j]),
          (grid.z[zi] - voxelArr.z[k]) / (voxelArr.z[k + 1] - voxelArr.z[k])]

        // Continue if at the end of the voxel array or distance difference is negative
        if (i >= (voxelArr.x.length - 2) || j >= (voxelArr.y.length - 2) || k >= (voxelArr.z.length - 2) || xd < 0 || yd < 0 || zd < 0) {
          continue
        }

        // Get the values for the 8 nearest points
        const c000 = data[i + xVoxels * (j + k * yVoxels)]
        const c001 = data[i + xVoxels * (j + (k + 1) * yVoxels)]
        const c010 = data[i + xVoxels * ((j + 1) + k * yVoxels)]
        const c011 = data[i + xVoxels * ((j + 1) + (k + 1) * yVoxels)]
        const c100 = data[(i + 1) + xVoxels * (j + k * yVoxels)]
        const c101 = data[(i + 1) + xVoxels * (j + (k + 1) * yVoxels)]
        const c110 = data[(i + 1) + xVoxels * ((j + 1) + k * yVoxels)]
        const c111 = data[(i + 1) + xVoxels * ((j + 1) + (k + 1) * yVoxels)]

        // Interpolate along x
        const c00 = c000 * (1 - xd) + c100 * xd
        const c01 = c001 * (1 - xd) + c101 * xd
        const c10 = c010 * (1 - xd) + c110 * xd
        const c11 = c011 * (1 - xd) + c111 * xd

        // Interpolate along y
        const c0 = c00 * (1 - yd) + c10 * yd
        const c1 = c01 * (1 - yd) + c11 * yd

        // Interpolate along z
        address = xi + grid.x.length * (yi + zi * grid.y.length)

        // Set the interpolated value
        c[address] = (c0 * (1 - zd)) + (c1 * zd)
      }
    }
  }
  return c
}

function getVoxelCenter (voxArr) {
  var position = voxArr
  position = position.map((val, i) => {
    return val + (voxArr[i + 1] - val) / 2
  })
  position.pop()

  return position
}

// export { DensityVolume, DoseComparisonVolume, DoseVolume, StructureSetVolume }
