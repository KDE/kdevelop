SUBDIRS = lib parts plugins src pics doc

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
	@echo "KDevelop 3.0 alpha6 has been installed."
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

