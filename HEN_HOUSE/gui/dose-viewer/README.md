# Dose Viewer

Dose viewer is a (WIP) web-based visualization tool for EGSnrc phantom and dose
files. To use, either upload .3ddose and .egsphant files or use the test files
available. If two or more dose files are uploaded, they can be compared in the
viewer. Dose contours can be added with the input box in the dose legend.
Contours can be toggled on and off by clicking the corresponding colour block in
the legend. Voxel information can be viewed by selecting the checkbox to show
voxel data and selecting a point in any of the three plots. Dose profiles can be
viewed by selecting the checkbox to plot dose profiles and choosing a point on
the plot to investigate.

## Usage

To run this webpage locally:

1.  Clone the repo
2.  Using the command line, `cd` into the dose-viewer folder
3.  If you have Python 3, start a local server using the command

         python -m http.server

    If you have Python 2, use the command

         python -m SimpleHTTPServer

4.  If the command is successful, it should give you a link (e.g.
    http://0.0.0.0:8000/) which you can paste into your browser to view the web page

## Acknowledgements

This project is a collaboration between Elise Badun and Magdalena
Bazalova-Carter of the University of Victoria, and Frederic Tessier, Reid
Townson, and Ernesto Mainegra-Hing of the NRC.
