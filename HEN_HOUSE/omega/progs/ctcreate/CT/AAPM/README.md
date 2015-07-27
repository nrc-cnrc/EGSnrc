## BEAMnrc ctcreate utility sample AAPM data

All files in this directory and in the `smithy` subdirectory are part of EGSnrc.

Some data files require strict formatting to work properly within EGSnrc, hence
it is not possible to insert a licence notice inside those files. They
are nevertheless distributed under the same terms as EGSnrc, and as a reminder a
copy of the LICENCE is included in this directory.

### Sample AAPM data

The files on this area are for converting AAPM format CT data to Pinnacle format
so that the dosxyz routine can read it in.

The Makefile should be used to compile on your machine with the `make` command.

See aapm2pinnacle.text for further information

The `smithy` directory contains a CT scan in AAPM format (name changed of course).
The complete AAPM file is not there, just the CT scan part.

Julie Zachman from the University of Wisconsin may be able to provide further
information regarding formats, etc.