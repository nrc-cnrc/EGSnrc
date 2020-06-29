var zoomTransform;

// TODO: Update rest of zoom functions to use these
// TODO: Disable zoom before upload
// Generic disable zoom function
var disableZoom = (obj) => {
  obj.call(d3.behavior.zoom().on("zoom", null));
};

// Generic enable zoom function
var enableZoom = (obj, width, height, zoomCallback, args) => {
  obj.call(
    d3
      .zoom()
      .extent([
        [0, 0],
        [width, height],
      ])
      .scaleExtent([1, 6])
      .on("zoom", () => zoomCallback(...args))
  );
};

//TODO: Disable until data is uploaded
// Zooming functionality for main plot
svgMarker.call(
  d3
    .zoom()
    .extent([
      [0, 0],
      [mainViewerDimensions.width, mainViewerDimensions.height],
    ])
    .scaleExtent([1, 6])
    .on("zoom", () => zoomedAll(d3.event.transform))
);

// Zooming for x dose profile
doseProfileBoxX.call(
  d3
    .zoom()
    .extent([
      [0, 0],
      [sideDoseProfileDimensions.width, sideDoseProfileDimensions.height],
    ])
    .scaleExtent([1, 6])
    .on("zoom", () => zoomedDoseProfile(d3.event.transform, doseProfileX))
);

// Zooming for y dose profile
doseProfileBoxY.call(
  d3
    .zoom()
    .extent([
      [0, 0],
      [sideDoseProfileDimensions.width, sideDoseProfileDimensions.height],
    ])
    .scaleExtent([1, 6])
    .on("zoom", () => zoomedDoseProfile(d3.event.transform, doseProfileY))
);

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
    doseProfile.svg.select(".x-axis").call(d3.axisBottom().scale(new_xScale));
    doseProfile.svg
      .select(".y-dose-axis")
      .call(d3.axisLeft().scale(new_yDoseScale).tickFormat(d3.format(".2e")));

    if (doseProfile.plotDensity) {
      var new_yDensityScale = transform.rescaleY(doseProfile.yDensityScale);
      doseProfile.svg
        .select(".y-density-axis")
        .call(d3.axisLeft().scale(new_yDensityScale));
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
    svgAxis.select(".x-axis").call(d3.axisBottom().scale(new_xScale));
    svgAxis.select(".y-axis").call(d3.axisLeft().scale(new_yScale));
  }
}
