SUBDIRS = lib parts plugins src pics

EXTRA_DIST = AUTHORS COPYING COPYING.LIB NEWS ChangeLog INSTALL README TODO HACKING admin configure.in.bot

apps_DATA = gideon.desktop
appsdir = $(kde_appsdir)/Development

mime_DATA = x-kdevelop.desktop
mimedir = $(kde_mimedir)/application

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in $(top_srcdir)/kdevelop.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in kdevelop.m4.in > acinclude.m4

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
	$(XGETTEXT) rc.cpp `cat files` -o $(podir)/gideon.pot
	rm -f rc.cpp dirs files

