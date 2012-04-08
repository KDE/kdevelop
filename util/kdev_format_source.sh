#!/bin/bash

# Author: David Nolden <david.nolden.kdevelop@art-master.de>
# This script is made available under the GPLv2 licence.
#
# This script formats a given source-file automatically by
# using formatting scripts as defined in specific meta-information
# "format_sources" files contained in the file system hierarchy.
#
# The rules within a "format_sources" file apply to all subdirectories,
# and follow this syntax:
#   Each line defines a list of wildcards followed by a colon and the formatting-command.
#   Example: "*.cpp *.h : my_custom_formatting_script.sh $TMPFILE"
#
# The file must be terminated by a trailing newline.
#
# If no colon and no wildcards are given, the command is
# used for everything, equivalently to the "*" wildcard.
#
# The contents is processed in linear order, and the first matching command is used.

ORIGFILE=$1
TMPFILE=$2

if ! [ "$ORIGFILE" ]; then
    echo "Usage: $(basename $0) [FILE] [TEMPFILE]"
    echo ""
    echo "Where FILE represents the original location of the formatted contents,"
    echo "and TEMPFILE is used as the actual, potentially different,"
    echo "contents of the file."
    exit
fi

ORIGFILE=$(readlink -f $ORIGFILE)

if ! [ $TMPFILE ]; then
    echo "No tempfile given, formatting the original file"
    TMPFILE=$ORIGFILE
else
    TMPFILE=$(readlink -f $TMPFILE)
fi

# Helper: Returns the relative path from a given source directory to a target path
function relativePath {
    source=$1
    target=$2

    common_part=$source
    back=
    while [ "${target#$common_part}" = "${target}" ]; do
    common_part=$(dirname $common_part)
    back="../${back}"
    done

    echo ${back}${target#$common_part/}
}

# Go to the directory of the original file, and start searching for "format_sources" files upwards
cd $(dirname $ORIGFILE)

while ! [ "$(pwd)" == "/" ]; do

    if [ -e "format_sources" ]; then
        echo "found $(readlink -f format_sources)"
        
        # Read line by line
        while read line
        do
#             echo "Line: $line"
            # Split by the ":" which is the delimiter between wildcards
            IFS="\:"
            array=
            pos=0
            
            # remove leading whitespace
            line="${line#"${line%%[![:space:]]*}"}"

            if [[ "$line" == \#* ]] || ! [ "$line" ]; then 
                # Ignore lines starting with #
                # Those can be used for comments.
                # Also ignore empty lines
                continue
            fi
            
            for item in $line;
            do
                array[$pos]=$item
                pos=$(($pos+1))
            done
            
            if [ $pos == "2" ]; then
                # We found the correct syntax with "wildcards : command"
                WILDCARDS=${array[0]}
                COMMAND=${array[1]}
                
                MATCHED=0
                
#                 echo "wildcards: $WILDCARDS"
                
                RELATIVE_ORIGFILE=$(relativePath $(pwd) $ORIGFILE)
#                 echo "relative path: $RELATIVE_ORIGFILE"
                
                IFS=" "
                set -f # This disables the wildcard expansion, we don't want it
                for WILDCARD in $WILDCARDS; do
                    set +f
                    # This if-command does wildcard matching
#                     echo "matching $RELATIVE_ORIGFILE and $WILDCARD"
                    if [[ "$RELATIVE_ORIGFILE" == $WILDCARD ]]; then
                        echo "matched $RELATIVE_ORIGFILE with wildcard $WILDCARD, using command \"$COMMAND\""
                        eval $COMMAND
                        exit
                    fi
                    set -f
                done
                set +f
            fi
            
            if [ $pos == "1" ]; then
                # We found the simple syntax without wildcards, and only with the command
                COMMAND=${array[0]}
                echo "matched without wildcard, using command $COMMAND"
                eval $COMMAND
                exit
            fi

        done < format_sources
    fi
    
    
    cd ..
done
