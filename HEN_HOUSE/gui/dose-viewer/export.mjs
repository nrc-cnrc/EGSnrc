// Set up the export to csv button
d3.select("#save-dose-profile").on("click", function () {
  // Check that dose profile data exists
  if (doseProfileX.data && doseProfileY.data) {
    // Create csv names
    var csvNameX = getAxis()[0] + "_" + getSliceNum() + "_dose_profile.csv";
    var csvNameY = getAxis()[1] + "_" + getSliceNum() + "_dose_profile.csv";

    var makeAndDownloadCsv = (data, name) => {
      // Create data blob
      var csvBlob = new Blob([d3.csvFormat(data)], {
        type: "text/csv;charset=utf-8;",
      });

      // Get data url for blob
      let csvString = (URL || webkitURL).createObjectURL(csvBlob);

      // Download the file
      downloadURI(csvString, name);
    };

    makeAndDownloadCsv(doseProfileX.data, csvNameX);
    makeAndDownloadCsv(doseProfileY.data, csvNameY);
  }
});

function downloadURI(uri, name) {
  var link = document.createElement("a");

  link.download = name;
  link.href = uri;
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
}

// Part from http://bl.ocks.org/Rokotyan/0556f8facbaf344507cdc45dc3622177
// https://github.com/aces/brainbrowser/blob/master/examples/volume-viewer-demo.js#L194-L248
// Set-up the export button
d3.select("#save-vis").on("click", function () {
  if (!densityVol.isEmpty() || !doseVol.isEmpty()) {
    // Remove marker from image
    let invisibleClasses = d3
      .select("input[name='show-dose-profile-checkbox']")
      .node().checked
      ? "circle.crosshair"
      : ".crosshair";

    d3.selectAll(invisibleClasses).style("display", "none");

    // If show marker is selected, show marker and voxel info (span#voxel-info)
    // If show crosshairs is selected, show crosshairs and dose profile plots (span#dose-profile-holder)
    let node = d3.select("#image-to-print").node();

    // TODO: Let user choose between png and svg
    let format = "png";

    // Define image width and height
    let imgHeight =
      Math.max(
        mainViewerDimensions.fullHeight,
        legendDimensions.fullHeight,
        sideDoseProfileDimensions.fullHeight
      ) * 1.1;

    let imgWidth = d3.select("input[name='show-dose-profile-checkbox']").node()
      .checked
      ? (mainViewerDimensions.fullWidth +
          legendDimensions.fullWidth +
          sideDoseProfileDimensions.fullWidth) *
        1.05
      : (mainViewerDimensions.fullWidth + legendDimensions.fullWidth) * 1.05;

    // html2canvas doesn't apply font family, so add it inline
    computedStyleToInlineStyle(node, {
      recursive: true,
      properties: [
        "font-family",
        "stroke-width",
        "stroke",
        "fill",
        "vector-effect",
        "opacity",
      ],
    });

    // Create image name
    var imageName = getAxis() + "_" + getSliceNum() + "image." + format;

    if (format === "png") {
      html2canvas(node, {
        scrollY: -window.scrollY,
        width: imgWidth,
        height: imgHeight,
      }).then(function (canvas) {
        var imgUrl = canvas.toDataURL("image/png");
        downloadURI(imgUrl, imageName);
      });
    } else if (format === "svg") {
      // Convert the div to string
      var imgString = getImgString(node);
      var imgsrc =
        "data:image/svg+xml;base64," +
        btoa(unescape(encodeURIComponent(imgString))); // Convert img string to data URL
      downloadURI(imgsrc, imageName + ".png");
    }

    // Show marker again
    d3.selectAll(invisibleClasses).style("display", null);
  }
});

function getImgString(node) {
  // Extract all CSS Rules
  // From https://developer.mozilla.org/en-US/docs/Web/API/StyleSheetList
  let allCSS = [...document.styleSheets]
    .map((styleSheet) => {
      try {
        return [...styleSheet.cssRules].map((rule) => rule.cssText).join("");
      } catch (e) {
        console.log(
          "Access to stylesheet %s is denied. Ignoring...",
          styleSheet.href
        );
      }
    })
    .filter(Boolean)
    .join("\n");

  // Append CSS to divNode
  var styleElement = document.createElement("style");
  styleElement.setAttribute("type", "text/css");
  styleElement.innerHTML = allCSS;
  var refNode = node.hasChildNodes() ? node.children[0] : null;
  node.insertBefore(styleElement, refNode);

  // Convert node to string
  var serializer = new XMLSerializer();
  var svgString = serializer.serializeToString(node);

  return svgString;
}
