#   Bash completion script for KDevelop
#
#   SPDX-FileCopyrightText: 2019 Kevin Funk <kfunk@kde.org>
#
#   SPDX-License-Identifier: LGPL-2.0-or-later


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
                    "$(kdevelop --list-sessions 2>/dev/null | grep -E '^{' | cut -f2 | sed 's/\s*\[running\]//;s/\(.*\):.*/\1/;s/ /\\ /g')" \
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
