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
/* global MAIN_VIEWER_DIMENSIONS */

// import { MAIN_VIEWER_DIMENSIONS } from './index.js'

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
 * The zoom callback function for the density plots.
 *
 * @param {Object} transform The zoom transform object.
 * @param {DensityProfile} densityVol The density profile to be zoomed.
 * @param {Object} sliceImg The image of the slice to be zoomed.
 * @param {Object} canvas The canvas element to be zoomed.
 * @param {string} axis The axis of the slice to be zoomed.
 */
function zoomedCanvas (transform, densityVol, sliceImg, canvas, axis) {
  // Get the image to draw
  const baseSlice = densityVol.baseSlices[axis]
  const context = canvas.node().getContext('2d')

  // Clear the canvas, apply transformations, and redraw
  context.save()
  context.clearRect(0, 0, canvas.node().width, canvas.node().height)
  context.translate(transform.x, transform.y)
  context.scale(transform.k, transform.k)
  context.drawImage(sliceImg, 0, 0, baseSlice.xVoxels, baseSlice.yVoxels, baseSlice.dxDraw, baseSlice.dyDraw, baseSlice.dWidthDraw, baseSlice.dHeightDraw)
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
      panel.prevSliceImg,
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

  axisElements['plot-dose']
    .selectAll('g.roi-contour')
    .attr('transform', transform.toString())

  // Zoom marker
  axisElements['plot-marker']
    .select('g.marker')
    .attr('transform', transform.toString())

  // Create new scale objects based on event
  var newXScale = transform.rescaleX(volume.baseSlices[panel.axis].xScale)
  var newYScale = transform.rescaleY(volume.baseSlices[panel.axis].yScale)

  // Update axes
  axisElements['axis-svg']
    .select('.x-axis')
    .call(d3.axisBottom().scale(newXScale).ticks(6))
  axisElements['axis-svg']
    .select('.y-axis')
    .call(d3.axisLeft().scale(newYScale).ticks(6))

  // Update grid
  axisElements['axis-svg']
    .select('.x-axis-grid')
    .call(
      d3
        .axisBottom()
        .scale(newXScale)
        .tickSize(-MAIN_VIEWER_DIMENSIONS.height)
        .tickFormat('')
        .ticks(6)
    )
  axisElements['axis-svg']
    .select('.y-axis-grid')
    .call(
      d3
        .axisLeft()
        .scale(newYScale)
        .tickSize(-MAIN_VIEWER_DIMENSIONS.width)
        .tickFormat('')
        .ticks(6)
    )
}

// export { getZoom, zoomedAll, zoomedDoseProfile }
