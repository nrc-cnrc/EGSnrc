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

import { volumeViewerList } from './index.js'
import { Slider } from './slider.js'

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
    this.prevSlice = { xy: {}, yz: {}, xz: {} }
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
   * Get a slice of data through an axis.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} sliceNum The number of the slice.
   * @param {string} dataName The type of data, either "density" or "dose".
   * @returns {Object}
   */
  getSlice (axis, slicePos, dataName, args) {
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

    const x = this.data.voxelArr[dim1]
    const y = this.data.voxelArr[dim2]
    const z = this.data.voxelArr[dim3]
    const totalSlices = this.data.voxelNumber[dim3]

    // Get the length in cm of the x and y dimensions
    var getLengthCm = (voxelArrDim) =>
      Math.abs(voxelArrDim[voxelArrDim.length - 1] - voxelArrDim[0])
    const [xLengthCm, yLengthCm] = [getLengthCm(x), getLengthCm(y)]

    // Initialize variables to make slice scales
    let xDomain,
      yDomain,
      xRange,
      yRange

    if (args !== undefined) {
      xDomain = args[axis].xScale.domain()
      yDomain = args[axis].yScale.domain()
      xRange = [Math.round(args[axis].xScale(x[0])), Math.round(args[axis].xScale(x[x.length - 1]))]
      yRange = [Math.round(args[axis].yScale(y[0])), Math.round(args[axis].yScale(y[y.length - 1]))]
    } else {
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
    }

    // TODO: Clamp scales
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

    // Define the screen pixel to volume voxel mapping
    const xPixelToVoxelScale = d3
      .scaleQuantile()
      .domain(xRange)
      .range(d3.range(0, this.data.voxelNumber[dim1], 1))
    const yPixelToVoxelScale = d3
      .scaleQuantile()
      .domain(yRange)
      .range(axis === 'xy' ? d3.range(0, this.data.voxelNumber[dim2], 1) : d3.range(this.data.voxelNumber[dim2], 0, -1))

    // Define the voxel to screen mapping for dose contours
    const contourXScale = d3
      .scaleLinear()
      .domain([0, this.data.voxelNumber[dim1]])
      .range(xRange)
    const contourYScale = d3
      .scaleLinear()
      .domain(axis === 'xy' ? [this.data.voxelNumber[dim2], 0] : [0, this.data.voxelNumber[dim2]])
      .range(axis === 'xy' ? yRange.reverse() : yRange)

    var sliceNum = zScale(slicePos)
    // TODO: Change scales to quantile to map exactly which pixels
    let slice = {
      dx: this.data.voxelSize[dim1],
      dy: this.data.voxelSize[dim2],
      xVoxels: this.data.voxelNumber[dim1],
      yVoxels: this.data.voxelNumber[dim2],
      x: x,
      y: y,
      totalSlices: totalSlices,
      xScale: xScale,
      yScale: yScale,
      zScale: zScale, // unit: pixels
      slicePos: slicePos,
      dimensions: this.dimensions,
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

    // If current slice number is larger than the total number of slices
    // set slice number to last slice
    sliceNum =
      sliceNum >= slice.totalSlices
        ? parseInt(slice.totalSlices - 1)
        : parseInt(sliceNum)

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
            sliceNum + this.data.voxelNumber.x * (i + j * slice.xVoxels)
        } else if (axis === 'xz') {
          address =
            i + slice.xVoxels * (sliceNum + j * this.data.voxelNumber.y)
        }
        const newAddress = i + slice.xVoxels * j
        sliceData[newAddress] = this.data[dataName][address]
      }
    }

    slice = {
      ...slice,
      sliceData: sliceData,
      sliceNum: sliceNum
    }

    this.prevSlice[axis] = slice
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
  // TODO: Remove maxDoseVar
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
  }

  /**
   * Sets the maximum dose value for dose contour plots.
   *
   * @param {number} val The maximum dose percentage.
   * @param {Object} panels The panels for which to update the dose plots.
   */
  setMaxDose (val, panels) {
    this.maxDoseVar = val * this.data.maxDose
    // Update the colour scheme and thresholds with the new max dose variable
    super.addColourScheme(d3.interpolateViridis, this.maxDoseVar, 0)
    this.updateThresholds();

    ['xy', 'yz', 'xz'].forEach((axis) =>
      this.drawDose(this.prevSlice[axis], panels[axis].zoomTransform)
    )

    if (d3.select("input[name='show-dose-profile-checkbox']").node().checked) {
      volumeViewerList.forEach((volumeViewer) => {
        volumeViewer.doseProfileList.forEach((doseProfile) =>
          doseProfile.plotData()
        )
      })
    }
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
  addThresholdPercent (thresholdPercent, panels) {
    this.thresholdPercents.push(thresholdPercent)
    this.thresholdPercents.sort()
    this.updateThresholds()
    this.initializeLegend();
    ['xy', 'yz', 'xz'].forEach((axis) =>
      this.drawDose(this.prevSlice[axis], panels[axis].zoomTransform)
    )
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
  clearDose (axis) {
    const svg = this.htmlElementObj[axis]
    // Clear dose plot
    svg.selectAll('g').remove()
  }

  /**
   * Make a dose contour plot of the given slice.
   *
   * @param {Object} slice The slice of the dose data.
   * @param {Object} [transform] The zoom transform of the plot.
   */
  drawDose (slice, transform) {
    const svg = this.htmlElementObj[slice.axis]

    // Clear dose plot
    svg.selectAll('g').remove()

    // Draw contours
    var contours = d3
      .contours()
      .size([slice.xVoxels, slice.yVoxels])
      .smooth(false)
      .thresholds(this.thresholds)(slice.sliceData)
      .map(slice.contourTransform)

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
class DoseComparisonVolume extends DoseVolume {
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
class DensityVolume extends Volume {
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
    this.prevSliceImg = { xy: {}, yz: {}, xz: {} }
  }

  /**
   * Adds data to the DensityVolume object.
   *
   * @param {Object} data The data from parsing the file.
   */
  addData (data, args) {
    this.data = data
    this.maxDensityVar = parseFloat(this.data.maxDensity)
    this.minDensityVar = parseFloat(this.data.minDensity)
    this.densityFormat = (args !== undefined) && (args.isDicom) ? d3.format('d') : d3.format('.2f')
    this.densityStep = (args !== undefined) && (args.isDicom) ? 1.0 : 0.01
    super.addColourScheme(d3.interpolateGreys, this.maxDensityVar, this.minDensityVar, true)
  }

  /**
   * Sets the maximum density value for density plots.
   *
   * @param {number} maxDensityVal The maximum density value.
   * @param {Object} panels The panels for which to update the dose plots.
   */
  setMaxDensityVar (maxDensityVal, panels) {
    this.maxDensityVar = parseFloat(maxDensityVal)

    // Update the colour scheme with the new max density variable
    super.addColourScheme(d3.interpolateGreys, this.maxDensityVar, this.minDensityVar, true);

    ['xy', 'yz', 'xz'].forEach((axis) =>
      this.drawDensity(this.prevSlice[axis], panels[axis].zoomTransform)
    )
  };

  /**
  * Sets the minimum density value for density plots.
  *
  * @param {number} minDensityVal The minimum density value.
  * @param {Object} panels The panels for which to update the dose plots.
  */
  setMinDensityVar (minDensityVal, panels) {
    this.minDensityVar = parseFloat(minDensityVal)

    // Update the colour scheme with the new max density variable
    super.addColourScheme(d3.interpolateGreys, this.maxDensityVar, this.minDensityVar, true);

    ['xy', 'yz', 'xz'].forEach((axis) =>
      this.drawDensity(this.prevSlice[axis], panels[axis].zoomTransform)
    )
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
  clearDensity (axis) {
    const svg = this.htmlElementObj[axis].node()

    // Clear density plot
    const context = svg.getContext('2d')
    context.clearRect(0, 0, svg.width, svg.height)
  }

  /**
   * Make a density plot of the given slice.
   *
   * @param {Object} slice The slice of the density data.
   * @param {Object} [transform] The zoom transform of the plot.
   */
  drawDensity (slice, transform) {
    const svg = this.htmlElementObj[slice.axis]
    // TODO: Make change slicenum function

    // For axis structure
    // https://bl.ocks.org/ejb/e2da5a23e9a09d494bd532803d8db61c

    // Create new canvas element and set the dimensions
    const canvas = document.createElement('canvas')
    canvas.width = this.dimensions.width
    canvas.height = this.dimensions.height

    // Get and clear the canvas context
    const context = canvas.getContext('2d')
    context.clearRect(0, 0, this.dimensions.width, this.dimensions.height)

    // Calcuate display pixel dimensions
    const dxScaled = Math.ceil(this.dimensions.width / slice.xVoxels)
    const dyScaled = Math.ceil(this.dimensions.height / slice.yVoxels)

    // Draw the image voxel by voxel
    for (let i = 0; i < slice.xVoxels; i++) {
      for (let j = 0; j < slice.yVoxels; j++) {
        const newAddress = i + slice.xVoxels * j
        context.fillStyle = this.colour(slice.sliceData[newAddress])
        context.fillRect(
          Math.ceil(slice.xScale(slice.x[i])),
          Math.ceil(slice.yScale(slice.y[j])),
          dxScaled,
          dyScaled
        )
      }
    }

    // Create a new image to set the canvas data as the image source
    var image = new Image()

    // Once the image has loaded, draw it on the context
    image.addEventListener('load', (e) => {
      imgContext.clearRect(0, 0, this.dimensions.width, this.dimensions.height)
      imgContext.save()
      // Apply transforms if needed
      if (transform) {
        imgContext.translate(transform.x, transform.y)
        imgContext.scale(transform.k, transform.k)
      }
      imgContext.drawImage(image, 0, 0)
      imgContext.restore()
    })

    image.src = canvas.toDataURL()

    // Get the canvas and context in the webpage
    const imgCanvas = svg.node()
    const imgContext = imgCanvas.getContext('2d')

    // Save the image as properties of the volume object
    this.prevSliceImg[slice.axis] = image
  }

  /**
   * Create the density legend.
   */
  initializeLegend () {
    const legendClass = 'densityLegend'
    const title = 'Density'
    const dims = this.legendDimensions

    function gradientUrl (colour, height, width, n = 150) {
      const canvas = document.createElement('canvas')
      const context = canvas.getContext('2d')
      const [maxVal, minVal] = colour.domain()

      for (let i = 0; i < height; ++i) {
        context.fillStyle = 'black'
        context.fillRect(0, i, 1, 1)
        context.fillStyle = colour(((n - i) / n) * (maxVal - minVal) + minVal)
        context.fillRect(1, i, width - 1, 1)
      }
      return canvas.toDataURL()
    }

    // Remove old text
    this.legendSvg.select('.' + legendClass).remove()
    this.legendSvg.select('text').remove()

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
      .map((i) => d3.quantile(this.colour.domain(), i / (n - 1)))
    const tickFormat = this.densityFormat
    const tickSize = 15

    const gradUrl = gradientUrl(
      this.colour,
      dims.height - 20,
      30
    )

    // Set height of legend
    const legendHeight = dims.height - 80

    // Create scale for ticks
    const scale = d3
      .scaleLinear()
      .domain([this.data.minDensity, this.data.maxDensity])
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
}

export { DensityVolume, DoseComparisonVolume, DoseVolume }
