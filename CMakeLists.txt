PROJECT(kdevelop)

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/KDE.cmake)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/KDEMacros.cmake)

SET(CMAKE_SKIP_RULE_DEPENDENCY 1)
SET(CMAKE_VERBOSE_MAKEFILE ON)

#create config.h and do other system checks
INCLUDE(CheckSystem.cmake)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib/interfaces ${CMAKE_CURRENT_SOURCE_DIR}/lib/util)

EXEC_PROGRAM(cmake ARGS -E copy ${CMAKE_SOURCE_DIR}/admin/install-sh ${CMAKE_SOURCE_DIR})
EXEC_PROGRAM(cmake ARGS -E copy ${CMAKE_SOURCE_DIR}/admin/config.sub ${CMAKE_SOURCE_DIR})
EXEC_PROGRAM(cmake ARGS -E copy ${CMAKE_SOURCE_DIR}/admin/config.guess ${CMAKE_SOURCE_DIR})

SUBDIRS( bdb lib parts languages buildtools editors vcs src)
# pics doc mimetypes embedded kdevdesigner )



#original Makefile.am contents follow:

#SUBDIRS = bdb lib parts languages buildtools editors vcs src pics doc mimetypes embedded kdevdesigner
#
#EXTRA_DIST = AUTHORS COPYING COPYING.LIB NEWS ChangeLog INSTALL README TODO HACKING HACKING.language admin configure.in.bot
#
#xdg_apps_DATA = kdevelop.desktop kdevassistant.desktop kdevelop_ruby.desktop \
#	kdevelop_c_cpp.desktop  kdevelop_kde_cpp.desktop  kdevelop_scripting.desktop
##appsdir = $(kde_appsdir)/Development
#
#xdg_directory_DATA = kde-development-kdevelop.directory
#
#mime_DATA = x-kdevelop.desktop
#mimedir = $(kde_mimedir)/application
#
#kde_bin_SCRIPTS = kdevprj2kdevelop
#
#install-data-local:
#	@echo ""
#	@echo ""
#	@echo "******************* Important *********************"
#	@echo ""
#	@echo "KDevelop $(VERSION) has been installed."
#	@echo ""
#	@echo "***************************************************"
#
#$(top_srcdir)/acinclude.m4: $(top_srcdir)/kdevelop.m4.in
#
#MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files
#
#package-messages:
#	$(MAKE) -f $(top_srcdir)/admin/Makefile.common package-messages
#	$(MAKE) -C po merge
#
#dist-hook:
#	cd $(top_distdir) && perl admin/am_edit -padmin
#	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
#
#messages:
#	find . -type d | grep -v CVS | sed -e 's,$$,/,' > dirs
#	mfs=`find . -name Makefile.am | xargs egrep -l '^messages:'` ;\
#	for dir in $$mfs; do \
#	  if test ! "$$dir" = "./Makefile.am"; then \
#	    dir=`dirname $$dir` ;\
#	    egrep -v "^$$dir" dirs > dirs.new && mv dirs.new dirs ;\
#	  fi ;\
#	done
#	dirs=`cat dirs` ;\
#	find $$dirs -maxdepth 1 -name "*.cpp" -print > files ;\
#	find $$dirs -maxdepth 1 -name "*.cc" -print >> files ;\
#	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.rc"` > rc.cpp	;\
#	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.ui"` >> rc.cpp    ;\
#	$(XGETTEXT) rc.cpp `cat files` -o $(podir)/kdevelop.pot
#	@rm -f rc.cpp dirs files
#
#$(top_srcdir)/Makefile.am.in: # faking
#include admin/deps.am
#
#
###
### Doxygen stuff
###
#
##DOXYGEN_INPUT = src/profileengine lib buildtools/projectmanager/interfaces doc/api
##DOXYGEN_INPUT = lib/interfaces
#
### Exclude the application wizard templates, and some file templates
##DOXYGEN_EXCLUDE = languages/ada/app_templates languages/cpp/app_templates languages/cpp/file_templates languages/cpp/subclassing_template languages/fortran/app_templates languages/haskell/app_templates languages/java/app_templates languages/pascal/app_templates languages/perl/app_templates languages/php/app_templates languages/python/app_templates languages/ruby/app_templates languages/sql/app_templates *face_skel.cpp *.ui.h lib/qextmdi vcs/cvsclient
#
##DOXYGEN_EXAMPLE_PATH = languages/cpp/app_templates/khello languages/cpp/app_templates/kdevpart
#
### Use KDE Libs tag files
##DOXYGEN_LINK_WITH_KDE_QT_APIDOX = YES
##DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
#
### Use Search engine (Versions 1.3.4 and above only!)
#DOXYGEN_SEARCHENGINE = YES
#DOXYGEN_EMPTY = YES
#DOXYGEN_PROJECTNAME = KDevelop Platform API
#include Doxyfile.am
