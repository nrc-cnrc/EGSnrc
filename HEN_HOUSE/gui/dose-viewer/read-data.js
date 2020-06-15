// TODO: Add loading bar/messages while processing files
// Read file
d3.select("#read-button").on("click", function () {
  let ext;

  // No files are selected when read button is clicked
  if (d3.select("#file-input").node().files.length == 0) {
    console.log("Error: No files selected");
    return;
  }

  let reader = new FileReader();
  let file = d3.select("#file-input").node().files[0];

  ext = file.name.split(".").pop();

  reader.addEventListener("loadstart", function () {
    console.log("File reading started");
    return true;
  });

  // File is successfully read
  // TODO: Allow files to be read in any order
  reader.addEventListener("load", function (event) {
    console.log("Successfully read file");
    let result = event.target.result;
    let resultSplit = result.split("\n");
    let data;
    if (ext === "egsphant") {
      data = processPhantomData(resultSplit);
      densityVol.addData(data);
      densityVol.initializeLegend();
      let slice = densityVol.getSlice(axis, sliceNum);
      let context = densityVol.getSliceImageContext(slice, canvas);
      updateSlider(slice);
    } else if (ext === "3ddose") {
      data = processDoseData(resultSplit);
      doseVol.addData(data);
      doseVol.initializeLegend();
      // TODO: Figure out a better layout for event listeners
      let slice = doseVol.getSlice(axis, sliceNum);
      let context = doseVol.getSliceImageContext(slice, svgPlot);

      updateSlider(slice);
    } else {
      console.log("Unknown file extension");
      return true;
    }
    console.log("Finished processing data");
    return true;
  });

  // File reading failed
  reader.addEventListener("error", function () {
    alert("Error: Failed to read file");
    return true;
  });

  // Read as text file
  reader.readAsText(file);
  return true;
});

// Process .egsphant files
// TODO: Test with other .egsphant files
var processPhantomData = function (data) {
  let curr = 0;
  let numMaterials = parseInt(data[curr++]);

  let materials = data.slice(curr, numMaterials + curr);
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

  // Skip material data
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

  let getMax = function (a) {
    return Math.max(...a.map((e) => (Array.isArray(e) ? getMax(e) : e)));
  };

  let maxDensity = getMax(densityGrid);

  density = densityGrid.flat().slice(0, numVoxX * numVoxY * numVoxZ);

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
    materials: materials, // The materials in the phantom
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
  [xArr, yArr, zArr] = data.slice(curr, curr + 3).map((subArr) => {
    return subArr
      .trim()
      .split(/\ +/)
      .map((v) => {
        return parseFloat(v);
      });
  });

  curr += 3;

  // TODO: If can easily check if value will be zero from string, parse float later after removing zeros
  let [doseDense, error] = data.slice(curr, curr + 2).map((subArr) => {
    return subArr
      .trim()
      .split(/\ +/)
      .slice(0, numVoxX * numVoxY * numVoxZ)
      .map((v) => {
        return parseFloat(v);
      });
  });

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
