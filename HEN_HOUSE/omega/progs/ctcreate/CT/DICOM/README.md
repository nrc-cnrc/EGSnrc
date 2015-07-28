## BEAMnrc ctcreate utility sample DICOM data

All files in this directory are part of EGSnrc.

Some data files require strict formatting to work properly within EGSnrc, hence
it is not possible to insert a licence notice inside those files. They are
nevertheless distributed under the same terms as EGSnrc, and as a reminder a
copy of the LICENCE is included in this directory.

### Sample DICOM data

The *.ct files in this directory are binary DICOM CT data files courtesy of
Peter Love in Cardiff, Wales.  The images were scanned on a Siemens machine.
Each file is a 512x512 slice (in the xy plane) through a bone phantom. The
xy cross sections appear as rectangles of bone with flanged ends surrounded
by material of approximately tissue density. The file slice_names is simply
a list of the *.ct files in order of increasing z and is to be used as the
input file name for ctcreate (e.g., `CT_create_DICOM.inp` also in this
directory).
