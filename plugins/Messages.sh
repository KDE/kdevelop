#! /usr/bin/env bash
rcfiles="`find $subdirs -name \*.rc`"
uifiles="`find $subdirs -name \*.ui`"
if [[ "$rcfiles" != "" ]] ; then
    $EXTRACTRC "$rcfiles" >> rc.cpp || exit 11
fi
if [[ "$uifiles" != "" ]] ; then
    $EXTRACTRC  >> rc.cpp || exit 12
fi
$XGETTEXT -kaliasLocal `find $subdirs -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdevelop.pot
rm -f rc.cpp

plugins="appwizard coverage cppdebugger documentview formatters grepview qtdesigner valgrind xtest"
for plugin in $plugins ; do
    rcfiles="`find $plugin -name \*.rc`"
    uifiles="`find $plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC "$rcfiles" >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC  >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find $plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

#builders, managers and languages each contain multiple plugins

builders="qmakebuilder cmakebuilder makebuilder"
for plugin in $builders ; do
    rcfiles="`find $plugin -name \*.rc`"
    uifiles="`find $plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC "$rcfiles" >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC  >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find builders/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

managers="qmake cmake custommake"
for plugin in $managers ; do
    rcfiles="`find $plugin -name \*.rc`"
    uifiles="`find $plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC "$rcfiles" >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC  >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find managers/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

languages="cpp"
for plugin in $languages ; do
    rcfiles="`find $plugin -name \*.rc`"
    uifiles="`find $plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC "$rcfiles" >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC  >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find languages/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done
