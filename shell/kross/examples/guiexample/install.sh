#!/bin/sh

cp kdevguiexample.desktop $KDEDIR/share/kde4/services/
mkdir -p $KDEDIR/lib/kde4/GUIExample/
cp GUIExample/*.py $KDEDIR/lib/kde4/GUIExample/
cp GUIExample/*.ui $KDEDIR/lib/kde4/GUIExample/
