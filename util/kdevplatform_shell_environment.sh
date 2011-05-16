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

if ! [ "$APPLICATION_HOST" ]; then
    export APPLICATION_HOST=$(hostname)
fi

if ! [ "$KDEV_DBUS_ID" ]; then
    echo "The required environment variable KDEV_DBUS_ID is not set. This variable defines the dbus id of the application instance instance which is supposed to be attached."
    exit 5
fi

function getSessionName {
    echo "$(qdbus $KDEV_DBUS_ID /kdevelop/SessionController org.kdevelop.kdevelop.KDevelop.SessionController.sessionName)"
}

function updateShellPrompt {
    # Makes the shell prompt show the current name of this session
#     SESSION_NAME=$(getSessionName)
    PS1="!$BASE_SHELL_PROMPT" # Just somehow set a mark that this session is attached
}

function help! {
    if ! [ "$SHORT_HELP" ]; then
    echo "You are controlling the $APPLICATION session '$(getSessionName)'"
    echo ""
    fi
    echo "Commands:"
    if ! [ "$SHORT_HELP" ]; then
    echo "raise!                                 - Raise the window."
    fi
    echo "sync!                                  - Synchronize the working directory with the currently open document."
    echo "open!   [file] ...                     - Open the file(s) within the attached application."
    echo "eopen!  [file] ...                     - Open the file(s) within an external application using kde-open."
    echo "create!  [file] [[text]]               - Create and open a new file."
    echo "search!   [pattern] [[locations]] ...  - Search for the given pattern here or at the optionally given location(s)."
    echo "dsearch!  [pattern] [[locations]] ...  - Same as search, but starts the search instantly instead of showing the dialog (using previous settings)."
    if ! [ "$SHORT_HELP" ]; then
    echo "ssh!  [ssh arguments]                  - Connect to a remote host via ssh, keeping the control-connection alive."
    echo "                                       - The whole dbus environment will be forwarded, KDevelop needs to be installed on both sides."
    echo "exec! [cmd] [args] [file] . ..         - Execute the given command on the client machine, referencing any number of local files."
    echo "                                       - The file paths will be re-encoded as fish:// urls as required."
    echo "cexec! [cmd] [args] [file] . ..        - Execute the given command on the client machine, referencing any number of local files."
    echo "                                       - The files will be COPIED to the client machine as required."
    fi
    echo "help!                                  - Show extended help."
    if ! [ "$SHORT_HELP" ]; then
    echo ""
    echo "Commands can be abbreviated by the first character(s), eg. r! instead of raise!, and se! instead of search!."
    fi
    echo ""
}

# Short versions of the commands:

function r! {
    raise! $@
}

function s! {
    sync! $@
}

function o! {
    open! $@
}

function eo! {
    eopen! $@
}

function e! {
    exec! $@
}

function ce! {
    cexec! $@
}

function c! {
    create! $@
}

function se! {
    search! $@
}

function ds! {
    dsearch! $@
}

function h! {
    help! $@
}

# Internals:

function openDocument {
    RESULT=$(qdbus $KDEV_DBUS_ID /org/kdevelop/DocumentController org.kdevelop.DocumentController.openDocumentSimple $1)
    if ! [ "$RESULT" == "true" ]; then
        echo "Failed to open $1"
    fi
}

# First argument: The full command. Second argument: The working directory.
function executeInApp {
    local CMD=$1
    local WD=$2
    if ! [ "$WD" ]; then
        WD=$(pwd)
    fi
    RESULT=$(qdbus $KDEV_DBUS_ID /org/kdevelop/ExternalScriptPlugin org.kdevelop.ExternalScriptPlugin.executeCommand "$CMD" "$WD")
    if ! [ "$RESULT" == "true" ]; then
        echo "Execution failed"
    fi
}

# Getter functions:

function getActiveDocument {
    qdbus $KDEV_DBUS_ID /org/kdevelop/DocumentController org.kdevelop.DocumentController.activeDocumentPath
}

function getActiveDocuments {
    qdbus $KDEV_DBUS_ID /org/kdevelop/DocumentController org.kdevelop.DocumentController.activeDocumentPaths
}

function raise! {
    qdbus $KDEV_DBUS_ID /kdevelop/MainWindow org.kdevelop.MainWindow.ensureVisible
}


# Main functions:

function raise! {
    qdbus $KDEV_DBUS_ID /kdevelop/MainWindow org.kdevelop.MainWindow.ensureVisible
}

function sync! {
    local P=$(getActiveDocument)
    if [ "$P" ]; then
        cd $(dirname $P)
    else
        echo "Got no path"
    fi
}

# Takes a relative file, returns an absolute file/url that should be valid on the client.
function mapFileToClient {
    local RELATIVE_FILE=$1
    FILE=$(readlink -f $RELATIVE_FILE)
    if ! [ -e "$FILE" ]; then
        # Try opening the file anyway, it might be an url or something else we don't understand here
        FILE=$RELATIVE_FILE
    else
        # We are referencing an absolute file, available on the file-system.
        # If we are forwarding, map it to the client somehow.
        # TODO: Map through fish protocol, or whatever. Check whether the same file is available
        #       on the client machine at the same path first.
        
        # Step 1. Check if the same filesystem is available at the client at the same path. If yes: Do nothing
        # We check by simple comparing the inode number.
        INODE=$(ls -i $FILE | cut -d' ' -f1)
        
        # 2. Check if the we have a valid ssh-map rule, if yes, use that one (TODO: Make this optional).
        # 3. Copy the file as a read-only copy to the client.
        
        if [ "$KDEV_SSH_FORWARD_CHAIN" ]; then
            # We can eventually map the file using the fish protocol
            # TODO: Make this optional
            if ! [[ "$KDEV_SSH_FORWARD_CHAIN" == *\,* ]]; then
                # We can only map through fish if the forward-chains contains no comma, which means that
                # we forward only once.
                FILE="fish://$KDEV_SSH_FORWARD_CHAIN$FILE"
            fi
        fi
        
        FILE=$FILE
        
    fi
    echo $FILE
}

function open! {
    FILES=$@
    for RELATIVE_FILE in $FILES; do
        # TODO: Support ':linenumber' at the end of the file
        FILE=$(mapFileToClient $RELATIVE_FILE)
        openDocument "$FILE"
    done
}

function eopen! {
    FILES=$@
    for RELATIVE_FILE in $FILES; do
        FILE=$(mapFileToClient $RELATIVE_FILE)
        executeInApp "kde-open $FILE"
    done
}

function exec! {
    FILES=$@
    ARGS=""
    for RELATIVE_FILE in $FILES; do
        if [ "$ARGS" == "" ]; then
            # Do not transform the command-name
            ARGS=$RELATIVE_FILE
        else
            FILE=$(mapFileToClient $RELATIVE_FILE)
            ARGS=$ARGS" "$FILE
        fi
    done
    echo "Executing: " $ARGS
    executeInApp "$ARGS"
}

function cexec! {
    FILES=$@
    ARGS=""
    PREFIX=""
    TMP=1
    for RELATIVE_FILE in $FILES; do
        if [ "$ARGS" == "" ]; then
            # Do not transform the command-name
            ARGS=$RELATIVE_FILE
        else
            FILE=$(mapFileToClient $RELATIVE_FILE)
            
            if [[ "$FILE" == fish://* ]]; then
                # Add a prefix to copy the file into a temporary file
                PREFIX+="TMP$TMP=\$(mktemp).$(basename $FILE); kioclient copy $FILE \$TMP$TMP;"
                # Use the temporary variable instead of the name 
                FILE="\$TMP$TMP"
                TMP=$(($TMP+1))
            fi
            
            ARGS=$ARGS" "$FILE
        fi
    done
    echo "Executing: " $PREFIX $ARGS
    executeInApp "$PREFIX $ARGS"
}

function create! {
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

function search! {
    PATTERN=$1
    
#     if ! [ "$PATTERN" ]; then
#         echo "Error: No pattern given."
#         return 1
#     fi

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

function dsearch! {
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

##### SSH DBUS FORWARDING --------------------------------------------------------------------------------------------------------------------

DBUS_SOCKET_TRANSFORMER=$KDEV_BASEDIR/kdev_dbus_socket_transformer

# We need this, to make sure that our forwarding-loops won't get out of control
# This configures the shell to kill background jobs when it is terminated
shopt -s huponexit

# TODO: This random number can lead to conflicts, but since only ssh notices these conflicts
#       during forwarding, it is very hard to deal with them.
export DBUS_FORWARDING_TCP_TARGET_PORT=$((5000+($RANDOM%50000)))

export DBUS_ABSTRACT_SOCKET_TARGET_BASE_PATH=/tmp/dbus-forwarded-$USER-$APPLICATION_HOST

export DBUS_FORWARDING_TCP_LOCAL_PORT=9000
export DBUS_FORWARDING_TCP_MAX_LOCAL_PORT=10000
export DBUS_ABSTRACT_SOCKET_TARGET_INDEX=1
export DBUS_ABSTRACT_SOCKET_MAX_TARGET_INDEX=1000

function getPortFromSSHCommand {
    # The port is given to ssh exclusively in the format "-p PORT"
    # This regular expression extracts the "4821" from "ssh -q bla1 -p 4821 bla2"
    local ARGS=$@
    local RET=$(echo "$@" | sed "s/.*-p \+\([0-9]*\).*/\1/")
    if [ "$ARGS" == "$RET" ]; then
        # There was no match
        echo ""
    else
        echo ":$RET"
    fi
}

function getLoginFromSSHCommand {
    # The login name can be given to ssh in the format "-l NAME"
    # This regular expression extracts the "NAME" from "ssh -q bla1 -l NAME bla2"
    local ARGS=$@
    local RET=$(echo "$ARGS" | sed "s/.*-l \+\([a-z,A-Z,_,0-9]*\).*/\1/")
    if [ "$RET" == "$ARGS" ] || [ "$RET" == "" ]; then
        # There was no match
        echo ""
    else
        echo "$RET@"
    fi
}

function getHostFromSSHCommand {
    # This regular expression extracts the "bla2" from "echo "ssh -q bla1 -p 4821 bla2"
    # Specifically, it finds the first argument which is not preceded by a "-x" parameter kind specification.
    
    local CLEANED=""
    local NEWCLEANED="$@"

    while ! [ "$NEWCLEANED" == "$CLEANED" ]; do
        CLEANED="$NEWCLEANED"
    # This expression removes one "-x ARG" parameter
        NEWCLEANED="$(echo $CLEANED | sed "s/\(.*\)\(-[a-z,A-Z] \+[a-z,0-9]*\)\ \(.*\)/\1\3/")"
    done

    # After cleaning, the result should only consist of the host-name followed by an optional command.
    # Select the host-name, by extracting the forst column.
    echo $CLEANED | cut --delimiter=" " -f 1
}

function getSSHForwardOptionsFromCommand {
    
    HOST="$(getLoginFromSSHCommand "$@")$(getHostFromSSHCommand "$@")$(getPortFromSSHCommand "$@")"
    
    if [ "$KDEV_SSH_FORWARD_CHAIN" ]; then
        # We are already forwarding, so we deal with a chain of multiple ssh commands.
        # We still record it, although it's not sure if we can use it somehow.
        echo "KDEV_SSH_FORWARD_CHAIN=\"$KDEV_SSH_FORWARD_CHAIN,$HOST\"";
    else
        echo "KDEV_SSH_FORWARD_CHAIN=$HOST"
    fi
}

function getDBusAbstractSocketSuffix {
    # From something like DBUS_SESSION_BUS_ADDRESS=unix:abstract=/tmp/dbus-wYmSkVH7FE,guid=b214dad39e0292a4299778d64d761a5b
    # extract the /tmp/dbus-wYmSkVH7FE
    echo $DBUS_SESSION_BUS_ADDRESS | sed 's/unix\:abstract\=.*\(,guid\=.*\)/\1/'
}

function keepForwardingDBusToTCPSocket {
    while ! $KDEV_BASEDIR/kdev_dbus_socket_transformer $DBUS_FORWARDING_TCP_LOCAL_PORT --bind-only; do
        if (($DBUS_FORWARDING_TCP_LOCAL_PORT<$DBUS_FORWARDING_TCP_MAX_LOCAL_PORT)); then
            export DBUS_FORWARDING_TCP_LOCAL_PORT=$(($DBUS_FORWARDING_TCP_LOCAL_PORT+1))
#             echo "Increased local port to " $DBUS_FORWARDING_TCP_LOCAL_PORT;
        else
            echo "Failed to allocate a local TCP port";
            return 1;
        fi
    done
        
    $KDEV_BASEDIR/kdev_dbus_socket_transformer $DBUS_FORWARDING_TCP_LOCAL_PORT&
    return 0;
}

function keepForwardingDBusFromTCPSocket {

    while ! $KDEV_BASEDIR/kdev_dbus_socket_transformer $FORWARD_DBUS_FROM_PORT ${DBUS_ABSTRACT_SOCKET_TARGET_BASE_PATH}-${DBUS_ABSTRACT_SOCKET_TARGET_INDEX} --bind-only; do
        if ((${DBUS_ABSTRACT_SOCKET_TARGET_INDEX}<${DBUS_ABSTRACT_SOCKET_MAX_TARGET_INDEX})); then
            export DBUS_ABSTRACT_SOCKET_TARGET_INDEX=$(($DBUS_ABSTRACT_SOCKET_TARGET_INDEX+1))
        else
            echo "Failed to allocate a local path for the abstract dbus socket";
            return 1;
        fi
    done
    
    local PATH=${DBUS_ABSTRACT_SOCKET_TARGET_BASE_PATH}-${DBUS_ABSTRACT_SOCKET_TARGET_INDEX}
    export DBUS_SESSION_BUS_ADDRESS=unix:abstract=$PATH${DBUS_SOCKET_SUFFIX}
    $KDEV_BASEDIR/kdev_dbus_socket_transformer $FORWARD_DBUS_FROM_PORT $PATH&
}

function ssh! {
#     echo "forwarding from $APPLICATION_HOST"
    keepForwardingDBusToTCPSocket # Should be automatically terminated when the function exits
#     echo "calling" ssh $@ -t -R localhost:$DBUS_FORWARDING_TCP_TARGET_PORT:localhost:$DBUS_FORWARDING_TCP_LOCAL_PORT \
#            "APPLICATION=$APPLICATION KDEV_BASEDIR=$KDEV_BASEDIR KDEV_DBUS_ID=$KDEV_DBUS_ID FORWARD_DBUS_FROM_PORT=$DBUS_FORWARDING_TCP_TARGET_PORT APPLICATION_HOST=$APPLICATION_HOST DBUS_SOCKET_SUFFIX=$(getDBusAbstractSocketSuffix) bash --init-file $KDEV_BASEDIR/kdevplatform_shell_environment.sh -i"
    ssh $@ -t -R localhost:$DBUS_FORWARDING_TCP_TARGET_PORT:localhost:$DBUS_FORWARDING_TCP_LOCAL_PORT \
           "APPLICATION=$APPLICATION KDEV_BASEDIR=$KDEV_BASEDIR KDEV_DBUS_ID=$KDEV_DBUS_ID FORWARD_DBUS_FROM_PORT=$DBUS_FORWARDING_TCP_TARGET_PORT APPLICATION_HOST=$APPLICATION_HOST DBUS_SOCKET_SUFFIX=$(getDBusAbstractSocketSuffix) $(getSSHForwardOptionsFromCommand "$@") bash --init-file $KDEV_BASEDIR/kdevplatform_shell_environment.sh -i"
    kill %1 # Kill the forwarding loop
}

if [ "$FORWARD_DBUS_FROM_PORT" ]; then
#     echo "Initializing DBUS forwarding to host $APPLICATION_HOST"
    export DBUS_SESSION_BUS_ADDRESS=unix:abstract=${DBUS_ABSTRACT_SOCKET_TARGET_BASE_PATH}-${DBUS_ABSTRACT_SOCKET_TARGET_INDEX}${DBUS_SOCKET_SUFFIX}
    keepForwardingDBusFromTCPSocket
fi

##### INITIALIZATION --------------------------------------------------------------------------------------------------------------------

updateShellPrompt

# SHORT_HELP="1" help!
echo "You are controlling the $APPLICATION session '$(getSessionName)'. Type help! for more information."

