# EGSnrc

[![badge-doi](https://badgen.net/static/doi/10.4224%2F40001303/)](https://doi.org/10.4224/40001303)
[![badge-doi](https://badgen.net/static/licence/AGPL-3.0-or-later/green)](./LICENCE)
[![badge-doi](https://badgen.net/static/cite/bibtex/orange)](https://github.com/nrc-cnrc/EGSnrc/wiki/Citing-EGSnrc#bibtex-entry)

EGSnrc is a software toolkit to perform Monte Carlo simulation of ionizing
radiation transport through matter. It models the propagation of photons,
electrons and positrons with kinetic energies between 1&nbsp;keV and
10&nbsp;GeV, in homogeneous materials. EGSnrc was first released in 2000,
as a complete overhaul of the Electron Gamma Shower (EGS) software package
originally developed at the Stanford Linear Accelerator Center (SLAC) in the
1970s. Most notably, EGSnrc incorporates crucial refinements in charged
particle transport, better low energy cross sections, and the egs++ class
library to model elaborate geometries and particle sources.

## Documentation

- [**Citing EGSnrc**](https://github.com/nrc-cnrc/EGSnrc/wiki/Citing-EGSnrc)
- [**Getting Started**](https://nrc-cnrc.github.io/EGSnrc/doc/getting-started.pdf) with guided tutorials
- [**EGSnrc**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs701-egsnrc.pdf) core manual (PIRS-701)
- [**BEAMnrc**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs509a-beamnrc.pdf) accelerators (PIRS-509a)
- [**DOSXYZnrc**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs794-dosxyznrc.pdf) voxel dose (PIRS-794)
- [**egs++**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs898/) geometries, sources and applications (PIRS-898)
- [**g**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs3100/) application reference manual (PIRS-3100) ([pdf](https://nrc-cnrc.github.io/EGSnrc/doc/pirs3100-g-refman.pdf))
- [**RZ and SPH apps**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs702-egsnrc-codes.pdf) user manual (PIRS-702)
- [**RZ GUI**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs801-egsinprz.pdf) egs_inprz (PIRS-801)
- [**BEAMDP basic**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs509e-beamdp-utility.pdf) manual (PIRS-509e)
- [**BEAMDP advanced**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs509c-beamdp.pdf) manual (PIRS-509c)
- [**STATDOSE**](https://nrc-cnrc.github.io/EGSnrc/doc/pirs509f-statdose.pdf) 3D dose processor (PIRS-509f)

## Licence

EGSnrc is distributed as free software under the terms of the GNU Affero
General Public Licence. Please review the
[LICENCE](https://github.com/nrc-cnrc/EGSnrc/blob/master/LICENCE.md) document
before downloading the software. In practice, this licence imposes no
restriction on using EGSnrc. However, if you want to further convey
verbatim or modified versions of the code, or any work based on any
EGSnrc component (including any such work operated remotely over a
network), you must do so under the same licence terms.
[Contact us](https://nrc.canada.ca/en/research-development/products-services/software-applications/egsnrc-software-tool-model-radiation-transport)
if you wish to licence EGSnrc under different terms.

## Prerequisites

EGSnrc can be installed on computers running Linux, macOS or Windows
operating systems. As a general-purpose Monte Carlo toolkit, EGSnrc
provides source code and utilities to build and run your own
radiation transport simulation applications. To use EGSnrc
on any platform you need:

- a Fortran compiler
- a C compiler
- a C++ compiler
- the GNU `make` utility
- the Tcl/Tk interpreter and widget toolkit (optional, for GUIs)
- the Grace plotting tool (optional, for plotting data)

Please read the
[installation instructions](https://github.com/nrc-cnrc/EGSnrc/wiki/Installation-overview)
in the wiki for more details about these software components.

## Installation

Installation involves downloading the EGSnrc source code, and then configuring
EGSnrc for your computer. We recommend using the `git` version control
system to obtain the source code. The following shell command will clone the
EGSnrc repository to your current working directory:

```bash
git clone https://github.com/nrc-cnrc/EGSnrc.git
```

Alternatively, you can download a
[zip archive](https://github.com/nrc-cnrc/EGSnrc/archive/master.zip) of the EGSnrc
directory. Once you have downloaded the source code, you need to run a
configuration GUI or script to set up and compile EGSnrc software components
for your computer. Please read the full
[installation instructions](https://github.com/nrc-cnrc/EGSnrc/wiki/Installation-overview)
for more details on how to download and configure EGSnrc.

## Support

For technical support, consider the EGSnrc
[Discussions](https://github.com/nrc-cnrc/EGSnrc/discussions), or
[contact us](https://nrc.canada.ca/en/research-development/products-services/software-applications/egsnrc-software-tool-model-radiation-transport).

## Issues

Post [Issues](https://github.com/nrc-cnrc/EGSnrc/issues) to report
genuine bugs, mistakes or even small typos in the EGSnrc project files. The
tracker lets you browse and search all documented issues, comment on open
issues, and track their progress. Note that issues are **not meant for
technical support;** open an issue only for an error which is specific and
reproducible.

## Contributing

You can contribute to the EGSnrc project by implementing new features,
creating new data sets, correcting errors, or improving documentation.
Feel free to submit small corrections and contributions as
[Issues](https://github.com/nrc-cnrc/EGSnrc/issues), and propose new features in
[Discussions](https://github.com/nrc-cnrc/EGSnrc/discussions). For more
extensive contributions, familiarize yourself with git and github,
work on your own EGSnrc fork and submit your changes via
[Pull requests](https://github.com/nrc-cnrc/EGSnrc/pulls).
