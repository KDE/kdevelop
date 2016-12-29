#!/usr/bin/env fish

set MAJOR_VERSION 5
set MINOR_VERSION 0
set PATCH_VERSION 80
set BRANCH 5.1

function do_replace
        git checkout $BRANCH
	sed -i "s/set(.*$argv""_VERSION_MAJOR .*)/set($argv""_VERSION_MAJOR $MAJOR_VERSION)/g" CMakeLists.txt
	sed -i "s/set(.*$argv""_VERSION_MINOR .*)/set($argv""_VERSION_MINOR $MINOR_VERSION)/g" CMakeLists.txt
	sed -i "s/set(.*$argv""_VERSION_PATCH .*)/set($argv""_VERSION_PATCH $PATCH_VERSION)/g" CMakeLists.txt
        git diff
        git commit -a -m "Update version number to $MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION"
        git push --set-upstream origin $BRANCH
end

cs kdev-python
do_replace KDEVPYTHON 
cs kdev-php
do_replace KDEVPHP
cs kdevelop
do_replace KDEVELOP
cs kdevplatform
do_replace KDEVPLATFORM

