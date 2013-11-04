#!/bin/sh
files=`find . -name \*.rc -o -name \*.ui`
if [ "x$files" != "x" ]; then
	$EXTRACTRC $files >> rc.cpp
fi
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h` -o $podir/kdevninja.pot
rm -f rc.cpp
