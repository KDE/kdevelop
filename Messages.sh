#!/bin/sh
files=`find . -name \*.rc -o -name \*.ui | grep -v '/tests/'`
if [ "x$files" != "x" ]; then
       $EXTRACTRC $files >> rc.cpp
fi
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/kdevqmljssupport.pot
$XGETTEXT_QT `find libs -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/tests/'` -j -o $podir/kdevqmljssupport.pot
rm -f rc.cpp
