## An MPI parallel implementation for EGSnrc

The purpose of this contribution is to present a parallel solution for the
EGSnrc Monte Carlo code system using the MPI programming model as an alternative
to the provided implementation, based on the use of a batch-queueing system.

Currently, the BEAMnrc and DOSXYZnrc user codes support this parallel solution
based on MPI. In the case of DOSXYZnrc, both the use of a phase space file
and BEAM shared library as sources has been tested. Both codes can be used
as models to introduce MPI features to other EGSnrc user codes.

## Authors

Edgardo Doerner (edoerner at fis.puc.cl)
Paola Caprile

Institute of Physics
Pontificia Universidad Catolica de Chile

## Method

This work incorporates MPI features to distribute the simulation between the
available compute units. These features are introduced through properly defined
macros, which are enabled depending on the compilation flags given by the user.
The workload balance is controlled by default via a 'job control file' as the
original implementation.

In order to ease the integration of MPI in EGSnrc, the compilation flags
needed to enable the MPI parallelization are added through the 'mpi' target,
defined in both the EGSnrc standard makefile (standard_makefile) and BEAMnrc
makefile (beam_makefile) inside $HEN_HOUSE/makefiles folder.

The result is a dedicated executable with MPI support that can be called by the
user. The advantage is that there is no need to modify or add the needed
compilation flags to the *.conf file during the installation process.

By default, it is expected that OpenMPI is installed in the system. If another
MPI implementation is desired, change the F77 macro inside the 'mpi' target to
the desired MPI compiler in the following Makefiles:

$HEN_HOUSE/makefiles/standard_makefile (line 169)
$HEN_HOUSE/makefiles/beam_makefile (line 165)

by default F77=mpif90 in both files.

## Usage

In order to compile an user code $(user_code) with MPI support go to
the $EGS_HOME/$(user_code) folder and type:

make mpi

This will enable the MPI features in the user code and will create an executable
called $(user_code)_mpi (i.e. the normal user code executable name with the
'_mpi' suffix attached to it). Then, use mpirun or similar to execute it with
MPI support:

mpirun -np #NUM_PROCS $(user_code)_mpi -i input_file -p pegs_file

## Issues

Two small issues remains in the following contribution:

1. When the DOSXYZnrc user code is used with a BEAM shared library as a source,
an error appears regarding the inability of locking the *.lock file used
to distribute the workload in the parallel simulation. This is triggered by the
shared library, not the dosxyznrc jobs or MPI processes involved.

This issue is inherited from the develop branch on EGSnrc, and causes all the
MPI processes to halt. As a temporal solution, the use of the file locking
mechanism is disabled in the BEAM shared library at compile time through a
proper compilation flag.

2. When the BEAMnrc user code is compiled with MPI support or DOSXYZnrc is
used with a BEAM shared library as a source, a console message
appears at the end of the simulation:

ls: ~/$EGS_HOME/BEAM_myaccel/myaccel_w*.egsdat: No such file or directory

Of course, it is expected to not have egsdat files if data arrays are not
stored during simulation. The presence of such files is tested in the
egs_combine_runs subroutine at the end of the simulation. When MPI support is
enabled, and in contrast with the standard compilation, the result of the
system command is echoed to the terminal. This does not affect the results
and it means only a minor annoyance during execution.

## Disabling Parallel Jobs functionality

Some systems (such as the National Laboratory of High Performance Computing
(NLHPC) in Chile) does not allow the use of a lock-file mechanism needed by the
EGSnrc parallel implementation and this MPI contribution to control access to
the job control file.

In such a case, the user can define the _NOPJOB macro during compilation to
disable the use of the control file. For example, in the following Makefiles add
the _NOPJOB macro definition as:

standard_makefile (line 169): FCFLAGS="$(FCFLAGS) -D_MPI -D_NOPJOB"
beam_makefile (line 165): FCFLAGS="$(FCFLAGS) -D_MPI -D_NOPJOB"

The result is that the simulation workload is now evenly distributed among
computing units, and therefore no job control file is needed.

## OpenMP features

The introduction of OpenMP features has been tested in the DOSXYZnrc user
code, with interesting results (see references below). However, some important
issues remains unsolved and definitely will need a more extended modification
of the user codes, namely:

1. The support of shared library sources in DOSXYZnrc when OpenMP is enabled.
2. The OpenMP implementation in BEAMnrc, considering the output to phase space
files.

An hybrid implementation, combining MPI and OpenMP features can be seen in the
pull request #341 (https://github.com/nrc-cnrc/EGSnrc/pull/341). For this
reason, it was decided to offer first a polished MPI implementation to the
EGSnrc community.

## References

This MPI implementation is contained in the following work:

Doerner E, Caprile P. An hybrid parallel implementation for EGSnrc Monte Carlo
user codes. Med. Phys. 45 (8), August 2018.

The aforementioned publication was based on an OpenMP-only solution which can be
reviewed in:

Doerner E, Caprile P. Parallel implementation of the EGSnrc Monte Carlo
simulation of ionizing radiation transport using OpenMP. Med. Phys. 44 (12),
December 2017
