/*
###############################################################################
#
#  EGSnrc online voxel and dose visualization tool
#  Copyright (C) 2020 Magdalena Bazalova-Carter and Elise Badun
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Elise Badun, 2020
#
#  Contributors:
#
###############################################################################
*/


var drawAxes = (zoomTransform, svgAxis, slice) => {
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
        (slice.dimensions.fullHeight - 25) +
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
        (25 - slice.dimensions.margin.left) +
        " ," +
        slice.dimensions.height / 2 +
        ") rotate(-90)"
    )
    .style("text-anchor", "middle")
    .text(slice.axis[1] + " (cm)");
};

/** @class Volume represents a dose or density file and includes classes the
 * get slices of data.  */
class Volume {
  // General volume structure
  // https://github.com/aces/brainbrowser/blob/fe0ce114c6cd8e317a6bdd9b7ef97cbf1c38309d/src/brainbrowser/volume-viewer/volume-loaders/minc.js#L88-L190

  /**
   * Creates an instance of a Volume.
   *
   * @constructor
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   */
  constructor(fileName, dimensions, legendDimensions) {
    this.fileName = fileName;
    this.dimensions = dimensions;
    this.legendDimensions = legendDimensions;
    this.prevSlice = { xy: {}, yz: {}, xz: {} };
  }

  /**
   * Sets the HTML elements used in plotting as properties of the volume.
   *
   * @param {Object} htmlElementObj Stores the HTML element of each axis (xy,
   * yz, and xz).
   * @param {Object} legendHolder The parent element of the legend.
   * @param {Object} legendSvg The child svg element of the legend.
   */
  // TODO: Remove html elements as properties and just pass them in
  setHtmlObjects(htmlElementObj, legendHolder, legendSvg) {
    this.htmlElementObj = htmlElementObj;
    this.legendHolder = legendHolder;
    this.legendSvg = legendSvg;
  }

  /**
   * Add the colour scheme used for the plots.
   *
   * @param {function} colourScheme The d3 colour scheme.
   * @param {number} maxVal The maximum value mapped to the colour scheme.
   * @param {number} maxVal The maximum value mapped to the colour scheme.
   * @param {boolean} invertScheme Whether to map from max to min or min to max.
   */
  addColourScheme(colourScheme, maxVal, minVal, invertScheme) {
    let domain = invertScheme ? [maxVal, minVal] : [minVal, maxVal];
    this.colour = d3.scaleSequentialSqrt(colourScheme).domain(domain);
  }

  /**
   * Get a slice of data through an axis.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} sliceNum The number of the slice.
   * @param {string} dataName The type of data, either "density" or "dose".
   * @returns {Object}
   */
  getSlice(axis, sliceNum, dataName) {
    // TODO: Cache previous slices
    // TODO: Only redefine slice attributes on axis change
    // For slice structure
    // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1502-L1546

    // Get the axes and slice dimensions
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

    // Get the length in cm of the x and y dimensions
    var getLengthCm = (voxelArrDim) =>
      Math.abs(voxelArrDim[voxelArrDim.length - 1] - voxelArrDim[0]);
    let [xLengthCm, yLengthCm] = [getLengthCm(x), getLengthCm(y)];

    // Initialize variables to make slice scales
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

    // TODO: Clamp scales

    // Define the screen pixel to volume voxel mapping
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
      contourXScale: contourXScale,
      contourYScale: contourYScale,
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

    // Get the slice data for the given axis and index
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

    this.prevSlice[axis] = slice;
    return slice;
  }

  /**
   * Get the data value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @param {string} dataName The type of data, either "density" or "dose".
   * @returns {number}
   */
  getDataAtVoxelCoords(voxelCoords, dataName) {
    let [x, y, z] = voxelCoords;
    let address =
      z * (this.data.voxelNumber.x * this.data.voxelNumber.y) +
      y * this.data.voxelNumber.x +
      x;

    return this.data[dataName][address];
  }
}

/** @class Volume represents a .3ddose file.  */
class DoseVolume extends Volume {
  /**
   * Creates an instance of a DoseVolume.
   *
   * @constructor
   * @extends Volume
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   * @param {Object} data The data from parsing the file.
   */
  constructor(fileName, dimensions, legendDimensions, data) {
    // Call the super class constructor
    super(fileName, dimensions, legendDimensions);
    this.addData(data);
  }

  /**
   * Adds data to the DoseVolume object.
   *
   * @param {Object} data The data from parsing the file.
   */
  // TODO: Remove maxDoseVar
  addData(data) {
    this.data = data;
    // Max dose used for dose contour plot
    this.maxDoseVar = data.maxDose;
    super.addColourScheme(d3.interpolateViridis, data.maxDose, 0);
    // Calculate the contour thresholds
    let contourInt = 0.1;
    this.thresholdPercents = d3.range(contourInt, 1.0 + contourInt, contourInt);
    this.updateThresholds();
    // The className function multiplies by 1000 and rounds because decimals are not allowed in class names
    this.className = (i) =>
      "col-" + d3.format("d")(this.thresholdPercents[i] * 1000);
  }

  /**
   * Sets the maximum dose value for dose contour plots.
   *
   * @param {number} val The maximum dose percentage.
   * @param {Object} panels The panels for which to update the dose plots.
   */
  setMaxDose(val, panels) {
    this.maxDoseVar = val * this.data.maxDose;
    // Update the colour scheme and thresholds with the new max dose variable
    super.addColourScheme(d3.interpolateViridis, this.maxDoseVar, 0);
    this.updateThresholds();

    ["xy", "yz", "xz"].forEach((axis) =>
      this.drawDose(this.prevSlice[axis], panels[axis].zoomTransform)
    );

    if (d3.select("input[name='show-dose-profile-checkbox']").node().checked) {
      volumeViewerList.forEach((volumeViewer) => {
        volumeViewer.doseProfileList.forEach((doseProfile) =>
          doseProfile.plotData()
        );
      });
    }
  }

  /**
   * Updates the threshold values used for creating the dose contours.
   */
  updateThresholds() {
    this.thresholds = this.thresholdPercents.map((i) => i * this.maxDoseVar);
  }

  /**
   * Add a new threshold value to create a new contour in the dose contour plots.
   *
   * @param {number} thresholdPercent The dose percentage to add.
   * @param {Object} panels The panels for which to update the dose plots.
   */
  addThresholdPercent(thresholdPercent, panels) {
    this.thresholdPercents.push(thresholdPercent);
    this.thresholdPercents.sort();
    this.updateThresholds();
    this.initializeLegend();
    ["xy", "yz", "xz"].forEach((axis) =>
      this.drawDose(this.prevSlice[axis], panels[axis].zoomTransform)
    );
  }

  /**
   * Get a slice of dose data for a given axis and slice index.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} sliceNum The number of the slice.
   * @returns {Object}
   */
  getSlice(axis, sliceNum) {
    return super.getSlice(axis, sliceNum, "dose");
  }

  /**
   * Clear the current dose plot.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   */
  clearDose(axis) {
    let svg = this.htmlElementObj[axis];
    // Clear dose plot
    svg.selectAll("g").remove();
  }

  /**
   * Make a dose contour plot of the given slice.
   *
   * @param {Object} slice The slice of the dose data.
   * @param {Object} [transform] The zoom transform of the plot.
   */
  drawDose(slice, transform) {
    let svg = this.htmlElementObj[slice.axis];

    // Clear dose plot
    svg.selectAll("g").remove();

    // Draw contours
    var contours = d3
      .contours()
      .size([slice.xVoxels, slice.yVoxels])
      .smooth(false)
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

    if (transform) {
      svg.select("g.dose-contour").attr("transform", transform.toString());
    }
  }

  /**
   * Get a class list of all the hidden dose contours.
   *
   * @returns {string[]}
   */
  getHiddenContourClassList() {
    let hiddenContourClassList = [];
    this.legendSvg.selectAll("g.cell.hidden").each(function (d, i) {
      hiddenContourClassList[i] =
        "." + d3.select(this).attr("class").split(" ")[1];
    });

    return hiddenContourClassList;
  }

  /**
   * Create the dose legend.
   */
  initializeLegend() {
    // Get list of class names of hidden contours
    let hiddenContourClassList = this.getHiddenContourClassList();
    let legendClass = "doseLegend";
    let parameters = {
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
    };

    var toggleContour = (className) => {
      Object.values(this.htmlElementObj).forEach((svg) => {
        svg
          .selectAll("path.contour-path." + className)
          .classed("hidden", function () {
            return !d3.select(this).classed("hidden");
          });
      });
    };

    // Clear and redraw current legend
    this.legendSvg.select("." + legendClass).remove();
    this.legendSvg.select("text").remove();

    // Make space for legend title
    this.legendSvg
      .append("g")
      .attr("class", legendClass)
      .style("transform", "translate(0px," + 20 + "px)");

    // Append title
    this.legendSvg
      .append("text")
      .attr("class", legendClass)
      .attr("x", this.legendDimensions.width / 2)
      .attr("y", this.legendDimensions.margin.top / 2)
      .attr("text-anchor", "middle")
      .style("font-size", "14px")
      .text("Dose");

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

    this.legendSvg.select("." + legendClass).call(legend);

    // Set the height of the svg so the div can scroll if need be
    let height =
      this.legendSvg
        .select("." + legendClass)
        .node()
        .getBoundingClientRect().height + 20;
    this.legendSvg.attr("height", height);

    // Add the appropriate classnames to each legend cell
    let len = this.thresholdPercents.length - 1;
    this.legendSvg
      .selectAll("g.cell")
      .attr("class", (d, i) => "cell " + this.className(len - i));

    if (hiddenContourClassList.length > 0) {
      // Apply hidden class to hidden contours
      let hiddenLegendCells = this.legendSvg
        .selectAll("g.cell")
        .filter(hiddenContourClassList.join(","));
      hiddenLegendCells.classed("hidden", !hiddenLegendCells.classed("hidden"));
    }
  }

  /**
   * Create the input box to add new dose contour thresholds.
   *
   * @param {Object} panels The panels to update when a dose contour is added.
   */
  initializeDoseContourInput(panels) {
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
          this.addThresholdPercent(newPercentage, panels);
        }
      }
    };

    // Remove existing dose contour inputs
    this.legendHolder.selectAll("input").remove();

    let doseContourInputWidth = 45;

    // Add number input box
    let submitDoseContour = this.legendHolder
      .append("input")
      .attr("type", "number")
      .attr("name", "add-dose-contour-line")
      .attr("id", "add-dose-contour-line")
      .attr("min", 0)
      .attr("max", 100)
      .attr("step", 1)
      .style("width", doseContourInputWidth + "px");

    // Add submit button
    this.legendHolder
      .append("input")
      .attr("type", "submit")
      .attr("name", "submit-dose-contour-line")
      .attr("id", "submit-dose-contour-line")
      .attr("value", "+")
      .on("click", addNewThresholdPercent);
  }

  /**
   * Get the dose value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "dose") / this.data.maxDose;
  }

  /**
   * Get the dose error value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getErrorAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "error");
  }

  /**
   * Create the max dose slider to choose the maximum dose in the contour plots.
   *
   * @param {Object} panels The panels for which to update on maximum dose change.
   */
  initializeMaxDoseSlider(panels) {
    let parentDiv = d3.select("#axis-slider-container");
    var onMaxDoseChangeCallback = (sliderVal) =>
      this.setMaxDose(sliderVal, panels);
    let doseSliderParams = {
      id: "max-dose",
      label: "Max Dose",
      format: d3.format(".0%"),
      startingVal: 1.0,
      minVal: 0.0,
      maxVal: 1.5,
      step: 0.01,
    };

    // Remove existing sliders
    parentDiv.selectAll(".slider-container").remove();

    let maxDoseSlider = new Slider(
      parentDiv,
      onMaxDoseChangeCallback,
      doseSliderParams
    );
  }
}

/** @class Volume represents the difference between two .3ddose files.  */
class DoseComparisonVolume extends DoseVolume {
  /**
   * Creates an instance of a DoseComparisonVolume.
   *
   * @constructor
   * @extends DoseVolume
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   * @param {Object} data The data from parsing the file.
   */
  constructor(fileName, dimensions, legendDimensions, data) {
    // Call the super class constructor
    super(fileName, dimensions, legendDimensions);
    this.addData(data);
  }

  /**
   * Adds data to the DoseComparisonVolume object.
   *
   * @param {Object} data The difference of the data from the two dose files.
   */
  addData(data) {
    this.data = data;
    // Max dose used for dose contour plot
    this.maxDoseVar = 1.0;
    super.addColourScheme(d3.interpolateViridis, 1.0, -1.0);

    // Calculate the contour thresholds
    let contourInt = 0.2;
    this.thresholdPercents = d3.range(
      -1.0 + contourInt,
      1.0 + contourInt,
      contourInt
    );
    // Thresholds and thresholdPercents are the same
    super.updateThresholds();

    // The className function multiplies by 1000 and rounds because decimals are not allowed in class names
    this.className = (i) =>
      "col-" + d3.format("d")(this.thresholdPercents[i] * 1000);
  }

  /**
   * Get the dose difference value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "dose");
  }
}

/** @class Volume represents a .egsphant file.  */
class DensityVolume extends Volume {
  /**
   * Creates an instance of a DoseVolume.
   *
   * @constructor
   * @extends Volume
   * @param {string} fileName The name of the file.
   * @param {Object} dimensions The pixel dimensions of the volume plots.
   * @param {Object} legendDimensions The pixel dimensions of legends.
   * @param {Object} data The data from parsing the file.
   */
  constructor(fileName, dimensions, legendDimensions, data) {
    super(fileName, dimensions, legendDimensions); // call the super class constructor
    this.addData(data);
    this.prevSliceImg = { xy: {}, yz: {}, xz: {} };
  }

  /**
   * Adds data to the DensityVolume object.
   *
   * @param {Object} data The data from parsing the file.
   */
  addData(data) {
    this.data = data;
    this.setWindow();
    this.setLevel();
    this.addColourScheme();
  }

  /**
   * Add the colour scheme used for the plots.
   */
  addColourScheme() {
    super.addColourScheme(
      d3.interpolateGreys,
      this.level + this.window / 2.0,
      this.level - this.window / 2.0,
      true
    );
  }

  /**
   * Set the window of density values displayed.
   *
   * @param {number} width The size of the window of densities to display.
   */
  setWindow(width) {
    // Window is whole range
    this.window = parseFloat(width) || this.data.maxDensity;
  }

  /**
   * Set the level of density values displayed.
   *
   * @param {number} level The midpoint of the window.
   */
  setLevel(level) {
    // Level is mid level
    this.level = parseFloat(level) || this.window / 2.0;
  }

  /**
   * Get a slice of density data for a given axis and slice index.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   * @param {number} sliceNum The number of the slice.
   * @returns {Object}
   */
  getSlice(axis, sliceNum) {
    return super.getSlice(axis, sliceNum, "density");
  }

  /**
   * Clear the current density plot.
   *
   * @param {string} axis The axis of the slice (xy, yz, or xz).
   */
  clearDensity(axis) {
    let svg = this.htmlElementObj[axis].node();

    // Clear density plot
    let context = svg.getContext("2d");
    context.clearRect(0, 0, svg.width, svg.height);
  }

  /**
   * Make a density plot of the given slice.
   *
   * @param {Object} slice The slice of the density data.
   * @param {Object} [transform] The zoom transform of the plot.
   */
  drawDensity(slice, transform) {
    let svg = this.htmlElementObj[slice.axis];
    // TODO: Make change slicenum function

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
      if (transform) {
        imgContext.translate(transform.x, transform.y);
        imgContext.scale(transform.k, transform.k);
      }
      imgContext.drawImage(image, 0, 0);
      imgContext.restore();
    });

    image.src = canvas.toDataURL();

    // Get the canvas and context in the webpage
    let imgCanvas = svg.node();
    let imgContext = imgCanvas.getContext("2d");

    // Save the image as properties of the volume object
    this.prevSliceImg[slice.axis] = image;
  }

  /**
   * Create the density legend.
   */
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
    this.legendSvg.select("." + legendClass).remove();
    this.legendSvg.select("text").remove();

    // Set dimensions of svg
    this.legendSvg
      .attr("width", dims.width)
      .attr("height", dims.height)
      .attr("viewBox", [0, 0, dims.width, dims.height])
      .style("overflow", "visible")
      .style("display", "block");

    // Define parameters for ticks
    let ticks = 4;
    let n = Math.round(ticks + 1);
    let tickValues = d3
      .range(n)
      .map((i) => d3.quantile(this.colour.domain(), i / (n - 1)));
    let tickFormat = d3.format(".3f");
    let tickSize = 15;

    let gradUrl = gradientUrl(
      this.colour,
      dims.height - 20,
      30,
      this.data.maxDensity
    );

    // Set height of legend
    let legendHeight = dims.height - 80;

    // Create scale for ticks
    let scale = d3
      .scaleLinear()
      .domain([0, this.data.maxDensity])
      .range([legendHeight, 0]);

    // Append title
    this.legendSvg
      .append("text")
      .attr("class", legendClass)
      .attr("x", dims.width / 2)
      .attr("y", dims.margin.top / 2)
      .attr("text-anchor", "middle")
      .style("font-size", "14px")
      .text(title);

    // Append gradient image
    this.legendSvg
      .append("g")
      .attr("class", legendClass)
      .append("image")
      .attr("y", dims.margin.top)
      .attr("width", dims.width)
      .attr("height", legendHeight)
      .attr("preserveAspectRatio", "none")
      .attr("xlink:href", gradUrl);

    // Append ticks
    this.legendSvg
      .append("g")
      .attr("transform", "translate(" + 0 + ", " + dims.margin.top + ")")
      .classed("label", true)
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

  /**
   * Get the density value at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {number}
   */
  getDataAtVoxelCoords(voxelCoords) {
    return super.getDataAtVoxelCoords(voxelCoords, "density");
  }

  /**
   * Get the material at the given coordinates.
   *
   * @param {number[]} voxelCoords The voxel position of the data.
   * @returns {string}
   */
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
