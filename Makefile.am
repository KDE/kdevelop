dist-hook:
	here=`cd $(top_srcdir) && pwd`; \
	cd $(top_distdir) && perl $$here/am_edit

# This file you have to edit. Change the name here
SUBDIRS =  doc kdevelop kdlgloader po

# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

EXTRA_DIST = BUGS FAQ kdevelop.kdevprj kdevelop.lsm README.htdig

$(top_srcdir)/acinclude.m4: $(top_srcdir)/acinclude.m4.in $(top_srcdir)/libtool.m4.in $(top_srcdir)/kdevelop.m4
	cd $(top_srcdir) && cat acinclude.m4.in libtool.m4.in kdevelop.m4 > acinclude.m4   