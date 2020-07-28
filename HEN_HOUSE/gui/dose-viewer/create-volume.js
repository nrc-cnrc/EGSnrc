// TODO: Make a plotting object that takes list of volumes, plots them
// Each plotting object has radio buttons, slider, axes, canvas, and svg
// TODO: Make a dataName variable to reduce code
// TODO: Make the max dose slider its own object

var drawAxes = (svgAxis, slice) => {
  svgAxis.selectAll(".x-axis, .y-axis, .x-axis-grid, .y-axis-grid").remove();

  // If there is existing transformation, apply it
  let xScale = zoomTransform
    ? zoomTransform.rescaleX(slice.xScale)
    : slice.xScale;
  let yScale = zoomTransform
    ? zoomTransform.rescaleY(slice.yScale)
    : slice.yScale;

  // Create and append the x and y axes
  var xAxis = d3.axisBottom().scale(xScale).ticks(6);
  var yAxis = d3.axisLeft().scale(yScale).ticks(6);

  svgAxis
    .append("g")
    .attr("class", "x-axis")
    .attr("transform", "translate(0," + slice.dimensions.height + ")")
    .style("font-size", "12px")
    .call(xAxis);
  svgAxis
    .append("g")
    .attr("class", "y-axis")
    .style("font-size", "12px")
    .call(yAxis);

  // Create and append the x and y grids
  var xAxisGrid = d3
    .axisBottom()
    .scale(xScale)
    .tickSize(-slice.dimensions.height)
    .tickFormat("")
    .ticks(6);
  var yAxisGrid = d3
    .axisLeft()
    .scale(yScale)
    .tickSize(-slice.dimensions.width)
    .tickFormat("")
    .ticks(6);

  svgAxis
    .append("g")
    .attr("class", "x-axis-grid")
    .attr("transform", "translate(0," + slice.dimensions.height + ")")
    .call(xAxisGrid);
  svgAxis.append("g").attr("class", "y-axis-grid").call(yAxisGrid);

  // Label for x axis
  svgAxis
    .append("text")
    .attr("class", "x-axis")
    .attr(
      "transform",
      "translate(" +
        slice.dimensions.width / 2 +
        " ," +
        (slice.dimensions.fullHeight - 15) +
        ")"
    )
    .style("text-anchor", "middle")
    .text(slice.axis[0] + " (cm)");

  // Label for y axis
  svgAxis
    .append("text")
    .attr("class", "y-axis")
    .attr("transform", "rotate(-90)")
    .attr(
      "transform",
      "translate(" +
        (15 - slice.dimensions.margin.left) +
        " ," +
        slice.dimensions.height / 2 +
        ") rotate(-90)"
    )
    .style("text-anchor", "middle")
    .text(slice.axis[1] + " (cm)");
};

class Volume {
  // General volume structure
  // https://github.com/aces/brainbrowser/blob/fe0ce114c6cd8e317a6bdd9b7ef97cbf1c38309d/src/brainbrowser/volume-viewer/volume-loaders/minc.js#L88-L190

  constructor(dimensions, legendDimensions, htmlElementObj) {
    this.dimensions = dimensions;
    this.legendDimensions = legendDimensions;
    this.data = {};
    this.prevSlice = {};
    this.prevAxis = "";
    this.htmlElementObj = htmlElementObj;
  }

  addData(data) {
    this.data = data;
    this.xWorldToVoxelScale = d3
      .scaleQuantile()
      .domain([data.voxelArr.x[0], data.voxelArr.x[data.voxelArr.x.length - 1]])
      .range(d3.range(0, data.voxelNumber.x, 1));
    this.yWorldToVoxelScale = d3
      .scaleQuantile()
      .domain([data.voxelArr.y[0], data.voxelArr.y[data.voxelArr.y.length - 1]])
      .range(d3.range(0, data.voxelNumber.y, 1));
    this.zWorldToVoxelScale = d3
      .scaleQuantile()
      .domain([data.voxelArr.z[0], data.voxelArr.z[data.voxelArr.z.length - 1]])
      .range(d3.range(0, data.voxelNumber.z, 1));
  }

  addColourScheme(colourScheme, maxVal, minVal, invertScheme) {
    let domain = invertScheme ? [maxVal, minVal] : [minVal, maxVal];
    this.colour = d3.scaleSequentialSqrt(colourScheme).domain(domain);
  }

  // dataName : density or dose
  getSlice(axis, sliceNum, dataName) {
    // TODO: Cache previous slices
    // TODO: Only redefine slice attributes on axis change
    // For slice structure
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1502-L1546

    let [dim1, dim2, dim3] =
      axis === "xy"
        ? ["x", "y", "z"]
        : axis === "yz"
        ? ["y", "z", "x"]
        : ["x", "z", "y"];

    let x = this.data.voxelArr[dim1];
    let y = this.data.voxelArr[dim2];
    let z = this.data.voxelArr[dim3];
    let totalSlices = this.data.voxelNumber[dim3];

    var getLengthCm = (voxelArrDim) =>
      Math.abs(voxelArrDim[voxelArrDim.length - 1] - voxelArrDim[0]);
    let [xLengthCm, yLengthCm] = [getLengthCm(x), getLengthCm(y)];
    let xDomain,
      yDomain,
      xRangeContour,
      yRangeContour,
      yPixelToVoxelScale,
      contourYScaleDomain;
    if (xLengthCm > yLengthCm) {
      xDomain = [x[0], x[x.length - 1]];
      yDomain = [y[y.length - 1] - xLengthCm, y[y.length - 1]];
      xRangeContour = [0, this.dimensions.width];
      yRangeContour = [this.dimensions.height * (yLengthCm / xLengthCm), 0];
      yPixelToVoxelScale = d3
        .scaleQuantile()
        .domain([yRangeContour[1], yRangeContour[0]])
        .range(d3.range(this.data.voxelNumber[dim2], 0, -1));
      contourYScaleDomain = [1, this.data.voxelNumber[dim2] + 1];
    } else {
      xDomain = [x[0], x[0] + yLengthCm];
      yDomain = [y[0], y[y.length - 1]];
      xRangeContour = [0, this.dimensions.width * (xLengthCm / yLengthCm)];
      yRangeContour = [this.dimensions.height, 0];
      yPixelToVoxelScale = d3
        .scaleQuantile()
        .domain([yRangeContour[1], yRangeContour[0]])
        .range(d3.range(this.data.voxelNumber[dim2] - 1, -1, -1));
      contourYScaleDomain = [0, this.data.voxelNumber[dim2]];
    }

    let xPixelToVoxelScale = d3
      .scaleQuantile()
      .domain(xRangeContour)
      .range(d3.range(0, this.data.voxelNumber[dim1], 1));

    let contourXScale = d3
      .scaleLinear()
      .domain([0, this.data.voxelNumber[dim1]])
      .range(xRangeContour);
    // Bump by 1 to fix misalignment after flipping y axis
    let contourYScale = d3
      .scaleLinear()
      .domain(contourYScaleDomain)
      .range(yRangeContour);
    // TODO: Change scales to quantile to map exactly which pixels
    let slice = {
      dx: this.data.voxelSize[dim1],
      dy: this.data.voxelSize[dim2],
      xVoxels: this.data.voxelNumber[dim1],
      yVoxels: this.data.voxelNumber[dim2],
      x: x,
      y: y,
      totalSlices: totalSlices,
      xScale: d3
        .scaleLinear()
        .domain(xDomain)
        .range([0, this.dimensions.width]),
      yScale: d3
        .scaleLinear()
        .domain(yDomain)
        .range([this.dimensions.height, 0]),
      zScale: d3
        .scaleLinear()
        .domain([z[0], z[z.length - 1]])
        .range([0, totalSlices]), // unit: pixels
      dimensions: this.dimensions,
      axis: axis,
      xPixelToVoxelScale: xPixelToVoxelScale,
      yPixelToVoxelScale: yPixelToVoxelScale,
      contourTransform: ({ type, value, coordinates }) => ({
        type,
        value,
        coordinates: coordinates.map((rings) =>
          rings.map((points) =>
            points.map(([i, j]) => [contourXScale(i), contourYScale(j)])
          )
        ),
      }),
    };

    // If current slice number is larger than the total number of slices
    // set slice number to last slice
    sliceNum =
      sliceNum >= slice.totalSlices
        ? parseInt(slice.totalSlices - 1)
        : parseInt(sliceNum);

    // For address calculations:
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1999-L2034
    let sliceData = new Array(slice.xVoxels * slice.yVoxels);

    for (let i = 0; i < slice.xVoxels; i++) {
      for (let j = 0; j < slice.yVoxels; j++) {
        let address;
        if (axis === "xy") {
          address = i + slice.xVoxels * (j + sliceNum * slice.yVoxels);
        } else if (axis === "yz") {
          address =
            sliceNum + this.data.voxelNumber.x * (i + j * slice.xVoxels);
        } else if (axis === "xz") {
          address =
            i + slice.xVoxels * (sliceNum + j * this.data.voxelNumber.y);
        }
        let new_address = i + slice.xVoxels * j;
        sliceData[new_address] = this.data[dataName][address];
      }
    }

    slice = {
      ...slice,
      sliceData: sliceData,
      sliceNum: sliceNum,
    };

    this.prevSlice = slice;
    return slice;
  }

  initializeLegend(legendSvg, legendClass, title, parameters) {
    // Clear and redraw current legend
    legendSvg.select("." + legendClass).remove();
    legendSvg.select("text").remove();

    // Make space for legend title
    legendSvg
      .append("g")
      .attr("class", legendClass)
      .style("transform", "translate(0px," + 20 + "px)");

    // Append title
    legendSvg
      .append("text")
      .attr("class", legendClass)
      .attr("x", this.legendDimensions.width / 2)
      .attr("y", this.legendDimensions.margin.top / 2)
      .attr("text-anchor", "middle")
      .style("font-size", "14px")
      .text(title);

    // Create legend
    var legend = d3
      .legendColor()
      .shapeWidth(10)
      .ascending(true)
      .orient("vertical")
      .scale(this.colour);

    // Apply all the parameters
    Object.entries(parameters).forEach(([name, val]) => {
      legend[name](...val);
    });

    legendSvg.select("." + legendClass).call(legend);

    // Set the height of the svg so the div can scroll if need be
    let height =
      legendSvg
        .select("." + legendClass)
        .node()
        .getBoundingClientRect().height + 20;
    legendSvg.attr("height", height);
  }

  isEmpty() {
    return Object.keys(this.data).length === 0;
  }

  getDataAtVoxelCoords(voxelCoords, dataName) {
    let [x, y, z] = voxelCoords;
    let address =
      z * (this.data.voxelNumber.x * this.data.voxelNumber.y) +
      y * this.data.voxelNumber.x +
      x;

    return this.data[dataName][address];
  }
}

class DoseVolume extends Volume {
  constructor(dimensions, legendDimensions, svgDoseObj) {
    super(dimensions, legendDimensions, svgDoseObj); // call the super class constructor
  }

  addData(data) {
    // TODO: Want user to be able to choose their own space between contours
    super.addData(data);
    // Max dose used for dose contour plot
    this.maxDoseVar = this.data.maxDose;
    super.addColourScheme(d3.interpolateViridis, this.data.maxDose, 0);
    // Calculate the contour thresholds
    let contourInt = 0.1;
    this.thresholdPercents = d3.range(0, 1.0 + contourInt, contourInt);
    this.updateThresholds();
    // The className function multiplies by 1000 and rounds because decimals are not allowed in class names
    this.className = (i) =>
      "col-" + d3.format("d")(this.thresholdPercents[i] * 1000);
  }

  setMaxDose(val) {
    this.maxDoseVar = val * this.data.maxDose;
    super.addColourScheme(d3.interpolateViridis, this.maxDoseVar, 0);
    this.updateThresholds();
    this.drawDose(this.prevSlice);
    if (d3.select("input[name='show-dose-profile-checkbox']").node().checked) {
      doseProfileX.plotData();
      doseProfileY.plotData();
    }
  }

  updateThresholds() {
    this.thresholds = this.thresholdPercents.map((i) => i * this.maxDoseVar);
  }

  // TODO: Don't reinitialize legend when adding a new threshold
  addThresholdPercent(thresholdPercent) {
    this.thresholdPercents.push(thresholdPercent);
    this.thresholdPercents.sort();
    this.updateThresholds();
    this.initializeLegend();
    this.drawDose(this.prevSlice);
  }

  getSlice(axis, sliceNum) {
    return super.getSlice(axis, sliceNum, "dose");
  }

  drawDose(slice) {
    let svg = this.htmlElementObj[slice.axis];
    //TODO: Don't rely on plugin for legend/colour scale
    // https://observablehq.com/@d3/color-legend

    // Clear dose plot
    svg.selectAll("g").remove();

    // Draw contours
    var contours = d3
      .contours()
      .size([slice.xVoxels, slice.yVoxels])
      .thresholds(this.thresholds)(slice.sliceData)
      .map(slice.contourTransform);

    let contourPaths = svg
      .append("g")
      .attr("class", "dose-contour")
      .attr("width", this.dimensions.width)
      .attr("height", this.dimensions.height)
      .attr("fill", "none")
      .attr("stroke", "#fff")
      .attr("stroke-opacity", 0.5)
      .attr("stroke-width", 0.1)
      .selectAll("path")
      .data(contours)
      .join("path")
      .classed("contour-path", true)
      .attr("class", (d, i) => "contour-path" + " " + this.className(i))
      .attr("fill", (d) => this.colour(d.value))
      .attr("fill-opacity", 0.5)
      .attr("d", d3.geoPath());

    // Get list of class names of hidden contours
    let hiddenContourClassList = this.getHiddenContourClassList();

    if (hiddenContourClassList.length > 0) {
      // Apply hidden class to hidden contours
      contourPaths
        .filter(hiddenContourClassList.join(","))
        .classed("hidden", true);
    }

    if (zoomTransform) {
      svg.select("g.dose-contour").attr("transform", zoomTransform.toString());
    }
  }

  getHiddenContourClassList() {
    let hiddenContourClassList = [];
    doseLegendSvg.selectAll("g.cell.hidden").each(function (d, i) {
      hiddenContourClassList[i] =
        "." + d3.select(this).attr("class").split(" ")[1];
    });

    return hiddenContourClassList;
  }

  initializeLegend() {
    // Get list of class names of hidden contours
    let hiddenContourClassList = this.getHiddenContourClassList();

    var toggleContour = (className) => {
      Object.values(this.htmlElementObj).forEach((svg) => {
        svg
          .selectAll("path.contour-path." + className)
          .classed("hidden", function () {
            return !d3.select(this).classed("hidden");
          });
      });
    };

    super.initializeLegend(doseLegendSvg, "doseLegend", "Dose", {
      labels: [
        this.thresholds.map((e) => d3.format(".0%")(e / this.maxDoseVar)),
      ],
      cells: [this.thresholds],
      on: [
        "cellclick",
        function (d) {
          let legendCell = d3.select(this);
          toggleContour(legendCell.attr("class").split(" ")[1]);
          legendCell.classed("hidden", !legendCell.classed("hidden"));
        },
      ],
    });

    // Add the appropriate classnames to each legend cell
    let len = this.thresholdPercents.length - 1;
    doseLegendSvg
      .selectAll("g.cell")
      .attr("class", (d, i) => "cell " + this.className(len - i));

    if (hiddenContourClassList.length > 0) {
      // Apply hidden class to hidden contours
      let hiddenLegendCells = doseLegendSvg
        .selectAll("g.cell")
        .filter(hiddenContourClassList.join(","));
      hiddenLegendCells.classed("hidden", !hiddenLegendCells.classed("hidden"));
    }
  }

  initializeDoseContourInput() {
    var addNewThresholdPercent = () => {
      let val = parseFloat(submitDoseContour.node().value);
      let newPercentage = val / 100.0;
      if (!Number.isNaN(newPercentage)) {
        // Check if valid or if percentage already exists
        if (
          val < 0 ||
          val > 100 ||
          !Number.isInteger(val) ||
          this.thresholdPercents.includes(newPercentage)
        ) {
          console.log("Invalid value or value already exists on plot");
          // Flash the submit box red
          submitDoseContour
            .transition()
            .duration(200)
            .style("background-color", "red")
            .transition()
            .duration(300)
            .style("background-color", "white");
        } else {
          this.addThresholdPercent(newPercentage);
        }
      }
    };

    let doseContourInputWidth = 45;

    // Add number input box
    let submitDoseContour = doseLegendHolder
      .append("input")
      .attr("type", "number")
      .attr("name", "add-dose-contour-line")
      .attr("id", "add-dose-contour-line")
      .attr("min", 0)
      .attr("max", 100)
      .attr("step", 1)
      .style("width", doseContourInputWidth + "px");

    // Add submit button
    doseLegendHolder
      .append("input")
      .attr("type", "submit")
      .attr("name", "submit-dose-contour-line")
      .attr("id", "submit-dose-contour-line")
      .attr("value", "+")
      .on("click", addNewThresholdPercent);
  }

  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "dose");
  }

  getErrorAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "error");
  }

  // TODO: Make a slider object for slice iteration and max dose setting
  // TODO: Connect slider to data
  initializeMaxDoseSlider() {
    let maxDosePercent = 1.5;
    let startingDosePercent = 1.0;
    let maxDoseSliderRange = d3.select("#max-dose-slider-range");
    var dosePercentFormat = d3.format(".0%");

    // Set slider step to be 1%
    maxDoseSliderRange.node().step = 0.01;

    // Enable slider
    if (maxDoseSliderRange.node().disabled)
      maxDoseSliderRange.node().disabled = false;

    // On increment button push
    d3.select("#max-dose-increment-slider").on("click", function () {
      let slider = d3.select("#max-dose-slider-range").node();
      slider.stepUp(1);

      // Update slider text
      d3.select("#max-dose-slider-value").node().value = dosePercentFormat(
        slider.value
      );

      doseVol.setMaxDose(slider.value);
    });

    // On decrement button push
    d3.select("#max-dose-decrement-slider").on("click", function () {
      let slider = d3.select("#max-dose-slider-range").node();
      slider.stepDown(1);

      // Update slider text
      d3.select("#max-dose-slider-value").node().value = dosePercentFormat(
        slider.value
      );

      doseVol.setMaxDose(slider.value);
    });

    // On slider input, update text
    maxDoseSliderRange.on("input", function () {
      // Update slider text
      d3.select("#max-dose-slider-value").node().value = dosePercentFormat(
        this.value
      );

      doseVol.setMaxDose(this.value);
      return true;
    });

    // Set max to max dose and current value to starting value define above
    maxDoseSliderRange
      .attr("max", maxDosePercent)
      .attr("max", maxDosePercent)
      .attr("value", startingDosePercent);

    // Show maximum value of slider
    d3.select("#max-dose-slider-max").node().value = dosePercentFormat(
      maxDosePercent
    );

    // Show minimum value of slider
    d3.select("#max-dose-slider-min").node().value = dosePercentFormat(0);

    // Show current value of slider
    d3.select("#max-dose-slider-value").node().value = dosePercentFormat(
      startingDosePercent
    );
  }
}

class DensityVolume extends Volume {
  constructor(dimensions, legendDimensions, canvDensityObj) {
    super(dimensions, legendDimensions, canvDensityObj); // call the super class constructor
  }

  addData(data) {
    super.addData(data);
    this.setWindow();
    this.setLevel();
    this.addColourScheme();
    // Calculate the contour thresholds
    this.thresholds = this.getThresholds(data);
  }

  addColourScheme() {
    super.addColourScheme(
      d3.interpolateGreys,
      this.level + this.window / 2.0,
      this.level - this.window / 2.0,
      true
    );
  }

  setWindow(window) {
    // Window is whole range
    this.window = parseFloat(window) || this.data.maxDensity;
  }

  setLevel(level) {
    // Level is mid level
    this.level = parseFloat(level) || this.window / 2.0;
  }

  getThresholds(data) {
    let thresholds = Array.from(new Set(data.density));
    if (thresholds.length < 10) {
      return thresholds.sort();
    }
    let maxThresh = Math.ceil(data.maxDensity * 10) / 10;
    return d3.range(0, maxThresh, 0.1);
  }

  getSlice(axis, sliceNum) {
    return super.getSlice(axis, sliceNum, "density");
  }

  drawDensity(slice) {
    let svg = this.htmlElementObj[slice.axis];
    // TODO: Make two new functions: change slicenum and change axes

    // For axis structure
    // https://bl.ocks.org/ejb/e2da5a23e9a09d494bd532803d8db61c

    // Create new canvas element and set the dimensions
    let canvas = document.createElement("canvas");
    canvas.width = this.dimensions.width;
    canvas.height = this.dimensions.height;

    // Get and clear the canvas context
    let context = canvas.getContext("2d");
    context.clearRect(0, 0, this.dimensions.width, this.dimensions.height);

    // Calcuate display pixel dimensions
    let dxScaled = Math.ceil(this.dimensions.width / slice.xVoxels);
    let dyScaled = Math.ceil(this.dimensions.height / slice.yVoxels);

    // Draw the image voxel by voxel
    for (let i = 0; i < slice.xVoxels; i++) {
      for (let j = 0; j < slice.yVoxels; j++) {
        let new_address = i + slice.xVoxels * j;
        context.fillStyle = this.colour(slice.sliceData[new_address]);
        context.fillRect(
          Math.ceil(slice.xScale(slice.x[i])),
          Math.ceil(slice.yScale(slice.y[j + 1])),
          dxScaled,
          dyScaled
        );
      }
    }

    // Create a new image to set the canvas data as the image source
    var image = new Image();

    // Once the image has loaded, draw it on the context
    image.addEventListener("load", (e) => {
      imgContext.clearRect(0, 0, this.dimensions.width, this.dimensions.height);
      imgContext.save();
      // Apply transforms if needed
      if (zoomTransform) {
        imgContext.translate(zoomTransform.x, zoomTransform.y);
        imgContext.scale(zoomTransform.k, zoomTransform.k);
      }
      imgContext.drawImage(image, 0, 0);
      imgContext.restore();
    });

    image.src = canvas.toDataURL();

    // Get the canvas and context in the webpage
    let imgCanvas = svg.node();
    let imgContext = imgCanvas.getContext("2d");

    // Save the image and axis as properties of the volume object
    this.prevSliceImg = image;
    this.prevAxis = axis;
  }

  initializeLegend() {
    let legendClass = "densityLegend";
    let title = "Density";
    let dims = this.legendDimensions;

    function gradientUrl(colour, height, width, max, n = 150) {
      let canvas = document.createElement("canvas");
      let context = canvas.getContext("2d");

      for (let i = 0; i < height; ++i) {
        context.fillStyle = "black";
        context.fillRect(0, i, 1, 1);
        context.fillStyle = colour(((n - i) / n) * max);
        context.fillRect(1, i, width - 1, 1);
      }
      return canvas.toDataURL();
    }

    // Remove old text
    densityLegendSvg.select("." + legendClass).remove();
    densityLegendSvg.select("text").remove();

    // Set dimensions of svg
    densityLegendSvg
      .attr("width", dims.width)
      .attr("height", dims.height / 2)
      .attr("viewBox", [0, 0, dims.width, dims.height / 2])
      .style("overflow", "visible")
      .style("display", "block");

    // Define parameters for ticks
    let ticks = 6;
    let n = Math.round(ticks + 1);
    let tickValues = d3
      .range(n)
      .map((i) => d3.quantile(this.colour.domain(), i / (n - 1)));
    let tickFormat = d3.format(".3f");
    let tickSize = 15;

    let gradUrl = gradientUrl(
      this.colour,
      dims.height / 2 - 20,
      30,
      this.data.maxDensity
    );

    // Set height of legend
    let legendHeight = dims.height / 2 - 80;

    // Create scale for ticks
    let scale = d3
      .scaleLinear()
      .domain([0, this.data.maxDensity])
      .range([legendHeight, 0]);

    // Append title
    densityLegendSvg
      .append("text")
      .attr("class", legendClass)
      .attr("x", dims.width / 2)
      .attr("y", dims.margin.top / 2)
      .attr("text-anchor", "middle")
      .style("font-size", "14px")
      .text(title);

    // Append gradient image
    densityLegendSvg
      .append("g")
      .attr("class", legendClass)
      .append("image")
      .attr("y", dims.margin.top)
      .attr("width", dims.width)
      .attr("height", legendHeight)
      .attr("preserveAspectRatio", "none")
      .attr("xlink:href", gradUrl);

    // Append ticks
    densityLegendSvg
      .append("g")
      .attr("transform", "translate(" + 0 + ", " + dims.margin.top + ")")
      .call(
        d3
          .axisRight()
          .ticks(ticks, tickFormat)
          .tickFormat(tickFormat)
          .tickSize(tickSize)
          .tickValues(tickValues)
          .scale(scale)
      );
  }

  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "density");
  }

  getMaterialAtVoxelCoords(voxelCoords) {
    let [x, y, z] = voxelCoords;
    let address =
      z * (this.data.voxelNumber.x * this.data.voxelNumber.y) +
      y * this.data.voxelNumber.x +
      x;

    let divisor = this.data.voxelNumber.x;
    let materialNumber = parseInt(
      this.data.material[Math.floor(address / divisor)][address % divisor]
    );
    return this.data.materialList[materialNumber - 1];
  }
}
