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
#  Author:          Elise Badun, 2021
#
#  Contributors:
#
###############################################################################
*/

// definitions for StandardJS formatter
/* global d3 */

// REMOVE THESE GLOBAL IMPORTS ONCE MODULES RE-IMPLEMENTED
/* global Volume */

// import { Volume } from './volume.js'

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
  // TODO: Speed this function up
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
    for (let idx = 0; idx < data.ROIs.length; idx++) {
      ROI = data.ROIs[idx]

      // If the ROI has contour data
      if (ROI.ContourSequence !== undefined) {
        const contourData = []
        const rangeVals = { x: [], y: [], z: [] }
        // const contourGeometricType = new Set()
        var prevZ = 0
        var values, sliceContourData, z

        // For each slice of the contour data
        ROI.ContourSequence.forEach((sequence) => {
          values = sequence.ContourData.split('\\')
          sliceContourData = []

          // The z value should be constant for each slice
          z = toCm(values[2])

          // For each coordinate in the slice
          for (let i = 0; i < values.length; i += 3) {
            updateRange(values.slice(i, i + 3).map((val) => toCm(val)), rangeVals)
            sliceContourData.push([toCm(values[i]), toCm(values[i + 1])])
          }

          // Add sliceContourData to contourData list
          if (prevZ === undefined || prevZ !== z) {
            contourData.push({ z: z, vals: [sliceContourData] })
            prevZ = z
          } else {
            contourData[contourData.length - 1].vals.push(sliceContourData)
          }
          // contourGeometricType.add(sequence.ContourGeometricType)
        })

        // Find the voxel positions of the grid to overlay the ROI data on
        // Increments for z array is the space between the slices
        const increments = { x: 0.2, y: 0.2, z: (rangeVals.z[1] - rangeVals.z[0]) / contourData.length }
        const voxelArr = {}
        Object.entries(rangeVals).forEach(([dim, range]) => {
          const divider = 1 / increments[dim]
          const min = Math.floor(range[0] * divider) / divider
          const max = Math.ceil(range[1] * divider) / divider
          const items = Math.round((max - min) / increments[dim])
          voxelArr[dim] = [...Array(items + 1)].map((x, y) => min + increments[dim] * y)
        })
        // Replace the z voxel array with the actual z values in the contour data
        voxelArr.z = contourData.map((e) => e.z)

        // Build the voxel information for the ROI array
        const voxelNumber = {}
        const scales = {}
        Object.entries(voxelArr).map(([dim, arr]) => {
          voxelNumber[dim] = arr.length
          scales[dim] = d3.scaleQuantize().domain([arr[0] - increments[dim] / 2, arr[arr.length - 1] + increments[dim] / 2]).range(d3.range(0, arr.length, 1))
        })

        // Build the ROI array
        var ROIarray = this.makeROIArray(contourData, voxelNumber, voxelArr, scales)

        ROIoutlines.push({
          label: ROI.ROIName || ROI.ROIObservationLabel,
          colour: 'rgb(' + ROI.ROIDisplayColor.replaceAll('\\', ', ') + ')',
          // contourGeometricType: contourGeometricType,
          voxelNumber: voxelNumber,
          scales: scales,
          ROIarray: ROIarray,
          voxelArr: voxelArr
        })
      }
    }
    return ROIoutlines
  }

  /**
     * Turns the polygon data into an array
     *
     * @param {Object} contourData The contour data of an ROI at each z position
     * @param {Object} voxelNumber The number of voxels for each axis in the ROI array
     * @param {Object} voxelArr The center position of voxels for each axis in the
     * ROI array
     * @param {Object} scales The d3 scale from position to index of the voxel array
     */
  makeROIArray (contourData, voxelNumber, voxelArr, scales) {
    // The constructor function for the line object
    function Line (start, end) {
      this.x0 = start[0]
      this.x1 = end[0]
      this.y0 = start[1]
      this.y1 = end[1]
      this.m = (this.y1 - this.y0) / (this.x1 - this.x0)

      this.getX = function (y) {
        return 1 / this.m * (y - this.y0) + this.x0
      }

      this.isValidY = function (y) {
        if (y >= this.y0 && y < this.y1) {
          return true
        }
        if (y >= this.y1 && y < this.y0) {
          return true
        }

        return false
      }
    }

    // Returns the y range of the polygon
    const polygonYRange = (polygon) => {
      var minY = polygon[0][1]; var maxY = polygon[0][1]

      polygon.forEach((point) => {
        if (point[1] < minY) minY = point[1]
        else if (point[1] > maxY) maxY = point[1]
      })
      return [minY, maxY]
    }

    // Returns the x position of the intersection of the lines with the y position
    const getMeetPoints = (y, lines) => {
      var meet = []

      lines.forEach((line) => {
        if (line.isValidY(y)) {
          meet.push(line.getX(y))
        }
      })

      return meet.sort((a, b) => (a - b))
    }

    // Initialize the ROI array
    const ROIarray = new Array(voxelNumber.x * voxelNumber.y * voxelNumber.z)

    // Build the array that represents the ROI polygons as a matrix mask
    for (let k = 0; k < voxelNumber.z; k++) {
      // For each contour slice
      contourData[k].vals.forEach((polygon) => {
        var [minY, maxY] = polygonYRange(polygon)

        // Build a list of edges between each vertex in the polygon
        var lines = []
        for (let i = 1; i < polygon.length; i++) {
          lines.push(new Line(polygon[i - 1], polygon[i]))
        }
        lines.push(new Line(polygon[polygon.length - 1], polygon[0]))

        // Move the scan line step by step from the smallest to the biggest
        // y-coordinate (voxArr.y is already sorted)
        voxelArr.y.forEach((yVal, j) => {
          var i0, i1

          if ((minY <= yVal) && (maxY >= yVal)) {
            // Get the x coordinate of polygon intersections with the scan line
            var meetPoints = getMeetPoints(yVal, lines)
            // For each pair of intersections
            for (let idx = 1; idx < meetPoints.length; idx += 2) {
              i0 = scales.x(meetPoints[idx - 1])
              i1 = scales.x(meetPoints[idx])
              // Fill in the ROI array between the points
              for (let i = i0; i <= i1; i++) {
                ROIarray[i + voxelNumber.x * (j + k * voxelNumber.y)] = 1
              }
            }
          }
        })
      })
    }
    return ROIarray
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

    var getArrRange = (arr) => ([arr[0], [arr[arr.length - 1]]])
    var getXYRange = (axis, voxArr) => ([getArrRange(voxArr[axis[0]]), getArrRange(voxArr[axis[1]])])

    // For each ROI in the range, calculate the data slice that intersects slicePos
    const slices = ROIOutlinesInRange.map((ROIOutline) => {
      const sliceNum = axis === 'xy' ? ROIOutline.scales.z(slicePos) : axis === 'yz' ? ROIOutline.scales.x(slicePos) : ROIOutline.scales.y(slicePos)
      const [xVoxels, yVoxels] = [ROIOutline.voxelNumber[axis[0]], ROIOutline.voxelNumber[axis[1]]]
      const [xRange, yRange] = getXYRange(axis, ROIOutline.voxelArr)

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
    var toCSSClass = (className) => className.replace(/[|~ ! @ $ % ^ & * ( ) + = , . / ' ; : " ? > < \[ \] \ \{ \} | ]/g, '') // eslint-disable-line no-useless-escape

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
        .thresholds([1])
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

  /**
     * Calculates the dose volume histogram for each of the ROIs.
     *
     * @param {DoseVolume} doseVolume The dose volume to use for DVH calculations.
     * @param {number} nThresholds    The number of thresholds/bins to use for
     * histogram calculations.
     */
  calculateDVH (doseVolume, nThresholds = 250) {
    // Convert index in 1D array to x, y, z coords
    var to3d = (idx, voxelNumber) => {
      const k = Math.floor(idx / (voxelNumber.y * voxelNumber.x))
      idx -= (k * voxelNumber.y * voxelNumber.x) // = idx % voxelNumber.y
      const j = Math.floor(idx / voxelNumber.x)
      const i = idx % voxelNumber.x
      return [i, j, k]
    }

    // Convert x, y, z coords to world coords
    var toPos = ([i, j, k], voxelArr) => [voxelArr.x[i], voxelArr.y[j], voxelArr.z[k]]

    // Set the parameters for the histogram
    var histogram = d3.histogram()
      .domain([0, doseVolume.data.maxDose])
      .thresholds(nThresholds)

    // Initialize ROIHistograms
    const ROIHistograms = []

    // For each ROI
    this.ROIoutlines.forEach((ROIOutline) => {
      // Initialize histogramList
      const doseList = []
      let coords, pos, dose
      var nVals = 0

      // Go through each element in the ROI array
      ROIOutline.ROIarray.forEach((val, i) => {
        coords = to3d(i, ROIOutline.voxelNumber)
        pos = toPos(coords, ROIOutline.voxelArr)
        dose = doseVolume.getDataAtPosition(pos) || 0
        doseList.push(dose)
        nVals++
      })

      // Create and add ROI histogram to list
      var bins = histogram(doseList)
      var binLength = bins.map((binList) => binList.length / nVals)
      ROIHistograms.push(binLength)
    })
    return ROIHistograms
  }
}

// export { StructureSetVolume }
