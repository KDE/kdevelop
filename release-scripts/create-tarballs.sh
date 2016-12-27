#!/usr/bin/env fish

set VERSION 5.0.80

# git://anongit.kde.org/releaseme
cs releaseme
for application in kdev-python kdev-php kdevplatform kdevelop
	./tarme.rb --version $VERSION --origin stable $application 
	./tagme.rb --version $VERSION
end
