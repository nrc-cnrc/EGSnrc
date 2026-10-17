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
/* global Image */
/* global Worker */

// REMOVE THESE GLOBAL IMPORTS ONCE MODULES RE-IMPLEMENTED
/* global Volume */

// import { Volume } from './volume.js'

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
    this.densityFormat = (args !== undefined) && (args.isDicom) ? d3.format('d') : d3.format('.2f')
    this.isDicom = (args !== undefined && args.isDicom) || false
    this.addColourScheme(this.data.maxDensity, this.data.minDensity)
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
            maxVal: this.data.maxDensity,
            minVal: this.data.minDensity
          }

          function handleMessage (e) {
            console.log('Message received from worker')
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
   * Make a density plot of the given slice.
   *
   * @param {Object} slice The slice of the density data.
   * @param {Object} [transform] The zoom transform of the plot.
   * @param {Object} svg The svg plot element.
   * @param {number} minDensityVar The minimum density to scale the image with.
   * @param {number} maxDensityVar The maximum density to scale the image with.
   * @returns {Object}
   */
  drawDensity (slice, transform, svg, minDensityVar, maxDensityVar) {
    // Get the canvas and context in the webpage
    const baseSlice = this.baseSlices[slice.axis]
    const imgCanvas = svg.node()
    const imgContext = imgCanvas.getContext('2d', { alpha: false })
    const imageData = this.getImageData(slice)

    // If the min and max density have been changed, apply colour map
    if ((minDensityVar !== this.data.minDensity) || (maxDensityVar !== this.data.maxDensity)) {
      const colourMap = d3.scaleSqrt().domain([minDensityVar, maxDensityVar]).range([0, 255])
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

// export { DensityVolume }
