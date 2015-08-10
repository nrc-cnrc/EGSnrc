\begin{figure}
\index{.environment file!example}
\begin{center}
\caption{{\tt tutor4.io} file needed to create an {\tt
EGS\_Windows} data file {\tt .egsgph}.
\label{fig_tutor4_environment}} 
\vspace*{6mm}
\begin{boxedverbatim}


# $Id tutor4.io,v 1.2 2003/11/11 19:10:52 dave Exp $
#
#  This file determines which files are to be connected to which Fortran 
#  I/O unit. Lines starting with # are ignored.
#  The first column is the Fortran I/O unit number, the second a 
#  file extension => e.g. unit 1 will be 
#  connected to output_file.egslst, etc. 
#
#  This .io file is for the NRC user code tutor4.
#  The only additional file is unit 13 for possible output for
#  the EGS_WINDOWS graphics package.
#
#  In the following, unit 13 is connected to test.egsgph  by default.
#  However, by using tutor4 -o file -p tutor_data      to run the code
#  one gets  file.egsgph
#
13 .egsgph


\end{boxedverbatim}
\end{center}
\end{figure}
