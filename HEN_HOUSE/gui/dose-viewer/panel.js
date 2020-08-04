// Detect clicks
// Store zoom, cursor info
// Draw axes
// https://github.com/aces/brainbrowser/blob/master/src/brainbrowser/volume-viewer/lib/panel.js#L165
// TODO: Make voxel info work when slice changes!!!

class Panel {
  constructor(
    dimensions,
    volume,
    axis,
    axisElements,
    sliceNum = 0,
    zoomTransform = null,
    markerPosition = null
  ) {
    this.dimensions = dimensions;
    this.volume = volume;
    this.axis = axis;
    this.axisElements = axisElements;
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
    axisElements["plot-marker"].call(mainViewerZoom);

    // Update circle marker position and voxel coords on click
    var updateMarkerAndVoxelInfo = (plotCoords) => {
      if (d3.event.defaultPrevented) return;

      // TODO: Trigger a marker moved event??
      this.updateSliceNum();
      this.updateMarker(plotCoords);
      updateVoxelCoords(
        plotCoords,
        this.axis,
        this.sliceNum,
        this.zoomTransform,
        true
      );
    };
    axisElements["plot-marker"].on("click", function () {
      let plotCoords = d3.mouse(this);
      updateMarkerAndVoxelInfo(plotCoords);
      return true;
    });
  }

  // Add listener to do it on slider change
  updateSliceNum() {
    this.sliceNum = this.volume.prevSlice.sliceNum;
  }

  setSlice(panel, sliceNum) {
    this.sliceNum = sliceNum;
    this.slice = this.volume.getSlice(
      sliceNum,
      panel.contrast,
      panel.brightness
    );
  }

  getDrag() {
    let axis = this.axis;
    let sliceNum = this.sliceNum;

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
      updateVoxelCoords([x, y], axis, sliceNum, null, true);
    }

    function dragended() {
      d3.select(this).attr("cursor", "grab");
    }

    return d3
      .drag()
      .on("start", dragstarted)
      .on("drag", dragged)
      .on("end", dragended);
  }

  updateMarker(coords) {
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
      .attr("cursor", "grab")
      .append("g")
      .attr("class", "marker-holder");

    // Add drag functionality
    markerHolder.call(this.getDrag());

    // Create centre circle
    markerHolder
      .append("circle")
      .attr("cx", x)
      .attr("cy", y)
      .classed("crosshair", true)
      .attr("r", 2)
      .style("cursor", "pointer")
      .style("display", this.showMarker() ? "" : "none");

    // Create horizontal line
    markerHolder
      .append("line")
      .classed("crosshair", true)
      .classed("crosshairX", true)
      .attr("x1", x)
      .attr("y1", 0)
      .attr("x2", x)
      .attr("y2", mainViewerDimensions.height)
      .style("display", this.showCrosshairs() ? "" : "none");

    // Create vertical line
    markerHolder
      .append("line")
      .classed("crosshair", true)
      .classed("crosshairY", true)
      .attr("x1", 0)
      .attr("y1", y)
      .attr("x2", mainViewerDimensions.width)
      .attr("y2", y)
      .style("display", this.showCrosshairs() ? "" : "none");
  }
}
