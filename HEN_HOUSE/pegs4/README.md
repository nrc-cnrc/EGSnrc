## EGSnrc pegs4 data

All files in this directory are part of EGSnrc.

Some data files require strict formatting to work properly within EGSnrc, hence
it is not possible to insert a licence notice inside those files. They
are nevertheless distributed under the same terms as EGSnrc, and as a reminder a
copy of the LICENCE is included in this directory.

#### McGill University XCOM data file

The file `pgs4pepr_xcom-full.dat` was obtained from Jan Seuntjens,
Wamied Abdel-Rahman and Fadi Hobeila from McGill University, in 2003.
The major impact of the change is for low-energy photon cross sections.
Note that `pgs4pepr_xcom_mcgill.dat` is an earlier version of the file
only updating the photo-electric cross sections.

To create pegs4 data using this data set, then one must run pegs4
from the command line with the `-x` option, as in
`-x $HEN_HOUSE/pegs4/pgs4pepr_xcom-full.dat`.
For full details see the PIRS877 manual.

Seuntjens et al. used the Berger and Hubbell XCOM program to generate
a dataset which contains the XCOM cross sections for the photo-electric effect,
coherent (Rayleigh) scattering and the pair production cross sections.
For more information, please consult the following papers:

- M. J. Berger and J. H. Hubbell, *XCOM: Photon Cross Sections on a
Personal Computer*,  NIST, 1987 Gaithersburg, MD20899 NBSIR87--3597.

- J. P. Seuntjens, I. Kawrakow, J. Borg, F. Hobeila and
D. W. O. Rogers, *Calculated and measured air-kerma response of ionization
chambers in low and medium energy photon beams, Recent developments in
accurate radiation dosimetry*, Proc. of an Int'l Workshop, 69-84, 2002,
Medical Physics Publishing, Madison WI.

- F. Hobeila and J.P. Seuntjens, *Effect of the XCOM photo-electric
cross-sections on dosimetric quantities calculated with EGSnrc*,
, in Standards and Codes of Practice in Medical Radiation Dosimetry Vol. 1,
[IAEX-CN-96-17P+](http://www-pub.iaea.org/mtcd/publications/pdf/pub1153/cd/p1153_1.pdf),
177-186 (2002).
