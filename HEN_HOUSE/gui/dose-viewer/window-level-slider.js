var windowSlider = d3.select("input.window-slider");
var levelSlider = d3.select("input.level-slider");

var initializeWindowAndLevelSlider = (densityVolume) => {
  windowSlider.attr("min", 0);
  windowSlider.attr("max", densityVolume.window);
  windowSlider.attr("step", 0.01);
  windowSlider.attr("value", densityVolume.window);

  levelSlider.attr("min", densityVolume.window / 2);
  levelSlider.attr(
    "max",
    densityVolume.data.maxDensity - densityVolume.window / 2
  );
  levelSlider.attr("step", 0.01);
  levelSlider.attr("value", densityVolume.level);
};

windowSlider.on("change", function () {
  densityVol.setWindow(this.value);
  densityVol.addColourScheme();
  densityVol.drawDensity(densityVol.prevSlice, canvDensity);

  // Fix level slider min and max vals
  levelSlider.attr("min", densityVol.window / 2);
  levelSlider.attr("max", densityVol.data.maxDensity - densityVol.window / 2);
  if (levelSlider.attr("value") > levelSlider.attr("max")) {
    levelSlider.attr("value", levelSlider.attr("max"));
  }
});

levelSlider.on("change", function () {
  densityVol.setLevel(this.value);
  densityVol.addColourScheme();
  densityVol.drawDensity(densityVol.prevSlice, canvDensity);
});
