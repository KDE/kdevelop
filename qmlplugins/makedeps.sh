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

# Index the identifiers declared and used in each module
> /tmp/qml_index

echo "Building index..."

ls *.qml | while read qmlfile
do
    package="$(basename "$qmlfile" .qml)"

    # Identifiers declared in this package
    cat "$qmlfile" | grep -E '^ +name: "[A-Z]' | cut -d '"' -f 2 | while read identifier
    do
        echo "$package $identifier " >> /tmp/qml_index
    done
done

# Identify the packages that have to be imported in each file
ls *.qml | while read qmlfile
do
    package="$(basename "$qmlfile" .qml)"

    echo "Uses for $package..."
    > "/tmp/$package.imports"

    # Identifiers used in this package
    cat "$qmlfile" | grep -Eo '(type|prototype): "[A-Za-z:]+"' | cut -d '"' -f 2 | cut -d ':' -f 1 | sort | uniq | while read identifier
    do
        imported="$(cat /tmp/qml_index | grep " $identifier " | head -n1 | cut -d ' ' -f 1)"

        if [ "x$imported" != "x" ]
        then
            echo "import $imported 1.0" >> "/tmp/$package.imports"
        fi
    done

    # Add the import statements to the beginning of the QML file
    cat "/tmp/$package.imports" | grep -v " $package " | sort | uniq > "/tmp/$package.header"
    cat "$qmlfile" | grep -v "^import" > "/tmp/$package.body"
    cat "/tmp/$package.header" "/tmp/$package.body" > "$qmlfile"
done

