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
/* global Blob */
/* global webkitURL */
/* global domtoimage */
/* global btoa */
/* global XMLSerializer */

const makeAndDownloadCsv = (data, name) => {
  // Create data blob
  var csvBlob = new Blob([d3.csvFormat(data)], {
    type: 'text/csv;charset=utf-8;'
  })

  // Get data url for blob
  const csvString = (URL || webkitURL).createObjectURL(csvBlob)

  // Download the file
  downloadURI(csvString, name)
}

/**
 * Set up the export dose profile to csv button behaviour. Downloads a csv for each dose
 * profile (x, y, z) of the volume viewer.
 */
// TODO: If plot density is selected, download density information as well.
var defineExportDoseProfileCSVButtonBehaviour = function (volumeViewer) { // eslint-disable-line no-unused-vars
  const dims = ['Z', 'X', 'Y']
  var doseProfiles = volumeViewer.doseProfileList.map((doseProfile) => doseProfile.data)
  var n = Math.max(...doseProfiles.map((doseProfile) => doseProfile.length))
  var doseProfileData = []

  // Iterate through each element of the X, Y, and Z dose profiles
  for (let i = 0; i < n; i++) {
    var item
    var data = {}
    dims.forEach((dim, j) => {
      item = doseProfiles[j][i]
      data['position' + dim] = item ? item.position : null
      data['value' + dim] = item ? item.value : null
      data['error' + dim] = item ? item.err : null
    })

    doseProfileData.push(data)
  }

  // Create csv name
  var csvName =
        volumeViewer.id + '-dose-profile.csv'

  makeAndDownloadCsv(doseProfileData, csvName)
}

/**
 * Set up the export DVH to csv button behaviour. Downloads a csv for the DVH of
 * the volume viewer.
 */
var defineExportDVHToCSVButtonBehaviour = function (volumeViewer) { // eslint-disable-line no-unused-vars
  if (volumeViewer.DVH) {
    // Create csv name
    const csvName = volumeViewer.id + '-DVH.csv'

    // Put data in desired format before converting to csv
    const ROIs = volumeViewer.DVH.data
    const DVHData = ROIs[0].values.map((val, idx) => {
      var obj = { dose: val.x }
      ROIs.forEach((ROI) => {
        obj[ROI.key] = ROI.values[idx].y
      })
      return obj
    })

    makeAndDownloadCsv(DVHData, csvName)
  }
}
/**
 * Set-up the export to png button behaviour. Takes a screenshot of the volume
 * viewer.
 *
 * Part from http://bl.ocks.org/Rokotyan/0556f8facbaf344507cdc45dc3622177
 * https://github.com/aces/brainbrowser/blob/master/examples/volume-viewer-demo.js#L194-L248
 */
var defineExportPNGButtonBehaviour = function (volumeViewer) { // eslint-disable-line no-unused-vars
  const node = volumeViewer.volHolder.node()

  // TODO: Let user choose between png and svg
  const format = 'png'

  // Define image width and height
  const imgWidth = node.clientWidth + 5
  const imgHeight = node.clientHeight + 5

  // Create image name
  let imageName = ''
  if (volumeViewer.densityVolume !== null) {
    imageName += volumeViewer.densityVolume.fileName + '-'
  }
  if (volumeViewer.doseVolume !== null) {
    imageName += volumeViewer.doseVolume.fileName + '-'
  }

  imageName += volumeViewer.id + '.' + format

  // Optional: Define elements to exclude from png image
  function filter (node) { // eslint-disable-line no-unused-vars
    return node.tagName !== 'input'
  }

  if (format === 'png') {
    // Use the domtoimage module to save the node
    domtoimage
      .toBlob(node, {
        // filter: filter,
        bgcolor: 'white',
        width: imgWidth,
        height: imgHeight
      })
      .then(function (blob) {
        window.saveAs(blob, imageName)
      })
  } else if (format === 'svg') {
    // Convert the div to string
    var imgString = getImgString(node)

    // Convert img string to data URL
    var imgSrc =
      'data:image/svg+xml;base64,' +
      btoa(unescape(encodeURIComponent(imgString)))
    downloadURI(imgSrc, imageName + '.png')
  }
}

/**
 * Download the URI to the users computer.
 *
 * @param {string} uri  The URI of the resource to download.
 * @param {string} name The name given to the downloaded file.
 */
function downloadURI (uri, name) {
  var link = document.createElement('a')

  link.download = name
  link.href = uri
  document.body.appendChild(link)
  link.click()
  document.body.removeChild(link)
}

/**
 * Encode the node as a string for downloading.
 *
 * @param {Object} node The element node to get a string representation of.
 */
function getImgString (node) {
  // Extract all CSS Rules
  // From https://developer.mozilla.org/en-US/docs/Web/API/StyleSheetList
  const allCSS = [...document.styleSheets]
    .map((styleSheet) => {
      try {
        return [...styleSheet.cssRules].map((rule) => rule.cssText).join('')
      } catch (e) {
        console.log(
          'Access to stylesheet %s is denied. Ignoring...',
          styleSheet.href
        )
      }
    })
    .filter(Boolean)
    .join('\n')

  // Append CSS to divNode
  var styleElement = document.createElement('style')
  styleElement.setAttribute('type', 'text/css')
  styleElement.innerHTML = allCSS
  var refNode = node.hasChildNodes() ? node.children[0] : null
  node.insertBefore(styleElement, refNode)

  // Convert node to string
  var serializer = new XMLSerializer()
  var svgString = serializer.serializeToString(node)

  return svgString
}

// export { defineExportDoseProfileCSVButtonBehaviour,
// defineExportDVHToCSVButtonBehaviour, defineExportPNGButtonBehaviour }
