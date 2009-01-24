#! /usr/bin/env bash
plugins="coverage cppdebugger documentview formatters grepview qtdesigner valgrind xtest"
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

#builders, managers and languages each contain multiple plugins

builders="qmakebuilder cmakebuilder makebuilder"
for plugin in $builders ; do
    rcfiles="`find builders/$plugin -name \*.rc`"
    uifiles="`find builders/$plugin -name \*.ui`"
    if [[ "$rcfiles" != "" ]] ; then
        $EXTRACTRC $rcfiles >> rc.cpp || exit 11
    fi
    if [[ "$uifiles" != "" ]] ; then
        $EXTRACTRC $uifiles >> rc.cpp || exit 12
    fi
    $XGETTEXT -kaliasLocal `find builders/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

managers="qmake cmake custommake"
#dir automake missing!
for plugin in $managers ; do
    #no rc and ui files
    #rcfiles="`find managers/$plugin -name \*.rc`"
    #uifiles="`find managers/$plugin -name \*.ui`"
    #if [[ "$rcfiles" != "" ]] ; then
    #    $EXTRACTRC "$rcfiles" >> rc.cpp || exit 11
    #fi
    #if [[ "$uifiles" != "" ]] ; then
    #    $EXTRACTRC "$uifiles" >> rc.cpp || exit 12
    #fi
    $XGETTEXT -kaliasLocal `find managers/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` -o $podir/kdev${plugin}.pot
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
