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

// import { MAIN_VIEWER_DIMENSIONS } from './index.js'
// import { updateVoxelCoords } from './voxel-coordinates.js'
// import { getZoom, zoomedAll } from './zoom.js'

/** @class Panel holds one axis view and detects clicks, stores information, and
 * updates plots */
// TODO: Build panel HTML inside panel object
class Panel {
  /**
   * Creates an instance of a Panel.
   *
   * @constructor
   * @param {Object} dimensions The pixel dimensions of the panel.
   * @param {DensityVolume} densityVol The density volume of the panel.
   * @param {DoseVolume} doseVol The dose volume of the panel.
   * @param {String} axis The axis (x, y, z) of the volume that is shown in the panel.
   * @param {Object} axisElements The HTML elements to plot the axes, density,
   * dose, and crosshairs.
   * @param {Slider} sliceSlider The slider object used to move through slices.
   * @param {Object} dispatch The event dispatcher used for actions on panel clicks.
   * @param {string} id The unique ID of the panel.
   * @param {Object} zoomTransform Holds information about the current transform
   * of the slice.
   * @param {number[]} markerPosition The current position of the marker.
   */
  constructor (
    dimensions,
    densityVol,
    doseVol,
    axis,
    axisElements,
    sliceSlider,
    dispatch,
    id,
    zoomTransform = null,
    markerPosition = null
  ) {
    this.dimensions = dimensions
    this.densityVol = densityVol
    this.doseVol = doseVol
    this.volume = doseVol || densityVol
    this.axis = axis
    this.axisElements = axisElements
    this.sliceSlider = sliceSlider
    this.dispatch = dispatch
    this.volumeViewerId = id
    this.zoomTransform = zoomTransform
    this.markerPosition = markerPosition
    this.densitySliceNum = null
    this.doseSliceNum = null
    this.slicePos = null

    // Properties to check values of voxel and dose profile checkboxes
    this.showMarker = () =>
      d3.select("input[name='show-marker-checkbox']").node().checked
    this.showCrosshairs = () =>
      d3.select("input[name='show-dose-profile-checkbox']").node().checked

    // Update circle marker position and voxel coords on click
    const panel = this
    axisElements['plot-marker'].on('click', function () {
      const plotCoords = d3.mouse(this)

      if (d3.event.defaultPrevented) return
      dispatch.call('markerchange', this, {
        plotCoords: plotCoords,
        panel: panel
      })

      return true
    })
  }

  /**
   * Set up zoom for panel.
   */
  setupZoom () {
    const mainViewerZoom = getZoom(
      MAIN_VIEWER_DIMENSIONS.width,
      MAIN_VIEWER_DIMENSIONS.height,
      zoomedAll,
      [this]
    )
    this.axisElements['plot-marker'].call(mainViewerZoom)
  }

  /**
   * Show crosshairs if plot dose checkbox is selected.
   */
  updateCrosshairDisplay () {
    this.axisElements['plot-marker']
      .selectAll('line.crosshair')
      .style('display', this.showCrosshairs() ? '' : 'none')
  }

  /**
   * Show circle marker if show voxel info checkbox is selected.
   */
  updateCircleMarkerDisplay () {
    this.axisElements['plot-marker']
      .select('circle.crosshair')
      .style('display', this.showMarker() ? '' : 'none')
  }

  /**
   * Change the slice of the loaded volumes in the panel.
   *
   * @param {number} sliceNum The number of the current slice displayed in the panel.
   */
  updateSlice (sliceNum) {
    let slicePos, slice

    if (this.densityVol) {
      slicePos = this.densityVol.baseSlices[this.axis].zScale.invert(sliceNum)
      slice = this.densityVol.getSlice(this.axis, slicePos)
      this.densityVol.drawDensity(slice, this.zoomTransform, this.axisElements['plot-density'])
      this.densitySliceNum = sliceNum
    }
    if (this.doseVol) {
      slicePos = slicePos || this.doseVol.baseSlices[this.axis].zScale.invert(sliceNum)
      slice = this.doseVol.getSlice(this.axis, slicePos)
      this.doseVol.drawDose(slice, this.zoomTransform, this.axisElements['plot-dose'])
      this.doseSliceNum = slice.sliceNum
    }
    this.slicePos = slicePos
  }

  /**
   * Create the drag behaviour of the circle marker and crosshairs.
   *
   * @returns {Object}
   */
  getDrag () {
    const panel = this

    // Define the drag attributes
    function dragstarted () {
      d3.select(this).raise()
      d3.select(this).attr('cursor', 'grabbing')
    }

    function dragged () {
      var x = d3.event.x
      var y = d3.event.y

      d3.select(this).select('circle').attr('cx', x).attr('cy', y)
      d3.select(this).select('line.crosshairX').attr('x1', x).attr('x2', x)
      d3.select(this).select('line.crosshairY').attr('y1', y).attr('y2', y)

      const worldCoords = panel.coordsToWorld(panel.zoomTransform ? panel.zoomTransform.apply([x, y]) : [x, y])
      updateVoxelCoords(
        panel.densityVol,
        panel.doseVol,
        worldCoords,
        panel.volumeViewerId
      )
    }

    function dragended () {
      d3.select(this).attr('cursor', 'grab')

      const plotCoords = panel.zoomTransform ? panel.zoomTransform.apply([d3.event.x, d3.event.y]) : [d3.event.x, d3.event.y]

      if (d3.event.defaultPrevented) return
      panel.dispatch.call('markerchange', this, {
        plotCoords: plotCoords,
        panel: panel
      })
    }

    return d3
      .drag()
      .on('start', dragstarted)
      .on('drag', dragged)
      .on('end', dragended)
  }

  /**
   * Update the marker position on the panel.
   *
   * @param {number[]} coords The coordinates of the new marker position.
   * @param {boolean} [activePanel = true] Whether it is the active panel (i.e.
   * most recently interacted with)
   */
  updateMarker (coords, activePanel = true) {
    this.markerPosition = coords

    // Remove old marker and crosshairs
    this.axisElements['plot-marker'].select('.marker').remove()

    // If there is existing transformation, calculate proper x and y coordinates
    const [x, y] = this.zoomTransform
      ? this.zoomTransform.invert(coords)
      : coords

    // Add new marker with modified coordinates so it can smoothly transform with other elements
    var markerHolder = this.axisElements['plot-marker']
      .append('g')
      .attr('class', 'marker')
      .attr(
        'transform',
        this.zoomTransform ? this.zoomTransform.toString() : ''
      )
      .attr('cursor', activePanel ? 'grab' : '')
      .append('g')
      .attr('class', 'marker-holder')

    // Add drag functionality if active panel
    if (activePanel) {
      markerHolder.call(this.getDrag())
    }

    // Create centre circle
    markerHolder
      .append('circle')
      .attr('cx', x)
      .attr('cy', y)
      .classed('crosshair', true)
      .attr('r', 2)
      .style('display', this.showMarker() ? '' : 'none')
      .classed('active', activePanel)

    // Create horizontal line
    markerHolder
      .append('line')
      .classed('crosshair', true)
      .classed('crosshairX', true)
      .attr('x1', x)
      .attr('y1', 0)
      .attr('x2', x)
      .attr('y2', MAIN_VIEWER_DIMENSIONS.height)
      .style('display', this.showCrosshairs() ? '' : 'none')
      .classed('active', activePanel)

    // Create vertical line
    markerHolder
      .append('line')
      .classed('crosshair', true)
      .classed('crosshairY', true)
      .attr('x1', 0)
      .attr('y1', y)
      .attr('x2', MAIN_VIEWER_DIMENSIONS.width)
      .attr('y2', y)
      .style('display', this.showCrosshairs() ? '' : 'none')
      .classed('active', activePanel)
  }

  /**
   * Update the current value of the slice slider.
   *
   * @param {number} sliceNum The number of the current slice displayed in the panel.
   */
  updateSlider (sliceNum) {
    this.sliceSlider.setCurrentValue(sliceNum)
  }

  /**
   * Convert the marker position coordinates to world coordinates.
   *
   * @param {number[]} coords The coordinates of the marker position.
   */
  coordsToWorld (coords) {
    const volume = this.densityVol || this.doseVol
    const axis = this.axis
    const sliceNum = this.densitySliceNum || this.doseSliceNum
    const transform = this.zoomTransform

    // Invert transformation if applicable then invert scale to get world coordinate
    const i = volume.baseSlices[axis].xScale.invert(
      transform ? transform.invert(coords)[0] : coords[0]
    )
    const j = volume.baseSlices[axis].yScale.invert(
      transform ? transform.invert(coords)[1] : coords[1]
    )

    const k = volume.baseSlices[axis].zScale.invert(parseInt(sliceNum))

    const [xVal, yVal, zVal] =
      axis === 'xy' ? [i, j, k] : axis === 'yz' ? [k, i, j] : [i, k, j]
    return [xVal, yVal, zVal]
  }

  setDensityVolume (densityVol, slicePos) {
    this.densityVol = densityVol
    this.slicePos = slicePos

    if (this.doseVol !== undefined) {
      this.adjustDoseBaseSlices()
    } else {
      this.setupZoom()
    }

    this.densitySliceNum = Math.round(this.densityVol.baseSlices[this.axis].zScale(slicePos))
    this.volume = densityVol
  }

  setDoseVolume (doseVol, slicePos) {
    this.doseVol = doseVol
    this.slicePos = slicePos

    // If existing density volume, adjust doseVol baseSlices
    if (this.densityVol !== undefined) {
      this.adjustDoseBaseSlices()
    } else {
      this.setupZoom()
      this.volume = doseVol
    }

    this.doseSliceNum = Math.round(this.doseVol.baseSlices[this.axis].zScale(slicePos))
  }

  adjustDoseBaseSlices () {
    const AXES = ['xy', 'yz', 'xz']

    AXES.forEach((axis) => {
      const densityBaseSlice = this.densityVol.baseSlices[axis]
      const x = this.doseVol.baseSlices[axis].x
      const y = this.doseVol.baseSlices[axis].y
      const xVoxels = this.doseVol.baseSlices[axis].xVoxels
      const yVoxels = this.doseVol.baseSlices[axis].yVoxels
      const xRange = [Math.round(densityBaseSlice.xScale(x[0])), Math.round(densityBaseSlice.xScale(x[x.length - 1]))]
      const yRange = [Math.round(densityBaseSlice.yScale(y[0])), Math.round(densityBaseSlice.yScale(y[y.length - 1]))]

      const contourXScale = d3
        .scaleLinear()
        .domain([0, xVoxels])
        .range(xRange)
      const contourYScale = d3
        .scaleLinear()
        .domain(axis === 'xy' ? [yVoxels, 0] : [0, yVoxels])
        .range(axis === 'xy' ? yRange.reverse() : yRange)

      this.doseVol.baseSlices[axis].contourTransform = ({ type, value, coordinates }) => ({
        type,
        value,
        coordinates: coordinates.map((rings) =>
          rings.map((points) =>
            points.map(([i, j]) => [contourXScale(i), contourYScale(j)])
          )
        )
      })
    })
  }
}

// export { Panel }
