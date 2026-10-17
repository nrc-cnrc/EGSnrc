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
    var interpolatedDose

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

      interpolatedDose = trilinearInterpolation(voxelArr, doseArr2, grid)
      doseDiff = interpolatedDose.map((interpolatedDose, i) => (doseArr1[i] - interpolatedDose))
    }

    // Set base slices
    this.baseSlices = doseVol1.baseSlices

    // Make new volume
    this.data = {
      ...doseVol1.data, // For voxelArr, voxelNumber, and voxelSize
      dose1: doseArr1,
      dose2: interpolatedDose || doseArr2,
      dose: doseDiff,
      error: error,
      maxDose: 1.0,
      units: 'RELATIVE'
    }

    super.buildWorldToVoxelScales(this.data)

    // Max dose used for dose contour plot
    super.addColourScheme(d3.interpolateViridis, 1.0, -1.0)
  }

  /**
     * Normalize the second dose volume according to the given normFactor and
     * update the Dose Comparison Volume data.
     *
     * @param {number} normFactor The factor to normalize the second dose volume by.
     */
  normalizeDose (normFactor) {
    // Initialize dose difference array
    var doseDiff = new Array(this.data.dose1.length)

    // Take the difference
    for (let i = 0; i < this.data.dose1.length; i++) {
      if (this.data.dose1[i] || this.data.dose2[i]) {
        doseDiff[i] = (this.data.dose1[i] || 0) - (this.data.dose2[i] * normFactor || 0)
      }
    }

    // Adjust data
    // We can adjust the actual data of the volume because a new Dose Comparison
    // Volume is created each time it is selected, so we are not messing with
    // the raw data
    this.data = {
      ...this.data, // For voxelArr, voxelNumber, and voxelSize
      dose: doseDiff
    }

    // Clear the slice cache
    this.sliceCache = { xy: {}, yz: {}, xz: {} }
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
    svg.selectAll('g.dose-contour').remove()

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

    svg.selectAll('g.dose-contour').lower()
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

// export { DoseComparisonVolume }
