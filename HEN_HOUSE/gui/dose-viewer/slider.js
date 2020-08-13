// TODO: Make slider for sliceNum
// TODO: Make slider for window/level
class Slider {
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
      .attr("id", "slider-value-" + id)
      .attr("value", " ");

    // Add break
    mainDiv.append("br");

    // Slider minimum output
    let sliderMin = mainDiv
      .append("output")
      .attr("type", "text")
      .attr("id", "slider-min-" + id)
      .attr("value", " ");

    // Actual slider component
    let slider = mainDiv
      .append("input")
      .attr("type", "range")
      .attr("class", "slider")
      .attr("id", "slider-range-" + id)
      .attr("min", 0)
      .attr("value", 0)
      .attr("disabled", disabled ? "disabled" : null);

    // Slider maximum output
    let sliderMax = mainDiv
      .append("output")
      .attr("type", "text")
      .attr("id", "slider-max-" + id)
      .attr("value", "");

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
    this.slider.attr("max", maxVal).attr("value", startingVal);

    // Show maximum value of slider
    this.sliderMax.text(format(maxVal));

    // Show minimum value of slider
    this.sliderMin.text(format(minVal));

    // Show current value of slider
    this.sliderValue.text(format(startingVal));
  }

  enableSlider() {
    if (this.slider.attr("disabled")) this.slider.attr("disabled", null);
  }

  setMaxValue(maxVal) {
    // Set max value
    this.slider.attr("max", maxVal);

    // Show maximum value of slider
    this.sliderMax.text(this.format(maxVal));
  }

  setCurrentValue(val) {
    // Update slider range
    this.slider.node().value = val;

    // Update slider text
    this.sliderValue.text(this.format(val));
  }
}
