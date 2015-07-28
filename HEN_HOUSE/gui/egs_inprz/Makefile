#****************************************************************************
#
# $Id: Makefile,v 1.7 2013/03/20 21:47:16 mainegra Exp $
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
	$(gui_make) -f Makefile_$(my_machine)

static: Makefile_static_$(my_machine)
	$(gui_make) -f Makefile_static_$(my_machine) my_machine=$(my_machine)

Makefile_$(my_machine): egs_inprz.pro
	$(QTDIR)/bin/qmake my_machine=$(my_machine) my_build=shared MAKEFILE=Makefile_$(my_machine)

Makefile_static_$(my_machine): egs_inprz.pro
	$(QTDIR)/bin/qmake my_machine=$(my_machine) my_build=static MAKEFILE=Makefile_static_$(my_machine)

clean: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine) clean

realclean: clean
	rm Makefile_*$(my_machine)

.PHONY: all clean
