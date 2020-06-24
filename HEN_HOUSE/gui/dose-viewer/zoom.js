var zoomTransform;

//TODO: Disable until data is uploaded
// Zooming functionality
svgMarker.call(
  d3
    .zoom()
    .extent([
      [0, 0],
      [width, height],
    ])
    .scaleExtent([1, 6])
    .on("zoom", () => zoomedAll(d3.event.transform))
);

[doseProfileXSvg, doseProfileYSvg].forEach((svg) => {
  svg.call(
    d3
      .zoom()
      .extent([
        [0, 0],
        [sideDoseProfileDimensions.width, sideDoseProfileDimensions.height],
      ])
      .scaleExtent([1, 6])
      .on("zoom", () => zoomedDoseProfile(d3.event.transform, svg))
  );
});

function zoomedDoseProfile(transform, svg) {
  console.log("zoomedDoseProfile");
  console.log("svg");
  if (!densityVol.isEmpty() || !doseVol.isEmpty()) {
    //zoomTransform = transform;

    svg.selectAll("g").attr("transform", transform.toString());

    // create new scale ojects based on event
    var new_xScale = transform.rescaleX(doseVol.prevSlice.xScale);
    var new_yScale = transform.rescaleY(doseVol.prevSlice.yScale);

    // update axes
    svgAxis.select(".x-axis").call(d3.axisBottom().scale(new_xScale));
    svgAxis.select(".y-axis").call(d3.axisLeft().scale(new_yScale));
  }
}

function zoomedAll(transform) {
  if (!densityVol.isEmpty() || !doseVol.isEmpty()) {
    zoomTransform = transform;
    let gDensity = svgDensity.select("g.density-contour");
    svgDensity
      .select("g.density-contour")
      .attr("transform", transform.toString());

    svgDose.select("g.dose-contour").attr("transform", transform.toString());

    svgMarker.select(".marker").attr("transform", transform.toString());

    // create new scale ojects based on event
    let vol = !densityVol.isEmpty() ? densityVol : doseVol;
    var new_xScale = transform.rescaleX(vol.prevSlice.xScale);
    var new_yScale = transform.rescaleY(vol.prevSlice.yScale);

    // update axes
    svgAxis.select(".x-axis").call(d3.axisBottom().scale(new_xScale));
    svgAxis.select(".y-axis").call(d3.axisLeft().scale(new_yScale));
  }
}
