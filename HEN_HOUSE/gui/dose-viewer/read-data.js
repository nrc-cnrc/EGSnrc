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
  });

  // File is successfully read
  reader.addEventListener("load", function (event) {
    console.log("Successfully read file");
    let result = event.target.result;
    let resultSplit = result.split("\n");
    let data;
    if (ext === "egsphant") {
      data = processPhantomData(resultSplit);
      const densityVol = new DensityVolume(10, 10, data);
      // TODO: Figure out a better layout for event listeners
      let slice = densityVol.getSlice("xy", 10);
      let context = densityVol.getSliceImageContext(slice);
      context.canvas;
    } else if (ext === "3ddose") {
      data = processDoseData(resultSplit);
    } else {
      console.log("Unknown file extension");
      return;
    }
    console.log("Finished processing data");
  });

  // File reading failed
  reader.addEventListener("error", function () {
    alert("Error: Failed to read file");
  });

  // Read as text file
  reader.readAsText(file);
});

// Process .egsphant files
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

  // [xArr, yArr, zArr] = [xArr, yArr, zArr].map((a) => {
  //   for (let i = 0; i < a.length; i++) {
  //     a[i] = a[i] + (a[i + 1] - a[i]) / 2;
  //   }
  //   return a;
  // });

  // xArr.pop();
  // yArr.pop();
  // zArr.pop();

  curr += numVoxY * numVoxZ + numVoxZ + 1;

  // Read the density data
  let lines = data.slice(
    curr,
    parseInt(curr) + parseInt(numVoxY) * parseInt(numVoxZ) + parseInt(numVoxZ)
  );

  let density = lines
    .map((subArr) => {
      return subArr
        .trim()
        .split(/\ +/)
        .map((v) => {
          return parseFloat(v);
        });
    })
    .flat()
    .slice(0, numVoxX * numVoxY * numVoxZ);

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
  [x, y, z] = data.slice(curr, curr + 3).map((subArr) => {
    return subArr
      .trim()
      .split(/\ +/)
      .map((v) => {
        return parseFloat(v);
      });
  });

  curr += 3;

  [x, y, z] = [x, y, z].map((a) => {
    for (let i = 0; i < a.length; i++) {
      a[i] = a[i] + (a[i + 1] - a[i]) / 2;
    }
    return a;
  });

  x.pop();
  y.pop();
  z.pop();

  // Read the dose data
  let lines = data.slice(
    curr,
    parseInt(curr) + parseInt(numVoxY) * parseInt(numVoxZ) + parseInt(numVoxZ)
  );

  let dose = lines
    .map((subArr) => {
      return subArr
        .trim()
        .split(/\ +/)
        .map((v) => {
          return parseFloat(v);
        });
    })
    .flat()
    .slice(0, numVoxX * numVoxY * numVoxZ);

  // TODO: Add error matrix
  return {
    numVoxX: numVoxX, // The number of x voxels
    numVoxY: numVoxY, // The number of y voxels
    numVoxZ: numVoxZ, // The number of z voxels
    x: x, // The dimensions of x voxels
    y: y, // The dimensions of y voxels
    z: z, // The dimensions of z voxels
    dose: dose, // The flattened dose matrix
    // error: error, // The flattened dose error matrix
  };
};