#!/bin/bash

set -x

# load ini file
source <(grep = VERSIONS.ini)

do_replace() {
    local project=$1

    echo $MINOR_VERSION

    git checkout $BRANCH
    sed -i -e "s/set(.*${project}_VERSION_MAJOR .*)/set(${project}_VERSION_MAJOR $MAJOR_VERSION)/g" CMakeLists.txt
    sed -i -e "s/set(.*${project}_VERSION_MINOR .*)/set(${project}_VERSION_MINOR $MINOR_VERSION)/g" CMakeLists.txt
    sed -i -e "s/set(.*${project}_VERSION_PATCH .*)/set(${project}_VERSION_PATCH $PATCH_VERSION)/g" CMakeLists.txt
    git diff CMakeLists.txt
    git commit CMakeLists.txt -m "Update version number to $MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION"
    git push origin $BRANCH
}

(cd kdevplatform; do_replace KDEVPLATFORM)
(cd kdevelop; do_replace KDEVELOP)
(cd kdev-python; do_replace KDEVPYTHON)
(cd kdev-php; do_replace KDEVPHP)
