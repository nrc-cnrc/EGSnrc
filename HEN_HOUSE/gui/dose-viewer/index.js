// TODO: Move all javascript outside of index.html
// TODO: Make add new volume viewer button work

// Define all size variables
const MAIN_VIEWER_DIMENSIONS = {
  fullWidth: 320,
  fullHeight: 320,
  margin: { top: 20, right: 15, bottom: 50, left: 55 },
  get width () {
    return this.fullWidth - this.margin.left - this.margin.right
  },
  get height () {
    return this.fullHeight - this.margin.top - this.margin.bottom
  }
}

// Define legend size variables
const LEGEND_DIMENSIONS = { // eslint-disable-line no-unused-vars
  fullWidth: 90,
  fullHeight: 215,
  margin: { top: 25, right: 10, bottom: 5, left: 10 },
  get width () {
    return this.fullWidth - this.margin.left - this.margin.right
  },
  get height () {
    return this.fullHeight - this.margin.top - this.margin.bottom
  }
}

// Define profile dose size variables
const DOSE_PROFILE_DIMENSIONS = { // eslint-disable-line no-unused-vars
  fullWidth: MAIN_VIEWER_DIMENSIONS.fullWidth,
  fullHeight: MAIN_VIEWER_DIMENSIONS.fullHeight * 0.6,
  margin: { top: 30, right: 15, bottom: 30, left: 55 },
  get width () {
    return this.fullWidth - this.margin.left - this.margin.right
  },
  get height () {
    return this.fullHeight - this.margin.top - this.margin.bottom
  }
}

// Define dose volume histogram size variables
const DVH_DIMENSIONS = { // eslint-disable-line no-unused-vars
  fullWidth: MAIN_VIEWER_DIMENSIONS.fullWidth * 3,
  fullHeight: MAIN_VIEWER_DIMENSIONS.fullHeight * 0.8,
  margin: DOSE_PROFILE_DIMENSIONS.margin,
  get width () {
    return this.fullWidth - this.margin.left - this.margin.right
  },
  get height () {
    return this.fullHeight - this.margin.top - this.margin.bottom
  }
}

// Global variables
var densityVolumeList = [] // eslint-disable-line no-unused-vars
var doseVolumeList = [] // eslint-disable-line no-unused-vars
var doseComparisonVolumeList = [] // eslint-disable-line no-unused-vars
var volumeViewerList = [] // eslint-disable-line no-unused-vars
var structureSetVolumeList = [] // eslint-disable-line no-unused-vars

// export {
//   DOSE_PROFILE_DIMENSIONS, LEGEND_DIMENSIONS, MAIN_VIEWER_DIMENSIONS,
//   densityVolumeList, doseComparisonVolumeList, doseVolumeList,
//   volumeViewerList, structureSetVolumeList
// }
