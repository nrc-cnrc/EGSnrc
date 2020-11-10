/* global dicomParser */

const elementProperties = {
  // General Study
  x00081030: { tag: '(0008,1030)', type: '3', keyword: 'StudyDescription', vm: 1, vr: 'LO' }, // Institution-generated description or classification of the study
  // General Series
  x00085100: { tag: '(0018,5100)', type: '2C', keyword: 'PatientPosition', vm: 1, vr: 'CS' }, // Usually HFS, if another value might need to flip
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
  // CT Image
  x00281052: { tag: '(0028,1052)', type: '1', keyword: 'RescaleIntercept', vm: 1, vr: 'DS' }, // The value b in relationship between stored values (SV) and the output units
  x00281053: { tag: '(0028,1053)', type: '1', keyword: 'RescaleSlope', vm: 1, vr: 'DS' } // The value m in the equation specified in Rescale Intercept
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
    val = new Uint16Array(dataSet.byteArray.buffer, dataElement.dataOffset, dataElement.length / 2)

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

function processDICOMData (arrayBuffer) {
  var byteArray = new Uint8Array(arrayBuffer)
  var kb = byteArray.length / 1024
  var mb = kb / 1024
  var byteStr = mb > 1 ? mb.toFixed(3) + ' MB' : kb.toFixed(0) + ' KB'
  console.log('Status: Parsing ' + byteStr + ' bytes, please wait..')

  try {
    var dataSet = dicomParser.parseDicom(byteArray, { untilTag: 'x7fe00010' })
    var propertyValues = {}

    // Iterate through all element properties
    for (const propertyAddress in elementProperties) {
      var element = dataSet.elements[propertyAddress]
      var property = elementProperties[propertyAddress]

      if (element !== undefined) {
        var val = getVal(dataSet, property.vr, propertyAddress)
        if (val !== undefined) propertyValues[property.keyword] = val
      } else {
        console.log(property.keyword + ' is undefined')
      }
    }

    // Map the values gathered from the DICOM file to the slice info
    const nRows = parseInt(propertyValues.Rows)
    const nCols = parseInt(propertyValues.Columns)

    const [Sx, Sy, Sz] = propertyValues.ImagePositionPatient.split('\\').map((v) => {
      return Number(v)
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

    var xArr = [...Array(nCols)].map((e, i) => (XY[0] * xVoxSize * i + Sx) / 10.0)
    var yArr = [...Array(nRows)].map((e, j) => (XY[4] * yVoxSize * j + Sy) / 10.0)

    // Get the maximum density
    var maxDensity = -100
    var m = parseFloat(propertyValues.RescaleSlope)
    var b = parseFloat(propertyValues.RescaleIntercept)

    const pixelDataScaled = new Float32Array(propertyValues.PixelData.length)

    for (var i = 0; i < propertyValues.PixelData.length; i++) {
      val = m * propertyValues.PixelData[i] + b
      pixelDataScaled[i] = val

      if (val > maxDensity) {
        maxDensity = val
      }
    }

    var DICOMdata = {
      voxelNumber: {
        x: nCols, // The number of x voxels
        y: nRows, // The number of y voxels
        z: parseInt(propertyValues.NumberOfFrames) || 1 // The number of z voxels
      },
      voxelArr: {
        x: xArr, // The dimensions of x voxels
        y: yArr, // The dimensions of y voxels
        z: [Sz / 10.0] // The dimensions of z voxels
      },
      voxelSize: {
        x: xVoxSize, // The voxel size in the x direction
        y: yVoxSize, // The voxel size in the y direction
        z: parseFloat(propertyValues.SliceThickness) / 10.0 || 0 // The voxel size in the z direction
      },
      density: pixelDataScaled, // The flattened density matrix
      // materialList: materialList, // The materials in the phantom
      // material: material, // The flattened material matrix
      maxDensity: maxDensity // The maximum density value
    }

    return { data: DICOMdata, type: 'density' }
  } catch (ex) {
    console.log('Error parsing byte stream', ex)
    return true
  }
}

export { processDICOMData }
