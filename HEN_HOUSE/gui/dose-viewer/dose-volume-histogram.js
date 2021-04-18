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
/* global DVH_DIMENSIONS */

// import { DOSE_PROFILE_DIMENSIONS } from './index.js'
/** @class DoseVolumeHistogram contains all information to build a dose volume
 * histogram from a given structure set volume and dose volume. */
class DoseVolumeHistogram { // eslint-disable-line no-unused-vars
  /**
     * Creates an instance of DoseVolumeHistogram.
     *
     * @constructor
     * @param {Object} dimensions The pixel dimensions of the dose volume histogram plot.
     * @param {Object} parentDiv  The svg that holds the dose volume histogram.
     * @param {String} id         The id of the dvh (used mostly for bounding boxes).
     */
  constructor (dimensions, parentDiv, id, volumeViewer) {
    this.dimensions = DVH_DIMENSIONS
    this.id = id
    this.volumeViewer = volumeViewer

    // Create main svg elements
    // TODO: Simplify to just one svg element???
    this.parentSvg = this.buildParentSvg(DVH_DIMENSIONS, parentDiv)
    this.svg = this.buildPlottingSvg(DVH_DIMENSIONS, this.parentSvg, id)

    // Set up zoom object
    this.zoomObj = d3
      .zoom()
      .extent([
        [0, 0],
        [dimensions.width, dimensions.height]
      ])
      .scaleExtent([1, 8])
      .on('zoom', () => this.zoomedDVH(d3.event.transform))

    // Enable zooming
    this.svg.select('rect.bounding-box').call(this.zoomObj)

    // Initialize all properties
    this.xDoseScale = null
    this.yVolumeScale = null
    this.maxDose = null
    this.data = null
  }

  /**
   * Build the parent svg object.
   *
   * @param {Object} dimensions The pixel dimensions of the dose volume histogram plot.
   * @param {Object} parentDiv  The div that holds the dose volume histogram.
   * @returns {Object}
   */
  buildParentSvg (dimensions, parentDiv) {
    // Initializing svgs for dose profile plots
    const parentSvg = parentDiv
      .append('svg')
      .attr('width', dimensions.fullWidth)
      .attr('height', dimensions.fullHeight)
      .style('display', 'none')

    return parentSvg
  }

  /**
   * Build the main g object for plotting.
   *
   * @param {Object} dimensions The pixel dimensions of the dose volume histogram  plot.
   * @param {Object} parentSvg  The parent svg to hold the plotting svg.
   * @param {String} id         The id of the dose volume histogram (used mostly for bounding boxes).
   * @returns {Object}
   */
  buildPlottingSvg (dimensions, parentSvg, id) {
    // Adding the plotting area to the parent svg
    const svg = parentSvg
      .append('g')
      .style(
        'transform',
        'translate(' +
        dimensions.margin.left +
        'px' +
        ',' +
        dimensions.margin.top +
        'px' +
        ')'
      )
      .classed('dvh-plot', true)

    // Create box to capture mouse events
    svg
      .append('rect')
      .attr('width', dimensions.width)
      .attr('height', dimensions.height)
      .attr('fill', 'white')
      .attr('class', 'bounding-box')

    // Create clip path to bound output after zooming
    svg
      .append('defs')
      .append('clipPath')
      .attr('id', 'clip-' + id)
      .append('rect')
      .attr('width', dimensions.width + 4)
      .attr('height', dimensions.height + 4)
      .style(
        'transform',
        'translate(-2px, -2px)'
      )

    return svg
  }

  /**
 * The zoom callback function for DVH plots.
 *
 * @param {Object} transform The zoom transform object.
 */
  zoomedDVH (transform) {
    this.svg
      .selectAll('path.roi-outline')
      .attr('transform', transform.toString())

    // Create new scale objects based on event
    var newxDoseScale = transform.rescaleX(this.xDoseScale)
    var newyVolumeScale = transform.rescaleY(this.yVolumeScale)

    // Create and append x and y axes
    const xAxis = d3
      .axisBottom()
      .scale(newxDoseScale)
      .tickFormat(d3.format('.0f'))
      .tickSize(-this.dimensions.height)

    const yAxis = d3
      .axisLeft()
      .scale(newyVolumeScale)
      .ticks(6)
      .tickFormat(d3.format('.0%'))
      .tickSize(-this.dimensions.width)

    // Update axes
    this.svg
      .select('g.dvh-x-axis')
      .call(xAxis)

    this.svg
      .select('g.dvh-y-axis')
      .call(yAxis)
  }

  /**
     * Resets the zoom of the DVH plot.
     */
  resetZoomTransform () {
    this.svg
      .select('rect.bounding-box')
      .call(this.zoomObj.transform, d3.zoomIdentity.scale(1))
  }

  /**
   * Set the dose profile data for the current slice
   *
   * @param {DoseVolume} structureSetVol The structure set volume object.
   * @param {DoseVolume} doseVol         The dose volume object.
   */
  setDVHData (structureSetVol, doseVol) {
    const ROIHistograms = structureSetVol.calculateDVH(doseVol)
    const maxDose = doseVol.data.maxDose // Max dose in cGy
    // Calculate the cumulative dose for each ROI
    this.data = ROIHistograms.map((histogram, idx) => {
      var cumSum = 0
      var cumulativeHistogram = histogram
        .reverse()
        .map((val) => {
          cumSum += val
          return cumSum
        })
        .reverse()
        .map((val, i) => ({ x: maxDose * (i / histogram.length), y: val }))

      return {
        key: structureSetVol.ROIoutlines[idx].label,
        colour: structureSetVol.ROIoutlines[idx].colour,
        values: cumulativeHistogram
      }
    })

    // Build the scales
    this.xDoseScale = d3.scaleLinear().domain([0, maxDose]).range([0, this.dimensions.width])
    this.yVolumeScale = d3.scaleLinear().domain([0, 1]).range([this.dimensions.height, 0])
    this.maxDose = maxDose
  }

  /**
   * Set up and plot the x and y axes for the DVH.
   */
  plotAxes () {
    // Clear existing axes and labels
    this.svg.selectAll('g.dvh-x-axis').remove()
    this.svg.selectAll('g.dvh-y-axis').remove()

    // Create and append x and y axes
    const xAxis = d3
      .axisBottom()
      .scale(this.xDoseScale)
      .tickFormat(d3.format('.0f'))
      .tickSize(-this.dimensions.height)

    const yAxis = d3
      .axisLeft()
      .scale(this.yVolumeScale)
      .ticks(6)
      .tickFormat(d3.format('.0%'))
      .tickSize(-this.dimensions.width)

    this.svg
      .append('g')
      .attr('class', 'dvh-x-axis')
      .attr('transform', 'translate(0,' + this.dimensions.height + ')')
      .call(xAxis)

    this.svg.append('g').attr('class', 'dvh-y-axis').call(yAxis)

    // Label for dose x axis
    this.svg
      .append('text')
      .attr('class', 'dvh-x-axis')
      .classed('dvh-axis-label', true)
      .attr(
        'transform',
        'translate(' +
          this.dimensions.width / 2 +
          ' ,' +
          (this.dimensions.height + this.dimensions.margin.top - 5) +
          ')'
      )
      .style('text-anchor', 'middle')
      .text('Dose (cGy)')

    // Label for volume y axis
    this.svg
      .append('text')
      .attr('class', 'dvh-y-axis')
      .classed('dvh-axis-label', true)
      .attr('transform', 'rotate(-90)')
      .attr(
        'transform',
        'translate(' +
          (15 - this.dimensions.margin.left) +
          ' ,' +
          this.dimensions.height / 2 +
          ') rotate(-90)'
      )
      .style('text-anchor', 'middle')
      .text('% Volume')
  }

  /**
   * Create the plot axes and plotting area.
   */
  createPlots () {
    // Plot the x and y axes
    this.plotAxes()

    // TODO: Is there a simpler way to clip the plot? clip-path: margin-box; ???
    this.svg
      .append('g')
      .attr('class', 'plotting-area')
      // .attr('clip-path', 'margin-box')
      .attr(
        'clip-path',
        this.svg.select('clipPath').node()
          ? 'url(#' + this.svg.select('clipPath').node().id + ')'
          : ''
      )
      .attr('fill', 'none')
      .attr('width', this.dimensions.width)
      .attr('height', this.dimensions.height)
  }

  /**
   * Plot the DVH data.
   *
   * @param {number} doseNorm The dose normalization factor.
   */
  // TODO: Add tooltip
  plotDVH (doseNorm) {
    // Function to convert ROI label to a valid CSS class
    const toCSSClass = (className) => className.replace(/[|~ ! @ $ % ^ & * ( ) + = , . / ' ; : " ? > < \[ \] \ \{ \} | ]/g, '') // eslint-disable-line no-useless-escape

    // A list of ROI classes that should be hidden
    const hiddenClassList = this.volumeViewer.getHiddenClassList(this.volumeViewer.ROILegendSvg)

    // Create the dose line
    const line = d3
      .line()
      .x((d) => this.xDoseScale(d.x))
      .y((d) => this.yVolumeScale(d.y))

    // Normalize the data if needed
    const data = doseNorm ? this.data.map((item) => ({ ...item, values: item.values.map((val) => ({ x: val.x * doseNorm, y: val.y })) })) : this.data

    if (doseNorm) {
      // Update the x axis
      this.xDoseScale = this.xDoseScale.domain([0, this.maxDose * doseNorm])

      const newXAxis = d3
        .axisBottom()
        .scale(this.xDoseScale)
        .tickFormat(d3.format('.0f'))
        .tickSize(-this.dimensions.height)

      this.svg.selectAll('g.dvh-x-axis').call(newXAxis)
    }

    // Clear the plotting area
    this.svg.select('g.plotting-area').selectAll('path').remove()

    // Plot the DVH lines
    const DVHLines = this.svg.select('g.plotting-area').selectAll('.line')
      .data(data)
      .enter()
      .append('path')
      .attr('fill', 'none')
      .attr('stroke', (d) => d.colour)
      .attr('stroke-width', 1.5)
      .attr('stroke-linejoin', 'round')
      .attr('stroke-linecap', 'round')
      .attr('class', (d) => 'roi-outline' + ' ' + toCSSClass(d.key))
      .attr('d', (d) => line(d.values))

    // Apply hidden class to hidden lines
    if (hiddenClassList.length > 0) {
      DVHLines.filter(hiddenClassList.join(','))
        .classed('hidden', true)
    }
  }
}
