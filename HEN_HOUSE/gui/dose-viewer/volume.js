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

// REMOVE THESE GLOBAL IMPORTS ONCE MODULES RE-IMPLEMENTED
/* global volumeViewerList */
/* global Slider */

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

/** @class Volume represents a .3ddose file.  */
class DoseVolume extends Volume {
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
  // TODO : Remove maxDoseVar
  addData (data) {
    this.data = data
    // Max dose used for dose contour plot
    this.maxDoseVar = data.maxDose
    super.addColourScheme(d3.interpolateViridis, data.maxDose, 0)
    // Calculate the contour thresholds
    const contourInt = 0.1
    this.thresholdPercents = d3.range(contourInt, 1.0 + contourInt, contourInt)
    this.updateThresholds()
    // The className function multiplies by 1000 and rounds because decimals are not allowed in class names
    this.className = (i) =>
      'col-' + d3.format('d')(this.thresholdPercents[i] * 1000)
    this.createBaseSlices(data, 'dose')
  }

  /**
   * Sets the maximum dose value for dose contour plots.
   *
   * @param {number} val The maximum dose percentage.
   * @param {Object} panels The panels for which to update the dose plots.
   */
  // TODO: Move to panel
  setMaxDose (val, panels) {
    this.maxDoseVar = val * this.data.maxDose
    // Update the colour scheme and thresholds with the new max dose variable
    super.addColourScheme(d3.interpolateViridis, this.maxDoseVar, 0)
    this.updateThresholds()

    Object.values(panels).forEach((panel) => {
      this.drawDose(this.sliceCache[panel.axis][panel.doseSliceNum], panel.zoomTransform, panel.axisElements['plot-dose'])
      if (panel.showDoseProfile()) {
        volumeViewerList[panel.volumeViewerId].doseProfileList.forEach((doseProfile) =>
          doseProfile.plotData()
        )
      }
    })
  }

  /**
   * Updates the threshold values used for creating the dose contours.
   */
  updateThresholds () {
    this.thresholds = this.thresholdPercents.map((i) => i * this.maxDoseVar)
  }

  /**
   * Add a new threshold value to create a new contour in the dose contour plots.
   *
   * @param {number} thresholdPercent The dose percentage to add.
   * @param {Object} panels The panels for which to update the dose plots.
   */
  // TODO: Move to panel
  addThresholdPercent (thresholdPercent, panels) {
    this.thresholdPercents.push(thresholdPercent)
    this.thresholdPercents.sort()
    this.updateThresholds()
    this.initializeLegend()

    Object.values(panels).forEach((panel) => {
      this.drawDose(this.sliceCache[panel.axis][panel.doseSliceNum], panel.zoomTransform, panel.axisElements['plot-dose'])
    })
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
   * Clear the current dose plot.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   */
  clearDose (axis, svg) {
    // Clear dose plot
    svg.selectAll('g').remove()

    // Clear dose legend
    this.legendSvg.selectAll('*').remove()

    // Remove existing dose contour inputs
    this.legendHolder.selectAll('input').remove()
  }

  /**
   * Make a dose contour plot of the given slice.
   *
   * @param {Object} slice The slice of the dose data.
   * @param {Object} [transform] The zoom transform of the plot.
   */
  drawDose (slice, transform, svg) {
    const baseSlice = this.baseSlices[slice.axis]

    // Clear dose plot
    svg.selectAll('g').remove()

    // Draw contours
    var contours = d3
      .contours()
      .size([baseSlice.xVoxels, baseSlice.yVoxels])
      .smooth(false)
      .thresholds(this.thresholds)(slice.sliceData)
      .map(baseSlice.contourTransform)

    const contourPaths = svg
      .append('g')
      .attr('class', 'dose-contour')
      .attr('width', this.dimensions.width)
      .attr('height', this.dimensions.height)
      .attr('fill', 'none')
      .attr('stroke', '#fff')
      .attr('stroke-opacity', 0.5)
      .attr('stroke-width', 0.1)
      .selectAll('path')
      .data(contours)
      .join('path')
      .classed('contour-path', true)
      .attr('class', (d, i) => 'contour-path' + ' ' + this.className(i))
      .attr('fill', (d) => this.colour(d.value))
      .attr('fill-opacity', 0.5)
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
   * Create the dose legend.
   */
  // TODO: Move to volume viewer
  initializeLegend () {
    // Get list of class names of hidden contours
    const hiddenContourClassList = this.getHiddenContourClassList()
    const legendClass = 'doseLegend'
    const parameters = {
      labels: [
        this.thresholds.map((e) => d3.format('.0%')(e / this.maxDoseVar))
      ],
      cells: [this.thresholds],
      on: [
        'cellclick',
        function (d) {
          const legendCell = d3.select(this)
          toggleContour(legendCell.attr('class').split(' ')[1])
          legendCell.classed('hidden', !legendCell.classed('hidden'))
        }
      ]
    }

    var toggleContour = (className) => {
      Object.values(this.htmlElementObj).forEach((svg) => {
        svg
          .selectAll('path.contour-path.' + className)
          .classed('hidden', function () {
            return !d3.select(this).classed('hidden')
          })
      })
    }

    // Clear and redraw current legend
    this.legendSvg.select('.' + legendClass).remove()
    this.legendSvg.select('text').remove()

    // Make space for legend title
    this.legendSvg
      .append('g')
      .attr('class', legendClass)
      .style('transform', 'translate(0px,' + 20 + 'px)')

    // Append title
    this.legendSvg
      .append('text')
      .attr('class', legendClass)
      .attr('x', this.legendDimensions.width / 2)
      .attr('y', this.legendDimensions.margin.top / 2)
      .attr('text-anchor', 'middle')
      .style('font-size', '14px')
      .text('Dose')

    // Create legend
    var legend = d3
      .legendColor()
      .shapeWidth(10)
      .ascending(true)
      .orient('vertical')
      .scale(this.colour)

    // Apply all the parameters
    Object.entries(parameters).forEach(([name, val]) => {
      legend[name](...val)
    })

    this.legendSvg.select('.' + legendClass).call(legend)

    // Set the height of the svg so the div can scroll if need be
    const height =
      this.legendSvg
        .select('.' + legendClass)
        .node()
        .getBoundingClientRect().height + 20
    this.legendSvg.attr('height', height)

    // Add the appropriate classnames to each legend cell
    const len = this.thresholdPercents.length - 1
    this.legendSvg
      .selectAll('g.cell')
      .attr('class', (d, i) => 'cell ' + this.className(len - i))

    if (hiddenContourClassList.length > 0) {
      // Apply hidden class to hidden contours
      const hiddenLegendCells = this.legendSvg
        .selectAll('g.cell')
        .filter(hiddenContourClassList.join(','))
      hiddenLegendCells.classed('hidden', !hiddenLegendCells.classed('hidden'))
    }
  }

  /**
   * Create the input box to add new dose contour thresholds.
   *
   * @param {Object} panels The panels to update when a dose contour is added.
   */
  initializeDoseContourInput (panels) {
    var addNewThresholdPercent = () => {
      const val = parseFloat(submitDoseContour.node().value)
      const newPercentage = val / 100.0
      if (!Number.isNaN(newPercentage)) {
        // Check if valid or if percentage already exists
        if (
          val < 0 ||
          val > 100 ||
          !Number.isInteger(val) ||
          this.thresholdPercents.includes(newPercentage)
        ) {
          console.log('Invalid value or value already exists on plot')
          // Flash the submit box red
          submitDoseContour
            .transition()
            .duration(200)
            .style('background-color', 'red')
            .transition()
            .duration(300)
            .style('background-color', 'white')
        } else {
          this.addThresholdPercent(newPercentage, panels)
        }
      }
    }

    // Remove existing dose contour inputs
    this.legendHolder.selectAll('input').remove()

    const doseContourInputWidth = 45

    // Add number input box
    const submitDoseContour = this.legendHolder
      .append('input')
      .attr('type', 'number')
      .attr('name', 'add-dose-contour-line')
      .attr('id', 'add-dose-contour-line')
      .attr('min', 0)
      .attr('max', 100)
      .attr('step', 1)
      .style('width', doseContourInputWidth + 'px')

    // Add submit button
    this.legendHolder
      .append('input')
      .attr('type', 'submit')
      .attr('name', 'submit-dose-contour-line')
      .attr('id', 'submit-dose-contour-line')
      .attr('value', '+')
      .on('click', addNewThresholdPercent)
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
    return this.error ? super.getDataAtVoxelCoords(voxelCoords, 'error') : 0
  }

  /**
   * Create the max dose slider to choose the maximum dose in the contour plots.
   *
   * @param {Object} panels The panels for which to update on maximum dose change.
   */
  initializeMaxDoseSlider (panels) {
    const parentDiv = d3.select('#axis-slider-container')
    var onMaxDoseChangeCallback = (sliderVal) =>
      this.setMaxDose(sliderVal, panels)

    const doseSliderParams = {
      id: 'max-dose',
      label: 'Max Dose',
      format: d3.format('.0%'),
      startingVal: 1.0,
      minVal: 0.0,
      maxVal: 1.5,
      step: 0.01,
      onSliderChangeCallback: onMaxDoseChangeCallback
    }

    // Remove existing sliders
    parentDiv.selectAll('.slider-container').remove()

    const maxDoseSlider = new Slider( // eslint-disable-line no-unused-vars
      parentDiv,
      doseSliderParams
    )
  }
}

/** @class Volume represents the difference between two .3ddose files.  */
class DoseComparisonVolume extends DoseVolume { // eslint-disable-line no-unused-vars
  /**
   * Creates an instance of a DoseComparisonVolume.
   *
   * @constructor
   * @extends DoseVolume
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   * @param {Object} data The data from parsing the file.
   */
  constructor (fileName, dimensions, legendDimensions, data, args) {
    // Call the super class constructor
    super(fileName, dimensions, legendDimensions, args)
    this.addData(data)
  }

  /**
   * Adds data to the DoseComparisonVolume object.
   *
   * @param {Object} data The difference of the data from the two dose files.
   */
  addData (data) {
    this.data = data
    // Max dose used for dose contour plot
    this.maxDoseVar = 1.0
    super.addColourScheme(d3.interpolateViridis, 1.0, -1.0)

    // Calculate the contour thresholds
    const contourInt = 0.2
    this.thresholdPercents = d3.range(
      -1.0 + contourInt,
      1.0 + contourInt,
      contourInt
    )
    // Thresholds and thresholdPercents are the same
    super.updateThresholds()

    // The className function multiplies by 1000 and rounds because decimals are not allowed in class names
    this.className = (i) =>
      'col-' + d3.format('d')(this.thresholdPercents[i] * 1000)

    this.createBaseSlices(data, 'density')
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
    this.maxDensityVar = parseFloat(data.maxDensity)
    this.minDensityVar = parseFloat(data.minDensity)
    this.densityFormat = (args !== undefined) && (args.isDicom) ? d3.format('d') : d3.format('.2f')
    this.densityStep = (args !== undefined) && (args.isDicom) ? 1.0 : 0.01
    this.addColourScheme(this.maxDensityVar, this.minDensityVar)
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
            maxVal: this.maxDensityVar,
            minVal: this.minDensityVar
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
   * Sets the maximum density value for density plots.
   *
   * @param {number} maxDensityVal The maximum density value.
   * @param {Object} panels The panels for which to update the dose plots.
   */
  // TODO: Move to panel
  setMaxDensityVar (maxDensityVal, panels) {
    this.maxDensityVar = parseFloat(maxDensityVal)

    // Redraw legend
    this.initializeLegend()

    // TODO: Perhaps move this to the panels?? call initialize legend and make
    // maxdensity var a panel attribute
    Object.values(panels).forEach((panel) => {
      panel.prevSliceImg = this.drawDensity(this.sliceCache[panel.axis][panel.densitySliceNum], panel.zoomTransform, panel.axisElements['plot-density'])
    })
  }

  /**
  * Sets the minimum density value for density plots.
  *
  * @param {number} minDensityVal The minimum density value.
  * @param {Object} panels The panels for which to update the dose plots.
  */
  // TODO: Move to panel
  setMinDensityVar (minDensityVal, panels) {
    this.minDensityVar = parseFloat(minDensityVal)

    // Redraw legend
    this.initializeLegend()

    Object.values(panels).forEach((panel) => {
      panel.prevSliceImg = this.drawDensity(this.sliceCache[panel.axis][panel.densitySliceNum], panel.zoomTransform, panel.axisElements['plot-density'])
    })
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
   * Clear the current density plot.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   */
  // TODO: Move clearDose and clearDensity panel functions
  clearDensity (axis, svg) {
    // Clear density plot
    const context = svg.getContext('2d')
    context.clearRect(0, 0, svg.width, svg.height)

    // Clear density legend
    this.legendSvg.selectAll('*').remove()
  }

  /**
   * Make a density plot of the given slice.
   *
   * @param {Object} slice The slice of the density data.
   * @param {Object} [transform] The zoom transform of the plot.
   * @returns {Object}
   */
  drawDensity (slice, transform, svg) {
    // Get the canvas and context in the webpage
    const baseSlice = this.baseSlices[slice.axis]
    const imgCanvas = svg.node()
    const imgContext = imgCanvas.getContext('2d', { alpha: false })
    const imageData = this.getImageData(slice)

    // If the min and max density have been changed, apply colour map
    if ((this.minDensityVar !== this.data.minDensity) || (this.maxDensityVar !== this.data.maxDensity)) {
      const colourMap = d3.scaleSqrt().domain([this.minDensityVar, this.maxDensityVar]).range([0, 255])
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
   * Create the density legend.
   */
  initializeLegend () {
    const legendClass = 'densityLegend'
    const title = 'Density'
    const dims = this.legendDimensions
    const colourMap = d3.scaleSqrt().domain([this.minDensityVar, this.maxDensityVar]).range([0, 255])

    function gradientUrl (colour, height, width, n = 150) {
      const canvas = document.createElement('canvas')
      const context = canvas.getContext('2d')
      const [maxVal, minVal] = colour.domain()
      var val
      for (let i = 0; i < height; ++i) {
        val = colour(((n - i) / n) * (maxVal - minVal) + minVal)
        context.fillStyle = 'black'
        context.fillRect(0, i, 1, 1)
        context.fillStyle = 'rgb(' + val + ', ' + val + ', ' + val + ')'
        context.fillRect(1, i, width - 1, 1)
      }
      return canvas.toDataURL()
    }

    // Remove old data
    this.legendSvg.selectAll('*').remove()

    // Set dimensions of svg
    this.legendSvg
      .attr('width', dims.width)
      .attr('height', dims.height)
      .attr('viewBox', [0, 0, dims.width, dims.height])
      .style('overflow', 'visible')
      .style('display', 'block')

    // Define parameters for ticks
    const ticks = 4
    const n = Math.round(ticks + 1)
    const tickValues = d3
      .range(n)
      .map((i) => d3.quantile(colourMap.domain(), i / (n - 1)))
    const tickFormat = this.densityFormat
    const tickSize = 15

    const gradUrl = gradientUrl(
      colourMap,
      dims.height - 20,
      30
    )

    // Set height of legend
    const legendHeight = dims.height - 80

    // Create scale for ticks
    const scale = d3
      .scaleLinear()
      .domain([this.minDensityVar, this.maxDensityVar])
      .range([legendHeight, 0])

    // Append title
    this.legendSvg
      .append('text')
      .attr('class', legendClass)
      .attr('x', dims.width / 2)
      .attr('y', dims.margin.top / 2)
      .attr('text-anchor', 'middle')
      .style('font-size', '14px')
      .text(title)

    // Append gradient image
    this.legendSvg
      .append('g')
      .attr('class', legendClass)
      .append('image')
      .attr('y', dims.margin.top)
      .attr('width', dims.width)
      .attr('height', legendHeight)
      .attr('preserveAspectRatio', 'none')
      .attr('xlink:href', gradUrl)

    // Append ticks
    this.legendSvg
      .append('g')
      .attr('transform', 'translate(' + 0 + ', ' + dims.margin.top + ')')
      .classed('label', true)
      .call(
        d3
          .axisRight()
          .ticks(ticks, tickFormat)
          .tickFormat(tickFormat)
          .tickSize(tickSize)
          .tickValues(tickValues)
          .scale(scale)
      )
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

  /**
  * Initialize the behaviour of the canvas density
  */
  initializeCanvas () {
    Object.values(this.htmlElementObj).forEach((svg) => {
      // Get the canvas and context in the webpage
      const imgCanvas = svg.node()
      const imgContext = imgCanvas.getContext('2d')

      // Disable smoothing to clearly show pixels
      imgContext.imageSmoothingEnabled = false
      imgContext.mozImageSmoothingEnabled = false
      imgContext.webkitImageSmoothingEnabled = false
      imgContext.msImageSmoothingEnabled = false
    })
  }
}

// export { DensityVolume, DoseComparisonVolume, DoseVolume }
