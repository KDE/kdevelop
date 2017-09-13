#!/bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp
$XGETTEXT `find . -name \*.cpp | grep -v tests` -o $podir/kdevdebuggercommon.pot
rm -f rc.cpp
