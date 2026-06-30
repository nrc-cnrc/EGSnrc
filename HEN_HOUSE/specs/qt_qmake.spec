
###############################################################################
#
#  EGSnrc qmake settings for Qt GUI builds
#  Copyright (C) 2026 National Research Council Canada
#
#  Include after $(EGS_CONFIG) in Qt GUI Makefiles.
#
###############################################################################

is_darwin := $(findstring darwin,$(canonical_system))

ifneq ($(is_darwin),)
  QMAKE_SPEC  = $(HEN_HOUSE)mkspecs/macx-g++-egsnrc
  QMAKE_FLAGS = -spec $(QMAKE_SPEC)
else
  QMAKE_FLAGS =
endif

QMAKE = $(QTDIR)/bin/qmake $(QMAKE_FLAGS)

ifeq ($(is_darwin),)
else
  ifeq ($(QTDIR),)
    $(error QTDIR is not set. On macOS install qt@5 (e.g. brew install qt@5) and source egsnrc_bashrc_additions)
  endif
endif
