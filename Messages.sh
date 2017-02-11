#! /usr/bin/env bash
subdirs="debugger interfaces language outputview project serialization shell sublime util vcs documentation"
rcfiles="`find $subdirs -name \*.rc | grep -v '/tests/'`"
uifiles="`find $subdirs -name \*.ui | grep -v '/tests/'`"
kcfgfiles="`find $subdirs -name \*.kcfg | grep -v '/tests/'`"
if [[ "$rcfiles" != "" ]] ; then
    $EXTRACTRC $rcfiles >> rc.cpp || exit 11
fi
if [[ "$uifiles" != "" ]] ; then
    $EXTRACTRC $uifiles >> rc.cpp || exit 12
fi
if [[ "$kcfgfiles" != "" ]] ; then
    $EXTRACTRC $kcfgfiles >> rc.cpp || exit 13
fi
$XGETTEXT -kaliasLocal `find $subdirs -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/tests/'` rc.cpp -o $podir/kdevplatform.pot
rm -f rc.cpp
