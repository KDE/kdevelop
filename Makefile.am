COMPILE_FIRST = lib parts

EXTRA_DIST = AUTHORS COPYING COPYING.LIB NEWS ChangeLog INSTALL README TODO HACKING HACKING.language admin configure.in.bot

xdg_apps_DATA = kdevelop.desktop kdevassistant.desktop kdevelop_ruby.desktop \
	kdevelop_c_cpp.desktop  kdevelop_kde_cpp.desktop  kdevelop_scripting.desktop
#appsdir = $(kde_appsdir)/Development

xdg_directory_DATA = kde-development-kdevelop.directory

mime_DATA = x-kdevelop.desktop
mimedir = $(kde_mimedir)/application

kde_bin_SCRIPTS = kdevprj2kdevelop

$(top_srcdir)/acinclude.m4: $(top_srcdir)/kdevelop.m4.in

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files

package-messages:
	$(MAKE) -f $(top_srcdir)/admin/Makefile.common package-messages
	$(MAKE) -C po merge

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs

messages:
	find . -type d | grep -v CVS | grep -v '\.svn' | sed -e 's,$$,/,' > dirs
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

DOXYGEN_SEARCHENGINE = YES
DOXYGEN_EMPTY = YES
DOXYGEN_PROJECTNAME = KDevelop Platform API
include Doxyfile.am
SUBDIRS=$(TOPSUBDIRS)
