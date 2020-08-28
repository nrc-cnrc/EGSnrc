let dropArea = d3.select("#drop-area");
let progressBar = d3.select("#progress-bar");
let progressBarNode = progressBar.node();
let totalFiles = 0;

function initializeProgress(numfiles) {
  progressBarNode.value = 0;
  // Show the progress bar
  progressBar.classed("hidden", false);
  totalFiles = numfiles;
}

function updateProgress(percent, fileNum) {
  progressBarNode.value = percent * (fileNum / totalFiles);
}

function endProgress() {
  // Hide the progress bar
  progressBar.classed("hidden", true);
}

// Turn off normal drop response on window
["dragover", "drop"].forEach((eventName) => {
  window.addEventListener(eventName, function (e) {
    e.preventDefault();
    e.stopPropagation();
  });
});

// Add highlight class to drop area when holding file overtop
dropArea.on("dragenter dragover", () => dropArea.classed("highlight", true));
dropArea.on("dragleave drop", () => dropArea.classed("highlight", false));

// Add event listener on drop to process files
dropArea.node().addEventListener("drop", function (e) {
  if (e.dataTransfer && e.dataTransfer.files.length) {
    e.preventDefault();
    e.stopPropagation();
    let files = [...e.dataTransfer.files];
    handleFiles(files);
  }
});

// Add event listener on button press to process files
d3.select("#file-input").on("change", function () {
  if (this.files.length) {
    let files = [...this.files];
    handleFiles(files);
  }
});

function handleFiles(files) {
  initializeProgress(files.length);
  files.forEach((file, fileNum) => readFile(file, fileNum + 1));
}

// Read file
function readFile(file, fileNum) {
  let reader = new FileReader();
  let fileName = file.name;
  let ext = fileName.split(".").pop();

  reader.addEventListener("loadstart", function () {
    console.log("File reading started");
    return true;
  });

  // Update progress bar
  reader.addEventListener("progress", function (e) {
    if (e.lengthComputable == true) {
      updateProgress(Math.floor((e.loaded / e.total) * 100), fileNum);
    }
  });

  reader.addEventListener("error", function () {
    alert("Error: Failed to read file");
    return true;
  });

  // TODO: Add check for dose and density distributions like https://github.com/nrc-cnrc/EGSnrc/blob/master/HEN_HOUSE/omega/progs/dosxyz_show/dosxyz_show.c#L1407-L1412
  // File is successfully read
  reader.addEventListener("load", function (event) {
    let result = event.target.result;
    let resultSplit = result.split("\n");
    let data;

    if (ext === "egsphant") {
      data = processPhantomData(resultSplit);
      let densityVol = new DensityVolume(
        fileName,
        mainViewerDimensions,
        legendDimensions,
        data
      );

      densityVolumeList.push(densityVol);
      volumeViewerList.forEach((volumeViewer) =>
        volumeViewer.updateDensityFileSelector(
          densityVol,
          densityVolumeList.length - 1
        )
      );
    } else if (ext === "3ddose") {
      data = processDoseData(resultSplit);
      let doseVol = new DoseVolume(
        fileName,
        mainViewerDimensions,
        legendDimensions,
        data
      );

      doseVolumeList.push(doseVol);
      volumeViewerList.forEach((volumeViewer) =>
        volumeViewer.updateDoseFileSelector(doseVol, doseVolumeList.length - 1)
      );
    } else {
      console.log("Unknown file extension");
      return true;
    }

    // If this is the first volume uploaded, load into first volume viewer
    if (volumeViewerList.length === 0) {
      const volViewer = new VolumeViewer(
        mainViewerDimensions,
        legendDimensions,
        sideDoseProfileDimensions,
        "vol-" + volumeViewerList.length
      );
      volumeViewerList.push(volViewer);

      if (densityVolumeList.length === 1 && doseVolumeList.length === 0) {
        volViewer.setDensityVolume(densityVolumeList[0]);
        volViewer.densitySelector.node().selectedIndex = 1;
      } else {
        volViewer.setDoseVolume(doseVolumeList[0]);
        volViewer.doseSelector.node().selectedIndex = 1;
      }
    }

    console.log("Finished processing data");
    return true;
  });

  reader.addEventListener("loadend", function () {
    // If all files have been loaded in
    if (fileNum === totalFiles) {
      // Set the bar progress to full
      progressBarNode.value = progressBarNode.max;
      // End the progress after 500 milliseconds
      window.setTimeout(endProgress, 500);
    }
  });

  // Read as text file
  reader.readAsText(file);
}
