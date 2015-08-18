#!/bin/sh
files=`find . -name \*.rc -o -name \*.ui | grep -v '/qmakebuilder/' | grep -v '/app_templates/' | grep -v '/tests/'`
if [ "x$files" != "x" ]; then
       $EXTRACTRC $files >> rc.cpp
fi
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/qmakebuilder/' | grep -v '/tests/'` -o $podir/kdevqmake.pot
rm -f rc.cpp
