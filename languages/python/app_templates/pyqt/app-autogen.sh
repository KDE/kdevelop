#!/bin/sh
echo "Running aclocal"
aclocal
echo "Running autoconf"
autoconf
echo "Processing Makefile.am"
python pyqt-am-preproc.py
echo "Running automake"
automake
echo "Done."
