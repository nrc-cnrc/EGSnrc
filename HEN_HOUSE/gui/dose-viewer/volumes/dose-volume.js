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
     * Get the dose value at the given coordinates.
     *
     * @param {number[]} voxelCoords The voxel position of the data.
     * @returns {number}
     */
  getDataAtVoxelCoords (voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, 'dose')
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

  /**
     * Get the dose value at the given world coordinates.
     *
     * @param {number[]} coords The world position of the data.
     * @returns {number}
     */
  getDataAtPosition (coords) {
    return super.getDataAtPosition(coords, 'dose')
  }
}

// export { DoseVolume }
