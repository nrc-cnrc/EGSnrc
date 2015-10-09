## What is EGSnrc?

EGSnrc is a software toolkit to perform Monte Carlo simulation of
ionizing radiation transport through matter. It can model the
propagation of photons, electrons and positrons with kinetic energies
between 1 keV and 10 GeV, in homogeneous compounds (comprising atomic
numbers ranging from 1 to 100). EGSnrc is an extended and improved
version of the **E**lectron **G**amma **S**hower software package
originally developed at the Stanford Linear Accelerator Center (SLAC),
starting in the 1970s. Most notably, it incorporates significant
refinements in charged particle transport, better low energy cross
sections, and the egs++ class library to model elaborate geometries and
particle sources.


## Licence

EGSnrc is distributed as free software under the terms of the GNU Affero
General Public Licence. Please review the [LICENSE](LICENCE.md) document
before downloading the software. In practice, this licence imposes no
restriction on using EGSnrc. However, if you want to further convey
verbatim or modified versions of the code, or any work based on any
EGSnrc component (including any such work operated remotely over a
computer network), you must do so under the same licence terms.
Contact us if you wish to licence EGSnrc under different terms.


## Prerequisites

EGSnrc can be installed on computers running Linux, OSX or Windows
operating systems. As a general-purpose Monte Carlo toolkit, EGSnrc
provides source code and utilities to build and run your own
radiation transport simulation applications. Hence, to use EGSnrc
on any platform you need:

- a Fortran compiler
- a C compiler
- a C++ compiler
- the GNU `make` utility

Additionally, in order to use the EGSnrc GUIs and visualize graphs
prepared by EGSnrc applications, you also need:

- the Tcl/Tk interpreter and widget toolkit
- the Grace plotting tool

Please read the [INSTALL](INSTALL.md) instructions for more details about
these software components.


## Installation

Installation involves downloading EGSnrc files, and then configuring
EGSnrc for your computer. There are two ways to obtain the source code:

1. click the `Download ZIP` button on the main project page
https://github.com/nrc-cnrc/EGSnrc to download a zipped archive of the
latest version of the code to your machine (and then unzip it);

2. simply *clone* the git repository on your computer using the git
revision control software, via the SSH clone URL
`git@github.com:nrc-cnrc/EGSnrc.git`.

Once you have downloaded the source code by either method, download the
latest release of the precompiled EGSnrc graphical user interfaces for
your platform, from the [releases](https://github.com/nrc-cnrc/EGSnrc/releases)
page.

Finally, you must run the configuration GUI or script to set up and
compile EGSnrc software components for your computer. Please read the
[INSTALL](INSTALL.md) instructions for more details on how to download
and configure EGSnrc.


## Documentation

The EGSnrc user manuals in PDF format can be downloaded from the
[releases](https://github.com/nrc-cnrc/EGSnrc/releases) page. Download
the `EGSnrc-manuals.zip` archive, and unzip it. If you installed EGSnrc,
you may want to move the extraced PDF files to the `EGSnrc/HEN_HOUSE/doc`
directory for convenience.


## Issues

Please use the [issue tracker](https://github.com/nrc-cnrc/EGSnrc/issues)
to report bugs, inaccuracies or even small typos in the EGSnrc project
files. The tracker lets you browse and search all documented issues,
post comments on open issues, and track their progress towards resolution.
Note that the issue tracker is *not meant for technical support*. Open
an issue only if it pertains to a factual error condition which is
precise and reproducible.


## Contributing

You may contribute to the EGSnrc project by implementing new features,
creating new data sets, correcting errors, and improving documentation.
You can submit small corrections and contributions as a new issue
in the issue tracker. For more extensive contributions, familiarize
yourself with git and github, work on your own EGSnrc project fork and
submit your changes via a pull request. Note that significant additions
will require a transfer of copyright to the National Research Council
of Canada before they can be merged into the EGSnrc distribution.
