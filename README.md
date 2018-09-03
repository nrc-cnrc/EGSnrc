## Parallel implementation of EGSnrc using OpenMP

This is a prototype for the parallelization of EGSnrc user codes using the OpenMP API.
It is currently implemented on the DOSXYZnrc user code. An advantage of this development
is that removes the necessity of the use of a job scheduler and/or lock-file mechanism to
handle parallel jobs and combination of results.

## Installation and usage

    1a. If you start from a fresh installation just install EGSnrc as usual. All the needed files 
    for OpenMP support are included in this fork.
    1b. Otherwise, once all the files are downloaded to HEN_HOUSE area, copy dosxyznrc folder 
    ($HEN_HOUSE/user_codes/dosxyznrc) to the user codes area (i.e. $EGS_HOME)
    2. In terminal, go to the dosxyznrc folder inside $EGS_HOME.
    3. If the provided standard_makefile is used, just type "make omp" to compile the user code
    with OpenMP support. It essentially compiles with -fopenmp flag (for gcc only). If you want to
    use another compiler, you must change this flag with the proper one inside standard_makefile.
    4. Execute the code as usual. The message "Number of threads in OpenMP parallel execution ..."
    should appear if OpenMP support was enabled

Due to the use of private variables (through the OpenMP THREADPRIVATE directive) it is needed
for some machines to increase the stack size of the OS. For example, on macOs or
GNU/Linux simply type on terminal "ulimit -s hard". In our case, we added that command on
the .bashrc configuration file.

We have tested sources 0 (Parallel Rectangular Beam Incident from Front) and 2 (Phase-Space
Source Incident from Any Direction), although with the notable exception of a BEAMnrc source
model the other sources should work without problems.

Also should be noted that the combination of results at the end of the simulation is
implemented only for the *.3ddose file. Information printed on console and output to
*.egslst file come only from the MASTER thread.

## Additional information

1. All relevant changes to the source files and makefiles are labeled with the #OMPEGS tag.
2. In dosxyznrc $NBATCH was selected as 1, in order to start the parallel region inside 
the ibatch loop just one time.
3. If dosxyznrc is compiled without OpenMP support (i.e. typing "make" in dosxyznrc user code folder) 
the original implementation is recovered. Therefore, it is possible to compare OpenMP execution 
with the parallelization using a BQS, for example.

## What is EGSnrc?

EGSnrc is a software toolkit to perform Monte Carlo simulation of
ionizing radiation transport through matter. It models the propagation
of photons, electrons and positrons with kinetic energies between
1&nbsp;keV and 10&nbsp;GeV, in homogeneous materials. EGSnrc was originally
released in 2000, as a complete overhaul of the Electron Gamma Shower (EGS)
software package originally developed at the Stanford Linear Accelerator
Center (SLAC) in the 1970s. Most notably, EGSnrc incorporates crucial
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
network), you must do so under the same licence terms. Please
[contact NRC](http://www.nrc-cnrc.gc.ca/eng/solutions/advisory/egsnrc_index.html)
if you wish to licence EGSnrc under different terms.


## Installation

EGSnrc can be installed on computers running Linux, macOS or Windows. Please
read the [installation instructions](https://github.com/nrc-cnrc/EGSnrc/wiki/Installation-overview)
for details on how to download and properly configure EGSnrc on your operating system.


## Issues

For technical support and questions, consider the
[EGSnrc Google+ community](https://plus.google.com/communities/106437507294474212197), or
[contact NRC](http://www.nrc-cnrc.gc.ca/eng/solutions/advisory/egsnrc_index.html). To report
genuine bugs, defects or even small typos in the EGSnrc project please
[submit an issue](https://github.com/nrc-cnrc/EGSnrc/issues). The issue tracker lets you
browse and search all documented issues, comment on open issues, and track their
progress. Note that the issue tracker is **not meant for technical support.**


## Contributing

You can contribute to the EGSnrc project by implementing new features, creating
new data sets, correcting errors, or improving documentation. For small
corrections and improvements, feel free to
[submit an issue](https://github.com/nrc-cnrc/EGSnrc/issues). For more extensive
contributions, familiarize yourself with git and github, work on your own EGSnrc
project fork and open a
[pull request](https://github.com/nrc-cnrc/EGSnrc/issues).
