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
/* global doseProfileAxis */
// import { getZoom, zoomedDoseProfile } from './zoom.js'
// import { DOSE_PROFILE_DIMENSIONS } from './index.js'

/** @class DoseProfile contains all information to build a dose profile at a line through a dose volume. */
class DoseProfile {
  /**
   * Creates an instance of DoseProfile.
   *
   * @constructor
   * @param {Object} dimensions The pixel dimensions of the dose profile plot.
   * @param {Object} parentDiv  The svg that holds the dose profile.
   * @param {String} id         The id of the dose profile (used mostly for bounding boxes).
   */
  constructor(dimensions, parentDiv, id) {
    this.dimensions = dimensions
    this.id = id

    // Create main svg elements
    this.buildSvg(dimensions, parentDiv, id)

    // Set up zoom object
    this.zoomObj = getZoom(
      dimensions.width,
      dimensions.height,
      zoomedDoseProfile,
      [this]
    )

    // Enable zooming
    this.svg.select('rect.bounding-box').call(this.zoomObj)

    // Check if the plot density checkbox is selected
    this.densityChecked = () =>
      d3.select("input[name='density-profile-checkbox']").node().checked

    // Initialize all properties
    this.xScale = null
    this.yDoseScale = null
    this.yDensityScale = null
    this.transform = null
    this.data = null
    this.doseVol = null
    this.yTicks = 6
    this.profileDim = null
  }

  /**
   * Set the transform variable used for zooming.
   */
  set zoomTransform(val) {
    this.transform = val
  }

  /**
   * Get the transform variable used for zooming.
   */
  get zoomTransform() {
    return this.transform
  }

  /**
   * Build the parent svg and main g objects for plotting.
   *
   * @param {Object} dimensions The pixel dimensions of the dose profile plot.
   * @param {Object} parentDiv  The svg that holds the dose profile.
   * @param {String} id         The id of the dose profile (used mostly for bounding boxes).
   */
  buildSvg(dimensions, parentDiv, id) {
    // Initializing svgs for dose profile plots
    this.parentSvg = parentDiv
      .append('svg')
      .attr('width', DOSE_PROFILE_DIMENSIONS.fullWidth)
      .attr('height', DOSE_PROFILE_DIMENSIONS.fullHeight)
      .style('display', 'none')

    this.svg = this.parentSvg
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
      .classed('dose-profile-plot', true)

    // Create box to capture mouse events
    this.svg
      .append('rect')
      .attr('width', dimensions.width)
      .attr('height', dimensions.height)
      .attr('fill', 'white')
      .attr('class', 'bounding-box')

    // Create clip path to bound output after zooming
    this.svg
      .append('defs')
      .append('clipPath')
      .attr('id', 'clip-' + id)
      .append('rect')
      .attr('width', dimensions.width)
      .attr('height', dimensions.height)
  }

  /**
   * Initializes the zoom of the dose profile plot using functions from the zoom file.
   */
  initializeZoom() {
    // Zooming for dose profile
    doseProfileAxis.zoomObj = getZoom(
      DOSE_PROFILE_DIMENSIONS.width,
      DOSE_PROFILE_DIMENSIONS.height,
      zoomedDoseProfile,
      [doseProfileAxis]
    )

    // Enable zooming
    doseProfileAxis.svg
      .select('rect.bounding-box')
      .call(doseProfileAxis.zoomObj)
  }

  /**
   * Resets the zoom of the dose profile plot.
   */
  resetZoomTransform() {
    this.svg
      .select('rect.bounding-box')
      .call(this.zoomObj.transform, d3.zoomIdentity.scale(1))
  }

  /**
   * Set the dose profile data for the current slice
   *
   * @param {DoseVolume} doseVol        The dose volume object.
   * @param {DensityVolume} densityVol  The density volume object (fine if undefined, only used if density checkbox is checked).
   * @param {String} profileDim         The dimension (x, y, z) of the dose profile.
   * @param {number[]} coords              The voxel position of the line through the volumes.
   */
  setDoseProfileData(doseVol, densityVol, profileDim, coords) {
    const [dim1, dim2, dim3] =
      profileDim === 'x'
        ? ['x', 'y', 'z']
        : profileDim === 'y'
          ? ['y', 'x', 'z']
          : ['z', 'x', 'y']

    const totalSlices = parseInt(doseVol.data.voxelNumber[dim1])
    const position = doseVol.data.voxelArr[dim1].slice()
    const xVoxels = parseInt(doseVol.data.voxelNumber[dim2])
    const yVoxels = parseInt(doseVol.data.voxelNumber[dim3])

    // Process position to get centre voxel position rather than boundaries
    position.map((val, i) => {
      return val + (position[i + 1] - val) / 2
    })
    position.pop()

    const doseProfileData = new Array(totalSlices)
    const plotDensity = this.densityChecked() && densityVol

    for (let i = 0; i < totalSlices; i++) {
      let address
      if (profileDim === 'z') {
        address = coords[0] + xVoxels * (coords[1] + i * yVoxels)
      } else if (profileDim === 'x') {
        address =
          i +
          parseInt(doseVol.data.voxelNumber.x) *
          (coords[0] + coords[1] * xVoxels)
      } else if (profileDim === 'y') {
        address =
          coords[0] +
          xVoxels * (i + coords[1] * parseInt(doseVol.data.voxelNumber.y))
      }

      if (plotDensity) {
        doseProfileData[i] = {
          position: position[i],
          value: doseVol.data.dose[address] || 0,
          err: doseVol.data.error ? doseVol.data.error[address] || 0 : 0
          // density: densityVol.data.density[address]
          // TODO: Pass in the address for the density coordinates
        }
      } else {
        doseProfileData[i] = {
          position: position[i],
          value: doseVol.data.dose[address] || 0,
          err: doseVol.data.error ? doseVol.data.error[address] || 0 : 0
        }
      }
    }
    this.data = doseProfileData
    this.doseVol = doseVol
    this.profileDim = profileDim
  }

  /**
   * Set the dose scales based on the loaded data.
   * */
  setDoseScales() {
    const [minPos, maxPos] = [
      this.data[0].position,
      this.data[this.data.length - 1].position
    ]

    // Create x and y scale
    this.xScale = d3
      .scaleLinear()
      .domain(minPos < maxPos ? [minPos, maxPos] : [maxPos, minPos])
      .range(
        minPos < maxPos
          ? [0, this.dimensions.width]
          : [this.dimensions.width, 0]
      )

    this.yDoseScale = d3
      .scaleLinear()
      .domain([0, 1.0])
      .range([this.dimensions.height, 0])

    if (this.densityChecked()) {
      const maxDensity = Math.max(...this.data.map((v) => v.density))

      this.yDensityScale = d3
        .scaleLinear()
        .domain([0, maxDensity])
        .range([this.dimensions.height, 0])
    }
  }

  /**
   * Create the x and y axes for the dose profile plot.
   */
  plotAxes() {
    // Clear existing axes and labels
    this.svg.selectAll('.profile-x-axis').remove()
    this.svg.selectAll('.profile-y-dose-axis').remove()

    // Create and append x and dose y axes
    const xAxis = d3
      .axisBottom()
      .scale(this.xScale)
      .tickSize(-this.dimensions.height)

    const yDoseAxis = d3
      .axisLeft()
      .scale(this.yDoseScale)
      .ticks(this.yTicks)
      .tickFormat(d3.format('.0%'))
      .tickSize(-this.dimensions.width)

    this.svg
      .append('g')
      .attr('class', 'profile-x-axis')
      .attr('transform', 'translate(0,' + this.dimensions.height + ')')
      .call(xAxis)

    this.svg.append('g').attr('class', 'profile-y-dose-axis').call(yDoseAxis)

    // Label for position x axis
    this.svg
      .append('text')
      .attr('class', 'profile-x-axis')
      .classed('dose-profile-axis-label', true)
      .attr(
        'transform',
        'translate(' +
        this.dimensions.width / 2 +
        ' ,' +
        (this.dimensions.height + this.dimensions.margin.top - 5) +
        ')'
      )
      .style('text-anchor', 'middle')
      .text(this.profileDim + ' (cm)')

    // Label for dose y axis
    this.svg
      .append('text')
      .attr('class', 'profile-y-dose-axis')
      .classed('dose-profile-axis-label', true)
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
      .text('Dose')

    if (this.densityChecked()) {
      // Clear existing axis and label
      this.svg.selectAll('.profile-y-density-axis').remove()

      // Create and append density y axes
      const yDensityAxis = d3
        .axisRight()
        .scale(this.yDensityScale)
        .ticks(this.yTicks)
        .tickFormat(d3.format('.2f'))
        .tickSize(-this.dimensions.width)

      this.svg
        .append('g')
        .attr('class', 'profile-y-density-axis')
        .attr('transform', 'translate(' + this.dimensions.width + ',0)')
        .call(yDensityAxis)

      // Label for density y axis
      this.svg
        .append('text')
        .attr('class', 'profile-y-density-axis')
        .classed('dose-profile-axis-label', true)
        .attr('transform', 'rotate(-90)')
        .attr(
          'transform',
          'translate(' +
          (this.dimensions.width + 45) +
          ' ,' +
          this.dimensions.height / 2 +
          ') rotate(90)'
        )
        .style('text-anchor', 'middle')
        .text('Density (g/cm\u00B3)')
    }
  }

  /**
   * Create the title of the plot with the correct coordinates.
   */
  makeTitle(coords) {
    // Clear existing title
    this.svg.select('.title').remove()

    const [dim1, dim2] =
      this.profileDim === 'x'
        ? ['y', 'z']
        : this.profileDim === 'y'
          ? ['x', 'z']
          : ['x', 'y']

    const format = d3.format('.2f')

    this.svg
      .append('text')
      .attr('class', 'title')
      .classed('dose-profile-axis-label', true)
      .attr('x', this.dimensions.width / 2)
      .attr('y', 0 - this.dimensions.margin.top / 2)
      .attr('text-anchor', 'middle')
      .style('text-decoration', 'underline')
      .text(
        this.profileDim +
        ' Axis Dose at (' +
        dim1 +
        ', ' +
        dim2 +
        '): (' +
        format(coords[0]) +
        ' cm, ' +
        format(coords[1]) +
        ' cm)'
      )
  }

  /**
   * Plot the dose profile data.
   */
  plotData() {
    const data = this.data
    const preYDoseScale = d3
      .scaleLinear()
      .domain([0, this.doseVol.maxDoseVar * 1.1])
      .range([0, 1.1])

    // Clear all existing elements
    this.svg.selectAll('.plotting-area').remove()

    const plotArea = this.svg
      .append('g')
      .attr('class', 'plotting-area')
      .attr(
        'clip-path',
        this.svg.select('clipPath').node()
          ? 'url(#' + this.svg.select('clipPath').node().id + ')'
          : ''
      )
      .attr('fill', 'none')
      .attr('width', this.dimensions.width)
      .attr('height', this.dimensions.height)

    // Create the dose error area
    const errorArea = d3
      .area()
      .x((d) => this.xScale(d.position))
      .y0((d) =>
        this.yDoseScale(preYDoseScale(d.value * (1.0 - parseFloat(d.err))))
      )
      .y1((d) =>
        this.yDoseScale(preYDoseScale(d.value * (1.0 + parseFloat(d.err))))
      )

    // Create the dose line
    const line = d3
      .line()
      .x((d) => this.xScale(d.position))
      .y((d) => this.yDoseScale(preYDoseScale(d.value)))

    // Plot error
    plotArea
      .append('path')
      .datum(data)
      .attr('fill', 'lightblue')
      .attr('class', 'lines')
      .attr('d', errorArea)

    // Plot dose
    plotArea
      .append('path')
      .datum(data)
      .attr('fill', 'none')
      .attr('stroke', 'steelblue')
      .attr('stroke-width', 1.5)
      .attr('stroke-linejoin', 'round')
      .attr('stroke-linecap', 'round')
      .attr('class', 'lines')
      .attr('d', line)

    if (this.densityChecked()) {
      // Create the density line
      const densityLine = d3
        .line()
        .x((d) => this.xScale(d.position))
        .y((d) => this.yDensityScale(d.density))

      // Plot density
      plotArea
        .append('path')
        .datum(data)
        .attr('fill', 'none')
        .attr('stroke', 'red')
        .attr('stroke-width', 1.5)
        .attr('stroke-linejoin', 'round')
        .attr('stroke-linecap', 'round')
        .attr('class', 'lines')
        .classed('density', true)
        .attr('d', densityLine)
    }

    if (this.transform) {
      this.svg
        .selectAll('path.lines')
        .attr('transform', this.transform.toString())
    }
  }

  /**
   * Update the x,y scales and axes.
   */
  updateAxes() {
    this.setDoseScales()
    this.plotAxes()
    if (this.zoomObj !== null) this.resetZoomTransform()
  }

  /**
   * Check if the axes need an update, then plot the data at the specified coordinates.
   */
  // TODO: Either combine this with getting the data or pass it in to reduce confusion
  plotDoseProfile(coords) {
    if (this.xScale === null) {
      this.updateAxes()
    }

    this.makeTitle(coords)
    this.plotData()
  }
}

// export { DoseProfile }
