#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp || exit 11
$EXTRACTRC `find ui -name \*.ui` >> rc.cpp || exit 12
$XGETTEXT -kaliasLocal `find . -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/subversion.pot
rm -f rc.cpp
