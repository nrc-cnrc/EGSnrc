## What is EGSnrc?

EGSnrc is a software toolkit to perform Monte Carlo simulation of
ionizing radiation transport through matter. It models the propagation
of photons, electrons and positrons with kinetic energies between
1&nbsp;keV and 10&nbsp;GeV, in homogeneous materials. EGSnrc is an
extended and improved version of the Electron Gamma Shower (EGS)
software package originally developed at the Stanford Linear Accelerator
Center (SLAC) in the 1970s. Most notably, it incorporates significant
refinements in charged particle transport, better low energy cross
sections, and the egs++ class library to model elaborate geometries and
particle sources.


## Documentation

The EGSnrc user manuals are available on the EGSnrc web page:
http://nrc-cnrc.github.io/EGSnrc/.


## Licence

EGSnrc is distributed as free software under the terms of the GNU Affero
General Public Licence. Please review the [LICENSE](LICENCE.md) document
before downloading the software. In practice, this licence imposes no
restriction on using EGSnrc. However, if you want to further convey
verbatim or modified versions of the code, or any work based on any
EGSnrc component (including any such work operated remotely over a
network), you must do so under the same licence terms. [Contact NRC]
(http://www.nrc-cnrc.gc.ca/eng/solutions/advisory/egsnrc_index.html)
if you wish to licence EGSnrc under different terms.


## Prerequisites

EGSnrc can be installed on computers running Linux, OS X or Windows
operating systems. As a general-purpose Monte Carlo toolkit, EGSnrc
provides source code and utilities to build and run your own
radiation transport simulation applications. To use EGSnrc
on any platform you need:

- a Fortran compiler
- a C compiler
- a C++ compiler
- the GNU `make` utility
- the Tcl/Tk interpreter and widget toolkit
- the Grace plotting tool

Please read the [installation instructions]
(https://github.com/nrc-cnrc/EGSnrc/wiki/Installation-overview) in the
wiki for more details about these software components.


## Installation

Installation involves downloading the EGSnrc source code, and then
configuring EGSnrc for your computer. We recommend using the `git`
revision control software to obtain the source code. The following shell
command will clone the EGSnrc repository to your current working
directory:
```bash
git clone https://github.com/nrc-cnrc/EGSnrc.git
```
Alternatively, you can download a [zip archive]
(https://github.com/nrc-cnrc/EGSnrc/archive/master.zip) of the EGSnrc
directory. Once you have downloaded the source code, you need to run a
configuration GUI or script to set up and compile EGSnrc software
components for your computer. Please read the full [installation
instructions]
(https://github.com/nrc-cnrc/EGSnrc/wiki/Installation-overview) for more
details on how to download and configure EGSnrc.


## Issues

Use the [issue tracker](https://github.com/nrc-cnrc/EGSnrc/issues) to
report bugs, inaccuracies or even small typos in the EGSnrc project
files. The tracker lets you browse and search all documented issues,
comment on open issues, and track their progress. Note that the issue
tracker is **not meant for technical support;** open an issue only if it
pertains to an error condition which is precise and reproducible.


## Contributing

You can contribute to the EGSnrc project by implementing new features,
creating new data sets, correcting errors, or improving documentation.
Feel free to submit small corrections and contributions as issues in the
[issue tracker](https://github.com/nrc-cnrc/EGSnrc/issues). For more
extensive contributions, familiarize yourself with git and github,
work on your own EGSnrc project fork and submit your changes via a pull
request. Note that significant contributions will require a transfer of
copyright to the National Research Council of Canada before they can be
merged into the EGSnrc distribution.
