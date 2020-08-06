var windowSlider = d3.select("input.window-slider");
var levelSlider = d3.select("input.level-slider");

var initializeWindowAndLevelSlider = (densityVolume) => {
  windowSlider.attr("min", 0);
  windowSlider.attr("max", densityVolume.window);
  windowSlider.attr("step", 0.01);
  windowSlider.attr("value", densityVolume.window);
  // Show the current window value
  d3.select("#window-value").node().value = densityVolume.window;

  levelSlider.attr("min", densityVolume.window / 2);
  levelSlider.attr(
    "max",
    densityVolume.data.maxDensity - densityVolume.window / 2
  );
  levelSlider.attr("step", 0.01);
  levelSlider.attr("value", densityVolume.level);

  // Show the current level value
  d3.select("#level-value").node().value = densityVolume.level;
};

windowSlider.on("input", function () {
  densityVol.setWindow(this.value);
  densityVol.addColourScheme();
  ["xy", "yz", "xz"].forEach((axis) =>
    densityVol.drawDensity(densityVol.prevSlice[axis])
  );

  // Show the current window value
  d3.select("#window-value").node().value = this.value;

  // Fix level slider min and max vals
  levelSlider.attr("min", densityVol.window / 2);
  levelSlider.attr("max", densityVol.data.maxDensity - densityVol.window / 2);
  if (levelSlider.attr("value") > levelSlider.attr("max")) {
    levelSlider.attr("value", levelSlider.attr("max"));
  }
});

levelSlider.on("input", function () {
  densityVol.setLevel(this.value);
  densityVol.addColourScheme();
  ["xy", "yz", "xz"].forEach((axis) =>
    densityVol.drawDensity(densityVol.prevSlice[axis])
  );

  // Show the current level value
  d3.select("#level-value").node().value = this.value;
});
