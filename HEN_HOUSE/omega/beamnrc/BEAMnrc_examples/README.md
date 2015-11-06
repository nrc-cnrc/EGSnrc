## BEAMnrc examples

These examples were authored by Blake Walters in 2004.

Each subdirectory contains the spec_module file, the input file and all the
output files associated with running the job in batch mode. To try any example:

1. copy the spec_module to $EGS_HOME/beamnrc/spec_modules

2. build the accelerator by typing `beam_build.exe xxxx`, where `xxxx` is the name of one of the examples, e.g., `EX16MVp`

3. copy the `xxxx.egsinp` file to `$EGS_HOME/BEAM_xxxx`

4. if you are running on Linux make sure your `.bashrc` (or .cshrc) file sources `$HEN_HOUSE/scripts/egsnrc_bashrc_additions`
   (or `$HEN_HOUSE/scripts/egsnrc_cshrc_additions`)

5. start beamnrc_gui and follow the instructions.

### Examples

 Name                 | Description
:---------------------|:----------------------
 EXslabs              | a single SLABS CM. The pegs data is in icru700
 EX16MVp              | a generic 16 MV photon beam accelerator
 EX10MeVe             | a generic 10 MeV electron beam accelerator
 EXphantom            | calculating dose components on the central axis using the CM CHAMBER
