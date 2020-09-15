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
/* global DensityVolume */
/* global mainViewerDimensions */
/* global legendDimensions */
/* global densityVolumeList */
/* global VolumeViewer */
/* global volumeViewerList */
/* global DoseVolume */
/* global doseVolumeList */
/* global sideDoseProfileDimensions */
/* global FileReader */
/* global processPhantomData */
/* global processDoseData */
/* global alert */

const dropArea = d3.select('#drop-area')
const progressBar = d3.select('#progress-bar')
const progressBarNode = progressBar.node()

/**
 * Initialize the progress bar for a new round of uploading files.
 *
 * @param {number} numfiles The number of files being uploaded.
 */
function initializeProgress () {
  progressBarNode.value = 0
  // Show the progress bar
  progressBar.classed('hidden', false)
}

/**
 * Initialize the progress bar for a new round of uploading files.
 *
 * @param {number} numfiles The number of files being uploaded.
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
 * @param {string} fileName The name of the .egsphant file.
 * @param {Object} data     The data object created from the .egsphant file.
 */
var makeDensityVolume = (fileName, data) => {
  const densityVol = new DensityVolume(
    fileName,
    mainViewerDimensions,
    legendDimensions,
    data
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
    mainViewerDimensions,
    legendDimensions,
    data
  )

  doseVolumeList.push(doseVol)
  volumeViewerList.forEach((volumeViewer) =>
    volumeViewer.updateDoseFileSelector(doseVol, doseVolumeList.length - 1)
  )
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
  // Add a new volume viewer
  const volViewer = new VolumeViewer(
    mainViewerDimensions,
    legendDimensions,
    sideDoseProfileDimensions,
    'vol-' + volumeViewerList.length
  )
  volumeViewerList.push(volViewer)

  // Read the JSON density file from the test files directory
  d3.json('./test-files/ismail-density.json').then((densityData) => {
    makeDensityVolume('ismail.egsphant', densityData)
    volViewer.setDensityVolume(densityVolumeList[0])
    volViewer.densitySelector.node().selectedIndex = 1
  })

  // Read the JSON dose file from the test files directory
  d3.json('./test-files/ismail100-dose.json').then((doseData) => {
    makeDoseVolume('ismail100.3ddose', doseData)
    volViewer.setDoseVolume(doseVolumeList[0])
    volViewer.doseSelector.node().selectedIndex = 1
  })
})

/**
 * Initializes the progress bar and reads each of the files.
 *
 * @param {File[]} files  The list of files to be processed.
 */
function handleFiles (files) {
  initializeProgress()
  files.forEach((file, i) => readFile(file, i + 1, files.length))
}

/**
 * Read each file and create a dose or density volume object.
 *
 * @param {File} file       The file to be processed.
 * @param {number} fileNum  The index of the file to be processed.
 * @param {File} totalFiles The total number of files to be processed.
 */
function readFile (file, fileNum, totalFiles) {
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
    const resultSplit = result.split('\n')
    let data

    if (ext === 'egsphant') {
      data = processPhantomData(resultSplit)

      // Create density volume
      makeDensityVolume(fileName, data)
    } else if (ext === '3ddose') {
      data = processDoseData(resultSplit)

      // Create dose volume
      makeDoseVolume(fileName, data)
    } else {
      console.log('Unknown file extension')
      return true
    }

    // If this is the first volume uploaded, load into first volume viewer
    if (volumeViewerList.length === 0) {
      const volViewer = new VolumeViewer(
        mainViewerDimensions,
        legendDimensions,
        sideDoseProfileDimensions,
        'vol-' + volumeViewerList.length
      )
      volumeViewerList.push(volViewer)

      if (densityVolumeList.length === 1 && doseVolumeList.length === 0) {
        volViewer.setDensityVolume(densityVolumeList[0])
        volViewer.densitySelector.node().selectedIndex = 1
      } else {
        volViewer.setDoseVolume(doseVolumeList[0])
        volViewer.doseSelector.node().selectedIndex = 1
      }
    }

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

  // Read as text file
  reader.readAsText(file)
}
