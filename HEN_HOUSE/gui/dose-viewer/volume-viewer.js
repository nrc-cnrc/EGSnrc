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

// REMOVE THESE GLOBAL IMPORTS ONCE MODULES RE-IMPLEMENTED
/* global enableCheckboxForDensityPlot */
/* global enableCheckboxForDoseProfilePlot */
/* global enableExportVisualizationButton */
/* global enableCheckboxForVoxelInformation */
/* global initializeMinMaxDensitySlider */
/* global doseVolumeList */
/* global densityVolumeList */
/* global doseComparisonVolumeList */
/* global volumeViewerList */
/* global DoseComparisonVolume */
/* global defineShowMarkerCheckboxBehaviour */
/* global defineShowProfileCheckboxBehaviour */
/* global defineExportPNGButtonBehaviour */
/* global defineExportCSVButtonBehaviour */
/* global buildVoxelInfoHtml */
/* global coordsToVoxel */
/* global updateVoxelCoords */
/* global DoseProfile */
/* global Panel */
/* global Slider */
/* global initializeMaxDoseSlider */

// import {
//   densityVolumeList, doseComparisonVolumeList, doseVolumeList, volumeViewerList
// } from './index.js'
// import {
//   defineShowMarkerCheckboxBehaviour, defineShowProfileCheckboxBehaviour,
//   enableCheckboxForDensityPlot, enableExportVisualizationButton
// } from './checkbox-button-helper.js'
// import { DoseProfile } from './dose-profile.js'
// import { Panel } from './panel.js'
// import { Slider } from './slider.js'
// import { DoseComparisonVolume } from './volume.js'
// import { buildVoxelInfoHtml, coordsToVoxel, updateVoxelCoords } from './voxel-coordinates.js'
// import { initializeMinMaxDensitySlider } from './min-max-density-slider.js'
// import { defineExportCSVButtonBehaviour, defineExportPNGButtonBehaviour } from './export.mjs'
// import {initializeMaxDoseSlider} from './max-dose-slider.js'

const AXES = ['xy', 'yz', 'xz']

/** @class VolumeViewer combines a density and/or dose file, three panels for
 * the three axes views, and three dose profile plots. */
class VolumeViewer { // eslint-disable-line no-unused-vars
  /**
   * Creates an instance of a VolumeViewer.
   *
   * @constructor
   * @param {Object} mainViewerDimensions The pixel dimensions of the main viewer.
   * @param {Object} legendDimensions The pixel dimensions of the legends.
   * @param {Object} sideDoseProfileDimensions The pixel dimensions of dose profiles.
   * @param {string} id The unique ID of the volume viewer.
   */
  // TODO: Avoid making the dimensions properties of the volume viewer
  constructor (
    mainViewerDimensions,
    legendDimensions,
    sideDoseProfileDimensions,
    id
  ) {
    // Set dimensions
    this.mainViewerDimensions = mainViewerDimensions
    this.legendDimensions = legendDimensions
    this.sideDoseProfileDimensions = sideDoseProfileDimensions

    // Set volume viewer ID
    this.id = id

    // Initialize class properties
    this.doseVolume = null
    this.densityVolume = null
    this.panels = null
    this.sliceSliders = {}
    this.doseProfileList = new Array(3)
    this.dispatch = d3.dispatch('markerchange')
    this.voxelInfoDiv = null

    this.buildBaseHtml(id)
    this.initializeDispatch()
  }

  drawAxes (zoomTransform, svgAxis, slice) {
    // If density is already plotted, move dose slice?
    const volume = this.densityVolume || this.doseVolume
    const baseSlice = volume.baseSlices[slice.axis]

    svgAxis.selectAll('.x-axis, .y-axis, .x-axis-grid, .y-axis-grid').remove()

    // TODO: Check for existing scale on axes
    // If there is existing transformation, apply it
    const xScale = zoomTransform
      ? zoomTransform.rescaleX(baseSlice.xScale)
      : baseSlice.xScale
    const yScale = zoomTransform
      ? zoomTransform.rescaleY(baseSlice.yScale)
      : baseSlice.yScale

    // Create and append the x and y axes
    var xAxis = d3.axisBottom().scale(xScale).ticks(6)
    var yAxis = d3.axisLeft().scale(yScale).ticks(6)

    svgAxis
      .append('g')
      .attr('class', 'x-axis')
      .attr('transform', 'translate(0,' + volume.dimensions.height + ')')
      .style('font-size', '12px')
      .call(xAxis)
    svgAxis
      .append('g')
      .attr('class', 'y-axis')
      .style('font-size', '12px')
      .call(yAxis)

    // Create and append the x and y grids
    var xAxisGrid = d3
      .axisBottom()
      .scale(xScale)
      .tickSize(-volume.dimensions.height)
      .tickFormat('')
      .ticks(6)
    var yAxisGrid = d3
      .axisLeft()
      .scale(yScale)
      .tickSize(-volume.dimensions.width)
      .tickFormat('')
      .ticks(6)

    svgAxis
      .append('g')
      .attr('class', 'x-axis-grid')
      .attr('transform', 'translate(0,' + volume.dimensions.height + ')')
      .call(xAxisGrid)
    svgAxis.append('g').attr('class', 'y-axis-grid').call(yAxisGrid)

    // Label for x axis
    svgAxis
      .append('text')
      .attr('class', 'x-axis')
      .attr(
        'transform',
        'translate(' +
        volume.dimensions.width / 2 +
        ' ,' +
        (volume.dimensions.fullHeight - 25) +
        ')'
      )
      .style('text-anchor', 'middle')
      .text(slice.axis[0] + ' (cm)')

    // Label for y axis
    svgAxis
      .append('text')
      .attr('class', 'y-axis')
      .attr('transform', 'rotate(-90)')
      .attr(
        'transform',
        'translate(' +
        (25 - volume.dimensions.margin.left) +
        ' ,' +
        volume.dimensions.height / 2 +
        ') rotate(-90)'
      )
      .style('text-anchor', 'middle')
      .text(slice.axis[1] + ' (cm)')
  }

  /**
   * Set the dose volume of the VolumeViewer.
   *
   * @param {DoseVolume} doseVol The dose volume to be set.
   */
  setDoseVolume (doseVol) {
    this.doseVolume = doseVol

    // Set the max dose variable
    this.maxDoseVar = parseFloat(doseVol.data.maxDose)
    this.initializeThresholds()

    // Set dose volume html elements
    doseVol.setHtmlObjects(
      this.svgObjs['plot-dose'],
      this.doseLegendHolder,
      this.doseLegendSvg
    )

    this.initializeDoseLegend(this.maxDoseVar, this.thresholds)
    this.initializeDoseContourInput()

    const dims = 'zxy'
    var sliceNum, slicePos

    // Get the average of items at index i and i+1
    const getPos = (arr, i) => (arr[i] + arr[i + 1]) / 2.0

    // Set the panel doseVolume object
    Object.values(this.panels).forEach((panel, i) => {
      // Get the slice position
      slicePos = panel.densityVol
        ? panel.slicePos
        : getPos(doseVol.data.voxelArr[dims[i]], Math.floor(doseVol.data.voxelNumber[dims[i]] / 2))

      // Set the dose volume in the panel
      panel.setDoseVolume(doseVol, slicePos)

      if (!panel.densityVol) {
        // Update the slider max values
        sliceNum = Math.round(doseVol.baseSlices[panel.axis].zScale(slicePos))
        this.sliceSliders[panel.axis].setMaxValue(
          doseVol.data.voxelNumber[dims[i]] - 1
        )
        this.sliceSliders[panel.axis].setCurrentValue(sliceNum)

        // Draw the slice
        const slice = doseVol.getSlice(panel.axis, slicePos)
        doseVol.drawDose(slice, panel.zoomTransform, panel.axisElements['plot-dose'], this.thresholds, this.className, this.maxDoseVar)

        // Update the axis
        this.drawAxes(
          panel.zoomTransform,
          this.svgObjs['axis-svg'][panel.axis],
          slice
        )
      } else {
        // Draw the slice
        const slice = doseVol.getSlice(panel.axis, slicePos)
        doseVol.drawDose(slice, panel.zoomTransform, panel.axisElements['plot-dose'], this.thresholds, this.className, this.maxDoseVar)
      }
    })

    if (this.densityVolume) {
      enableCheckboxForDensityPlot()
    }
    this.enableCheckboxForDoseProfilePlot()
    enableExportVisualizationButton()
    this.enableCheckboxForVoxelInformation()
    initializeMaxDoseSlider(this.maxDoseParentDiv, doseVol, this)
  }

  /**
   * Set the density volume of the VolumeViewer.
   *
   * @param {DensityVolume} densityVol The density volume to be set.
   */
  setDensityVolume (densityVol) {
    this.densityVolume = densityVol

    // Set the min and max density variables
    this.maxDensityVar = parseFloat(densityVol.data.maxDensity)
    this.minDensityVar = parseFloat(densityVol.data.minDensity)

    densityVol.setHtmlObjects(
      this.svgObjs['plot-density'],
      this.densityLegendHolder,
      this.densityLegendSvg
    )

    densityVol.initializeLegend(this.minDensityVar, this.maxDensityVar)
    densityVol.initializeCanvas()
    const dims = 'zxy'
    var sliceNum, slicePos

    // Get the average of items at index i and i+1
    const getPos = (arr, i) => (arr[i] + arr[i + 1]) / 2.0

    // Set the panel densityVolume object
    Object.values(this.panels).forEach((panel, i) => {
      // Get the slice position
      slicePos = panel.doseVol
        ? panel.slicePos
        : getPos(densityVol.data.voxelArr[dims[i]], Math.floor(densityVol.data.voxelNumber[dims[i]] / 2))

      // Set the density volume in the panel
      panel.setDensityVolume(densityVol, slicePos)

      // Update the slider max values
      sliceNum = Math.round(densityVol.baseSlices[panel.axis].zScale(slicePos))
      this.sliceSliders[panel.axis].setMaxValue(
        densityVol.data.voxelNumber[dims[i]] - 1
      )
      this.sliceSliders[panel.axis].setCurrentValue(sliceNum)

      // Draw the slice
      const densitySlice = densityVol.getSlice(panel.axis, slicePos)
      panel.prevSliceImg = densityVol.drawDensity(densitySlice, panel.zoomTransform, panel.axisElements['plot-density'], this.minDensityVar, this.maxDensityVar)

      // Update the axis
      this.drawAxes(
        panel.zoomTransform,
        this.svgObjs['axis-svg'][panel.axis],
        densitySlice
      )

      if (panel.doseVol) {
        // Redraw dose contours
        const doseSlice = panel.doseVol.sliceCache[panel.axis][panel.doseSliceNum]
        panel.doseVol.drawDose(doseSlice, panel.zoomTransform, panel.axisElements['plot-dose'], this.thresholds, this.className, this.maxDoseVar)
      }
    })

    if (this.doseVolume) {
      enableCheckboxForDensityPlot()
    }
    enableExportVisualizationButton()
    // TODO: Move this outside volume viewer and assume all loaded egsphants
    // have same density range
    initializeMinMaxDensitySlider(
      this.minParentDiv,
      this.maxParentDiv,
      densityVol,
      this
    )

    this.enableCheckboxForVoxelInformation()
  }

  /**
   * Remove the current dose volume of the VolumeViewer.
   */
  removeDoseVolume () {
    this.doseVolume = null

    // Remove the volume object from panels
    Object.values(this.panels).forEach((panel) => {
      // Clear the panel
      if (panel.doseVol) panel.doseVol.clearDose(panel.axis, panel.axisElements['plot-dose'])

      // Set the volume object to density vol if need be
      if (panel.volume === panel.doseVol) panel.volume = panel.densityVol

      panel.doseVol = null
    })
  }

  /**
   * Remove the current density volume of the VolumeViewer.
   */
  removeDensityVolume () {
    this.densityVolume = null

    // Remove the min and max density sliders
    this.minParentDiv.select('*').remove()
    this.maxParentDiv.select('*').remove()

    // Remove the volume object from panels
    Object.values(this.panels).forEach((panel) => {
      // Clear the panel
      if (panel.densityVol) panel.densityVol.clearDensity(panel.axis, panel.axisElements['plot-density'])

      // Set the volume object to density vol if need be
      if (panel.volume === panel.densityVol) panel.volume = panel.doseVol

      panel.densityVol = null
    })

    // TODO: Remove legend as well
  }

  /**
   * Get a string representation of the margin object to style HTML elements.
   *
   * @param {Object} margin An object containing numbers of pixels for the top,
   * right, bottom, and left margins.
   */
  getMarginStr (margin) {
    return (
      margin.top +
      'px ' +
      margin.right +
      'px ' +
      margin.bottom +
      'px ' +
      margin.left +
      'px'
    )
  }

  /**
   * Update the dose file dropdown selector when a new dose file is added.
   *
   * @param {DoseVolume} doseVol The new dose volume to be added to the
   * dropdown.
   * @param {number} i The index of the dose volume.
   */
  // TODO: Create event listeners instead of calling this every time
  updateDoseFileSelector (doseVol, i) {
    this.doseSelector.append('option').attr('value', i).text(doseVol.fileName)

    this.doseComparisonSelector
      .append('option')
      .attr('value', i)
      .text(doseVol.fileName)

    // TODO: Wait until first dose selector has dose loaded to enable
    if (doseVolumeList.length >= 2) {
      this.doseComparisonSelector.attr('disabled', null)
    }
  }

  /**
   * Update the density file dropdown selector when a new density file is added.
   *
   * @param {DensityVolume} densityVol The new density volume to be added to the
   * dropdown.
   * @param {number} i The index of the density volume.
   */
  updateDensityFileSelector (densityVol, i) {
    this.densitySelector
      .append('option')
      .attr('value', i)
      .text(densityVol.fileName)
  }

  /**
   * Populate the file selectors with uploaded dose and density volumes.
   */
  setUpFileSelectors () {
    const volumeViewer = this

    // For existing dose and density files uploaded, add to file selectors
    densityVolumeList.forEach((densityVol, i) =>
      this.updateDensityFileSelector(densityVol, i)
    )
    doseVolumeList.forEach((doseVol, i) =>
      this.updateDoseFileSelector(doseVol, i)
    )

    // Add behaviour, when volume is selected, change the volume viewer property
    this.doseSelector.on('change', function () {
      if (parseInt(this.value) === -1) {
        // If the base text is chosen, remove dose volume if loaded
        volumeViewer.removeDoseVolume()
      } else {
        volumeViewer.setDoseVolume(doseVolumeList[this.value])
      }
    })

    this.densitySelector.on('change', function () {
      if (parseInt(this.value) === -1) {
        volumeViewer.removeDensityVolume()
      } else {
        volumeViewer.setDensityVolume(densityVolumeList[this.value])
      }
    })

    this.doseComparisonSelector.on('change', function () {
      if (volumeViewer.doseVolume) {
        if (parseInt(this.value) === -1) {
          // If the base text is chosen, remove density volume if loaded
          const volIndex = parseInt(
            volumeViewerList[0].doseSelector.node().value
          )
          if (volIndex >= 0) {
            volumeViewer.setDoseVolume(doseVolumeList[volIndex])
          } else {
            volumeViewer.removeDoseVolume()
          }
        } else if (volumeViewer.doseVolume === doseVolumeList[this.value]) {
          console.log(
            'Select a different dose volume than one that is already loaded'
          )
        } else {
          volumeViewer.makeDoseComparison(
            volumeViewer.doseVolume,
            doseVolumeList[this.value]
          )
        }
      }
    })
  }

  /**
   * Create a dose comparison volume from two given dose volumes.
   *
   * @param {DoseVolume} doseVol1 The first dose volume to compare.
   * @param {DoseVolume} doseVol2 The second dose volume to compare.
   */
  // TODO: Move this logic to DoseComparisonVolume class
  makeDoseComparison (doseVol1, doseVol2) {
    // First normalize the dose data to turn into a percentage
    const doseArr1 = doseVol1.data.dose.map(
      (doseVal) => doseVal / doseVol1.data.maxDose
    )
    const doseArr2 = doseVol2.data.dose.map(
      (doseVal) => doseVal / doseVol2.data.maxDose
    )

    // Take the difference
    const doseDiff = new Array(doseArr1.length)
    for (let i = 0; i < doseArr1.length; i++) {
      if (doseArr1[i] || doseArr2[i]) {
        doseDiff[i] = (doseArr1[i] || 0) - (doseArr2[i] || 0)
      }
    }
    // Calculate the error for each
    const errArr2 = doseVol2.data.error
    const error = doseVol1.data.error.map((err1, i) =>
      Math.sqrt(err1 * err1 + errArr2[i] * errArr2[i])
    )

    // Make new volume
    const newData = {
      ...doseVol1.data,
      dose: doseDiff,
      error: error,
      maxDose: 1.0
    }

    const combinedFileName = doseVol1.fileName + '_' + doseVol2.fileName

    const doseComparisonVol = new DoseComparisonVolume(
      combinedFileName,
      this.mainViewerDimensions,
      this.legendDimensions,
      newData
    )

    doseComparisonVolumeList.push(doseComparisonVol)

    // Set dose to new difference volume
    this.setDoseVolume(doseComparisonVol)
  }

  /**
   * Build the HTML of the volume viewer object.
   *
   * @param {string} id The unique ID of the volume viewer.
   */
  buildBaseHtml (id) {
    // Select main div
    const base = d3.select('#image-to-print')

    // Add div to hold the panels, legend, and dose profiles
    this.volHolder = base
      .append('div')
      .attr('id', 'volume-holder-' + id)
      .attr('class', 'volume-holder')

    // Add the file selector dropdowns
    const fileSelector = this.volHolder
      .append('div')
      .attr('class', 'file-selector')
    this.densitySelector = fileSelector
      .append('select')
      .attr('name', 'density-file')
    this.densitySelector
      .append('option')
      .attr('value', -1)
      .text('Choose a density file')
    this.doseSelector = fileSelector.append('select').attr('name', 'dose-file')
    this.doseSelector
      .append('option')
      .attr('value', -1)
      .text('Choose a dose file')
    this.doseComparisonSelector = fileSelector
      .append('select')
      .attr('name', 'dose-file-comparison')
      .attr('disabled', 'disabled')
    this.doseComparisonSelector
      .append('option')
      .attr('value', -1)
      .text('Choose a dose file to compare')

    // Set up the file selector dropdowns
    this.setUpFileSelectors()

    // Put the checkboxes, buttons, and sliders in the option holder div
    const optionHolder = this.volHolder.append('div').attr('class', 'option-holder')

    // Add checkboxes for voxel information and dose profile plots
    const checkboxHolder = optionHolder.append('div').attr('class', 'option')
    const addCheckbox = (id, value, label, onChangeFunc) => {
      const checkboxDiv = checkboxHolder.append('div').attr('class', 'checkbox')
      const checkbox = checkboxDiv.append('input')
        .attr('type', 'checkbox')
        .attr('id', id)
        .attr('name', id)
        .attr('value', value)
        .attr('disabled', 'disabled')

      checkboxDiv.append('label')
        .attr('for', id)
        .text(label)

      checkbox.on('change', () => onChangeFunc(this, checkbox.node()))
      return checkbox
    }

    this.showVoxelInfoCheckbox = addCheckbox('show-marker-checkbox', 'ShowMarker', 'Show voxel information on click?',
      defineShowMarkerCheckboxBehaviour)
    this.showDoseProfileCheckbox = addCheckbox('show-dose-profile-checkbox', 'ShowDoseProfile',
      'Plot dose profile at crosshairs?', defineShowProfileCheckboxBehaviour)

    // Add buttons to export visualization and export to csv
    const buttonHolder = optionHolder.append('div').attr('class', 'option')
    const addButtons = (id, label, onClickFunc) => {
      const button = buttonHolder.append('button')
        .attr('id', id)
        .attr('class', 'button-text')
        .attr('disabled', 'disabled')
        .text(label)

      button.on('click', () => onClickFunc(this))
    }

    addButtons('save-vis', 'Export visualization to PNG', defineExportPNGButtonBehaviour)
    addButtons('save-dose-profile', 'Export dose profiles to CSV', defineExportCSVButtonBehaviour)

    // Add min and max density sliders
    const minMaxSliderHolder = optionHolder.append('div').attr('class', 'option')
    this.minParentDiv = minMaxSliderHolder
      .append('div')
      .attr('class', 'min-max-container')
    this.maxParentDiv = minMaxSliderHolder
      .append('div')
      .attr('class', 'min-max-container')

    // Add max dose slider
    const doseSliderHolder = optionHolder.append('div').attr('class', 'option')
    this.maxDoseParentDiv = doseSliderHolder
      .append('div')
      .attr('id', 'axis-slider-container')

    // Add voxel information
    this.voxelInfoDiv = buildVoxelInfoHtml(this.volHolder, id)

    // Append div to hold the panels
    this.viewerContainer = this.volHolder
      .append('span')
      .attr('class', 'container')
      .style('vertical-align', 'top')

    // Append div to hold the legend
    this.legendHolder = this.volHolder
      .append('span')
      .attr('class', 'legend-holder')

    // Build other html and class objects
    this.buildViewerContainer(this.mainViewerDimensions)
    this.buildLegend(this.legendDimensions)
    this.buildPanels(this.mainViewerDimensions)
  }

  /**
   * Build the HTML for the viewer container to hold the panels.
   *
   * @param {Object} mainViewerDimensions The pixel dimensions of the main viewer.
   */
  buildViewerContainer (mainViewerDimensions) {
    // TODO: In panel class, have build html instead
    const dimensions = ['z', 'x', 'y']

    // For each axis and plot class, make html
    const classes = ['axis-svg', 'plot-density', 'plot-dose', 'plot-marker']
    const type = ['svg', 'canvas', 'svg', 'svg']
    this.svgObjs = {
      'axis-svg': {},
      'plot-density': {},
      'plot-dose': {},
      'plot-marker': {}
    }

    this.axisObjs = {
      xy: {},
      yz: {},
      xz: {}
    }

    // Add html for panels and slice sliders
    AXES.forEach((axis, i) => {
      const selectedDiv = this.viewerContainer
        .append('div')
        .classed('panel-' + axis, true)
        .style('display', 'inline-block')

      // Slice slider callback and parameters
      var onSliceChangeCallback = (sliderVal) => {
        const currPanel = this.panels[axis]
        // Update slice of current panel
        this.updateSlice(axis, parseInt(sliderVal))

        // TODO: Fix this, bug after zooming/translating and changing slice
        // Update marker position, voxel information and dose profile
        const plotCoords = currPanel.markerPosition
        if (currPanel.showMarker()) {
          // ISSUE HERE: don't need to redraw other panels on slider change,
          // just marker position and voxel info
          const voxelCoords = coordsToVoxel(
            plotCoords,
            currPanel.axis,
            currPanel.densitySliceNum || currPanel.doseSliceNum,
            currPanel.volume,
            currPanel.zoomTransform
          )

          Object.values(this.panels).forEach((panel) => {
            if (panel.axis !== currPanel.axis) {
              let voxelNums
              if (panel.axis === 'xy') {
                voxelNums = [voxelCoords[0], voxelCoords[1]]
              } else if (panel.axis === 'yz') {
                voxelNums = [voxelCoords[1], voxelCoords[2]]
              } else {
                voxelNums = [voxelCoords[0], voxelCoords[2]]
              }

              // Convert voxel number to pixel value for both x and y coordinates
              const xScale = panel.volume.baseSlices[panel.axis].xPixelToVoxelScale.invertExtent
              const yScale = panel.volume.baseSlices[panel.axis].yPixelToVoxelScale.invertExtent

              let coords
              if (panel.zoomTransform) {
                coords = panel.zoomTransform.apply([
                  Math.ceil(xScale(voxelNums[0]).reduce((total, num) => total + num) / 2),
                  Math.ceil(yScale(voxelNums[1]).reduce((total, num) => total + num) / 2)
                ])
              } else {
                coords = [
                  Math.ceil(xScale(voxelNums[0]).reduce((total, num) => total + num) / 2),
                  Math.ceil(yScale(voxelNums[1]).reduce((total, num) => total + num) / 2)
                ]
              }

              panel.updateMarker(coords, false)
            }
          })

          const worldCoords = currPanel.coordsToWorld(plotCoords)
          updateVoxelCoords(
            currPanel.densityVol,
            currPanel.doseVol,
            worldCoords,
            currPanel.volumeViewerId,
            currPanel.showMarker,
            currPanel.showDoseProfile
          )
        }
      }

      const sliceSliderParams = {
        id: 'slice-number-' + axis,
        label: 'Slice Number',
        format: d3.format('d'),
        startingVal: 0,
        minVal: 0,
        maxVal: 1,
        step: 1,
        margin: {
          top: 0,
          right: mainViewerDimensions.margin.right,
          bottom: 0,
          left: mainViewerDimensions.margin.left
        },
        style: { 'text-align': 'center' },
        onSliderChangeCallback: onSliceChangeCallback
      }

      // Build new slider
      this.sliceSliders[axis] = new Slider(
        selectedDiv,
        sliceSliderParams
      )

      // Build div to hold the panel
      const imageHolder = selectedDiv
        .append('div')
        .classed('imageholder-' + axis, true)
        .classed('parent', true)

      classes.forEach((className, i) => {
        const layerNum = i + 1
        const plot = imageHolder
          .append('div')
          .classed('layer-' + layerNum, true)
          .style('z-index', layerNum)
          .append(type[i])
          .classed(className, true)
          .classed(axis, true)
          .classed('plot', true)
          .attr('width', mainViewerDimensions.width)
          .attr('height', mainViewerDimensions.height)
          .style('margin', this.getMarginStr(mainViewerDimensions.margin))

        this.svgObjs[className][axis] = plot
        this.axisObjs[axis][className] = plot
      })

      // Add dose profile to bottom of panel
      this.doseProfileList[i] = new DoseProfile(
        this.sideDoseProfileDimensions,
        selectedDiv,
        this.id + '-' + dimensions[i]
      )
    })
  }

  /**
   * Build the HTML for the dose and density legends.
   *
   * @param {Object} legendDimensions The pixel dimensions of the legends.
   */
  buildLegend (legendDimensions) {
    // Set up legends
    var getLegendHolderAndSvg = (className) => {
      const legendHolder = this.legendHolder
        .append('span')
        .attr('id', className + '-legend-holder')
        .style('width', legendDimensions.width + 'px')
        .style('height', legendDimensions.height + 'px')
        .style('margin', this.getMarginStr(legendDimensions.margin))

      const legendSvg = legendHolder
        .append('svg')
        .attr('id', className + '-legend-svg')
        .attr('class', 'legend')

      return [legendHolder, legendSvg]
    };

    [this.doseLegendHolder, this.doseLegendSvg] = getLegendHolderAndSvg('dose');
    [this.densityLegendHolder, this.densityLegendSvg] = getLegendHolderAndSvg(
      'density'
    )
  }

  /**
   * Create the panel object for each axis.
   *
   * @param {Object} mainViewerDimensions The pixel dimensions of the main viewer.
   */
  buildPanels (mainViewerDimensions) {
    this.panels = AXES.reduce((obj, axis, i) => {
      return {
        ...obj,
        [axis]: new Panel(
          mainViewerDimensions,
          this.densityVol,
          this.doseVol,
          axis,
          this.axisObjs[axis],
          this.sliceSliders[axis],
          this.showVoxelInfoCheckbox,
          this.showDoseProfileCheckbox,
          this.dispatch,
          this.id
        )
      }
    }, {})
  }

  /**
   * Set up the event dispatcher.
   */
  initializeDispatch () {
    // Set up marker coord change event
    const panels = this.panels
    const volumeViewer = this

    this.dispatch.on('markerchange.panels', function (d) {
      d.panel.updateMarker(d.plotCoords)

      // Want to get the voxel coords then change the sliceNum of the other volume panels
      const voxelCoords = coordsToVoxel(
        d.plotCoords,
        d.panel.axis,
        d.panel.densitySliceNum || d.panel.doseSliceNum,
        d.panel.volume,
        d.panel.zoomTransform
      )

      Object.values(panels).forEach((panel) => {
        if (panel.axis !== d.panel.axis) {
          let sliceNum, voxelNums
          if (panel.axis === 'xy') {
            sliceNum = voxelCoords[2]
            voxelNums = [voxelCoords[0], voxelCoords[1]]
          } else if (panel.axis === 'yz') {
            sliceNum = voxelCoords[0]
            voxelNums = [voxelCoords[1], voxelCoords[2]]
          } else {
            sliceNum = voxelCoords[1]
            voxelNums = [voxelCoords[0], voxelCoords[2]]
          }

          // Convert voxel number to pixel value for both x and y coordinates
          const xScale = panel.volume.baseSlices[panel.axis].xPixelToVoxelScale.invertExtent
          const yScale = panel.volume.baseSlices[panel.axis].yPixelToVoxelScale.invertExtent

          let coords

          if (panel.zoomTransform) {
            coords = panel.zoomTransform.apply([
              Math.ceil(xScale(voxelNums[0]).reduce((total, num) => total + num) / 2),
              Math.ceil(yScale(voxelNums[1]).reduce((total, num) => total + num) / 2)
            ])
          } else {
            coords = [
              Math.ceil(xScale(voxelNums[0]).reduce((total, num) => total + num) / 2),
              Math.ceil(yScale(voxelNums[1]).reduce((total, num) => total + num) / 2)
            ]
          }

          panel.updateMarker(coords, false)
          volumeViewer.updateSlice(panel.axis, sliceNum)
          panel.updateSlider(sliceNum)
        }
      })
    })

    this.dispatch.on('markerchange.voxelinfo', function (d) {
      const worldCoords = d.panel.coordsToWorld(d.plotCoords)
      updateVoxelCoords(
        d.panel.densityVol,
        d.panel.doseVol,
        worldCoords,
        d.panel.volumeViewerId,
        d.panel.showMarker,
        d.panel.showDoseProfile
      )
    })
  }

  /**
   * Enable the checkbox to view voxel information on click.
   */
  enableCheckboxForVoxelInformation () {
    if (this.showVoxelInfoCheckbox.node().disabled) this.showVoxelInfoCheckbox.node().disabled = false
  }

  /**
   * Enable the checkbox for the dose profile plots.
   */
  enableCheckboxForDoseProfilePlot () {
    if (this.showDoseProfileCheckbox.node().disabled) this.showDoseProfileCheckbox.node().disabled = false
  }

  /**
   * Sets the maximum density value for density plots.
   *
   * @param {number} maxDensityVal The maximum density value.
   */
  setMaxDensityVar (maxDensityVal) {
    this.maxDensityVar = parseFloat(maxDensityVal)

    // Redraw legend
    this.densityVolume.initializeLegend(this.minDensityVar, this.maxDensityVar)

    Object.values(this.panels).forEach((panel) => {
      panel.prevSliceImg = this.densityVolume.drawDensity(this.densityVolume.sliceCache[panel.axis][panel.densitySliceNum],
        panel.zoomTransform, panel.axisElements['plot-density'], this.minDensityVar, this.maxDensityVar)
    })
  }

  /**
   * Sets the minimum density value for density plots.
   *
   * @param {number} minDensityVal The minimum density value.
   */
  setMinDensityVar (minDensityVal) {
    this.minDensityVar = parseFloat(minDensityVal)

    // Redraw legend
    this.densityVolume.initializeLegend(this.minDensityVar, this.maxDensityVar)

    Object.values(this.panels).forEach((panel) => {
      panel.prevSliceImg = this.densityVolume.drawDensity(this.densityVolume.sliceCache[panel.axis][panel.densitySliceNum],
        panel.zoomTransform, panel.axisElements['plot-density'], this.minDensityVar, this.maxDensityVar)
    })
  }

  /**
   * Sets the maximum dose value for dose contour plots.
   *
   * @param {number} val The maximum dose percentage.
   */
  setMaxDose (val) {
    this.maxDoseVar = val * this.doseVolume.data.maxDose
    // Update the colour scheme and thresholds with the new max dose variable
    this.doseVolume.addColourScheme(d3.interpolateViridis, this.maxDoseVar, 0)
    this.updateThresholds()

    Object.values(this.panels).forEach((panel) => {
      this.doseVolume.drawDose(this.doseVolume.sliceCache[panel.axis][panel.doseSliceNum], panel.zoomTransform, panel.axisElements['plot-dose'], this.thresholds, this.className, this.maxDoseVar)
      if (panel.showDoseProfile()) {
        this.doseProfileList.forEach((doseProfile) =>
          doseProfile.plotData()
        )
      }
    })
  }

  initializeThresholds () {
    // Calculate the contour thresholds
    const contourInt = 0.1
    this.thresholdPercents = d3.range(contourInt, 1.0 + contourInt, contourInt)
    this.updateThresholds()
    // The className function multiplies by 1000 and rounds because decimals are not allowed in class names
    this.className = (i) =>
      'col-' + this.id + '-' + d3.format('d')(this.thresholdPercents[i] * 1000)
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
   */
  addThresholdPercent (thresholdPercent) {
    this.thresholdPercents.push(thresholdPercent)
    this.thresholdPercents.sort()
    this.updateThresholds()
    this.initializeDoseLegend(this.maxDoseVar, this.thresholds)

    Object.values(this.panels).forEach((panel) => {
      this.doseVolume.drawDose(this.doseVolume.sliceCache[panel.axis][panel.doseSliceNum], panel.zoomTransform, panel.axisElements['plot-dose'], this.thresholds, this.className, this.maxDoseVar)
    })
  }

  /**
 * Create the dose legend.
 *
 * @param {number} maxDoseVar The maximum dose to scale the contours with.
 * @param {number[]} thresholds The contour thresholds.
 */
  initializeDoseLegend (maxDoseVar = this.doseVolume.data.maxDose, thresholds) {
    // Get list of class names of hidden contours
    const colourFcn = d3.scaleSequentialSqrt(d3.interpolateViridis).domain([0, maxDoseVar])

    const hiddenContourClassList = this.doseVolume.getHiddenContourClassList()
    const legendSvg = this.doseLegendSvg
    const legendClass = 'doseLegend'
    const parameters = {
      labels: [
        this.thresholds.map((e) => d3.format('.0%')(e / maxDoseVar))
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
      Object.values(this.svgObjs['plot-dose']).forEach((svg) => {
        svg.selectAll('path.contour-path.' + className)
          .classed('hidden', function () {
            return !d3.select(this).classed('hidden')
          })
      })
    }

    // Clear and redraw current legend
    legendSvg.select('.' + legendClass).remove()
    legendSvg.select('text').remove()

    // Make space for legend title
    legendSvg
      .append('g')
      .attr('class', legendClass)
      .style('transform', 'translate(0px,' + 20 + 'px)')

    // Append title
    legendSvg
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
      .scale(colourFcn)

    // Apply all the parameters
    Object.entries(parameters).forEach(([name, val]) => {
      legend[name](...val)
    })

    legendSvg.select('.' + legendClass).call(legend)

    // Set the height of the svg so the div can scroll if need be
    const height =
      legendSvg
        .select('.' + legendClass)
        .node()
        .getBoundingClientRect().height + 20
    legendSvg.attr('height', height)

    // Add the appropriate classnames to each legend cell
    const len = this.thresholds.length - 1
    legendSvg
      .selectAll('g.cell')
      .attr('class', (d, i) => 'cell ' + this.className(len - i))

    if (hiddenContourClassList.length > 0) {
      // Apply hidden class to hidden contours
      const hiddenLegendCells = legendSvg
        .selectAll('g.cell')
        .filter(hiddenContourClassList.join(','))
      hiddenLegendCells.classed('hidden', !hiddenLegendCells.classed('hidden'))
    }
  }

  /**
   * Create the input box to add new dose contour thresholds.
   */
  initializeDoseContourInput () {
    const legendHolder = this.doseLegendHolder
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
          this.addThresholdPercent(newPercentage)
        }
      }
    }

    // Remove existing dose contour inputs
    legendHolder.selectAll('input').remove()

    const doseContourInputWidth = 45

    // Add number input box
    const submitDoseContour = legendHolder
      .append('input')
      .attr('type', 'number')
      .attr('name', 'add-dose-contour-line')
      .attr('id', 'add-dose-contour-line')
      .attr('min', 0)
      .attr('max', 100)
      .attr('step', 1)
      .style('width', doseContourInputWidth + 'px')

    // Add submit button
    legendHolder
      .append('input')
      .attr('type', 'submit')
      .attr('name', 'submit-dose-contour-line')
      .attr('id', 'submit-dose-contour-line')
      .attr('value', '+')
      .on('click', addNewThresholdPercent)
  }

  /**
 * Change the slice of the loaded volumes in the panel.
 *
 * @param {string} axis The axis of the current panel.
 * @param {number} sliceNum The number of the current slice displayed in the panel.
 */
  updateSlice (axis, sliceNum) {
    const panel = this.panels[axis]
    let slicePos, slice

    if (this.densityVolume) {
      slicePos = this.densityVolume.baseSlices[axis].zScale.invert(sliceNum)
      slice = this.densityVolume.getSlice(axis, slicePos)
      panel.prevSliceImg = this.densityVolume.drawDensity(slice, panel.zoomTransform, panel.axisElements['plot-density'], this.minDensityVar, this.maxDensityVar)
      panel.densitySliceNum = sliceNum
    }
    if (this.doseVolume) {
      slicePos = slicePos || this.doseVolume.baseSlices[axis].zScale.invert(sliceNum)
      slice = this.doseVolume.getSlice(axis, slicePos)
      this.doseVolume.drawDose(slice, panel.zoomTransform, panel.axisElements['plot-dose'], this.thresholds, this.className, this.maxDoseVar)
      panel.doseSliceNum = slice.sliceNum
    }
    panel.slicePos = slicePos
  }
}

// export { VolumeViewer }
