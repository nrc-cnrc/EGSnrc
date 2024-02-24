#!/bin/bash

# If EGS_HOME is not set then source ~/.bashrc
if [ -z "$EGS_HOME" ]; then
    source ~/.bashrc
fi

# Build the GUI Applications
echo "::step::Building the EGSnrc GUI applications..."

export QTDIR=/usr/lib/qt5
export EGS_BASE=$(pwd)
cd ${EGS_BASE}/HEN_HOUSE/gui/ && make --quiet --print-directory
cd ${EGS_BASE}/HEN_HOUSE/egs++/view/ && make --quiet --print-directory
