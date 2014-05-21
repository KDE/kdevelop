#!/bin/bash
#
#  Copyright (C) 2014 by Denis Steckelmacher <steckdenis@yahoo.fr>
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#

builtinsIndex="1"

find "$1" -name '*.qmltypes' | while read qmlfile
do
    # The file contains a line like :
    # This file was auto-generated with the command 'qml1plugindump Qt.labs.folderlistmodel 1.0'
    #
    # This line can be used to find the package described by the file
    packageline=($(cat "$qmlfile" | grep "plugindump"))

    if [ "x$packageline" = "x" ]
    then
        continue
    fi

    # The package is always in packageline.section(' ', -2, -2)
    package="${packageline[-2]}"

    if [ ! "x${package:0:2}" = "xQt" ]
    then
        # Every package should start with "Qt". If this is not the case, then
        # the package is a builtin
        package="Builtins$builtinsIndex"
        builtinsIndex="$(($builtinsIndex + 1))"
    fi

    echo "Found $package..." $qmlfile

    # Copy the file to its package
    cp "$qmlfile" "$package.qml"
done