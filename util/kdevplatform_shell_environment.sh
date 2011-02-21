#!/bin/sh

# This file is part of KDevelop
# Copyright 2011 David Nolden <david.nolden.kdevelop@art-master.de>
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
# 
# You should have received a copy of the GNU Library General Public License
# along with this library; see the file COPYING.LIB.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

source ~/.bashrc

export BASE_SHELL_PROMPT=$PS1

if ! [ "$KDEV_DBUS_ID" ]; then
    echo "The required environment variable KDEV_DBUS_ID is not set. This variable defines the dbus id of the application instance instance which is supposed to be attached."
    exit 5
fi

function getSessionName {
    echo "$(qdbus $KDEV_DBUS_ID /kdevelop/SessionController org.kdevelop.kdevelop.KDevelop.SessionController.sessionName)"
}

function updateShellPrompt {
    # Makes the shell prompt show the current name of this session
    SESSION_NAME=$(getSessionName)
    PS1="$SESSION_NAME   - $BASE_SHELL_PROMPT"
}

function help {
    echo "You are controlling the $APPLICATION session '$(getSessionName)'"
    echo ""
    echo "Available commands:"
    echo "raise                               - Raise the window"
    echo "open [file] ...                     - Open the files, referenced by relative or absolute paths"
    echo "create [file] [[text]]              - Create and open a new file with the given optional text"
    echo "search [pattern] [[locations]] ...  - Search for the given pattern here or at the optionally given location(s)."
    echo "dsearch [pattern] [[locations]] ... - Same as search, but starts the search instantly instead of showing the dialog."
    echo ""
}

function openDocument {
    RESULT=$(qdbus $KDEV_DBUS_ID /org/kdevelop/DocumentController org.kdevelop.DocumentController.openDocumentSimple $1)
    if ! [ "$RESULT" == "true" ]; then
        echo "Failed to open $1"
    fi
}

# Main functions:

function raise {
    qdbus $KDEV_DBUS_ID /kdevelop/MainWindow org.kdevelop.MainWindow.ensureVisible
}

function open {
    FILES=$@
    for RELATIVE_FILE in $FILES; do
        # TODO: Support ':linenumber' at the end of the file
        FILE=$(readlink -f $RELATIVE_FILE)
        if ! [ "$FILE" ]; then
            # Try opening the file anyway, it might be an url or something else we don't understand here
            FILE=$RELATIVE_FILE
        fi
        openDocument "$FILE"
    done
}

function create {
    FILE=$(readlink -f $1)
    if ! [ "$FILE" ]; then
        echo "Error: Bad arguments."
        return 1
    fi
    if [ -e "$FILE" ]; then
        echo "The file $FILE already exists"
        return 2
    fi
    echo $2 > $FILE
    openDocument $FILE
}

function search {
    PATTERN=$1
    
    if ! [ "$PATTERN" ]; then
        echo "Error: No pattern given."
        return 1
    fi

    LOCATION=$2

    if ! [ "$LOCATION" ]; then
        LOCATION="."
    fi
    
    LOCATION=$(readlink -f $LOCATION)
    
    for LOC in $*; do
        if [ "$LOC" == "$1" ]; then
            continue;
        fi
        if [ "$LOC" == "$2" ]; then
            continue;
        fi
        LOCATION="$LOCATION;$(readlink -f $LOC)"
    done
    
    qdbus $KDEV_DBUS_ID /org/kdevelop/GrepViewPlugin org.kdevelop.kdevelop.GrepViewPlugin.startSearch "$PATTERN" "$LOCATION" true
}

function dsearch {
    PATTERN=$1
    
    if ! [ "$PATTERN" ]; then
        echo "Error: No pattern given."
        return 1
    fi

    LOCATION=$2

    if ! [ "$LOCATION" ]; then
        LOCATION="."
    fi
    
    LOCATION=$(readlink -f $LOCATION)
    
    for LOC in $*; do
        if [ "$LOC" == "$1" ]; then
            continue;
        fi
        if [ "$LOC" == "$2" ]; then
            continue;
        fi
        LOCATION="$LOCATION;$(readlink -f $LOC)"
    done
    
    qdbus $KDEV_DBUS_ID /org/kdevelop/GrepViewPlugin org.kdevelop.kdevelop.GrepViewPlugin.startSearch "$PATTERN" "$LOCATION" false
}

# Initialization:

updateShellPrompt

help

