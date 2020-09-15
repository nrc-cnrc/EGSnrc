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
/* global getZoom */
/* global mainViewerDimensions */
/* global zoomedAll */
/* global updateVoxelCoords */
/* global applyTransform */
/* global invertTransform */

/** @class Panel holds one axis view and detects clicks, stores information, and
 * updates plots */
// TODO: Build panel HTML inside panel object
class Panel { // eslint-disable-line no-unused-vars
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
      mainViewerDimensions.width,
      mainViewerDimensions.height,
      zoomedAll,
      [this]
    )
    this.axisElements['plot-marker'].call(mainViewerZoom)
  }

  /**
   * Return the slice number of the current volume loaded in the panel.
   *
   * @returns {number}
   */
  get sliceNum () {
    return this.volume.prevSlice[this.axis].sliceNum
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
    let slice

    if (this.densityVol) {
      slice = this.densityVol.getSlice(this.axis, sliceNum)
      this.densityVol.drawDensity(slice, this.zoomTransform)
    }
    if (this.doseVol) {
      slice = this.doseVol.getSlice(this.axis, sliceNum)
      this.doseVol.drawDose(slice, this.zoomTransform)
    }
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

      // The d3.event coords are same regardless of zoom, so pass in null as transform
      updateVoxelCoords(
        panel.densityVol,
        panel.doseVol,
        [x, y],
        panel.axis,
        panel.sliceNum,
        null,
        panel.volumeViewerId
      )
    }

    function dragended () {
      d3.select(this).attr('cursor', 'grab')

      const x = panel.zoomTransform
        ? applyTransform(d3.event.x, panel.zoomTransform, 'x')
        : d3.event.x
      const y = panel.zoomTransform
        ? applyTransform(d3.event.y, panel.zoomTransform, 'y')
        : d3.event.y

      if (d3.event.defaultPrevented) return
      panel.dispatch.call('markerchange', this, {
        plotCoords: [x, y],
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
    const x = this.zoomTransform
      ? invertTransform(coords[0], this.zoomTransform, 'x')
      : coords[0]
    const y = this.zoomTransform
      ? invertTransform(coords[1], this.zoomTransform, 'y')
      : coords[1]

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
      .attr('y2', mainViewerDimensions.height)
      .style('display', this.showCrosshairs() ? '' : 'none')
      .classed('active', activePanel)

    // Create vertical line
    markerHolder
      .append('line')
      .classed('crosshair', true)
      .classed('crosshairY', true)
      .attr('x1', 0)
      .attr('y1', y)
      .attr('x2', mainViewerDimensions.width)
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
}
