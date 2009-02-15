#! /usr/bin/env bash
subdirs="app"
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

plugins="formatters xtest"
for plugin in $plugins ; do
    rcfiles="`find $plugin -name \*.rc`"
    uifiles="`find $plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC $rcfiles >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC $uifiles >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find $plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done


debuggers="cppdebugger valgrind"
for plugin in $debuggers ; do
    rcfiles="`find debugger/$plugin -name \*.rc`"
    uifiles="`find debugger/$plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC $rcfiles >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC $uifiles >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find debugger/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

tools="coverage documentview grepview qtdesigner"
for plugin in $tools ; do
    rcfiles="`find tools/$plugin -name \*.rc`"
    uifiles="`find tools/$plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC $rcfiles >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC $uifiles >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find tools/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

builders="qmakebuilder cmakebuilder makebuilder"
for plugin in $builders ; do
    rcfiles="`find projectbuilders/$plugin -name \*.rc`"
    uifiles="`find projectbuilders/$plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC $rcfiles >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC $uifiles >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find projectbuilders/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

managers="qmake cmake custommake"
#dir automake missing!
for plugin in $managers ; do
    rcfiles="`find projectmanagers/$plugin -name \*.rc`"
    uifiles="`find projectmanagers/$plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC "$rcfiles" >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC "$uifiles" >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find projectmanagers/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

languages="cpp"
for plugin in $languages ; do
    rcfiles="`find languages/$plugin -name \*.rc`"
    uifiles="`find languages/$plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC $rcfiles >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC $uifiles >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find languages/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

