
C##############################################################################
C
C  EGSnrc egs_isdir subroutine v2
C  Copyright (C) 2015 National Research Council Canada
C
C  This file is part of EGSnrc.
C
C  EGSnrc is free software: you can redistribute it and/or modify it under
C  the terms of the GNU Affero General Public License as published by the
C  Free Software Foundation, either version 3 of the License, or (at your
C  option) any later version.
C
C  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
C  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
C  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
C  more details.
C
C  You should have received a copy of the GNU Affero General Public License
C  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
C
C##############################################################################
C
C  Author:          Iwan Kawrakow, 2003
C
C  Contributors:
C
C##############################################################################


C*****************************************************************************
C
C  egs_isdir(file_name)  Returns .true., if the string file_name points to
C                        an existing directory.
C                        This version uses the inquire intrinsic, which
C                        only tells us whether file_name points to an existing
C                        object (file, directory, link, etc.), not specifically
C                        an existing directory. But that's all I need.
C                        The reason for having different versions of this
C                        function is that on some systems (e.g. OSF1) the
C                        inquire intrinsic only returns .true. for existing
C                        files, not for existing directories.
C
C*****************************************************************************

      logical function egs_isdir(file_name)
      implicit none
      character*(*) file_name
      logical ex
      inquire(file=file_name,exist=ex)
      egs_isdir = ex
      return
      end
