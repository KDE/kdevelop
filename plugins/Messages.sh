#! /usr/bin/env bash
plugins="appwizard coverage cppdebugger documentview formatters grepview qtdesigner valgrind xtest"
for plugin in $plugins ; do
    $EXTRACTRC `find $plugin -name \*.rc` >> rc.cpp || exit 11
    $EXTRACTRC `find $plugin -name \*.ui` >> rc.cpp || exit 12
    $XGETTEXT -kaliasLocal `find $plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

#builders, managers and languages each contain multiple plugins

builders="qmakebuilder cmakebuilder makebuilder"
for plugin in $builders ; do
    $EXTRACTRC `find builders/$plugin -name \*.rc` >> rc.cpp || exit 11
    $EXTRACTRC `find builders/$plugin -name \*.ui` >> rc.cpp || exit 12
    $XGETTEXT -kaliasLocal `find builders/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

managers="qmake cmake custommake"
for plugin in $managers ; do
    $EXTRACTRC `find managers/$plugin -name \*.rc` >> rc.cpp || exit 11
    $EXTRACTRC `find managers/$plugin -name \*.ui` >> rc.cpp || exit 12
    $XGETTEXT -kaliasLocal `find managers/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done

languages="cpp"
for plugin in $languages ; do
    $EXTRACTRC `find languages/$plugin -name \*.rc` >> rc.cpp || exit 11
    $EXTRACTRC `find languages/$plugin -name \*.ui` >> rc.cpp || exit 12
    $XGETTEXT -kaliasLocal `find languages/$plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done
