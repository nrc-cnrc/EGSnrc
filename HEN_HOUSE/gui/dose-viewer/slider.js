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

/**
 * @class Generic number slider class.
 * */
class Slider {
  /**
   * Creates an instance of a Slider.
   *
   * @constructor
   * @param {Object} parentDiv The parent HTML element to build the slider on.
   * @param {function} onValChangeCallback The function to call when the slider
   * changes value.
   * @param {Object} params The parameters of the slider that includes id, label,
   * format, startingVal, minVal, maxVal, and step.
   */
  constructor (parentDiv, onValChangeCallback, params) {
    this.format = params.format
    this.onValChangeCallback = onValChangeCallback
    this.buildSliderHtml(
      parentDiv,
      params.id,
      params.label,
      params.margin,
      params.style
    )
    this.initializeBehaviour(
      params.format,
      params.startingVal,
      params.minVal,
      params.maxVal,
      params.step
    )
  }

  /**
   * Get current value of slider.
   *
   * @returns {number}
   */
  get value () {
    return this.slider.node().value
  }

  /**
   * Build the slider element on the webpage.
   *
   * @params {Object} parentDiv The parent HTML element to build the slider on.
   * @params {string} id The unique ID of the slider element.
   * @params {string} labelStr The text label of the slider.
   * @params {Object} [margin] The margin dimensions of the slider.
   * @params {Object} [style] The style applied to the slider.
   * @params {boolean} [incrementButtons = true] Whether or not to add increment
   * and decrement buttons.
   * @params {boolean} [disabled = true] Whether or not the slider is initially disabled.
   */
  buildSliderHtml (
    parentDiv,
    id,
    labelStr,
    margin,
    style,
    incrementButtons = true,
    disabled = false
  ) {
    const mainDiv = parentDiv.append('div').attr('class', 'slider-container')

    if (margin) {
      var getMarginStr = (margin) =>
        margin.top +
        'px ' +
        margin.right +
        'px ' +
        margin.bottom +
        'px ' +
        margin.left +
        'px'
      mainDiv.style('margin', getMarginStr(margin))
    }

    if (style) {
      Object.entries(style).forEach((styleEntry) =>
        mainDiv.style(...styleEntry)
      )
    }

    // Add label and slider value output
    const sliderValue = mainDiv
      .append('label')
      .attr('for', id)
      .text(labelStr + ': ')
      .append('output')
      .attr('type', 'text')
      .attr('id', 'slider-value-' + id)

    // Add break
    mainDiv.append('br')

    // Slider minimum output
    const sliderMin = mainDiv
      .append('output')
      .attr('type', 'text')
      .attr('id', 'slider-min-' + id)

    // Actual slider component
    const slider = mainDiv
      .append('input')
      .attr('type', 'range')
      .attr('class', 'slider')
      .attr('id', 'slider-range-' + id)
      .attr('min', 0)
      .attr('disabled', disabled ? 'disabled' : null)

    // Slider maximum output
    const sliderMax = mainDiv
      .append('output')
      .attr('type', 'text')
      .attr('id', 'slider-max-' + id)

    if (incrementButtons) {
      // Increment and decrement buttons
      const decrementNode = mainDiv
        .append('button')
        .attr('id', 'decrement-slider-' + id)
        .text('-')

      const incrementNode = mainDiv
        .append('button')
        .attr('id', 'increment-slider-' + id)
        .text('+')

      this.decrementNode = decrementNode
      this.incrementNode = incrementNode
    }

    this.slider = slider
    this.sliderMin = sliderMin
    this.sliderMax = sliderMax
    this.sliderValue = sliderValue
  }

  /**
   * Setup the behaviour of the slider.
   *
   * @params {function} format The function that formats the number output.
   * @params {number} startingVal The starting value of the slider.
   * @params {number} minVal The minimum value of the slider.
   * @params {number} maxVal The maximum value of the slider.
   * @params {number} step The interval between two values on the slider.
   */
  initializeBehaviour (format, startingVal, minVal, maxVal, step) {
    const sliderNode = this.slider.node()

    var updateSlider = (val) => {
      // Update slider text
      this.sliderValue.text(format(val))

      // Call value callback
      this.onValChangeCallback(val)
    }

    // On slider input, update text
    this.slider.on('input', function () {
      updateSlider(this.value)
    })

    if (this.incrementNode) {
      // On increment button push
      this.incrementNode.on('click', function () {
        sliderNode.stepUp(1)
        updateSlider(sliderNode.value)
      })
    }

    if (this.decrementNode) {
      // On decrement button push
      this.decrementNode.on('click', function () {
        sliderNode.stepDown(1)
        updateSlider(sliderNode.value)
      })
    }

    // Set the slider step
    this.slider.attr('step', step)

    // Set max and current value
    this.slider.attr('max', maxVal).node().value = startingVal

    // Show maximum value of slider
    this.sliderMax.text(format(maxVal))

    // Show minimum value of slider
    this.sliderMin.text(format(minVal))

    // Show current value of slider
    this.sliderValue.text(format(startingVal))
  }

  /**
   * Enable the slider if disabled.
   */
  enableSlider () {
    if (this.slider.attr('disabled')) this.slider.attr('disabled', null)
  }

  /**
   * Set the minimum value of the slider.
   *
   * @params {number} minVal The minimum value of the slider.
   */
  setMinValue (minVal) {
    // Set min value
    this.slider.attr('min', minVal)

    // Show maximum value of slider
    this.sliderMin.text(this.format(minVal))

    // Change current value to min if it is smaller
    if (this.sliderValue.text() < minVal) {
      this.setCurrentValue(minVal)
    }
  }

  /**
   * Set the maximum value of the slider.
   *
   * @params {number} maxVal The maximum value of the slider.
   */
  setMaxValue (maxVal) {
    // Set max value
    this.slider.attr('max', maxVal)

    // Show maximum value of slider
    this.sliderMax.text(this.format(maxVal))

    // Change current value to max if it is larger
    if (this.sliderValue.text() > maxVal) {
      this.setCurrentValue(maxVal)
    }
  }

  /**
   * Set the current value of the slider.
   *
   * @params {number} val The value of the slider to be set to.
   */
  setCurrentValue (val) {
    // Update slider range
    this.slider.node().value = val

    // Update slider text
    this.sliderValue.text(this.format(val))
  }
}

export { Slider }
