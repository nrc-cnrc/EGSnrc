// Set up the export to csv button
d3.select("#save-dose-profile").on("click", function () {
  volumeViewerList.forEach((volumeViewer, i) => {
    volumeViewer.doseProfileList.forEach((doseProfile) => {
      // Check that dose profile data exists
      if (doseProfile.data) {
        // Create csv name
        var csvName =
          "vol" + i + "_" + doseProfile.profileDim + "_dose_profile.csv";

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

        makeAndDownloadCsv(doseProfile.data, csvName);
      }
    });
  });
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
  // If show marker is selected, show marker and voxel info
  // If show crosshairs is selected, show crosshairs and dose profile plots

  volumeViewerList.forEach((volumeViewer, i) => {
    let node = volumeViewer.volHolder.node();

    // TODO: Let user choose between png and svg
    let format = "png";

    // Define image width and height
    let imgWidth = node.clientWidth + 5;
    let imgHeight = node.clientHeight + 5;

    // Create image name
    var removeFileExt = (fileName) =>
      fileName.substr(0, fileName.lastIndexOf("."));

    let imageName = "";
    if (volumeViewer.densityVolume !== null) {
      imageName += removeFileExt(volumeViewer.densityVolume.fileName) + "_";
    }
    if (volumeViewer.doseVolume !== null) {
      imageName += removeFileExt(volumeViewer.doseVolume.fileName) + "_";
    }

    imageName += "image" + i + "." + format;

    // Optional: define elements to exclude from png image
    function filter(node) {
      return node.tagName !== "input";
    }

    if (format === "png") {
      domtoimage
        .toBlob(node, {
          // filter: filter,
          bgcolor: "white",
          width: imgWidth,
          height: imgHeight,
        })
        .then(function (blob) {
          window.saveAs(blob, imageName);
        });
    } else if (format === "svg") {
      // Convert the div to string
      var imgString = getImgString(node);
      var imgsrc =
        "data:image/svg+xml;base64," +
        btoa(unescape(encodeURIComponent(imgString))); // Convert img string to data URL
      downloadURI(imgsrc, imageName + ".png");
    }
  });
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
