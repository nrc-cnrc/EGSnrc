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

// definitions for StandardJS formatter
/* global d3 */
/* global FileReader */
/* global alert */
/* global XMLHttpRequest */

// REMOVE THESE GLOBAL IMPORTS ONCE MODULES RE-IMPLEMENTED
/* global MAIN_VIEWER_DIMENSIONS */
/* global LEGEND_DIMENSIONS */
/* global DOSE_PROFILE_DIMENSIONS */
/* global volumeViewerList */
/* global densityVolumeList */
/* global doseVolumeList */
/* global VolumeViewer */
/* global DensityVolume */
/* global DoseVolume */
/* global StructureSetVolume */
/* global structureSetVolumeList */
/* global combineDICOMDensityData */
/* global combineDICOMDoseData */
/* global processPhantomData */
/* global processDoseData */
/* global processDICOMSlice */

// import {
//   DOSE_PROFILE_DIMENSIONS, LEGEND_DIMENSIONS, MAIN_VIEWER_DIMENSIONS,
//   densityVolumeList, doseVolumeList, volumeViewerList, structureSetVolumeList
// } from './index.js'
// import { processDoseData, processPhantomData } from './read-data.js'
// import { VolumeViewer } from './volume-viewer.js'
// import { DensityVolume } from './volumes/density-volume.js'
// import { DoseVolume } from './volumes/dose-volume.js'
// import { StructureSetVolume } from './volumes/structure-set-volume.js'
// import { combineDICOMDensityData, combineDICOMDoseData, processDICOMSlice } from './dicom.js'

const dropArea = d3.select('#drop-area')
const progressBar = d3.select('#progress-bar')
const progressBarNode = progressBar.node()

/**
 * Initialize the progress bar for a new round of uploading files.
 */
function initializeProgress () {
  progressBarNode.value = 0
  // Show the progress bar
  progressBar.classed('hidden', false)
}

/**
 * Initialize the progress bar for a new round of uploading files.
 *
 * @param {number} percent    The percent progress completed of the current file.
 * @param {number} fileNum    The number of the current file.
 * @param {number} totalFiles The total number of files.
 */
function updateProgress (percent, fileNum, totalFiles) {
  progressBarNode.value = percent * (fileNum / totalFiles)
}

/**
 * Once files are uploaded, end progress by hiding the bar.
 */
function endProgress () {
  progressBar.classed('hidden', true)
}

// Turn off normal drop response on window
['dragover', 'drop'].forEach((eventName) => {
  window.addEventListener(eventName, function (e) {
    e.preventDefault()
    e.stopPropagation()
  })
})

// Add highlight class to drop area when holding file overtop
dropArea.on('dragenter dragover', () => dropArea.classed('highlight', true))
dropArea.on('dragleave drop', () => dropArea.classed('highlight', false))

/**
 * Make a DensityVolume object and add it to the density volume list.
 *
 * @param {string} fileName The name of the .egsphant or DICOM file.
 * @param {Object} data     The data object created from the .egsphant or DICOM file.
 */
var makeDensityVolume = (fileName, data, args) => {
  const densityVol = new DensityVolume(
    fileName,
    MAIN_VIEWER_DIMENSIONS,
    LEGEND_DIMENSIONS,
    data,
    args
  )

  densityVolumeList.push(densityVol)
  volumeViewerList.forEach((volumeViewer) =>
    volumeViewer.updateDensityFileSelector(
      densityVol,
      densityVolumeList.length - 1
    )
  )
}

/**
 * Make a DoseVolume object and add it to the density volume list.
 *
 * @param {string} fileName The name of the .3ddose file.
 * @param {Object} data     The data object created from the .3ddose file.
 */
var makeDoseVolume = (fileName, data) => {
  const doseVol = new DoseVolume(
    fileName,
    MAIN_VIEWER_DIMENSIONS,
    LEGEND_DIMENSIONS,
    data
  )

  doseVolumeList.push(doseVol)
  volumeViewerList.forEach((volumeViewer) =>
    volumeViewer.updateDoseFileSelector(doseVol, doseVolumeList.length - 1)
  )
}

/**
 * Make a StructureSetVolume object and add it to the density volume list.
 *
 * @param {string} fileName The name of the DICOM file.
 * @param {Object} data     The data object created from the DICOM file.
 */
var makeStructureSetVolume = (fileName, data) => {
  const structureSetVol = new StructureSetVolume(
    fileName,
    MAIN_VIEWER_DIMENSIONS,
    LEGEND_DIMENSIONS,
    data
  )

  structureSetVolumeList.push(structureSetVol)
}

/**
 * Add event listener on drop to process files.
 */
dropArea.node().addEventListener('drop', function (e) {
  if (e.dataTransfer && e.dataTransfer.files.length) {
    e.preventDefault()
    e.stopPropagation()
    const files = [...e.dataTransfer.files]
    handleFiles(files)
  }
})

/**
 * Add event listener on button press to process files.
 */
d3.select('#file-input').on('change', function () {
  if (this.files.length) {
    const files = [...this.files]
    handleFiles(files)
  }
})

/**
 * If the test files link is pressed, process test files.
 */
d3.select('#test-files').on('click', function () {
  const testFiles = ['pediatric.egsphant',
    'RD.2.16.840.1.114362.1.5.6.1.121121.6102256374.313565332.637.1446.dcm',
    'RS.2.16.840.1.114362.1.5.6.1.121121.6102256374.313565331.1073.1444.dcm']

  var volViewer
  if (volumeViewerList.length === 0) {
    // Add a new volume viewer
    volViewer = new VolumeViewer(
      MAIN_VIEWER_DIMENSIONS,
      LEGEND_DIMENSIONS,
      DOSE_PROFILE_DIMENSIONS,
      'vol-' + volumeViewerList.length
    )
    volumeViewerList.push(volViewer)
  } else {
    volViewer = volumeViewerList[0]
  }

  testFiles.forEach((testFile, i) => {
    const request = new XMLHttpRequest()
    if (i !== 0) request.responseType = 'arraybuffer'

    // Get each of the test files
    request.open('GET', './test-files/' + testFile, true)

    request.onreadystatechange = function () {
      if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {
        // Extract extension and file name
        const ext = request.responseURL.split('.').pop()
        const fileName = request.responseURL.split('/').pop()

        var response, data

        if (i === 0) { // If the egsphant test file
          // Make density volume
          response = request.response.split('\n')
          data = processPhantomData(response)
          makeDensityVolume(fileName.split('.')[0], data)

          // Set volume viewer selector to test file
          volViewer.setDensityVolume(densityVolumeList[densityVolumeList.length - 1])
          volViewer.densitySelector.node().selectedIndex = densityVolumeList.length
        } else if (i === 1) { // If the DICOM dose file
          // Make dose volume
          response = request.response
          data = processDICOMSlice(response)
          const DICOMData = combineDICOMDoseData([{ data: data, ext: ext, fileName: fileName }])

          // Set volume viewer selector to test file
          makeDoseVolume(fileName.slice(0, 9), DICOMData, { isDicom: true })
          volViewer.setDoseVolume(doseVolumeList[doseVolumeList.length - 1])
          volViewer.doseSelector.node().selectedIndex = doseVolumeList.length
        } else { // If the DICOM structure set file
          // Make structure set volume
          response = request.response
          data = processDICOMSlice(response)
          makeStructureSetVolume(fileName.split('.')[0], data)

          // Enable ROI checkboxes
          volViewer.enableCheckbox(volViewer.showROIOutlinesCheckbox)
          if (volViewer.isDVHAllowed()) volViewer.enableCheckbox(volViewer.showDVHCheckbox)
        }
      }
    }

    request.send()
  })
})

/**
 * Initializes the progress bar and reads each of the files.
 *
 * @param {File[]} files  The list of files to be processed.
 */
function handleFiles (files) {
  initializeProgress()
  const promises = []

  files.forEach((file, i) => {
    const filePromise = new Promise((resolve, reject) => {
      readFile(resolve, reject, file, i + 1, files.length)
    })

    promises.push(filePromise)
  })

  Promise.allSettled(promises)
    .then((results) => results.filter(result => result.status === 'fulfilled')) // Filter out rejected files
    .then((results) => results.map(result => result.value)) // Replace the promise with value
    .then(files => {
      const dicomDensityList = files.filter(file => (file.ext === 'dcm' || file.ext === 'DCM') && file.data.type === 'CT Image Storage')
      const dicomDoseList = files.filter(file => (file.ext === 'dcm' || file.ext === 'DCM') && file.data.type === 'RT Dose Storage')
      const egsphantList = files.filter(file => file.ext === 'egsphant')
      const doseList = files.filter(file => file.ext === '3ddose')
      const structureSetList = files.filter(file => (file.ext === 'dcm' || file.ext === 'DCM') && file.data.type === 'RT Structure Set Storage')

      // If DICOM density files
      if (dicomDensityList.length > 0) {
        const DICOMData = combineDICOMDensityData(dicomDensityList)
        // TODO: Have a naming system for dicom files, perhaps return name from combineDICOMDensityData
        makeDensityVolume(dicomDensityList[0].fileName.slice(0, 9), DICOMData, { isDicom: true })
      }

      // If DICOM dose files
      if (dicomDoseList.length > 0) {
        const DICOMData = combineDICOMDoseData(dicomDoseList)
        makeDoseVolume(dicomDoseList[0].fileName.slice(0, 9), DICOMData, { isDicom: true })
      }

      // If egsphant files
      if (egsphantList.length > 0) {
        // Create density volume
        egsphantList.forEach(file => makeDensityVolume(file.fileName.split('.')[0], file.data))
      }

      // If 3ddose files
      if (doseList.length > 0) {
        // Create dose volume
        doseList.forEach(file => makeDoseVolume(file.fileName.split('.')[0], file.data))
      }

      // If structure set files
      if (structureSetList.length > 0) {
        // Create structure set volume
        structureSetList.forEach(file => makeStructureSetVolume(file.fileName.split('.')[0], file.data))
      }

      // If this is the first volume uploaded, load into first volume viewer
      if (volumeViewerList.length === 0) {
        const volViewer = new VolumeViewer(
          MAIN_VIEWER_DIMENSIONS,
          LEGEND_DIMENSIONS,
          DOSE_PROFILE_DIMENSIONS,
          'vol-' + volumeViewerList.length
        )
        volumeViewerList.push(volViewer)
      }

      const volViewer = volumeViewerList[0]

      // If this is the first dose file uploaded, display on first volume viewer
      if (doseVolumeList.length >= 1 && volViewer.doseVolume === null) {
        volViewer.setDoseVolume(doseVolumeList[0])
        volViewer.doseSelector.node().selectedIndex = 1
      }

      // If this is the first density uploaded, display on first volume viewer
      if (densityVolumeList.length >= 1 && volViewer.densityVolume === null) {
        volViewer.setDensityVolume(densityVolumeList[0])
        volViewer.densitySelector.node().selectedIndex = 1
      }

      volumeViewerList.forEach(volViewer => {
        // Update the dose comparison selectors
        if (doseVolumeList.length >= 2) {
          const idx = doseVolumeList.findIndex((vol) => vol.fileName === volViewer.doseVolume.fileName)
          volViewer.initializeDoseComparisonSelector(idx)
        }

        // Update the ROI checkboxes
        if (structureSetVolumeList.length > 0) {
          volViewer.enableCheckbox(volViewer.showROIOutlinesCheckbox)
          if (volViewer.isDVHAllowed()) volViewer.enableCheckbox(volViewer.showDVHCheckbox)
        }
      })
    })
}

/**
 * Read each file and create a dose or density volume object.
 *
 * @param {File} file       The file to be processed.
 * @param {number} fileNum  The index of the file to be processed.
 * @param {File} totalFiles The total number of files to be processed.
 */
function readFile (resolve, reject, file, fileNum, totalFiles) {
  const reader = new FileReader()
  const fileName = file.name
  const ext = fileName.split('.').pop()

  reader.addEventListener('loadstart', function () {
    console.log('File reading started')
    return true
  })

  // Update progress bar
  reader.addEventListener('progress', function (e) {
    if (e.lengthComputable === true) {
      updateProgress(
        Math.floor((e.loaded / e.total) * 100),
        fileNum,
        totalFiles
      )
    }
  })

  reader.addEventListener('error', function () {
    alert('Error: Failed to read file')
    return true
  })

  // TODO: Add check for dose and density distributions like
  // https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1407-L1412
  // File is successfully read
  reader.addEventListener('load', function (e) {
    const result = e.target.result
    let data

    if (ext === 'egsphant') {
      const resultSplit = result.split('\n')
      data = processPhantomData(resultSplit)
    } else if (ext === '3ddose') {
      const resultSplit = result.split('\n')
      data = processDoseData(resultSplit)
    } else if (ext === 'dcm' || ext === 'DCM') {
      data = processDICOMSlice(result)
    } else {
      console.log('Unknown file extension')
      reject('Unknown file extension')
      return true
    }

    resolve({ data: data, ext: ext, fileName: fileName })

    console.log('Finished processing data')
    return true
  })

  reader.addEventListener('loadend', function () {
    // If all files have been loaded in
    if (fileNum === totalFiles) {
      // Set the bar progress to full
      progressBarNode.value = progressBarNode.max
      // End the progress after 500 milliseconds
      window.setTimeout(endProgress, 500)
    }
  })

  if (ext === 'dcm' || ext === 'DCM') {
    // Read as array buffer if DICOM
    reader.readAsArrayBuffer(file)
  } else {
    // Otherwise read as text file
    reader.readAsText(file)
  }
}
