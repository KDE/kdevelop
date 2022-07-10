#!/bin/bash
#
# build KDevelop with -fsanitize=address,undefined and then source this script
#

kdev_source_dir=$(readlink -f $(dirname ${BASH_SOURCE[0]}))

# optionally set fast_unwind_on_malloc=false to get full backtraces even when framepointers are missing
# but that is very slow
export LSAN_OPTIONS=print_suppressions=0,suppressions=$kdev_source_dir/lsan.supp,fast_unwind_on_malloc=true

export UBSAN_OPTIONS=print_stacktrace=1,suppressions=$kdev_source_dir/ubsan.supp

# there seems to be a funky clash with sigaltstack usage in llvm, so disable it for the sanitizers
# see also: https://github.com/google/sanitizers/issues/849
export ASAN_OPTIONS=detect_stack_use_after_return=1,use_sigaltstack=0
