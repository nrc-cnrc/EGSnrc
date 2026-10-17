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

/**
 * Extract data from .egsphant files.
 *
 * @param {Object} data The .egsphant file read as text.
 * @returns {Object}
 */
var processPhantomData = function (data) { // eslint-disable-line no-unused-vars
  var getMax = function (a) {
    return Math.max(...a.map((e) => (Array.isArray(e) ? getMax(e) : e)))
  }

  var getMin = function (a) {
    return Math.min(...a.map((e) => (Array.isArray(e) ? getMin(e) : e)))
  }

  // The current line of the text file being read
  let curr = 0

  // Get number and type of materials
  const numMaterials = parseInt(data[curr++])
  const materialList = data.slice(curr, numMaterials + curr).map(mat => mat.trim())
  curr += numMaterials
  curr += (data[curr].trim().split(/ +/).length === numMaterials) ? 1 : numMaterials

  // Get number of x, y, and z voxels
  const [numVoxX, numVoxY, numVoxZ] = data[curr++]
    .trim()
    .split(/ +/)
    .map((v) => {
      return parseInt(v)
    })

  // Get x, y, and z arrays
  const [xArr, yArr, zArr] = data.slice(curr, curr + 3).map((subArr) => { // eslint-disable-line no-global-assign
    return subArr
      .trim()
      .split(/ +/)
      .map((v) => {
        return parseFloat(v)
      })
  })

  curr += 3

  // Read the material data
  const material = data
    .slice(
      curr,
      parseInt(curr) + parseInt(numVoxY) * parseInt(numVoxZ) + parseInt(numVoxZ)
    )
    .map((subArr) => subArr.trim())
    .filter((subArr) => subArr.length > 0)

  curr += numVoxY * numVoxZ + numVoxZ + 1

  // Read the density data
  const lines = data
    .slice(
      curr,
      parseInt(curr) + parseInt(numVoxY) * parseInt(numVoxZ) + parseInt(numVoxZ)
    )
    .map((subArr) => subArr.trim())
    .filter((subArr) => subArr.length > 0)

  const densityGrid = lines.map((subArr) => {
    return subArr
      .trim()
      .split(/ +/)
      .map((v) => {
        return parseFloat(v)
      })
  })

  const minDensity = getMin(densityGrid)
  const maxDensity = getMax(densityGrid)

  // TODO: .flat() does not work in Safari, find an alternative
  const density = densityGrid.flat().slice(0, numVoxX * numVoxY * numVoxZ)

  return {
    voxelNumber: {
      x: numVoxX, // The number of x voxels
      y: numVoxY, // The number of y voxels
      z: numVoxZ // The number of z voxels
    },
    voxelArr: {
      x: xArr, // The dimensions of x voxels
      y: yArr, // The dimensions of x voxels
      z: zArr // The dimensions of x voxels
    },
    voxelSize: {
      x: xArr[1] - xArr[0],
      y: yArr[1] - yArr[0],
      z: zArr[1] - zArr[0]
    },
    density: density, // The flattened density matrix
    materialList: materialList, // The materials in the phantom
    material: material, // The flattened material matrix
    minDensity: minDensity, // The minimum density value
    maxDensity: maxDensity // The maximum density value
  }
}

/**
 * Extract data from .3ddose files.
 *
 * @param {Object} data The .3ddose file read as text.
 * @returns {Object}
 */
var processDoseData = function (data) { // eslint-disable-line no-unused-vars
  // The current line of the text file being read
  let curr = 0

  // Get number of x, y, and z voxels
  const [numVoxX, numVoxY, numVoxZ] = data[curr++]
    .trim()
    .split(/ +/)
    .map((v) => {
      return parseInt(v)
    })

  // Get x, y, and z arrays
  const [xArr, yArr, zArr] = [numVoxX, numVoxY, numVoxZ].map((numVox) => {
    const arr = []
    while (arr.length <= numVox) {
      arr.push(
        ...data[curr++]
          .trim()
          .split(/ +/)
          .map((v) => parseFloat(v))
      )
    }
    return arr
  })

  // Get the dose and error arrays
  let [doseDense, error] = [[], []]
  const prevCurr = curr

  try {
    // This method works if there are line breaks throughout the data
    [doseDense, error].forEach((arr) => {
      while (arr.length < numVoxX * numVoxY * numVoxZ) {
        arr.push(
          ...data[curr++]
            .trim()
            .split(/ +/)
            .map((v) => parseFloat(v))
        )
      }
    })
  } catch (e) {
    if (e instanceof RangeError) {
      // If range error, the length of each line is too long for the spread syntax, now assuming all data is in one line
      [doseDense, error] = data.slice(prevCurr, prevCurr + 2).map((arr) => {
        return arr
          .trim()
          .split(/ +/)
          .slice(0, numVoxX * numVoxY * numVoxZ)
          .map((v) => parseFloat(v))
      })
    } else {
      throw e
    }
  }

  // Convert dose matrix to be sparse
  let maxDose = 0
  const dose = new Array(numVoxX * numVoxY * numVoxZ)

  // Populate sparse dose array
  doseDense.forEach((elem, i) => {
    if (elem !== 0) {
      dose[i] = elem
      if (dose[i] > maxDose) {
        maxDose = dose[i]
      }
    }
  })

  return {
    voxelNumber: {
      x: numVoxX, // The number of x voxels
      y: numVoxY, // The number of y voxels
      z: numVoxZ // The number of z voxels
    },
    voxelArr: {
      x: xArr, // The dimensions of x voxels
      y: yArr, // The dimensions of x voxels
      z: zArr // The dimensions of x voxels
    },
    voxelSize: {
      x: xArr[1] - xArr[0],
      y: yArr[1] - yArr[0],
      z: zArr[1] - zArr[0]
    },
    dose: dose, // The flattened dose matrix
    error: error, // The flattened error matrix
    maxDose: maxDose, // The maximum dose value
    units: 'RELATIVE' // The dose units
  }
}

// export { processDoseData, processPhantomData }
