// Detect clicks
// Store zoom, cursor info
// Draw axes
// https://github.com/aces/brainbrowser/blob/master/src/brainbrowser/volume-viewer/lib/panel.js#L165
// TODO: Make voxel info work when slice changes!!!

class Panel {
  constructor(
    dimensions,
    densityVol,
    doseVol,
    axis,
    axisElements,
    sliceSlider,
    dispatch,
    sliceNum = 0,
    zoomTransform = null,
    markerPosition = null
  ) {
    this.dimensions = dimensions;
    this.densityVol = densityVol;
    this.doseVol = doseVol;
    this.volume = doseVol || densityVol;
    this.axis = axis;
    this.axisElements = axisElements;
    this.sliceSlider = sliceSlider;
    this.dispatch = dispatch;
    this.sliceNum = sliceNum;
    this.zoomTransform = zoomTransform;
    this.markerPosition = markerPosition;
    this.showMarker = () =>
      d3.select("input[name='show-marker-checkbox']").node().checked;
    this.showCrosshairs = () =>
      d3.select("input[name='show-dose-profile-checkbox']").node().checked;

    // Set up zoom for panel
    let mainViewerZoom = getZoom(
      mainViewerDimensions.width,
      mainViewerDimensions.height,
      zoomedAll,
      [this]
    );
    this.axisElements["plot-marker"].call(mainViewerZoom);

    // Update circle marker position and voxel coords on click
    let panel = this;
    axisElements["plot-marker"].on("click", function () {
      let plotCoords = d3.mouse(this);

      if (d3.event.defaultPrevented) return;
      dispatch.call("markerchange", this, {
        plotCoords: plotCoords,
        panel: panel,
      });

      return true;
    });
  }

  updateSliceNum() {
    this.sliceNum = this.volume.prevSlice[this.axis].sliceNum;
  }

  updateCrosshairDisplay() {
    this.axisElements["plot-marker"]
      .selectAll("line.crosshair")
      .style("display", this.showCrosshairs() ? "" : "none");
  }

  updateCircleMarkerDisplay() {
    this.axisElements["plot-marker"]
      .select("circle.crosshair")
      .style("display", this.showMarker() ? "" : "none");
  }

  updateSlice(sliceNum) {
    this.sliceNum = sliceNum;

    let slice;

    if (this.densityVol) {
      slice = this.densityVol.getSlice(this.axis, sliceNum);
      this.densityVol.drawDensity(slice, this.zoomTransform);
    }
    if (this.doseVol) {
      slice = this.doseVol.getSlice(this.axis, sliceNum);
      this.doseVol.drawDose(slice, this.zoomTransform);
    }
  }

  getDrag() {
    let panel = this;

    // Define the drag attributes
    function dragstarted() {
      d3.select(this).raise();
      d3.select(this).attr("cursor", "grabbing");
    }

    function dragged() {
      var x = d3.event.x;
      var y = d3.event.y;

      d3.select(this).select("circle").attr("cx", x).attr("cy", y);
      d3.select(this).select("line.crosshairX").attr("x1", x).attr("x2", x);
      d3.select(this).select("line.crosshairY").attr("y1", y).attr("y2", y);

      // The d3.event coords are same regardless of zoom, so pass in null as transform
      updateVoxelCoords(
        panel.densityVol,
        panel.doseVol,
        [x, y],
        panel.axis,
        panel.sliceNum,
        null,
        true
      );
    }

    function dragended() {
      d3.select(this).attr("cursor", "grab");

      let x = panel.zoomTransform
        ? applyTransform(d3.event.x, panel.zoomTransform, "x")
        : d3.event.x;
      let y = panel.zoomTransform
        ? applyTransform(d3.event.y, panel.zoomTransform, "y")
        : d3.event.y;

      if (d3.event.defaultPrevented) return;
      panel.dispatch.call("markerchange", this, {
        plotCoords: [x, y],
        panel: panel,
      });
    }

    return d3
      .drag()
      .on("start", dragstarted)
      .on("drag", dragged)
      .on("end", dragended);
  }

  updateMarker(coords, activePanel = true) {
    this.markerPosition = coords;

    // Remove old marker and crosshairs
    this.axisElements["plot-marker"].select(".marker").remove();

    // If there is existing transformation, calculate proper x and y coordinates
    let x = this.zoomTransform
      ? invertTransform(coords[0], this.zoomTransform, "x")
      : coords[0];
    let y = this.zoomTransform
      ? invertTransform(coords[1], this.zoomTransform, "y")
      : coords[1];

    // Add new marker with modified coordinates so it can smoothly transform with other elements
    var markerHolder = this.axisElements["plot-marker"]
      .append("g")
      .attr("class", "marker")
      .attr(
        "transform",
        this.zoomTransform ? this.zoomTransform.toString() : ""
      )
      .attr("cursor", activePanel ? "grab" : "")
      .append("g")
      .attr("class", "marker-holder");

    // Add drag functionality if active panel
    if (activePanel) {
      markerHolder.call(this.getDrag());
    }

    // Create centre circle
    markerHolder
      .append("circle")
      .attr("cx", x)
      .attr("cy", y)
      .classed("crosshair", true)
      .attr("r", 2)
      .style("display", this.showMarker() ? "" : "none")
      .classed("active", activePanel);

    // Create horizontal line
    markerHolder
      .append("line")
      .classed("crosshair", true)
      .classed("crosshairX", true)
      .attr("x1", x)
      .attr("y1", 0)
      .attr("x2", x)
      .attr("y2", mainViewerDimensions.height)
      .style("display", this.showCrosshairs() ? "" : "none")
      .classed("active", activePanel);

    // Create vertical line
    markerHolder
      .append("line")
      .classed("crosshair", true)
      .classed("crosshairY", true)
      .attr("x1", 0)
      .attr("y1", y)
      .attr("x2", mainViewerDimensions.width)
      .attr("y2", y)
      .style("display", this.showCrosshairs() ? "" : "none")
      .classed("active", activePanel);
  }

  updateSlider(sliceNum) {
    this.sliceSlider.setCurrentValue(sliceNum);
  }
}
