#!/bin/sh
echo "Running aclocal"
aclocal
echo "Running autoconf"
autoconf
#echo "Processing Makefile.am"
echo "Running automake"
automake
echo "Done."
