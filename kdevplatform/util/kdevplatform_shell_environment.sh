#!/bin/sh

# SPDX-FileCopyrightText: 2011 David Nolden <david.nolden.kdevelop@art-master.de>
#
# SPDX-License-Identifier: LGPL-2.0-or-later

_shell=$(ps -cp $$ -o command="")

# Since this runs as a replacement for the init-files, we need to chain in the 'real' rcs.
# We ignore profile, login & logout rcs, as we want no login shells.
case $_shell in
    zsh)
        alias shopt=':'
        test -f "$OLD_ZDOTDIR/.zshenv" && . "$OLD_ZDOTDIR/.zshenv"
        test -f "$OLD_ZDOTDIR/.zshrc"  && . "$OLD_ZDOTDIR/.zshrc"
        ;; #zsh still also sources the systemwide rcs when called with $ZDOTDIR set.
    bash)
        test -f ~/.bash_profile  && source ~/.bash_profile
        test -f /etc/bash.bashrc && source /etc/bash.bashrc
        test -f ~/.bashrc && source ~/.bashrc
        ;;
esac

if ! [ "$APPLICATION_HOST" ]; then
    export APPLICATION_HOST=$(hostname)
fi

if ! [ "$KDEV_SHELL_ENVIRONMENT_ID" ]; then
    export KDEV_SHELL_ENVIRONMENT_ID="default"
fi

if ! [ "$KDEV_DBUS_ID" ]; then
    echo "The required environment variable KDEV_DBUS_ID is not set. This variable defines the dbus id of the application instance which is supposed to be attached."
    exit 5
fi

# Eventually, if we are forwarding to another host, and kdevplatform_shell_environment.sh
# has been located through "which kdevplatform_shell_environment.sh", then we need to update KDEV_BASEDIR.
if ! [ -e "$KDEV_BASEDIR/kdevplatform_shell_environment.sh" ]; then
    KDEV_BASEDIR=$(dirname $(which kdevplatform_shell_environment.sh))
fi

if ! [ -e "$KDEV_BASEDIR/kdev_dbus_socket_transformer" ]; then
    echo "The $KDEV_BASEDIR/kdev_dbus_socket_transformer utility is missing, controlling the application across ssh is not possible"
fi

# Takes a list of tools, and prints a warning of one of them is not available in the path
function checkToolsInPath {
    for TOOL in $@; do
        command -v $TOOL &> /dev/null || echo "The utility $TOOL is not in your path, the shell integration will not work properly."
    done
}

# Check if all required tools are there (on the host machine)
checkToolsInPath sed ls cut dirname mktemp basename readlink hostname

# special handling for qdbus variants
_qdbus=qdbus-qt5
if ! command -v $_qdbus &> /dev/null; then
    _qdbus=qdbus
    if ! command -v $_qdbus &> /dev/null; then
        echo "The utility qdbus (or qdbus-qt5) is not in your path, the shell integration will not work properly."
    fi
fi

if ! [ "$KDEV_SSH_FORWARD_CHAIN" ]; then
    # Check for additional utilities that are required on the client machine
    checkToolsInPath kioclient5
fi

# Queries the session name from the running application instance
function getSessionName {
    echo "$($_qdbus $KDEV_DBUS_ID /org/kdevelop/SessionController org.kdevelop.SessionController.sessionName)"
}

function getSessionDir {
    echo "$($_qdbus $KDEV_DBUS_ID /org/kdevelop/SessionController org.kdevelop.SessionController.sessionDir)"
}

function getCurrentShellEnvPath {
    local ENV_ID=$KDEV_SHELL_ENVIRONMENT_ID
    if [ "$1" ]; then
        ENV_ID=$1
    fi

    echo "$(getSessionDir)/${ENV_ID}.sh"
}

function help! {
    echo "You are controlling the $APPLICATION session '$(getSessionName)'"
    echo ""
    if [[ "$1" == "" ]]; then
    echo "Standard commands:"
    echo "raise!                                 - Raise the window."
    echo "sync!                                  - Synchronize the working directory with the currently open document. See \"help! sync\""
    echo "open!   [file] ...                     - Open the file(s) within the attached application. See \"help! open\""
    echo "eopen!  [file] ...                     - Open the file(s) within an external application using kde-open."
    echo "create!  [file] [[text]]               - Create and open a new file."
    echo "search!   [pattern] [[locations]] ...  - Search for the given pattern here or at the optionally given location(s)."
    echo "dsearch!  [pattern] [[locations]] ...  - Same as search, but starts the search instantly instead of showing the dialog (using previous settings)."
    echo "ssh!  [ssh arguments]                  - Connect to a remote host via ssh, keeping the control-connection alive. See \"help! remote\""
    echo ""
    echo "help!                                  - Show help."
    echo "help! open                             - Show extended help about file opening commands."
    echo "help! sync                             - Show extended help about path synchronization commands."
    echo "help! remote                           - Show extended help about remote shell-integration through ssh."
    echo "help! env                              - Show extended help about the environment."
    echo ""
    echo "Most commands can be abbreviated by the first character(s), eg. r! instead of raise!, and se! instead of search!."
    fi

    if [[ "$1" == "open" ]]; then
    echo "Extended opening:"
    echo "The open! command can also be used to open files in specific tool-view configurations, by adding split-separators:"
    echo "- Files around the / separator will be arranged horizontally by split-view."
    echo "- Files around the - separator will be arranged vertically by split-view."
    echo "- Parens [ ... ] can be used to disambiguate the hierarchy (there must be spaces between filename and paren)."
    echo "- If a file is missing around a separator, the currently active view is inserted into the position."
    echo ""
    echo "Examples:"
    echo "open! file1 / file2                 - The active view is split horizontally."
    echo "                                      file1 is opened in the left view, and file2 in the right view."
    echo "open! file1 / [ file2 - file3 ]     - The active view is split horizontally, and the right split-view is split vertically."
    echo "                                      file1 is opened in the left view, file2 in the right upper view, and file3 in the right lower view."
    echo "open! / file1                       - The active view is split horizontally."
    echo "                                    - The active document is kept in the left split-view, and file1 is opened in the right split-view."
    echo ""
    echo "Short forms: o! = open!, eo! = eopen!, c! = create!"
    fi

    if [[ "$1" == "sync" ]]; then
    echo "Extended syncing:"
    echo "sync!    [[project-name]]           - If no project-name is given, then the sync! command synchronizes to the currently active document."
    echo "                                      If no document is active, then it synchronizes to the currently selected item in the project tree-view."
    echo "                                      If a case-insensitive project name prefix is given, then it synchronizes to the base folder of the matching project."
    echo "syncsel!                            - Synchronizes to the currently selected item in the project tree-view, independently of the active document."
    echo "project! [[project-name]]           - Map from a path within the build directory to the corresponding path in the source directory."
    echo "                                      If we're already in the source directory, map to the root of the surrounding project."
    echo "bdir!    [[project-name]]           - Map from a path within the source directory to the corresponding path in the build directory."
    echo "                                      If we're already in the build directory, map to the root of the build directory."
    echo ""
    echo "Short forms: s! = sync!, ss! = syncsel!, p! = project!, b! = bdir!"
    fi

    if [[ "$1" == "remote" ]]; then
    echo "Extended remote commands:"
    echo "ssh!  [ssh arguments]                  - Connect to a remote host via ssh, keeping the control-connection alive."
    echo "                                       - The whole dbus environment is forwarded, KDevelop needs to be installed on both sides."
    echo "ssw!  [ssh arguments]                  - Like ssh!, but preserves the current working directory."
    echo "exec! [cmd] [args] [file] . ..         - Execute the given command on the client machine, referencing any number of local files on the host machine."
    echo "                                       - The file paths will be re-encoded as fish:// urls if required."
    echo "cexec! [cmd] [args] [file] . ..        - Execute the given command on the client machine, referencing any number of local files on the host machine."
    echo "                                       - The files will be COPIED to the client machine if required."
    echo "copytohost! [client path] [host path]  - Copy a file/directory through the fish protocol from the client machine th the host machine."
    echo "copytoclient! [host path] [client path]- Copy a file/directory through the fish protocol from the host machine to the client machine."
    echo ""
    echo "Short forms: e! = exec!, ce! = cexec!, cth! = copytohost!, ctc! = copytoclient!"
    fi

    if [[ "$1" == "env" ]]; then
      echo "Environment management:"
      echo "The environment can be used to store session-specific macros and generally manipulate the shell environment"
      echo "for embedded shell sessions. The environment is sourced into the shell when the shell is initialized, and"
      echo "whenever setenv! is called."
      echo ""
      echo "env!                                 - List all available shell environment-ids for this session."
      echo "setenv! [id]                         - Set the shell environmnet-id for this session to the given id, or update the current one."
      echo "editenv! [id]                        - Edit the current shell environment or the one with the optionally given id."
      echo "showenv! [id]                        - Show the current shell environment or the one with the optionally given id."
      echo ""
      echo "Short forms: sev! = setenv!, ee! = editenv!, shenv! = showenv!"
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

function ss! {
    syncsel!
}

function syncsel! {
    sync! '[selection]'
}

function p! {
    if [ "$@" ]; then
        s! $@
    fi
    project!
}

function b! {
    if [ "$@" ]; then
        s! $@
    fi
    bdir!
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

function cth! {
    copytohost! $@
}

function ctc! {
    copytoclient! $@
}

function sev! {
    setenv! $@
}

function ee! {
    editenv! $@
}

function shev! {
    showenv! $@
}

# Internals:

# Opens a document in internally in the application
function openDocument {
    RESULT=$($_qdbus $KDEV_DBUS_ID /org/kdevelop/DocumentController org.kdevelop.DocumentController.openDocumentSimple $1)
    if [[ "$RESULT" != "true" ]]; then
        echo "Failed to open $1"
    fi
}

# Opens a document in internally in the application
function openDocuments {
    if [[ $_shell == "zsh" ]]; then
        arr=(${=1})
    else
        arr=("$1")
    fi
    RESULT=$($_qdbus $KDEV_DBUS_ID /org/kdevelop/DocumentController org.kdevelop.DocumentController.openDocumentsSimple "(" $arr ")")
    if [[ "$RESULT" != "true" ]]; then
        echo "Failed to open $1"
    fi
}

# Executes a command on the client machine using the custom-script integration.
# First argument: The full command. Second argument: The working directory.
function executeInApp {
    local CMD="$1"
    local WD=$2
    if ! [ "$WD" ]; then
        WD=$(pwd)
    fi
    RESULT=$($_qdbus $KDEV_DBUS_ID /org/kdevelop/ExternalScriptPlugin org.kdevelop.ExternalScriptPlugin.executeCommand "$CMD" "$WD")
    if [[ "$RESULT" != "true" ]]; then
        echo "Execution failed"
    fi
}

# First argument: The full command. Second argument: The working directory.
# Executes the command silently and synchronously, and returns the output
function executeInAppSync {
    local CMD=$1
    local WD=$2
    if ! [ "$WD" ]; then
        WD=$(pwd)
    fi
    RESULT=$($_qdbus $KDEV_DBUS_ID /org/kdevelop/ExternalScriptPlugin org.kdevelop.ExternalScriptPlugin.executeCommandSync "$CMD" "$WD")
    echo "$RESULT"
}

# Getter functions:

function getActiveDocument {
    $_qdbus $KDEV_DBUS_ID /org/kdevelop/DocumentController org.kdevelop.DocumentController.activeDocumentPath $@
}

function getOpenDocuments {
    $_qdbus $KDEV_DBUS_ID /org/kdevelop/DocumentController org.kdevelop.DocumentController.activeDocumentPaths
}

function bdir! {
    TARG=$($_qdbus $KDEV_DBUS_ID /org/kdevelop/ProjectController org.kdevelop.ProjectController.mapSourceBuild "$(pwd)" false)
    if [ "$TARG" ]; then
        cd $TARG
    else
        echo "Got no path"
    fi
}

function project! {
    TARG=$($_qdbus $KDEV_DBUS_ID /org/kdevelop/ProjectController org.kdevelop.ProjectController.mapSourceBuild "$(pwd)" true)
    if [ "$TARG" ]; then
        cd $TARG
    else
        echo "Got no path"
    fi
}


# Main functions:

function raise! {
    $_qdbus $KDEV_DBUS_ID /kdevelop/MainWindow org.kdevelop.MainWindow.ensureVisible
}

function sync! {
    local P=$(getActiveDocument $@)
    if [ "$P" ]; then

        if [[ "$P" == fish://* ]]; then
            # This regular expression filters the user@host:port out of fish:///user@host:port/path/...
            LOGIN=$(echo $P | sed "s/fish\:\/\/*\([^\/]*\)\(\/.*\)/\1/")
            P_ON_HOST=$(echo $P | sed "s/fish\:\/\/*\([^\/]*\)\(\/.*\)/\2/")
            if [[ "$KDEV_SSH_FORWARD_CHAIN" == "$LOGIN" ]]; then
                P="$P_ON_HOST"
            else
                if [[ "$KDEV_SSH_FORWARD_CHAIN" == "" ]]; then
                    # Try to ssh to the host machine
                    # We need to split away the optional ":port" suffix, because the ssh command does not allow that syntax
                    HOST=$(echo $LOGIN | cut --delimiter=':' -f 1)

                    CMD="ssh!"

                    if [[ "$LOGIN" == *:* ]]; then
                        # If there is a port, extract it
                        PORT=$(echo $LOGIN | cut --delimiter=':' -f 2)
                        CMD="$CMD -p $PORT"
                    fi

                    CMD="$CMD $HOST"
                    # Execute the ssh command
                    echo "Executing $CMD"
                    KDEV_WORKING_DIR="$(dirname $P_ON_HOST)"
                    $CMD
                    return
                else
                    echo "Cannot synchronize the working directory, because the host-names do not match (app: $LOGIN, shell: $KDEV_SSH_FORWARD_CHAIN)"
                    return
                fi
            fi

        elif [[ "$P" == file://* ]]; then
            P=$(echo $P | sed 's$^file://$$')

        elif [ "$KDEV_SSH_FORWARD_CHAIN" ]; then
            # This session is being forwarded to another machine, but the current document is not
            # However, we won't complain, because it's possible that the machines share the same file-system
            if [[ $(isEqualFileOnHostAndClient $P) != "yes" ]]; then
                echo "Cannot synchronize the working directory, because the file systems do not match"
                return
            fi
        fi

        [ -d "$P" ] || P=$(dirname "$P")
        cd "$P"
    else
        echo "Got no path"
    fi
}

# Take a path, and returns "yes" if the equal file is available on the host and the client
# The check is performed by comparing inode-numbers
function isEqualFileOnHostAndClient {
    function trimWhiteSpace() {
        echo $1
    }

    FILE=$1
    INODE_HOST=$(trimWhiteSpace $(ls --color=never -i $FILE | cut -d' ' -f1))
    INODE_CLIENT=$(trimWhiteSpace $(executeInAppSync "ls --color=never -i $FILE | cut -d' ' -f1" "$(dirname $FILE)"))
    if [[ "$INODE_HOST" == "$INODE_CLIENT" ]]; then
        echo "yes"
    else
        echo ""
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

        if [ "$KDEV_SSH_FORWARD_CHAIN" ]; then
            # If we are forwarding, map it to the client somehow.
            if [[ "$(isEqualFileOnHostAndClient "$FILE")" != "yes" ]]; then
                    # We can eventually map the file using the fish protocol
                    FISH_HOST=$KDEV_SSH_FORWARD_CHAIN
                    if [[ "$FISH_HOST" == *\,* ]]; then
                        # Extracts everything before the first comma
                        FISH_HOST=$(echo $FISH_HOST | sed 's/\([^,]*\),\(.*\)/\1/')
                        echo "ssh chain is too long: $KDEV_SSH_FORWARD_CHAIN mapping anyway using $FISH_HOST" 1>&2
                    fi
                    # Theoretically, we can only map through fish if the forward-chains contains no comma, which means that
                    # we forward only once. Try anyway, there might be the same filesystem on the whole forward-chain.
                    FILE="fish://$FISH_HOST$FILE"
            fi
        fi
    fi
    echo $FILE
}

function open! {
    NEWFILES=""
    for RELATIVE_FILE; do
        if [[ "$RELATIVE_FILE" == "/" || "$RELATIVE_FILE" == "-" ]]; then
            FILE=$RELATIVE_FILE
        else
            FILE=$(mapFileToClient $RELATIVE_FILE)
        fi
        NEWFILES="$NEWFILES $FILE"
    done

    openDocuments "$NEWFILES"
}

function eopen! {
    for RELATIVE_FILE; do
        FILE=$(mapFileToClient $RELATIVE_FILE)
        executeInApp "kde-open5 $FILE"
    done
}

function exec! {
    ARGS=""
    for RELATIVE_FILE; do
        if [[ "$ARGS" == "" ]]; then
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

function copytohost! {
    executeInApp "kioclient5 copy $1 $(mapFileToClient $2)"
}

function copytoclient! {
    executeInApp "kioclient5 copy $(mapFileToClient $1) $2"
}

function cexec! {
    ARGS=""
    PREFIX=""
    TMP=1
    for RELATIVE_FILE; do
        if [[ "$ARGS" == "" ]]; then
            # Do not transform the command-name
            ARGS=$RELATIVE_FILE
        else
            FILE=$(mapFileToClient $RELATIVE_FILE)

            if [[ "$FILE" == fish://* ]]; then
                # Add a prefix to copy the file into a temporary file
                # Keep the baseline as suffix, so that applications can easily recognize the mimetype
                PREFIX+="FILE$TMP=\$(mktemp).$(basename $FILE); kioclient5 copy $FILE \$FILE$TMP;"
                # Use the temporary variable instead of the name
                FILE="\$FILE$TMP"
                TMP=$(($TMP+1))
            fi

            ARGS=$ARGS" "$FILE
        fi
    done
    echo "Executing: " $ARGS
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

    openDocument $(mapFileToClient $FILE)
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

    LOCATION=$(mapFileToClient $LOCATION)

    for LOC in $*; do
        if [[ "$LOC" == "$1" ]]; then
            continue;
        fi
        if [[ "$LOC" == "$2" ]]; then
            continue;
        fi
        LOCATION="$LOCATION;$(mapFileToClient $LOC)"
    done

    $_qdbus $KDEV_DBUS_ID /org/kdevelop/GrepViewPlugin org.kdevelop.GrepViewPlugin.startSearch "$PATTERN" "$LOCATION" true
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

    LOCATION=$(mapFileToClient $LOCATION)

    for LOC in $*; do
        if [[ "$LOC" == "$1" ]]; then
            continue;
        fi
        if [[ "$LOC" == "$2" ]]; then
            continue;
        fi
        LOCATION="$LOCATION;$(mapFileToClient $LOC)"
    done

    $_qdbus $KDEV_DBUS_ID /org/kdevelop/GrepViewPlugin org.kdevelop.GrepViewPlugin.startSearch "$PATTERN" "$LOCATION" false
}

##### SSH DBUS FORWARDING --------------------------------------------------------------------------------------------------------------------

DBUS_SOCKET_TRANSFORMER=$KDEV_BASEDIR/kdev_dbus_socket_transformer

# We need this, to make sure that our forwarding-loops won't get out of control
# This configures the shell to kill background jobs when it is terminated
shopt -s huponexit

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
    if [[ "$ARGS" == "$RET" ]]; then
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
    if [[ "$RET" == "$ARGS"  ||  "$RET" == "" ]]; then
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

    while [[ "$NEWCLEANED" != "$CLEANED" ]]; do
        CLEANED="$NEWCLEANED"
    # This expression removes one "-x ARG" parameter
        NEWCLEANED="$(echo $CLEANED | sed "s/\(.*\)\(-[a-z,A-Z] \+[a-z,0-9]*\)\ \(.*\)/\1\3/")"
    done

    # After cleaning, the result should only consist of the host-name followed by an optional command.
    # Select the host-name, by extracting the first column.
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
    keepForwardingDBusToTCPSocket # Start the dbus forwarding subprocess
    DBUS_FORWARDING_TCP_TARGET_PORT=$((5000+($RANDOM%50000)))

    ssh $@ -t -R localhost:$DBUS_FORWARDING_TCP_TARGET_PORT:localhost:$DBUS_FORWARDING_TCP_LOCAL_PORT \
         " APPLICATION=$APPLICATION \
           KDEV_BASEDIR=$KDEV_BASEDIR \
           KDEV_DBUS_ID=$KDEV_DBUS_ID \
           FORWARD_DBUS_FROM_PORT=$DBUS_FORWARDING_TCP_TARGET_PORT \
           APPLICATION_HOST=$APPLICATION_HOST \
           KDEV_WORKING_DIR=$KDEV_WORKING_DIR \
           KDEV_SHELL_ENVIRONMENT_ID=$KDEV_SHELL_ENVIRONMENT_ID \
           DBUS_SOCKET_SUFFIX=$(getDBusAbstractSocketSuffix) \
           $(getSSHForwardOptionsFromCommand "$@") \
              bash --init-file \
                        \$(if [ -e \"$KDEV_BASEDIR/kdevplatform_shell_environment.sh\" ]; \
                                then echo \"$KDEV_BASEDIR/kdevplatform_shell_environment.sh\"; \
                           elif [ -e \"$(which kdevplatform_shell_environment.sh)\" ]; then
                                echo \"$(which kdevplatform_shell_environment.sh)\"; \
                           else \
                                echo \"~/.kdevplatform_shell_environment.sh\"; \
                           fi) \
                   -i"



    if [ "$FORWARD_DBUS_FROM_PORT" ]; then
        # We created the 2nd subprocess
        kill %2 # Stop the dbus forwarding subprocess
    else
        # We created the 1st subprocess
        kill %1 # Stop the dbus forwarding subprocess
    fi
}

# A version of ssh! that preserves the current working directory
function ssw! {
    KDEV_WORKING_DIR=$(pwd)
    ssh! $@
}

function env! {
    FILES="$(executeInAppSync "ls $(getSessionDir)/*.sh" "")"
    for FILE in $FILES; do
        FILE=$(basename $FILE)
        ID=${FILE%.sh} # This ugly construct strips away the .sh suffix
        if [[ "$ID" == "$KDEV_SHELL_ENVIRONMENT_ID" ]]; then
            echo "$ID   [current]"
        else
            echo "$ID"
        fi
    done
}

function editenv! {
    local ENV_ID=$KDEV_SHELL_ENVIRONMENT_ID
    if [ "$1" ]; then
        ENV_ID=$1
    fi
    # If the environment-file doesn't exist yet, create it
    executeInAppSync "if ! [ -e $(getCurrentShellEnvPath $ENV_ID) ]; then touch $(getCurrentShellEnvPath $ENV_ID); fi" ""
    # Open it
    openDocument "$(getCurrentShellEnvPath $ENV_ID)"
}

function setenv! {
    if [ "$1" ]; then
        KDEV_SHELL_ENVIRONMENT_ID=$1
    fi

    # Execute the contents of the shell-environment
    # note: keep compatible with FreeBSD: https://bugs.kde.org/show_bug.cgi?id=311186
    local TEMP=$(mktemp /tmp/$USER-XXXXXXXX)
    RESULT=$(executeInAppSync "cat \"$(getCurrentShellEnvPath)\"" "")
    echo "$RESULT" >| $TEMP
    if ! [ "$RESULT" ]; then
        # If the environment shell file doesn't exist, create it
        executeInAppSync "if ! [ -e $(getCurrentShellEnvPath) ]; then touch $(getCurrentShellEnvPath); fi" ""
    fi
    source $TEMP
    rm -f $TEMP
}

function showenv! {
    local ENV_ID=$KDEV_SHELL_ENVIRONMENT_ID
    if [ "$1" ]; then
        ENV_ID=$1
    fi

    echo "Environment $ENV_ID:"

    # Execute the contents of the shell-environment
    echo $(executeInAppSync "cat \"$(getCurrentShellEnvPath $ENV_ID)\"" "")
}

if [ "$FORWARD_DBUS_FROM_PORT" ]; then
    # Start the target-side dbus forwarding, transforming from the ssh pipe to the abstract unix domain socket
    export DBUS_SESSION_BUS_ADDRESS=unix:abstract=${DBUS_ABSTRACT_SOCKET_TARGET_BASE_PATH}-${DBUS_ABSTRACT_SOCKET_TARGET_INDEX}${DBUS_SOCKET_SUFFIX}
    keepForwardingDBusFromTCPSocket
fi

setenv!

##### INITIALIZATION --------------------------------------------------------------------------------------------------------------------

# Mark that this session is attached, by prepending a '!' character
PS1="!$PS1"

echo "You are controlling the $APPLICATION session '$(getSessionName)'. Type help! for more information."

if [ "$KDEV_WORKING_DIR" ]; then
    cd $KDEV_WORKING_DIR
fi
