SUBDIRS = lib parts plugins src pics doc mimetypes

EXTRA_DIST = AUTHORS COPYING COPYING.LIB NEWS ChangeLog INSTALL README TODO HACKING HACKING.language admin configure.in.bot

apps_DATA = gideon.desktop
appsdir = $(kde_appsdir)/Development

mime_DATA = x-kdevelop.desktop
mimedir = $(kde_mimedir)/application

kde_bin_SCRIPTS = kdevprj2kdevelop

install-data-local:
	@echo ""
	@echo ""
	@echo "******************* Important *********************"
	@echo ""
	@echo "KDevelop 3.0 alpha7 has been installed."
	@echo ""
	@echo "Note: The binary name for the alpha releases is"
	@echo "      'gideon'.  This will revert to 'kdevelop'"
	@echo "      once 3.0 is considered stable."
	@echo ""
	@echo "***************************************************"

$(top_srcdir)/acinclude.m4: $(top_srcdir)/kdevelop.m4.in

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs

messages:
	find . -type d | grep -v CVS | sed -e 's,$$,/,' > dirs
	mfs=`find . -name Makefile.am | xargs egrep -l '^messages:'` ;\
	for dir in $$mfs; do \
	  if test ! "$$dir" = "./Makefile.am"; then \
	    dir=`dirname $$dir` ;\
	    egrep -v "^$$dir" dirs > dirs.new && mv dirs.new dirs ;\
	  fi ;\
	done
	dirs=`cat dirs` ;\
	find $$dirs -maxdepth 1 -name "*.cpp" -print > files ;\
	find $$dirs -maxdepth 1 -name "*.cc" -print >> files ;\
	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.rc"` > rc.cpp	;\
	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.ui"` >> rc.cpp    ;\
	$(XGETTEXT) rc.cpp `cat files` -o $(podir)/kdevelop.pot
	@rm -f rc.cpp dirs files

$(top_srcdir)/Makefile.am.in: # faking
include admin/deps.am


##
## Doxygen stuff
##

DOXYGEN_INPUT = src lib parts plugins doc/api

## Exclude the application wizard templates
DOXYGEN_EXCLUDE = parts/appwizard/kmod/ parts/appwizard/kbearplugin/ parts/appwizard/kbearimportfilter/ parts/appwizard/adahello/ parts/appwizard/qtopiasimple/ parts/appwizard/adahello/ parts/appwizard/chello_gba/ parts/appwizard/chello/ parts/appwizard/bashhello/ parts/appwizard/javahello/ parts/appwizard/imports/ parts/appwizard/gnomeapp/ parts/appwizard/fpcsharedlib/ parts/appwizard/fpchello/ parts/appwizard/fpcgtk/ parts/appwizard/fortranhello/ parts/appwizard/dcopservice/  parts/appwizard/cppsdlhello/ parts/appwizard/cpphello/ parts/appwizard/wxhello/ parts/appwizard/superwaba/ parts/appwizard/rubyhello/ parts/appwizard/qtopiaapp/ parts/appwizard/qmakesimple/ parts/appwizard/qmakeapp/ parts/appwizard/pythonhello/ parts/appwizard/pyqt/ parts/appwizard/prc-tool/ parts/appwizard/phphello/ parts/appwizard/perlhello/ parts/appwizard/pascalhello/ parts/appwizard/noatunui/ parts/appwizard/noatunvisual/ parts/appwizard/kscreensaver/ parts/appwizard/kpartplugin/ parts/appwizard/kpartapp/ parts/appwizard/konqnavpanel/ parts/appwizard/kofficepart/ parts/appwizard/kioslave/ parts/appwizard/kicker/ parts/appwizard/khello/ parts/appwizard/kdevpart2/ parts/appwizard/kdevpart/ parts/appwizard/kdedcop/ parts/appwizard/kcmodule/ parts/appwizard/kateplugin/ parts/appwizard/kateplugin2/ parts/appwizard/common/ parts/appwizard/kapp/ parts/appwizard/haskellhello/ parts/appwizard/xulhello/ parts/appwizard/kappjava/ parts/appwizard/kdevlang parts/appwizard/generichello parts/cppsupport/subclassing_template parts/pascalsupport/ parts/javasupport lib/antlr

## Use KDE Libs tag files
DOXYGEN_LINK_WITH_KDE_QT_APIDOX = YES
DOXYGEN_REFERENCES = kdecore kdeui khtml kjs kio kparts dcop kdefx interfaces kmdi kutils arts

## Use Search engine (Versions 1.3.4 and above only!)
DOXYGEN_SEARCHENGINE = NO

include Doxyfile.am

install-data-local: install-gideon-kdevelop

## We need this until we change the application name from gideon back to kdevelop
install-gideon-kdevelop:
	@if test ! -d $(DESTDIR)$(kde_htmldir)/en/gideon; then \
		pushd `pwd`;\
		echo "*** Creating a softlink from kdevelop to gideon to make the menu item Help -> Gideon Handbook work:";\
		cd $(DESTDIR)$(kde_htmldir)/en;\
		echo $(LN_S) kdevelop gideon;\
		$(LN_S) kdevelop gideon;\
		popd;\
	fi
