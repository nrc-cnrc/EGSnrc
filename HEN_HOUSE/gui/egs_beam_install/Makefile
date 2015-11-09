#****************************************************************************
#
# $Id$
#
# A Makefile for building this GUI
# Once EGS_CONFIG is set, one can just type make in this 
# directory instead of using
#   qmake my_machine=$(my_machine) MAKEFILE=Makefile_$(my_machine)
#   make -f Makefile_$(my_machine)
#
#****************************************************************************

include $(EGS_CONFIG)

all: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine) 

Makefile_$(my_machine): egs_beam_install.pro
	$(QTDIR)/bin/qmake my_machine=$(my_machine) MAKEFILE=Makefile_$(my_machine) the_dir=.

clean: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine) clean

.PHONY: all clean
