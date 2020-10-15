#!/bin/bash
#
# build KDevelop with -fsanitize=address,undefined and then source this script
#

kdev_source_dir=$(readlink -f $(dirname ${BASH_SOURCE[0]}))

# optionally set fast_unwind_on_malloc=false to get full backtraces even when framepointers are missing
# but that is very slow
export LSAN_OPTIONS=print_suppressions=0,suppressions=$kdev_source_dir/lsan.supp,fast_unwind_on_malloc=true

export UBSAN_OPTIONS=print_stacktrace=1,suppressions=$kdev_source_dir/ubsan.supp

export ASAN_OPTIONS=detect_stack_use_after_return=1
