// Detect clicks
// Store zoom, cursor info
// Draw axes
// Handle zoom???
// https://github.com/aces/brainbrowser/blob/master/src/brainbrowser/volume-viewer/lib/panel.js#L165

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

    // Update circle marker position and voxel coords on click
    var updateMarkerAndVoxelInfo = (plotCoords) => {
      if (d3.event.defaultPrevented) return;

      // TODO: Trigger a marker moved event??
      this.updateSliceNum();
      this.updateMarker(plotCoords);
      updateVoxelCoords(plotCoords, this.axis, this.sliceNum, true);
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

  updateMarker(coords) {
    // Define the drag attributes
    function dragstarted() {
      d3.select(this).raise();
      d3.select(this).attr("cursor", "grabbing");
    }

    function dragged() {
      // TODO: Update voxel position on drag / fire event?
      var x = d3.event.x - coords[0];
      var y = d3.event.y - coords[1];
      d3.select(this).attr("transform", "translate(" + x + "," + y + ")");
    }

    function dragended() {
      d3.select(this).attr("cursor", "grab");
    }

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
    var marker = this.axisElements["plot-marker"]
      .append("g")
      .attr("class", "marker")
      .attr(
        "transform",
        this.zoomTransform ? this.zoomTransform.toString() : ""
      )
      .attr("cursor", "grab");

    // Add drag functionality
    marker.call(
      d3
        .drag()
        .on("start", dragstarted)
        .on("drag", dragged)
        .on("end", dragended)
    );

    // Create centre circle
    marker
      .append("circle")
      .attr("cx", x)
      .attr("cy", y)
      .classed("crosshair", true)
      .attr("r", 2)
      .style("cursor", "pointer")
      .style("display", this.showMarker() ? "" : "none");

    // Create horizontal line
    marker
      .append("line")
      .classed("crosshair", true)
      .classed("crosshairX", true)
      .attr("x1", x)
      .attr("y1", 0)
      .attr("x2", x)
      .attr("y2", mainViewerDimensions.height)
      .style("display", this.showCrosshairs() ? "" : "none");

    // Create vertical line
    marker
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
