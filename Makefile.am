## kdebase/Makefile.am
## (C) 1997 Stephan Kulow

AUTOMAKE_OPTIONS = foreign 1.4
SUBDIRS = kdevelop pics doc

install-data-local:
	@echo ""
	@echo ""
	@echo ""
	@echo "*************** Important *************************"
	@echo ""
	@echo " Add "$(bindir)" to your PATH"
	@echo ""
	@echo " Please report bugs with our web form at"
	@echo " http://bugs.kde.org."
	@echo ""
	@echo " Stephan Kulow <coolo@kde.org> and all the KDE"
	@echo " developers wish you fun with the KDE."
	@echo "****************************************************"
	@echo ""

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in $(top_srcdir)/kdevelop.m4
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in kdevelop.m4 > acinclude.m4 

$(top_srcdir)/subdirs:
	@cd $(top_srcdir) && echo kdevelop > subdirs
	@cd $(top_srcdir) && echo pics >> subdirs
	@cd $(top_srcdir) && echo doc >> subdirs

$(top_srcdir)/configure.in:
	@cd $(top_srcdir) && cat admin/configure.in.min configure.in.in > configure.in
	

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 SUBDIRS

EXTRA_DIST = admin debian kdevelop.m4 BUGS FAQ kdevelop.kdevprj kdevelop.lsm

dist-hook:
	cd $(top_distdir) && perl $(top_srcdir)/admin/am_edit -padmin
