#!/bin/sh

cp kdevcompileone.desktop $KDEDIR/share/kde4/services/
mkdir -p $KDEDIR/lib/kde4/CompileOne/
cp CompileOne/*.py $KDEDIR/lib/kde4/CompileOne/
