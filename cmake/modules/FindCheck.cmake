# Find Check, a C unit testing framework.
#
# This script tries to set the following variables:
#
#  CHECK_FOUND - what you'd expect
#  CHECK_INCLUDE_DIR - the TDB include directory
#  CHECK_LIBRARIES - Link these to use TDB
#
# Copyright (c) 2008, Manuel Breugelmans <mbr.nxi@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

find_path(CHECK_INCLUDE_DIR
    check.h
    /usr/include /usr/local/include)

find_library(CHECK_LIBRARY
    NAMES check
    PATH /usr/lib /usr/local/lib)

if (CHECK_INCLUDE_DIR AND CHECK_LIBRARY)
    set(CHECK_FOUND on)
endif (CHECK_INCLUDE_DIR AND CHECK_LIBRARY)
macro_log_feature(CHECK_FOUND "Check" "A unit testing framework for C" "http://check.sourceforge.net/" FALSE "" "Needed for building the Check test runner")
