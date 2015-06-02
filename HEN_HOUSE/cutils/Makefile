
###############################################################################
#
#  EGSnrc Makefile to build C utilities
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
#  Author:          Iwan Kawrakow, 2004
#  
#  Contributors:    Frederic Tessier   
#
###############################################################################


include $(EGS_CONFIG)

INC_DIR = ..$(DSEP)lib$(DSEP)$(my_machine)
DEST_DIR = $(INC_DIR)$(DSEP)

all: $(DEST_DIR)load_beamlib.o $(DEST_DIR)egs_c_utils.o $(DEST_DIR)load_vculib.o

$(DEST_DIR)load_beamlib.o: load_beamlib.c $(DEST_DIR)egs_config1.h
	$(CC) -c -I$(INC_DIR) $(C_DEFINES) $(C_FLAGS) $(COUT)$@ load_beamlib.c 

$(DEST_DIR)egs_c_utils.o: egs_c_utils.c egs_c_utils.h $(DEST_DIR)egs_config1.h
	$(CC) -c -I$(INC_DIR) $(C_FLAGS) $(C_DEFINES) $(COUT)$@ egs_c_utils.c

$(DEST_DIR)load_vculib.o: load_vculib.c $(DEST_DIR)egs_config1.h
	$(CC) -c -I$(INC_DIR) $(C_DEFINES) $(C_FLAGS) $(COUT)$@ load_vculib.c

clean:
	$(REMOVE) $(DEST_DIR)load_beamlib.o $(DEST_DIR)load_vculib.o $(DEST_DIR)egs_c_utils.o
