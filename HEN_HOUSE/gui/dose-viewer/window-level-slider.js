var initializeWindowAndLevelSlider = (
  levelParentDiv,
  windowParentDiv,
  densityVol,
  panels
) => {
  // Make level slider
  var levelSliderChangeCallback = (sliderVal) => {
    densityVol.setLevel(sliderVal);
    densityVol.addColourScheme();
    ["xy", "yz", "xz"].forEach((axis) =>
      densityVol.drawDensity(
        densityVol.prevSlice[axis],
        panels[axis].zoomTransform
      )
    );
  };

  let levelSliderParams = {
    id: "level",
    label: "Level",
    format: d3.format(".2f"),
    startingVal: densityVol.level,
    minVal: densityVol.window / 2,
    maxVal: densityVol.data.maxDensity - densityVol.window / 2,
    step: 0.01,
  };

  let levelSlider = new Slider(
    levelParentDiv,
    levelSliderChangeCallback,
    levelSliderParams
  );

  // Make window slider
  var windowSliderChangeCallback = (sliderVal) => {
    densityVol.setWindow(sliderVal);
    densityVol.addColourScheme();
    ["xy", "yz", "xz"].forEach((axis) =>
      densityVol.drawDensity(
        densityVol.prevSlice[axis],
        panels[axis].zoomTransform
      )
    );
    // Fix level slider min and max vals
    levelSlider.setMinValue(densityVol.window / 2);
    levelSlider.setMaxValue(densityVol.data.maxDensity - densityVol.window / 2);

    // Set level in density volume if changed
    densityVol.setLevel(levelSlider.value());
  };

  let windowSliderParams = {
    id: "window",
    label: "Window",
    format: d3.format(".2f"),
    startingVal: densityVol.window,
    minVal: 0.0,
    maxVal: densityVol.window,
    step: 0.01,
  };

  let windowSlider = new Slider(
    windowParentDiv,
    windowSliderChangeCallback,
    windowSliderParams
  );
};
