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
  constructor(parentDiv, onValChangeCallback, params) {
    this.format = params.format;
    this.onValChangeCallback = onValChangeCallback;
    this.buildSliderHtml(parentDiv, params.id, params.label);
    this.initializeBehaviour(
      params.format,
      params.startingVal,
      params.minVal,
      params.maxVal,
      params.step
    );
  }

  /**
   * Get current value of slider.
   *
   * @returns {number}
   */
  get value() {
    return this.slider.node().value;
  }

  /**
   * Build the slider element on the webpage.
   *
   * @params {Object} parentDiv The parent HTML element to build the slider on.
   * @params {string} id The unique ID of the slider element.
   * @params {string} labelStr The text label of the slider.
   * @params {boolean} [incrementButtons = true] Whether or not to add increment
   * and decrement buttons.
   * @params {boolean} [disabled = true] Whether or not the slider is initially disabled.
   */
  buildSliderHtml(
    parentDiv,
    id,
    labelStr,
    incrementButtons = true,
    disabled = false
  ) {
    let mainDiv = parentDiv.append("div").attr("class", "slider-container");

    // Add label and slider value output
    let sliderValue = mainDiv
      .append("label")
      .attr("for", id)
      .text(labelStr + ": ")
      .append("output")
      .attr("type", "text")
      .attr("id", "slider-value-" + id);

    // Add break
    mainDiv.append("br");

    // Slider minimum output
    let sliderMin = mainDiv
      .append("output")
      .attr("type", "text")
      .attr("id", "slider-min-" + id);

    // Actual slider component
    let slider = mainDiv
      .append("input")
      .attr("type", "range")
      .attr("class", "slider")
      .attr("id", "slider-range-" + id)
      .attr("min", 0)
      .attr("disabled", disabled ? "disabled" : null);

    // Slider maximum output
    let sliderMax = mainDiv
      .append("output")
      .attr("type", "text")
      .attr("id", "slider-max-" + id);

    if (incrementButtons) {
      // Increment and decrement buttons
      let decrementNode = mainDiv
        .append("button")
        .attr("id", "decrement-slider-" + id)
        .text("-");

      let incrementNode = mainDiv
        .append("button")
        .attr("id", "increment-slider-" + id)
        .text("+");

      this.decrementNode = decrementNode;
      this.incrementNode = incrementNode;
    }

    this.slider = slider;
    this.sliderMin = sliderMin;
    this.sliderMax = sliderMax;
    this.sliderValue = sliderValue;
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
  initializeBehaviour(format, startingVal, minVal, maxVal, step) {
    let sliderNode = this.slider.node();

    var updateSlider = (val) => {
      // Update slider text
      this.sliderValue.text(format(val));

      // Call value callback
      this.onValChangeCallback(val);
    };

    // On slider input, update text
    this.slider.on("input", function () {
      updateSlider(this.value);
    });

    if (this.incrementNode) {
      // On increment button push
      this.incrementNode.on("click", function () {
        sliderNode.stepUp(1);
        updateSlider(sliderNode.value);
      });
    }

    if (this.decrementNode) {
      // On decrement button push
      this.decrementNode.on("click", function () {
        sliderNode.stepDown(1);
        updateSlider(sliderNode.value);
      });
    }

    // Set the slider step
    this.slider.attr("step", step);

    // Set max and current value
    this.slider.attr("max", maxVal).node().value = startingVal;

    // Show maximum value of slider
    this.sliderMax.text(format(maxVal));

    // Show minimum value of slider
    this.sliderMin.text(format(minVal));

    // Show current value of slider
    this.sliderValue.text(format(startingVal));
  }

  /**
   * Enable the slider if disabled.
   */
  enableSlider() {
    if (this.slider.attr("disabled")) this.slider.attr("disabled", null);
  }

  /**
   * Set the minimum value of the slider.
   *
   * @params {number} minVal The minimum value of the slider.
   */
  setMinValue(minVal) {
    // Set min value
    this.slider.attr("min", minVal);

    // Show maximum value of slider
    this.sliderMin.text(this.format(minVal));

    // Change current value to min if it is smaller
    if (this.sliderValue.text() < minVal) {
      this.setCurrentValue(minVal);
    }
  }

  /**
   * Set the maximum value of the slider.
   *
   * @params {number} maxVal The maximum value of the slider.
   */
  setMaxValue(maxVal) {
    // Set max value
    this.slider.attr("max", maxVal);

    // Show maximum value of slider
    this.sliderMax.text(this.format(maxVal));

    // Change current value to max if it is larger
    if (this.sliderValue.text() > maxVal) {
      this.setCurrentValue(maxVal);
    }
  }

  /**
   * Set the current value of the slider.
   *
   * @params {number} val The value of the slider to be set to.
   */
  setCurrentValue(val) {
    // Update slider range
    this.slider.node().value = val;

    // Update slider text
    this.sliderValue.text(this.format(val));
  }
}
