/* global dicomParser */

function combineDICOMDoseData (DICOMList) { // eslint-disable-line no-unused-vars
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
    maxDose: maxDose, // The maximum dose value
    studyInstanceUID: sampleData.studyInstanceUID, // The study instance identifier
    units: sampleData.units // The dose units, either GY or RELATIVE
  }

  return DICOMData
}

function combineDICOMDensityData (DICOMList) { // eslint-disable-line no-unused-vars
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
    minDensity: -1024, // The minimum density value -1024 HU
    studyInstanceUID: data.studyInstanceUID // The study instance identifier
  }

  return DICOMData
}

// TODO: Change object name to keyword
const elementProperties = {
  // // Modality
  // x00080060: { tag: '(0008,0060)', type: '1', keyword: 'Modality', vm: 1, vr: 'CS' }, // Institution-generated description or classification of the study
  // General Study
  x00081030: { tag: '(0008,1030)', type: '3', keyword: 'StudyDescription', vm: 1, vr: 'LO' }, // Institution-generated description or classification of the study
  x0020000d: { tag: '(0020,000D)', type: '1', keyword: 'StudyInstanceUID', vm: 1, vr: 'UI' }, // Unique identifier for the study
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
  x00281053: { tag: '(0028,1053)', type: '1', keyword: 'RescaleSlope', vm: 1, vr: 'DS' }, // The value m in the equation specified in Rescale Intercept
  // RT Series
  x0020000e: { tag: '(0020,000E)', type: '1', keyword: 'SeriesInstanceUID', vm: 1, vr: 'UI' }, // Unique identifier of the series
  x00080060: { tag: '(0008,0060)', type: '1', keyword: 'Modality', vm: 1, vr: 'CS' }, // Type of equipment that originally acquired the data
  // Structure Set
  x30060002: { tag: '(3006,0002)', type: '1', keyword: 'StructureSetLabel', vm: 1, vr: 'SH' }, // User-defined label for Structure Set
  x30060020: { tag: '(3006,0020)', type: '1', keyword: 'StructureSetROISequence', vm: 1, vr: 'SQ' }, // ROIs for current Structure Set
  x30060022: { tag: '(3006,0022)', type: '1', keyword: 'ROINumber', vm: 1, vr: 'IS' }, // Identification number of the ROI
  x30060024: { tag: '(3006,0024)', type: '1', keyword: 'ReferencedFrameOfReferenceUID', vm: 1, vr: 'UI' }, // Sequence describing Frames of Reference in which the ROIs are defined
  x30060026: { tag: '(3006,0026)', type: '2', keyword: 'ROIName', vm: 1, vr: 'LO' }, // User-defined name for ROI
  // ROI Contour
  x30060039: { tag: '(3006,0039)', type: '1', keyword: 'ROIContourSequence', vm: 1, vr: 'SQ' }, // Sequence of Contour Sequences defining ROIs
  x3006002a: { tag: '(3006,002A)', type: '3', keyword: 'ROIDisplayColor', vm: 3, vr: 'IS' }, // RGB triplet color representation for ROI, specified using the range 0-255
  x30060040: { tag: '(3006,0040)', type: '3', keyword: 'ContourSequence', vm: 1, vr: 'SQ' }, // Sequence of Contours defining ROI
  x30060042: { tag: '(3006,0042)', type: '1', keyword: 'ContourGeometricType', vm: 1, vr: 'CS' }, // Geometric type of contour
  x30060046: { tag: '(3006,0046)', type: '1', keyword: 'NumberOfContourPoints', vm: 1, vr: 'IS' }, // Number of points (triplets) in Contour Data
  x30060050: { tag: '(3006,0050)', type: '1', keyword: 'ContourData', vm: '3-3n', vr: 'DS' }, // Sequence of (x,y,z) triplets defining a contour
  // RT ROI Observations
  x30060080: { tag: '(3006,0080)', type: '1', keyword: 'RTROIObservationsSequence', vm: 1, vr: 'SQ' }, // Sequence of observations related to ROIs defined in the ROI Module
  x30060082: { tag: '(3006,0082)', type: '1', keyword: 'ObservationNumber', vm: 1, vr: 'IS' }, // Identification number of the Observation
  x30060084: { tag: '(3006,0084)', type: '1', keyword: 'ReferencedROINumber', vm: 1, vr: 'IS' }, // Uniquely identifies the referenced ROI described in the Structure Set ROI Sequence
  x30060085: { tag: '(3006,0085)', type: '3', keyword: 'ROIObservationLabel', vm: 1, vr: 'SH' }, // User-defined label for ROI Observation
  // x30060086: { tag: '(3006,0086)', type: '3', keyword: 'RTROIIdentificationCodeSequence', vm: 1, vr: 'SQ' }, // Sequence containing Code used to identify ROI
  // x300600a4: { tag: '(3006,00A4)', type: '2', keyword: 'RTROIInterpretedType', vm: 1, vr: 'CS' }, // Type of ROI
  xfffee000: { tag: '(FFFE,E000)', type: '1', keyword: 'Item', vm: 1, vr: '' } // An item in a sequence
}

const uids = {
  '1.2.840.10008.5.1.4.1.1.2': 'CT Image Storage',
  '1.2.840.10008.5.1.4.1.1.481.2': 'RT Dose Storage',
  '1.2.840.10008.5.1.4.1.1.481.3': 'RT Structure Set Storage'
}

var isStringVr = (vr) => !(
  vr === 'AT' ||
  vr === 'OB' ||
  vr === 'OW' ||
  vr === 'SQ' ||
  vr === 'US'
)

var getVal = function (dataSet, vr, propertyAddress) {
  var val
  var text = ''

  // If the value representation is a string
  if (isStringVr(vr)) {
    val = dataSet.string(propertyAddress)

    // If the value representation is an attribute tag
  } else if (vr === 'AT') {
    var group = dataSet.uint16(propertyAddress, 0)
    var groupHexStr = ('0000' + group.toString(16)).substr(-4)
    var xelement = dataSet.uint16(propertyAddress, 1)
    var elementHexStr = ('0000' + xelement.toString(16)).substr(-4)
    val = 'x' + groupHexStr + elementHexStr

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

    // If the value representation is unsigned short
  } else if (vr === 'US') {
    text += dataSet.uint16(propertyAddress)
    for (var i = 1; i < dataSet.elements[propertyAddress].length / 2; i++) {
      text += '\\' + dataSet.uint16(propertyAddress, i)
    }
    val = text
  }
  return val
}

function getTag (tag) {
  var group = tag.substring(1, 5).toLowerCase()
  var element = tag.substring(5, 9).toLowerCase()
  var tagIndex = 'x' + group + element
  var attr = elementProperties[tagIndex]
  return attr
}

function dumpDataSet (dataSet, values) {
  for (const elementAddress in dataSet.elements) {
    var element = dataSet.elements[elementAddress]
    var property = elementProperties[elementAddress]
    var tag = getTag(element.tag)
    if (tag === undefined) {
      continue
    }
    if (element.items) {
      values[property.keyword] = new Array(element.items.length)
      element.items.forEach((item, i) => {
        values[property.keyword][i] = {}
        dumpDataSet(item.dataSet, values[property.keyword][i])
      })
    } else {
      var vr = (element.vr !== undefined) ? element.vr : tag.vr
      values[property.keyword] = getVal(dataSet, vr, elementAddress)
    }
  }

  return values
}

function processDICOMSlice (arrayBuffer) { // eslint-disable-line no-unused-vars
  const byteArray = new Uint8Array(arrayBuffer)

  try {
    const dataSet = dicomParser.parseDicom(byteArray) //, { untilTag: 'x7fe00010' })
    const dicomType = uids[dataSet.string('x00020002')]

    const propertyValues = {}
    dumpDataSet(dataSet, propertyValues)

    if (dicomType === 'CT Image Storage' || dicomType === 'RT Dose Storage') {
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
        zPos: Sz,
        studyInstanceUID: propertyValues.StudyInstanceUID
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

        for (let i = 0; i < propertyValues.PixelData.length; i++) {
          pixelDataScaled[i] = m * propertyValues.PixelData[i] + b

          DICOMslice.density = pixelDataScaled
        }
      }
      return DICOMslice
    } else if (dicomType === 'RT Structure Set Storage') {
      const numROIs = propertyValues.ROIContourSequence.length
      const ROIs = new Array(numROIs)
      for (let i = 0; i < numROIs; i++) {
        const contourSequence = propertyValues.ROIContourSequence[i]

        const ROINum = parseInt(contourSequence.ReferencedROINumber)

        const structureSetSequence = propertyValues.StructureSetROISequence.find((item) => {
          if (parseInt(item.ROINumber) === ROINum) return true
        })

        const RTROIObservationsSequence = propertyValues.RTROIObservationsSequence.find((item) => {
          if (parseInt(item.ReferencedROINumber) === ROINum) return true
        })

        ROIs[i] = { type: dicomType, ...contourSequence, ...structureSetSequence, ...RTROIObservationsSequence }
      }
      return { type: dicomType, ROIs: ROIs, studyInstanceUID: propertyValues.StudyInstanceUID }
    }
  } catch (ex) {
    console.log('Error parsing byte stream', ex)
    return true
  }
}

// export { combineDICOMDensityData, combineDICOMDoseData, processDICOMSlice }
