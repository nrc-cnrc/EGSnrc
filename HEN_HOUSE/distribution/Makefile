
###############################################################################
#
#  EGSnrc makefile to create EGSnrc distribution files
#  Copyright (C) 2018 National Research Council Canada
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
#  Author:       Iwan Kwrakow
#
#  Contributors: Ernesto Mainegra-Hing
#
###############################################################################
#
#  Builds GUIs and egs++ geometry library for distribution (including Windows
#  files).
#
#  Drawback:
#
#  one must have created the necessary Windows files on a Windows machine by
#  going to $HEN_HOUSE/pieces/windows and executing `make'. This requires:
#
#  1) a Qt4 library statically built with the GNU compiler
#  2) the same GNU compiler version as used in 1.
#
#  If no static Qt library is available, one can use a dynamically linked Qt,
#  which should work as long as EGSnrc is installed on an identical system
#  with the same Qt version.
#
#  To create only Linux distribution files use: make linux_distribution
#
###############################################################################


egsnrc_list=distribution/egs_guis_list
version = EGSnrc
arch=$(shell echo `getconf LONG_BIT`)

unix_list = guis-linux
win_list = guis-windows

unix_files = $(addprefix $(version)-,$(unix_list))
win_files = $(addprefix $(version)-,$(win_list))
tar_files = $(addsuffix .tar,$(unix_files))
self_files_unix = $(addsuffix .egsself,$(unix_files))
self_files_win = $(addsuffix .egsself,$(win_files))

all: distribution

linux_distribution: guis first $(version)-configure-linux

distribution: guis first $(version)-configure-linux $(version)-configure-windows.exe

configure: $(version)-configure-linux $(version)-configure-windows.exe

first:
	make make_distribution
	make mark_egszip
	cd .. && ./distribution/make_distribution -l $(egsnrc_list) -p all
	mv ../*.list ../*.gz ../*.tar ../*.egsself .

fast: guis $(version)-configure-linux $(version)-configure-windows.exe

%.egszip: %.egsself
	rm -f $@
	./mark_egszip $@
	cat $< >>$@

guis:
	cd $(HEN_HOUSE)/pieces/linux && make clean && time make

$(version)-configure-linux: egs_configuration_$(arch) $(self_files_unix)
	rm -f $@
	cp $< $@
	./mark_egszip $@
	cat $(self_files_unix) >>$@

$(version)-configure-windows.exe: egs_configuration.exe $(self_files_win)
	rm -f $@
	cp $< $@
	./mark_egszip $@
	cat $(self_files_win) >>$@

make_distribution: makefile_md
	make -f makefile_md

mark_egszip: mark_self_exe.cpp
	g++ -O2 -o $@ $^

egs_configuration_$(arch): ../pieces/linux/egs_configuration_$(arch)
	cp $< $@

egs_configuration.exe: ../pieces/windows/egs_configuration.exe
	cp $< $@

makefile_md: distribution.pro
	$(QTDIR)/bin/qmake MAKEFILE=makefile_md

clean:
	rm -f *.o

realclean:
	rm -f *.o make_distribution mark_egszip makefile_md egs_configuration* $(version)-*

.PHONY: all first fast guis distribution linux_distribution clean realclean
