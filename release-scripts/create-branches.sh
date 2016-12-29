#!/usr/bin/env fish
set newbranch 5.1
for repo in kdevelop kdevplatform kdev-python kdev-php;
	cs $repo
	git checkout master
	git pull --rebase
	git checkout -b $newbranch
	git push origin $newbranch
end
