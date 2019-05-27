#   Bash completion script for KDevelop
#
#   Copyright 2019 Kevin Funk <kfunk@kde.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU Library General Public License as
#   published by the Free Software Foundation; either version 2 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public
#   License along with this program; if not, write to the
#   Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


# Usage:
#
# Under Bash:
#   source kdevelop_completions.bash
#
# Under ZSH:
#   autoload bashcompinit
#   bashcompinit
#   source kdevelop_completions.bash
#
#   (cf. https://stackoverflow.com/questions/3249432/can-a-bash-tab-completion-script-be-used-in-zsh)

_kdevelop_completions()
{
    local cur prev

    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    case $prev in
        -s)
            local IFS=$'\n'
            CANDIDATES=(
                $(compgen -W \
                    "$(kdevelop --list-sessions 2>/dev/null | egrep '^{' | cut -f2 | sed 's/\s*\[running\]//;s/\(.*\):.*/\1/;s/ /\\ /g')" \
                    -- "${cur}"
                )
            )

            # Need to jump through hoops to properly handle spaces in results, cf. https://stackoverflow.com/a/11536437/592636
            # Correctly set our candidates to COMPREPLY
            if [ ${#CANDIDATES[*]} -eq 0 ]; then
                COMPREPLY=()
            else
                COMPREPLY=($(printf '%q\n' "${CANDIDATES[@]}"))
            fi
            ;;
    esac
}

complete -F _kdevelop_completions kdevelop

# ex: filetype=sh
