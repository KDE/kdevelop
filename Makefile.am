####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = doc kdevelop po 

EXTRA_DIST = AUTHORS BUGS COPYING ChangeLog FAQ INSTALL Menushortcuts.html README README.htdig TODO acconfig.h config.guess config.h.bot configure.in.in kdevelop.lsm kdevelop.m4 acinclude.m4 configure.files subdirs aclocal.m4 configure.in stamp-h.in config.h.in kdevelop.kdevprj.save admin configure.in.mid debian 

####### kdevelop will overwrite this part!!! (end)############
# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in $(top_srcdir)/kdevelop.m4
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in kdevelop.m4 > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

datadir = $(kde_datadir)/kdevelop
data_DATA = eventsrc

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
