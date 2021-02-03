// TODO: Move all javascript outside of index.html
// TODO: Make add new volume viewer button work

// Define all size variables
const MAIN_VIEWER_DIMENSIONS = {
  fullWidth: 320,
  fullHeight: 320,
  margin: { top: 20, right: 15, bottom: 50, left: 55 },
  get width() {
    return this.fullWidth - this.margin.left - this.margin.right
  },
  get height() {
    return this.fullHeight - this.margin.top - this.margin.bottom
  }
}

// Define legend size variables
const LEGEND_DIMENSIONS = {
  fullWidth: 90,
  fullHeight: MAIN_VIEWER_DIMENSIONS.fullHeight,
  margin: { top: 25, right: 10, bottom: 25, left: 10 },
  get width() {
    return this.fullWidth - this.margin.left - this.margin.right
  },
  get height() {
    return this.fullHeight - this.margin.top - this.margin.bottom
  }
}

// Define profile dose size variables
const DOSE_PROFILE_DIMENSIONS = {
  fullWidth: MAIN_VIEWER_DIMENSIONS.fullWidth,
  fullHeight: MAIN_VIEWER_DIMENSIONS.fullHeight * 0.6,
  margin: { top: 30, right: 15, bottom: 30, left: 55 },
  get width() {
    return this.fullWidth - this.margin.left - this.margin.right
  },
  get height() {
    return this.fullHeight - this.margin.top - this.margin.bottom
  }
}

// Global variables
var densityVolumeList = []
var doseVolumeList = []
var doseComparisonVolumeList = []
var volumeViewerList = []

// export {
//   DOSE_PROFILE_DIMENSIONS, LEGEND_DIMENSIONS, MAIN_VIEWER_DIMENSIONS,
//   densityVolumeList, doseComparisonVolumeList, doseVolumeList, volumeViewerList
// }
