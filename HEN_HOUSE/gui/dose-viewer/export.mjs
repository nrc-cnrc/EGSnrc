// Part from http://bl.ocks.org/Rokotyan/0556f8facbaf344507cdc45dc3622177
// Set-up the export button
d3.select("#saveVis").on("click", function () {
  if (!densityVol.isEmpty() || !doseVol.isEmpty()) {
    // Get html as string
    var imgString = getImgString(d3.select("#imageholder").node());

    // Create image name
    var imageName = getAxis() + "_" + getSliceNum() + "image.svg";

    var imgsrc =
      "data:image/svg+xml;base64," +
      btoa(unescape(encodeURIComponent(imgString))); // Convert img string to data URL

    const link = document.createElement("a");
    document.body.appendChild(link);
    link.setAttribute("href", imgsrc);
    link.setAttribute("download", imageName);
    link.click();
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
  svgString = svgString.replace(/(\w+)?:?xlink=/g, "xmlns:xlink="); // Fix root xlink without namespace
  svgString = svgString.replace(/NS\d+:href/g, "xlink:href"); // Safari NS namespace fix

  return svgString;
}
