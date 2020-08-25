// Process .egsphant files
// TODO: Test with other .egsphant files
var processPhantomData = function (data) {
  var getMax = function (a) {
    return Math.max(...a.map((e) => (Array.isArray(e) ? getMax(e) : e)));
  };

  let curr = 0;
  let numMaterials = parseInt(data[curr++]);

  let materialList = data.slice(curr, numMaterials + curr);
  curr += numMaterials * 2;

  // Get number of x, y, and z voxels
  let [numVoxX, numVoxY, numVoxZ] = data[curr++]
    .trim()
    .split(/\ +/)
    .map((v) => {
      return parseInt(v);
    });

  // Get x, y, and z arrays
  [xArr, yArr, zArr] = data.slice(curr, curr + 3).map((subArr) => {
    return subArr
      .trim()
      .split(/\ +/)
      .map((v) => {
        return parseFloat(v);
      });
  });

  curr += 3;

  // Read the material data
  let material = data
    .slice(
      curr,
      parseInt(curr) + parseInt(numVoxY) * parseInt(numVoxZ) + parseInt(numVoxZ)
    )
    .filter((subArr) => subArr.length > 0);

  curr += numVoxY * numVoxZ + numVoxZ + 1;

  // Read the density data
  let lines = data
    .slice(
      curr,
      parseInt(curr) + parseInt(numVoxY) * parseInt(numVoxZ) + parseInt(numVoxZ)
    )
    .filter((subArr) => subArr.length > 0);

  let densityGrid = lines.map((subArr) => {
    return subArr
      .trim()
      .split(/\ +/)
      .map((v) => {
        return parseFloat(v);
      });
  });

  let maxDensity = getMax(densityGrid);

  // TODO: .flat() does not work in Safari, find an alternative
  let density = densityGrid.flat().slice(0, numVoxX * numVoxY * numVoxZ);

  return {
    voxelNumber: {
      x: numVoxX, // The number of x voxels
      y: numVoxY, // The number of y voxels
      z: numVoxZ, // The number of z voxels
    },
    voxelArr: {
      x: xArr, // The dimensions of x voxels
      y: yArr, // The dimensions of x voxels
      z: zArr, // The dimensions of x voxels
    },
    voxelSize: {
      x: xArr[1] - xArr[0],
      y: yArr[1] - yArr[0],
      z: zArr[1] - zArr[0],
    },
    density: density, // The flattened density matrix
    materialList: materialList, // The materials in the phantom
    material: material, // The flattened material matrix
    maxDensity: maxDensity, // The maximum density value
  };
};

// Process .3ddose files
// TODO: Test with other .3ddose files
var processDoseData = function (data) {
  let curr = 0;

  // Get number of x, y, and z voxels
  let [numVoxX, numVoxY, numVoxZ] = data[curr++]
    .trim()
    .split(/\ +/)
    .map((v) => {
      return parseInt(v);
    });

  // Get x, y, and z arrays
  const [xArr, yArr, zArr] = [numVoxX, numVoxY, numVoxZ].map((numVox) => {
    let arr = [];
    while (arr.length <= numVox) {
      arr.push(
        ...data[curr++]
          .trim()
          .split(/\ +/)
          .map((v) => parseFloat(v))
      );
    }
    return arr;
  });

  // Get the dose and error arrays
  let [doseDense, error] = [[], []];
  let prevCurr = curr;

  try {
    [doseDense, error].forEach((arr) => {
      while (arr.length < numVoxX * numVoxY * numVoxZ) {
        arr.push(
          ...data[curr++]
            .trim()
            .split(/\ +/)
            .map((v) => parseFloat(v))
        );
      }
    });
  } catch (e) {
    if (e instanceof RangeError) {
      // If range error, the length of each line is too long for the spread syntax, now assuming all data is in one line
      [doseDense, error] = data.slice(prevCurr, prevCurr + 2).map((arr) => {
        return arr
          .trim()
          .split(/\ +/)
          .slice(0, numVoxX * numVoxY * numVoxZ)
          .map((v) => parseFloat(v));
      });
    } else {
      throw e;
    }
  }

  // Convert dose matrix to be sparse
  let maxDose = 0;
  let dose = new Array(numVoxX * numVoxY * numVoxZ);

  // Populate sparse dose array
  doseDense.forEach((elem, i) => {
    if (elem !== 0) {
      dose[i] = elem;
      if (dose[i] > maxDose) {
        maxDose = dose[i];
      }
    }
  });

  return {
    voxelNumber: {
      x: numVoxX, // The number of x voxels
      y: numVoxY, // The number of y voxels
      z: numVoxZ, // The number of z voxels
    },
    voxelArr: {
      x: xArr, // The dimensions of x voxels
      y: yArr, // The dimensions of x voxels
      z: zArr, // The dimensions of x voxels
    },
    voxelSize: {
      x: xArr[1] - xArr[0],
      y: yArr[1] - yArr[0],
      z: zArr[1] - zArr[0],
    },
    dose: dose, // The flattened dose matrix
    error: error, // The flattened error matrix
    maxDose: maxDose, // The maximum dose value
  };
};
