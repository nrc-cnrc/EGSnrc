/*
###############################################################################
#
#  EGSnrc DICOM reader header file
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Blake Walters, 2004
#
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################
*/


#define slice_thickness_tag 0x00180050

#define slice_location_tag 0x00201041

#define rows_tag 0x00280010

#define columns_tag 0x00280011

#define pixel_spacing_tag 0x00280030

#define pixeldata_tag 0x7fe00010

#define image_position_tag 0x00200032

#define image_sequence_tag 0x00080008

#define image_orientation_tag 0x00200037

#define rescale_intercept_tag 0x00281052

#define rescale_slope_tag 0x00281053
