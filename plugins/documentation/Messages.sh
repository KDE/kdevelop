#! /usr/bin/env bash
subdirs="interfaces tools plugins protocols data"
$EXTRACTRC `find $subdirs -name \*.rc` >> rc.cpp || exit 11
$EXTRACTRC `find $subdirs -name \*.ui` >> rc.cpp || exit 12
$XGETTEXT -kaliasLocal `find $subdirs -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/documentation.pot
rm -f rc.cpp
