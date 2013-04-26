#!/bin/bash

## We use xargs to prevent passing too many filenames to sed, but
## this command will fail if filenames contain spaces or newlines.

# Add KDEVPLATFORM_ prefix to all patterns matching [A-Z].*_H$
#
# Use carrefully and precautious review needed as it won't match
# if there's as example, trailing whitespace, #define FOO_H 1, or define FOO_H_
# It can also match some non wanted pattern like #undef HAVE_STDINT_H
# in util/google/sparsehash/sparseconfig_windows.h 
find . -name '*.h' -print | xargs sed -i 's/[A-Z].*_H$/KDEVPLATFORM_&/'
#find plugins/ -name '*.h' -print | xargs sed -i 's/[A-Z].*_H$/PLUGIN_&/'
