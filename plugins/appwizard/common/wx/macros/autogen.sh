#!/bin/sh
aclocal
libtoolize --automake --force --copy
automake -a -c
autoconf

