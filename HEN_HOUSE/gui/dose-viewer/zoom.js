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
/* global mainViewerDimensions */
/* global  */
/* global  */
/* global  */
/* global  */
/* global  */
/* global  */

/**
 * Builds a zoom object according to the specs and callback passed in.
 *
 * @param {number} width The width of the zooming extent.
 * @param {number} height The height of the zooming extent.
 * @param {function} zoomCallback The function to determine what happens on
 * zoom.
 * @param {Array} args Any arguments to be passed into zoomCallback.
 * @returns {Object}
 */
var getZoom = (width, height, zoomCallback, args) => // eslint-disable-line no-unused-vars
  d3
    .zoom()
    .extent([
      [0, 0],
      [width, height]
    ])
    .scaleExtent([1, 8])
    .on('zoom', () => zoomCallback(d3.event.transform, ...args))

/**
 * The zoom callback function for dose profiles.
 *
 * @param {Object} transform The zoom transform object.
 * @param {DoseProfile} doseProfile The dose profile to be zoomed.
 */
function zoomedDoseProfile (transform, doseProfile) { // eslint-disable-line no-unused-vars
  doseProfile.zoomTransform = transform
  doseProfile.svg
    .selectAll('path.lines')
    .attr('transform', transform.toString())

  // Create new scale objects based on event
  var newxScale = transform.rescaleX(doseProfile.xScale)
  var newyDoseScale = transform.rescaleY(doseProfile.yDoseScale)

  // Update axes
  doseProfile.svg
    .select('.profile-x-axis')
    .call(
      d3.axisBottom().scale(newxScale).tickSize(-doseProfile.dimensions.height)
    )

  doseProfile.svg
    .select('.profile-y-dose-axis')
    .call(
      d3
        .axisLeft()
        .scale(newyDoseScale)
        .ticks(doseProfile.yTicks)
        .tickFormat(d3.format('.0%'))
        .tickSize(-doseProfile.dimensions.width)
    )

  if (doseProfile.densityChecked()) {
    var newyDensityScale = transform.rescaleY(doseProfile.yDensityScale)
    doseProfile.svg
      .select('.profile-y-density-axis')
      .call(
        d3
          .axisLeft()
          .scale(newyDensityScale)
          .ticks(doseProfile.yTicks)
          .tickSize(-doseProfile.dimensions.width)
      )
  }
}

/**
 * The zoom callback function for the density plots.
 *
 * @param {Object} transform The zoom transform object.
 * @param {DensityProfile} densityVol The density profile to be zoomed.
 * @param {Object} canvas The canvas element to be zoomed.
 * @param {string} axis The axis of the slice to be zoomed.
 */
function zoomedCanvas (transform, densityVol, canvas, axis) {
  // Get the image to draw
  const image = densityVol.prevSliceImg[axis]
  const context = canvas.node().getContext('2d')

  // Clear the canvas, apply transformations, and redraw
  context.clearRect(0, 0, canvas.node().width, canvas.node().height)
  context.save()
  context.translate(transform.x, transform.y)
  context.scale(transform.k, transform.k)
  context.drawImage(image, 0, 0)
  context.restore()
}

/**
 * The zoom callback function density canvas, dose svg, axes, and markers.
 *
 * @param {Object} transform The zoom transform object.
 * @param {Panel} panel The panel to be zoomed on.
 */
function zoomedAll (transform, panel) { // eslint-disable-line no-unused-vars
  panel.zoomTransform = transform
  const axisElements = panel.axisElements
  const volume = panel.volume
  const axis = panel.axis

  // Zoom on canvas
  if (panel.densityVol) {
    zoomedCanvas(
      transform,
      panel.densityVol,
      axisElements['plot-density'],
      axis
    )
  }

  // Zoom dose plot
  if (panel.doseVol) {
    axisElements['plot-dose']
      .select('g.dose-contour')
      .attr('transform', transform.toString())
  }

  // Zoom marker
  axisElements['plot-marker']
    .select('g.marker')
    .attr('transform', transform.toString())

  // Create new scale ojects based on event
  var newxScale = transform.rescaleX(volume.prevSlice[panel.axis].xScale)
  var newyScale = transform.rescaleY(volume.prevSlice[panel.axis].yScale)

  // Update axes
  axisElements['axis-svg']
    .select('.x-axis')
    .call(d3.axisBottom().scale(newxScale).ticks(6))
  axisElements['axis-svg']
    .select('.y-axis')
    .call(d3.axisLeft().scale(newyScale).ticks(6))

  // Update grid
  axisElements['axis-svg']
    .select('.x-axis-grid')
    .call(
      d3
        .axisBottom()
        .scale(newxScale)
        .tickSize(-mainViewerDimensions.height)
        .tickFormat('')
        .ticks(6)
    )
  axisElements['axis-svg']
    .select('.y-axis-grid')
    .call(
      d3
        .axisLeft()
        .scale(newyScale)
        .tickSize(-mainViewerDimensions.width)
        .tickFormat('')
        .ticks(6)
    )
}
