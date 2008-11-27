#! /usr/bin/env bash
subdirs="src"
rcfiles="`find $subdirs -name \*.rc`"
uifiles="`find $subdirs -name \*.ui`"
if [[ "$rcfiles" != "" ]] ; then
    $EXTRACTRC "$rcfiles" >> rc.cpp || exit 11
fi
if [[ "$uifiles" != "" ]] ; then
    $EXTRACTRC "$uifiles" >> rc.cpp || exit 12
fi
$XGETTEXT -kaliasLocal `find $subdirs -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdevelop.pot
rm -f rc.cpp
