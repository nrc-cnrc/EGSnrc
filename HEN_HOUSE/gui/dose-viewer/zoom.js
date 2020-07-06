var zoomTransform;

// TODO: Update rest of zoom functions to use these
// TODO: Disable zoom before upload
// Generic disable zoom function
var disableZoom = (obj) => {
  obj.call(d3.behavior.zoom().on("zoom", null));
};

// Generic get zoom function
var getZoom = (width, height, zoomCallback, args) =>
  d3
    .zoom()
    .extent([
      [0, 0],
      [width, height],
    ])
    .scaleExtent([1, 6])
    .on("zoom", () => zoomCallback(d3.event.transform, ...args));

// Generic reset zoom function
var resetZoom = (obj, zoom) => {
  obj.call(zoom.transform, d3.zoomIdentity.scale(1));
};

//TODO: Disable until data is uploaded
// Zooming functionality for main plot
let mainViewerZoom = getZoom(
  mainViewerDimensions.width,
  mainViewerDimensions.height,
  zoomedAll,
  []
);
svgMarker.call(mainViewerZoom);

// Zooming for x dose profile
let doseProfileXZoom = getZoom(
  sideDoseProfileDimensions.width,
  sideDoseProfileDimensions.height,
  zoomedDoseProfile,
  [doseProfileX]
);
doseProfileX.svg.select("rect.bounding-box").call(doseProfileXZoom);
doseProfileX.zoomObj = doseProfileXZoom;

// Zooming for y dose profile
let doseProfileYZoom = getZoom(
  sideDoseProfileDimensions.width,
  sideDoseProfileDimensions.height,
  zoomedDoseProfile,
  [doseProfileY]
);
doseProfileY.svg.select("rect.bounding-box").call(doseProfileYZoom);
doseProfileY.zoomObj = doseProfileYZoom;

function zoomedDoseProfile(transform, doseProfile) {
  doseProfile.zoomTransform = transform;
  if (
    !doseVol.isEmpty() &&
    d3.select("svg#plot-marker").select(".crosshair").node()
  ) {
    doseProfile.svg
      .selectAll("path.lines")
      .attr("transform", transform.toString());

    // Create new scale ojects based on event
    var new_xScale = transform.rescaleX(doseProfile.xScale);
    var new_yDoseScale = transform.rescaleY(doseProfile.yDoseScale);

    // Update axes
    doseProfile.svg
      .select(".profile-x-axis")
      .call(
        d3
          .axisBottom()
          .scale(new_xScale)
          .tickSize(-doseProfile.dimensions.height)
      );

    doseProfile.svg
      .select(".profile-y-dose-axis")
      .call(
        d3
          .axisLeft()
          .scale(new_yDoseScale)
          .tickFormat(d3.format(".0%"))
          .tickSize(-doseProfile.dimensions.width)
      );

    if (doseProfile.plotDensity) {
      var new_yDensityScale = transform.rescaleY(doseProfile.yDensityScale);
      doseProfile.svg
        .select(".profile-y-density-axis")
        .call(
          d3
            .axisLeft()
            .scale(new_yDensityScale)
            .tickSize(-doseProfile.dimensions.width)
        );
    }
  }
}

function zoomedAll(transform) {
  if (!densityVol.isEmpty() || !doseVol.isEmpty()) {
    zoomTransform = transform;

    svgDensity
      .select("g.density-contour")
      .attr("transform", transform.toString());

    svgDose.select("g.dose-contour").attr("transform", transform.toString());

    svgMarker.select(".marker").attr("transform", transform.toString());

    // Create new scale ojects based on event
    let vol = !densityVol.isEmpty() ? densityVol : doseVol;
    var new_xScale = transform.rescaleX(vol.prevSlice.xScale);
    var new_yScale = transform.rescaleY(vol.prevSlice.yScale);

    // Update axes
    svgAxis
      .select(".x-axis")
      .call(
        d3.axisBottom().scale(new_xScale).tickSize(-mainViewerDimensions.height)
      );
    svgAxis
      .select(".y-axis")
      .call(
        d3.axisLeft().scale(new_yScale).tickSize(-mainViewerDimensions.width)
      );
  }
}