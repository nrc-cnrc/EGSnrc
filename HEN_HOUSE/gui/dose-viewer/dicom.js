/* global dicomParser */

function combineDICOMDoseData(DICOMList) {
  // Use the first item in the list to fill in extra information
  const sampleData = DICOMList[0].data
  const numVox = sampleData.voxelNumber

  let zVoxNum, zVoxSize, zArr
  if (numVox.z !== undefined) {
    // If all frame data is given in the DICOM file, use data from sample
    zVoxNum = numVox.z
    zVoxSize = sampleData.voxelSize.z
    zArr = sampleData.voxelArr.z
  } else {
    // Sort in slice order
    DICOMList.sort((a, b) => (a.data.zPos - b.data.zPos))

    // Map out the positions of the z voxel centres
    const zArrVoxelCenter = DICOMList.map((e) => (e.data.zPos))
    zVoxNum = DICOMList.length
    zVoxSize = zArrVoxelCenter[1] - zArrVoxelCenter[0]
    zArr = [zArrVoxelCenter[0] - zVoxSize * 0.5]

    // Get the voxel boundary positions
    zArrVoxelCenter.forEach((e, i) => { zArr.push(e + zVoxSize * 0.5) })
  }

  // Add the dose matricies together
  const doseArrays = DICOMList.map((e) => Array.from((e.data.dose)))
  const doseDense = doseArrays.flat()

  // Find the max dose and create the dose matrix
  let maxDose = 0
  const dose = new Array(numVox.x * numVox.y * zVoxNum)

  // Populate sparse dose array
  doseDense.forEach((elem, i) => {
    if (elem !== 0) {
      dose[i] = elem
      if (dose[i] > maxDose) {
        maxDose = dose[i]
      }
    }
  })

  var DICOMData = {
    voxelNumber: {
      x: numVox.x, // The number of x voxels
      y: numVox.y, // The number of y voxels
      z: zVoxNum // The number of z voxels
    },
    voxelArr: {
      x: sampleData.voxelArr.x, // The dimensions of x voxels
      y: sampleData.voxelArr.y, // The dimensions of y voxels
      z: zArr // The dimensions of z voxels
    },
    voxelSize: {
      x: sampleData.voxelSize.x, // The voxel size in the x direction
      y: sampleData.voxelSize.y, // The voxel size in the y direction
      z: zVoxSize // The voxel size in the z direction
    },
    dose: dose, // The flattened dose matrix
    // error: error, // The flattened error matrix
    maxDose: maxDose // The maximum dose value
  }

  return DICOMData
}

function combineDICOMDensityData(DICOMList) {
  // Sort in slice order
  DICOMList.sort((a, b) => (a.data.zPos - b.data.zPos))

  // Map out the positions of the z voxel centres
  const zArrVoxelCenter = DICOMList.map((e) => (e.data.zPos))
  const zVoxSize = zArrVoxelCenter[1] - zArrVoxelCenter[0]
  const zArr = [zArrVoxelCenter[0] - zVoxSize * 0.5]

  // Get the voxel boundary positions
  zArrVoxelCenter.forEach((e, i) => { zArr.push(e + zVoxSize * 0.5) })

  // Add the density matricies together
  const densityArrays = DICOMList.map((e) => Array.from((e.data.density)))
  const density = densityArrays.flat()
  // TODO: Remove the use of flat to be compatible with Safari
  // var density = densityArrays.reduce(function (a, b) {
  //   return a.concat(b)
  // })

  const data = DICOMList[0].data

  const DICOMData = {
    voxelNumber: {
      x: data.voxelNumber.x, // The number of x voxels
      y: data.voxelNumber.y, // The number of y voxels
      z: DICOMList.length // The number of z voxels
    },
    voxelArr: {
      x: data.voxelArr.x, // The dimensions of x voxels
      y: data.voxelArr.y, // The dimensions of y voxels
      z: zArr // The dimensions of z voxels
    },
    voxelSize: {
      x: data.voxelSize.x, // The voxel size in the x direction
      y: data.voxelSize.y, // The voxel size in the y direction
      z: zVoxSize // The voxel size in the z direction
    },
    density: density, // The flattened density matrix
    // materialList: materialList, // The materials in the phantom
    // material: material, // The flattened material matrix
    maxDensity: 3071, // The maximum density value 3071 HU
    minDensity: -1024 // The minimum density value -1024 HU
  }

  return DICOMData
}

// TODO: Change object name to keyword
const elementProperties = {
  // // Modality
  // x00080060: { tag: '(0008,0060)', type: '1', keyword: 'Modality', vm: 1, vr: 'CS' }, // Institution-generated description or classification of the study
  // General Study
  x00081030: { tag: '(0008,1030)', type: '3', keyword: 'StudyDescription', vm: 1, vr: 'LO' }, // Institution-generated description or classification of the study
  // General Series
  x00085100: { tag: '(0018,5100)', type: '2C', keyword: 'PatientPosition', vm: 1, vr: 'CS' }, // Usually HFS, if another value might need to flip
  // General Image
  x00200013: { tag: '(0020,0013)', type: '2', keyword: 'InstanceNumber', vm: 1, vr: 'IS' }, // Gives the order of images, if empty, use ImagePositionPatient
  // Image Plane
  x00180050: { tag: '(0018,0050)', type: '2', keyword: 'SliceThickness', vm: 1, vr: 'DS' }, // Optional, the slice thickness in mm
  x00200032: { tag: '(0020,0032)', type: '1', keyword: 'ImagePositionPatient', vm: 3, vr: 'DS' }, // The position of the first voxel transmitted
  x00200037: { tag: '(0020,0037)', type: '1', keyword: 'ImageOrientationPatient', vm: 6, vr: 'DS' }, // The direction cosines of the first row and the first column with respect to the patient
  x00280030: { tag: '(0028,0030)', type: '1', keyword: 'PixelSpacing', vm: 2, vr: 'DS' }, //  The row and column spacing
  // Image Pixel
  x00280002: { tag: '(0028,0002)', type: '1', keyword: 'SamplesPerPixel', vm: 1, vr: 'US' }, // Number of samples (planes) in this image
  x00280004: { tag: '(0028,0004)', type: '1', keyword: 'PhotometricInterpretation', vm: 1, vr: 'CS' }, // Intended interpretation of the image pixel data
  x00280010: { tag: '(0028,0010)', type: '1', keyword: 'Rows', vm: 1, vr: 'US' }, // Number of rows in the image
  x00280011: { tag: '(0028,0011)', type: '1', keyword: 'Columns', vm: 1, vr: 'US' }, // Number of columns in the image
  x00280100: { tag: '(0028,0100)', type: '1', keyword: 'BitsAllocated', vm: 1, vr: 'US' }, // Number of bits allocated for each pixel sample
  x00280101: { tag: '(0028,0101)', type: '1', keyword: 'BitsStored', vm: 1, vr: 'US' }, // Number of bits stored for each pixel sample
  x00280103: { tag: '(0028,0103)', type: '1', keyword: 'PixelRepresentation', vm: 1, vr: 'US' }, // Data representation of the pixel samples
  x00287fe0: { tag: '(0028,7FE0)', type: '1C', keyword: 'PixelDataProviderURL', vm: 1, vr: 'UR' }, // A URL of a provider service that supplies the pixel data of the Image
  x7fe00010: { tag: '(7FE0,0010)', type: '1C', keyword: 'PixelData', vm: 1, vr: 'OB' }, // Pixel data for this image
  // Multi-Frame
  x00280008: { tag: '(0028,0008)', type: '1', keyword: 'NumberOfFrames', vm: 1, vr: 'IS' }, // Number of frames for this image
  x00280009: { tag: '(0028,0009)', type: '1', keyword: 'FrameIncrementPointer', vm: '1-n', vr: 'AT' }, // Determines the sequential order of the frames
  // RT-Dose
  x30040002: { tag: '(3004,0002)', type: '1', keyword: 'DoseUnits', vm: 1, vr: 'CS' }, // Either GY or RELATIVE
  x30040004: { tag: '(3004,0004)', type: '1', keyword: 'DoseType', vm: 1, vr: 'CS' }, // Either PHYSICAL,  EFFECTIVE, or ERROR
  x3004000c: { tag: '(3004,000C)', type: '1C', keyword: 'GridFrameOffsetVector', vm: '2-n', vr: 'DS' }, // Contains the dose image plane offsets in mm
  // CT Image
  x00281052: { tag: '(0028,1052)', type: '1', keyword: 'RescaleIntercept', vm: 1, vr: 'DS' }, // The value b in relationship between stored values (SV) and the output units
  x00281053: { tag: '(0028,1053)', type: '1', keyword: 'RescaleSlope', vm: 1, vr: 'DS' } // The value m in the equation specified in Rescale Intercept
}

const dicomTypeDict = {
  '1.2.840.10008.5.1.4.1.1.481.2': 'RT Dose Storage',
  '1.2.840.10008.5.1.4.1.1.2': 'CT Image Storage'
}

var isStringVr = (vr) => !(vr === 'AT' ||
  vr === 'OB' ||
  vr === 'OW' ||
  vr === 'US'
)

var getVal = function (dataSet, vr, propertyAddress) {
  var val
  var text = ''

  // If the value representation is a string
  if (isStringVr(vr)) {
    val = dataSet.string(propertyAddress)

    // If the value representation is unsigned short
  } else if (vr === 'US') {
    text += dataSet.uint16(propertyAddress)
    for (var i = 1; i < dataSet.elements[propertyAddress].length / 2; i++) {
      text += '\\' + dataSet.uint16(propertyAddress, i)
    }
    val = text

    // If the value representation is other byte string or other word string
  } else if (vr === 'OB' || vr === 'OW') {
    var dataElement = dataSet.elements[propertyAddress]
    const bitsAllocated = dataSet.uint16('x00280100')

    // If the offset is not divisible the the byte number, slice the buffer
    if (bitsAllocated === 16) {
      const arrayLength = dataElement.length / Uint16Array.BYTES_PER_ELEMENT
      if ((dataElement.dataOffset % Uint16Array.BYTES_PER_ELEMENT) === 0) {
        val = new Uint16Array(dataSet.byteArray.buffer, dataElement.dataOffset, arrayLength)
      } else {
        val = new Uint16Array(dataSet.byteArray.buffer.slice(dataElement.dataOffset), 0, arrayLength)
      }
    } else if (bitsAllocated === 32) {
      const arrayLength = dataElement.length / Uint32Array.BYTES_PER_ELEMENT
      if ((dataElement.dataOffset % Uint32Array.BYTES_PER_ELEMENT) === 0) {
        val = new Uint32Array(dataSet.byteArray.buffer, dataElement.dataOffset, arrayLength)
      } else {
        val = new Uint32Array(dataSet.byteArray.buffer.slice(dataElement.dataOffset), 0, arrayLength)
      }
    } else {
      console.log('Unknown bits allocated')
    }

    // If the value representation is an attribute tag
  } else if (vr === 'AT') {
    var group = dataSet.uint16(propertyAddress, 0)
    var groupHexStr = ('0000' + group.toString(16)).substr(-4)
    var xelement = dataSet.uint16(propertyAddress, 1)
    var elementHexStr = ('0000' + xelement.toString(16)).substr(-4)
    val = 'x' + groupHexStr + elementHexStr
  }
  return val
}

function processDICOMSlice(arrayBuffer) {
  const byteArray = new Uint8Array(arrayBuffer)

  try {
    const dataSet = dicomParser.parseDicom(byteArray, { untilTag: 'x7fe00010' })
    const dicomType = dicomTypeDict[dataSet.string('x00020002')]
    const propertyValues = {}

    // Iterate through all element properties
    for (const propertyAddress in elementProperties) {
      var element = dataSet.elements[propertyAddress]
      var property = elementProperties[propertyAddress]

      if (element !== undefined) {
        var val = getVal(dataSet, property.vr, propertyAddress)
        if (val !== undefined) propertyValues[property.keyword] = val
      }
    }

    // Map the values gathered from the DICOM file to the slice info
    const nRows = parseInt(propertyValues.Rows)
    const nCols = parseInt(propertyValues.Columns)

    const [Sx, Sy, Sz] = propertyValues.ImagePositionPatient.split('\\').map((v) => {
      return Number(v) / 10.0
    })
    const XY = propertyValues.ImageOrientationPatient.split('\\').map((v) => {
      return Number(v)
    })
    const [xVoxSize, yVoxSize] = propertyValues.PixelSpacing.split('\\').map((v) => {
      return Number(v) / 10.0
    })

    // var Px = (i, j) => Xx * xVoxSize * i + Yx * yVoxSize * j + Sx
    // var Py = (i, j) => Xy * xVoxSize * i + Yy * yVoxSize * j + Sy
    // var Pz = (i, j) => Xz * xVoxSize * i + Yz * yVoxSize * j + Sz

    var xArr = [...Array(nCols + 1)].map((e, i) => (XY[0] * xVoxSize * (i - 0.5) + Sx))
    var yArr = [...Array(nRows + 1)].map((e, j) => (XY[4] * yVoxSize * (j - 0.5) + Sy))

    var DICOMslice = {
      type: dicomType,
      sliceNum: parseInt(propertyValues.InstanceNumber),
      voxelNumber: {
        x: nCols, // The number of x voxels
        y: nRows // The number of y voxels
      },
      voxelArr: {
        x: xArr, // The dimensions of x voxels (length === voxelNumber.x + 1)
        y: yArr // The dimensions of y voxels
      },
      voxelSize: {
        x: xVoxSize, // The voxel size in the x direction
        y: yVoxSize // The voxel size in the y direction
      },
      zPos: Sz
    }

    // If there are multiple frames
    if (propertyValues.FrameIncrementPointer !== undefined) {
      // Position relative to Image Position (patient)
      const gridFrames = dataSet.string(propertyValues.FrameIncrementPointer).split('\\').map((v) => {
        return Number(v) / 10.0
      })
      const nSlices = parseInt(propertyValues.NumberOfFrames)
      const zVoxSize = Math.abs(gridFrames[1] - gridFrames[0])
      const zArr = gridFrames.map((frameOffset) => (Sz + frameOffset + zVoxSize * 0.5))
      zArr.unshift(Sz - zVoxSize * 0.5)

      DICOMslice.voxelNumber.z = nSlices
      DICOMslice.voxelArr.z = zArr
      DICOMslice.voxelSize.z = zVoxSize
    }

    if (dicomType === 'RT Dose Storage') {
      DICOMslice.dose = propertyValues.PixelData
      DICOMslice.units = propertyValues.DoseUnits
    } else if (dicomType === 'CT Image Storage') {
      // TODO: materialList and material matrix
      // Rescale the density values
      const m = parseFloat(propertyValues.RescaleSlope)
      const b = parseFloat(propertyValues.RescaleIntercept)
      const pixelDataScaled = new Float32Array(propertyValues.PixelData.length)

      for (var i = 0; i < propertyValues.PixelData.length; i++) {
        val = m * propertyValues.PixelData[i] + b
        pixelDataScaled[i] = val

        DICOMslice.density = pixelDataScaled
      }
    }
    return DICOMslice
  } catch (ex) {
    console.log('Error parsing byte stream', ex)
    return true
  }
}

// export { combineDICOMDensityData, combineDICOMDoseData, processDICOMSlice }
