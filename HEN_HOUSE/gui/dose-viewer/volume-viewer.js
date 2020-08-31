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


/** @class VolumeViewer combines a density and/or dose file, three panels for
 * the three axes views, and three dose profile plots. */
class VolumeViewer {
  /**
   * Creates an instance of a VolumeViewer.
   *
   * @constructor
   * @param {Object} mainViewerDimensions The pixel dimensions of the main viewer.
   * @param {Object} legendDimensions The pixel dimensions of the legends.
   * @param {Object} sideDoseProfileDimensions The pixel dimensions of dose profiles.
   * @param {string} id The unique ID of the volume viewer.
   */
  // TODO: Avoid making the dimensions properties of the volume viewer
  constructor(
    mainViewerDimensions,
    legendDimensions,
    sideDoseProfileDimensions,
    id
  ) {
    // Set dimensions
    this.mainViewerDimensions = mainViewerDimensions;
    this.legendDimensions = legendDimensions;
    this.sideDoseProfileDimensions = sideDoseProfileDimensions;

    // Set volume viewer ID
    this.id = id;

    // Initialize class properties
    this.doseVolume = null;
    this.densityVolume = null;
    this.panels = null;
    this.sliceSliders = {};
    this.doseProfileList = new Array(3);
    this.dispatch = d3.dispatch("markerchange");

    this.buildBaseHtml(id);
    this.initializeDispatch();
  }

  /**
   * Set the dose volume of the VolumeViewer.
   *
   * @param {DoseVolume} doseVol The dose volume to be set.
   */
  setDoseVolume(doseVol) {
    this.doseVolume = doseVol;

    // Set dose volume html elements
    doseVol.setHtmlObjects(
      this.svgObjs["plot-dose"],
      this.doseLegendHolder,
      this.doseLegendSvg
    );

    doseVol.initializeMaxDoseSlider(this.panels);
    doseVol.initializeLegend();
    doseVol.initializeDoseContourInput(this.panels);

    let dims = "zxy";
    let sliceNum = {};

    axes.forEach((axis, i) => {
      // Get the correct slice number
      sliceNum[axis] = this.densityVolume
        ? this.densityVolume.prevSlice[axis].sliceNum
        : Math.floor(doseVol.data.voxelNumber[dims[i]] / 2);

      let slice = doseVol.getSlice(axis, sliceNum[axis]);
      doseVol.drawDose(slice, this.panels[axis].zoomTransform);
      // Update the axis
      drawAxes(
        this.panels[axis].zoomTransform,
        this.svgObjs["axis-svg"][axis],
        slice
      );
      this.sliceSliders[axis].setCurrentValue(sliceNum[axis]);
    });

    // Set the panel doseVolume object
    Object.values(this.panels).forEach((panel) => {
      panel.doseVol = doseVol;
      if (!panel.volume) {
        panel.volume = doseVol;
        panel.setupZoom();
        // Update the slider max values
        axes.forEach((axis, i) => {
          this.sliceSliders[axis].setMaxValue(
            doseVol.data.voxelNumber[dims[i]]
          );
          this.sliceSliders[axis].setCurrentValue(sliceNum[axis]);
        });
      }
    });

    if (this.densityVolume) {
      enableCheckboxForDensityPlot();
    }
    enableCheckboxForDoseProfilePlot();
    enableExportVisualizationButton();
    enableCheckboxForVoxelInformation();
  }

  /**
   * Set the density volume of the VolumeViewer.
   *
   * @param {DensityVolume} densityVol The density volume to be set.
   */
  setDensityVolume(densityVol) {
    this.densityVolume = densityVol;

    densityVol.setHtmlObjects(
      this.svgObjs["plot-density"],
      this.densityLegendHolder,
      this.densityLegendSvg
    );

    densityVol.initializeLegend();
    let dims = "zxy";
    let sliceNum = {};

    axes.forEach((axis, i) => {
      // Get the correct slice number
      sliceNum[axis] = this.doseVolume
        ? this.doseVolume.prevSlice[axis].sliceNum
        : Math.floor(densityVol.data.voxelNumber[dims[i]] / 2);

      let slice = densityVol.getSlice(axis, sliceNum[axis]);
      densityVol.drawDensity(slice, this.panels[axis].zoomTransform);
      // Update the axis
      drawAxes(
        this.panels[axis].zoomTransform,
        this.svgObjs["axis-svg"][axis],
        slice
      );
    });

    // Set the panel densityVolume object
    Object.values(this.panels).forEach((panel) => {
      panel.densityVol = densityVol;
      if (!panel.volume) {
        panel.volume = densityVol;
        panel.setupZoom();
        // Update the slider max values
        axes.forEach((axis, i) => {
          this.sliceSliders[axis].setMaxValue(
            densityVol.data.voxelNumber[dims[i]]
          );
          this.sliceSliders[axis].setCurrentValue(sliceNum[axis]);
        });
      }
    });

    if (this.doseVolume) {
      enableCheckboxForDensityPlot();
    }
    enableExportVisualizationButton();
    // TODO: Move this outside volume viewer and assume all loaded egsphants
    // have same density range
    initializeWindowAndLevelSlider(
      this.levelParentDiv,
      this.windowParentDiv,
      densityVol,
      this.panels
    );
    enableCheckboxForVoxelInformation();
  }

  /**
   * Remove the current dose volume of the VolumeViewer.
   */
  removeDoseVolume() {
    this.doseVolume = null;

    // Remove the volume object from panels
    Object.values(this.panels).forEach((panel) => {
      // Clear the panel
      if (panel.doseVol) panel.doseVol.clearDose(panel.axis);

      // Set the volume object to density vol if need be
      if (panel.volume === panel.doseVol) panel.volume = panel.densityVol;

      panel.doseVol = null;
    });
  }

  /**
   * Remove the current density volume of the VolumeViewer.
   */
  removeDensityVolume() {
    this.densityVolume = null;

    // Remove the level and dose sliders
    this.levelParentDiv.select("*").remove();
    this.windowParentDiv.select("*").remove();

    // Remove the volume object from panels
    Object.values(this.panels).forEach((panel) => {
      // Clear the panel
      if (panel.densityVol) panel.densityVol.clearDensity(panel.axis);

      // Set the volume object to density vol if need be
      if (panel.volume === panel.densityVol) panel.volume = panel.doseVol;

      panel.densityVol = null;
    });

    // TODO: Remove legend as well
  }

  /**
   * Get a string representation of the margin object to style HTML elements.
   *
   * @param {Object} margin An object containing numbers of pixels for the top,
   * right, bottom, and left margins.
   */
  getMarginStr(margin) {
    return (
      margin.top +
      "px " +
      margin.right +
      "px " +
      margin.bottom +
      "px " +
      margin.left +
      "px"
    );
  }

  /**
   * Update the dose file dropdown selector when a new dose file is added.
   *
   * @param {DoseVolume} doseVol The new dose volume to be added to the
   * dropdown.
   * @param {number} i The index of the dose volume.
   */
  // TODO: Create event listeners instead of calling this every time
  updateDoseFileSelector(doseVol, i) {
    this.doseSelector.append("option").attr("value", i).text(doseVol.fileName);

    this.doseComparisonSelector
      .append("option")
      .attr("value", i)
      .text(doseVol.fileName);

    // TODO: Wait until first dose selector has dose loaded to enable
    if (doseVolumeList.length >= 2) {
      this.doseComparisonSelector.attr("disabled", null);
    }
  }

  /**
   * Update the density file dropdown selector when a new density file is added.
   *
   * @param {DensityVolume} densityVol The new density volume to be added to the
   * dropdown.
   * @param {number} i The index of the density volume.
   */
  updateDensityFileSelector(densityVol, i) {
    this.densitySelector
      .append("option")
      .attr("value", i)
      .text(densityVol.fileName);
  }

  /**
   * Populate the file selectors with uploaded dose and density volumes.
   */
  setUpFileSelectors() {
    let volumeViewer = this;

    // For existing dose and density files uploaded, add to file selectors
    densityVolumeList.forEach((densityVol, i) =>
      this.updateDensityFileSelector(densityVol, i)
    );
    doseVolumeList.forEach((doseVol, i) =>
      this.updateDoseFileSelector(doseVol, i)
    );

    // Add behaviour, when volume is selected, change the volume viewer property
    this.doseSelector.on("change", function () {
      if (this.value == -1) {
        // If the base text is chosen, remove dose volume if loaded
        volumeViewer.removeDoseVolume();
      } else {
        volumeViewer.setDoseVolume(doseVolumeList[this.value]);
      }
    });

    this.densitySelector.on("change", function () {
      if (this.value == -1) {
        volumeViewer.removeDensityVolume();
      } else {
        volumeViewer.setDensityVolume(densityVolumeList[this.value]);
      }
    });

    this.doseComparisonSelector.on("change", function () {
      if (volumeViewer.doseVolume) {
        if (this.value == -1) {
          // If the base text is chosen, remove density volume if loaded
          let volIndex = parseInt(
            volumeViewerList[0].doseSelector.node().value
          );
          if (volIndex >= 0) {
            volumeViewer.setDoseVolume(doseVolumeList[volIndex]);
          } else {
            volumeViewer.removeDoseVolume();
          }
        } else if (volumeViewer.doseVolume === doseVolumeList[this.value]) {
          console.log(
            "Select a different dose volume than one that is already loaded"
          );
        } else {
          volumeViewer.makeDoseComparison(
            volumeViewer.doseVolume,
            doseVolumeList[this.value]
          );
        }
      }
    });
  }

  /**
   * Create a dose comparison volume from two given dose volumes.
   *
   * @param {DoseVolume} doseVol1 The first dose volume to compare.
   * @param {DoseVolume} doseVol2 The second dose volume to compare.
   */
  // TODO: Move this logic to DoseComparisonVolume class
  makeDoseComparison(doseVol1, doseVol2) {
    // First normalize the dose data to turn into a percentage
    let doseArr1 = doseVol1.data.dose.map(
      (doseVal) => doseVal / doseVol1.data.maxDose
    );
    let doseArr2 = doseVol2.data.dose.map(
      (doseVal) => doseVal / doseVol2.data.maxDose
    );

    // Take the difference
    let doseDiff = new Array(doseArr1.length);
    for (let i = 0; i < doseArr1.length; i++) {
      if (doseArr1[i] || doseArr2[i]) {
        doseDiff[i] = (doseArr1[i] || 0) - (doseArr2[i] || 0);
      }
    }
    // Calculate the error for each
    let errArr2 = doseVol2.data.error;
    let error = doseVol1.data.error.map((err1, i) =>
      Math.sqrt(err1 * err1 + errArr2[i] * errArr2[i])
    );

    // Make new volume
    let newData = {
      ...doseVol1.data,
      dose: doseDiff,
      error: error,
      maxDose: 1.0,
    };

    let combinedFileName = doseVol1.fileName + "_" + doseVol2.fileName;

    let doseComparisonVol = new DoseComparisonVolume(
      combinedFileName,
      this.mainViewerDimensions,
      this.legendDimensions,
      newData
    );

    doseComparisonVolumeList.push(doseComparisonVol);

    // Set dose to new difference volume
    this.setDoseVolume(doseComparisonVol);
  }

  /**
   * Build the HTML of the volume viewer object.
   *
   * @param {string} id The unique ID of the volume viewer.
   */
  buildBaseHtml(id) {
    // Select main div
    let base = d3.select("#image-to-print");

    // Add div to hold the panels, legend, and dose profiles
    this.volHolder = base
      .append("div")
      .attr("id", "volume-holder-" + id)
      .attr("class", "volume-holder");

    // Add the file selector dropdowns
    let fileSelector = this.volHolder
      .append("div")
      .attr("class", "file-selector");
    this.densitySelector = fileSelector
      .append("select")
      .attr("name", "density-file");
    this.densitySelector
      .append("option")
      .attr("value", -1)
      .text("Choose a density file");
    this.doseSelector = fileSelector.append("select").attr("name", "dose-file");
    this.doseSelector
      .append("option")
      .attr("value", -1)
      .text("Choose a dose file");
    this.doseComparisonSelector = fileSelector
      .append("select")
      .attr("name", "dose-file-comparison")
      .attr("disabled", "disabled");
    this.doseComparisonSelector
      .append("option")
      .attr("value", -1)
      .text("Choose a dose file to compare");

    // Set up the file selector dropdowns
    this.setUpFileSelectors();

    // Add window and level sliders
    this.levelParentDiv = this.volHolder
      .append("div")
      .attr("class", "window-level-container");
    this.windowParentDiv = this.volHolder
      .append("div")
      .attr("class", "window-level-container");

    // Add voxel information
    buildVoxelInfoHtml(this.volHolder, id);

    // Append div to hold the panels
    this.viewerContainer = this.volHolder
      .append("span")
      .attr("class", "container")
      .style("vertical-align", "top");

    // Append div to hold the legend
    this.legendHolder = this.volHolder
      .append("span")
      .attr("class", "legend-holder");

    // Build other html and class objects
    this.buildViewerContainer(this.mainViewerDimensions);
    this.buildLegend(this.legendDimensions);
    this.buildPanels(mainViewerDimensions);
  }

  /**
   * Build the HTML for the viewer container to hold the panels.
   *
   * @param {Object} mainViewerDimensions The pixel dimensions of the main viewer.
   */
  buildViewerContainer(mainViewerDimensions) {
    // TODO: In panel class, have build html instead
    let dimensions = ["z", "x", "y"];

    // For each axis and plot class, make html
    let classes = ["axis-svg", "plot-density", "plot-dose", "plot-marker"];
    let type = ["svg", "canvas", "svg", "svg"];
    this.svgObjs = {
      "axis-svg": {},
      "plot-density": {},
      "plot-dose": {},
      "plot-marker": {},
    };

    this.axisObjs = {
      xy: {},
      yz: {},
      xz: {},
    };

    let dispatch = this.dispatch;

    // Add html for panels and slice sliders
    axes.forEach((axis, i) => {
      let selectedDiv = this.viewerContainer
        .append("div")
        .classed("panel-" + axis, true)
        .style("display", "inline-block");

      // Slice slider callback and parameters
      var onSliceChangeCallback = (sliderVal) => {
        let currPanel = this.panels[axis];
        // Update slice of current panel
        currPanel.updateSlice(parseInt(sliderVal));

        // TODO: Fix this, bug after zooming/translating and changing slice
        // Update marker position, voxel information and dose profile
        let plotCoords = currPanel.markerPosition;
        if (plotCoords) {
          dispatch.call("markerchange", this, {
            plotCoords: plotCoords,
            panel: currPanel,
          });
        }
      };

      let sliceSliderParams = {
        id: "slice-number-" + axis,
        label: "Slice Number",
        format: d3.format("d"),
        startingVal: 0,
        minVal: 0,
        maxVal: 1,
        step: 1,
        margin: {
          top: 0,
          right: mainViewerDimensions.margin.right,
          bottom: 0,
          left: mainViewerDimensions.margin.left,
        },
        style: { "text-align": "center" },
      };

      // Build new slider
      this.sliceSliders[axis] = new Slider(
        selectedDiv,
        onSliceChangeCallback,
        sliceSliderParams
      );

      // Build div to hold the panel
      let imageHolder = selectedDiv
        .append("div")
        .classed("imageholder-" + axis, true)
        .classed("parent", true);

      classes.forEach((className, i) => {
        let layerNum = i + 1;
        let plot = imageHolder
          .append("div")
          .classed("layer-" + layerNum, true)
          .style("z-index", layerNum)
          .append(type[i])
          .classed(className, true)
          .classed(axis, true)
          .classed("plot", true)
          .attr("width", mainViewerDimensions.width)
          .attr("height", mainViewerDimensions.height)
          .style("margin", this.getMarginStr(mainViewerDimensions.margin));

        this.svgObjs[className][axis] = plot;
        this.axisObjs[axis][className] = plot;
      });

      // Add dose profile to bottom of panel
      this.doseProfileList[i] = new DoseProfile(
        sideDoseProfileDimensions,
        selectedDiv,
        this.id + "-" + dimensions[i]
      );
    });
  }

  /**
   * Build the HTML for the dose and density legends.
   *
   * @param {Object} legendDimensions The pixel dimensions of the legends.
   */
  buildLegend(legendDimensions) {
    // Set up legends
    var getLegendHolderAndSvg = (className) => {
      let legendHolder = this.legendHolder
        .append("span")
        .attr("id", className + "-legend-holder")
        .style("width", legendDimensions.width + "px")
        .style("height", legendDimensions.height + "px")
        .style("margin", this.getMarginStr(legendDimensions.margin));

      let legendSvg = legendHolder
        .append("svg")
        .attr("id", className + "-legend-svg")
        .attr("class", "legend");

      return [legendHolder, legendSvg];
    };

    [this.doseLegendHolder, this.doseLegendSvg] = getLegendHolderAndSvg("dose");
    [this.densityLegendHolder, this.densityLegendSvg] = getLegendHolderAndSvg(
      "density"
    );
  }

  /**
   * Create the panel object for each axis.
   *
   * @param {Object} mainViewerDimensions The pixel dimensions of the main viewer.
   */
  buildPanels(mainViewerDimensions) {
    this.panels = axes.reduce((obj, axis, i) => {
      return {
        ...obj,
        [axis]: new Panel(
          mainViewerDimensions,
          this.densityVol,
          this.doseVol,
          axis,
          this.axisObjs[axis],
          this.sliceSliders[axis],
          this.dispatch,
          this.id
        ),
      };
    }, {});
  }

  /**
   * Set up the event dispatcher.
   */
  initializeDispatch() {
    // Set up marker coord change event
    let panels = this.panels;

    this.dispatch.on("markerchange.panels", function (d) {
      d.panel.updateMarker(d.plotCoords);

      // Want to get the voxel coords then change the sliceNum of the other volume panels
      let voxelCoords = coordsToVoxel(
        d.plotCoords,
        d.panel.axis,
        d.panel.sliceNum,
        d.panel.volume,
        d.panel.zoomTransform
      );

      Object.values(panels).forEach((panel) => {
        if (panel.axis !== d.panel.axis) {
          let sliceNum, voxelNums;
          if (panel.axis === "xy") {
            sliceNum = voxelCoords[2];
            voxelNums = [voxelCoords[0], voxelCoords[1]];
          } else if (panel.axis === "yz") {
            sliceNum = voxelCoords[0];
            voxelNums = [voxelCoords[1], voxelCoords[2]];
          } else {
            sliceNum = voxelCoords[1];
            voxelNums = [voxelCoords[0], voxelCoords[2]];
          }

          // Convert voxel number to pixel value for both x and y coordinates
          let xScale, yScale;
          if (panel.zoomTransform) {
            xScale = panel.zoomTransform.rescaleX(
              panel.volume.prevSlice[panel.axis].contourXScale
            );
            yScale = panel.zoomTransform.rescaleY(
              panel.volume.prevSlice[panel.axis].contourYScale
            );
          } else {
            xScale = panel.volume.prevSlice[panel.axis].contourXScale;
            yScale = panel.volume.prevSlice[panel.axis].contourYScale;
          }

          let coords = [
            Math.ceil(xScale(voxelNums[0])),
            Math.ceil(yScale(voxelNums[1])),
          ];

          panel.updateMarker(coords, false);
          panel.updateSlice(sliceNum);
          panel.updateSlider(sliceNum);
        }
      });
    });

    this.dispatch.on("markerchange.voxelinfo", function (d) {
      updateVoxelCoords(
        d.panel.densityVol,
        d.panel.doseVol,
        d.plotCoords,
        d.panel.axis,
        d.panel.sliceNum,
        d.panel.zoomTransform,
        d.panel.volumeViewerId
      );
    });
  }
}
