if GTKDOCS
GTKTOC = gtk.toc
else
GTKTOC =
endif

tocdir = ${kde_datadir}/kdevdoctreeview/tocs
toc_DATA = python.toc perl.toc php.toc gnustep.toc $(GTKTOC)

indexdir = ${kde_datadir}/kdevdoctreeview/indices
index_DATA = python.index

EXTRA_DIST = python.toc.in perl.toc.in php.toc.in gnustep.toc.in glib.toc.in \
	     gdk.toc.in gdk-pixbuf.toc.in gtk.toc.in gnome.toc.in gnomeui.toc.in python.index.in
DISTCLEANFILES = python.toc perl.toc php.toc gnustep.toc gtk.toc python.index

perl.toc: $(srcdir)/perl.toc.in
	echo "<!DOCTYPE gideontoc>" > perl.toc ; \
	echo "<gideontoc>" >> perl.toc ; \
	echo "<title>Perl</title>" >> perl.toc ; \
	cat $(srcdir)/perl.toc.in >> perl.toc
	echo "</gideontoc>" >> perl.toc ; \
	echo "" >> perl.toc

python.toc: $(srcdir)/python.toc.in
	echo "<!DOCTYPE gideontoc>" > python.toc ; \
	echo "<gideontoc>" >> python.toc ; \
	echo "<title>Python</title>" >> python.toc ; \
	echo "<base href=\"$(PYTHONDOCDIR)\"/>" >> python.toc ; \
	cat $(srcdir)/python.toc.in >> python.toc ; \
	echo "</gideontoc>" >> python.toc ; \
	echo "" >> python.toc

php.toc: $(srcdir)/php.toc.in
	echo "<!DOCTYPE gideontoc>" > php.toc ; \
	echo "<gideontoc>" >> php.toc ; \
	echo "<title>PHP</title>" >> php.toc ; \
	echo "<base href=\"$(PHPDOCDIR)\"/>" >> php.toc ; \
	cat $(srcdir)/php.toc.in >> php.toc ; \
	echo "</gideontoc>" >> php.toc ; \
	echo "" >> php.toc

gnustep.toc: $(srcdir)/gnustep.toc.in
	echo "<!DOCTYPE gideontoc>" > gnustep.toc ; \
	echo "<gideontoc>" >> gnustep.toc ; \
	echo "<title>GNUStep</title>" >> gnustep.toc ; \
	echo "<base href=\"$(GNUSTEP_SYSTEM_ROOT)\"/>" >> gnustep.toc ; \
	cat $(srcdir)/gnustep.toc.in >> gnustep.toc ; \
	echo "</gideontoc>" >> gnustep.toc ; \
	echo "" >> gnustep.toc

gtk.toc: $(srcdir)/glib.toc.in $(srcdir)/gdk.toc.in $(srcdir)/gdk-pixbuf.toc.in $(srcdir)/gtk.toc.in $(srcdir)/gnome.toc.in $(srcdir)/gnomeui.toc.in 
	echo "<!DOCTYPE gideontoc>" > gtk.toc ; \
	echo "<gideontoc>" >> gtk.toc ; \
	echo "<title>Gtk</title>" >> gtk.toc ; \
	echo "<base href=\"$(GLIBDOCDIR)\"/>" >> gtk.toc ; \
	echo "<tocsect1 name=\"GLib\" url=\"index.html\">" >> gtk.toc ; \
        cat $(srcdir)/glib.toc.in >> gtk.toc ; \
	echo "</tocsect1>" >> gtk.toc ; \
	echo "<base href=\"$(GDKDOCDIR)\"/>" >> gtk.toc ; \
	cat $(srcdir)/gdk.toc.in >> gtk.toc ; \
	echo "<base href=\"$(GDKPBDOCDIR)\"/>" >> gtk.toc ; \
	echo "<tocsect1 name=\"Gdk-Pixbuf\" url=\"index.html\">" >> gtk.toc ; \
	cat $(srcdir)/gdk-pixbuf.toc.in >> gtk.toc ; \
	echo "</tocsect1>" >> gtk.toc ; \
	echo "<base href=\"$(GTKDOCDIR)\"/>" >> gtk.toc ; \
	echo "<tocsect1 name=\"Gtk\" url=\"index.html\">" >> gtk.toc ; \
	cat $(srcdir)/gtk.toc.in >> gtk.toc ; \
	echo "</tocsect1>" >> gtk.toc ; \
	echo "<base href=\"$(GNOMEDOCDIR)\"/>" >> gtk.toc ; \
	cat $(srcdir)/gnome.toc.in >> gtk.toc ; \
	echo "<base href=\"$(GNOMEUIDOCDIR)\"/>" >> gtk.toc ; \
	cat $(srcdir)/gnomeui.toc.in >> gtk.toc ; \
	echo "</gideontoc>" >> gtk.toc ; \
	echo "" >> gtk.toc

python.index: $(srcdir)/python.index.in
	echo "<!DOCTYPE gideonindex>" > python.index ; \
	echo "<gideonindex>" >> python.index
	echo "<title>Python</title>" >> python.index ; \
	echo "<base href=\"$(PYTHONDOCDIR)\"/>" >> python.index ; \
	cat $(srcdir)/python.index.in >> python.index ; \
	echo "</gideonindex>" >> python.index ; \
	echo "" >> python.index

python-toc:
	perl ../tools/toc-python \
	  --prefix=tut \
	  --title=Tutorial \
	  --main=tut/tut \
	  /usr/share/doc/packages/python/html/tut/tut.html > python.toc.in ; \
	perl ../tools/toc-python \
	  --prefix=ref \
	  --title='Language Reference' \
	  --main=ref/ref \
	  /usr/share/doc/packages/python/html/ref/ref.html >> python.toc.in
	perl ../tools/toc-python \
	  --prefix=ext \
	  --title='Extending and Embedding' \
	  --main=ext/ext \
	  /usr/share/doc/packages/python/html/ext/ext.html >> python.toc.in
	perl ../tools/toc-python \
	  --prefix=api \
	  --title='Python/C API Reference' \
	  --main=api/api \
	  /usr/share/doc/packages/python/html/api/api.html >> python.toc.in
	perl ../tools/toc-python \
	  --prefix=lib \
	  --title=Library \
	  --main=lib/lib \
	  /usr/share/doc/packages/python/html/lib/lib.html >> python.toc.in

php-toc:
	perl ../tools/toc-docbook \
	  /usr/share/doc/packages/phpdoc/manual/manual.html > php.toc.in

gnustep-toc:
	perl ../tools/toc-gsdoc \
	  --prefix=Documentation/Base/Reference \
	  --title=Base \
	  --main=base \
	  Base.gsdoc > gnustep.toc.in ; \
	perl ../tools/toc-gsdoc \
	  --prefix=Documentation/Gui/Reference \
	  --title=Gui \
	  --main=gui \
	  Gui.gsdoc >> gnustep.toc.in

gtk-toc:
	perl ../tools/toc-docbook \
	  --depth=2 \
	  /usr/share/doc/packages/glib/index.html > glib.toc.in ; \
	perl ../tools/toc-docbook \
	  /usr/share/doc/packages/gdk/index.html > gdk.toc.in ; \
	perl ../tools/toc-docbook \
	  --depth=2 \
	  /usr/share/doc/packages/gdk-pixbuf/html/index.html > gdk-pixbuf.toc.in ; \
	perl ../tools/toc-docbook \
	  --depth=2 \
	  /usr/share/doc/packages/gtk/index.html > gtk.toc.in ; \
	perl ../tools/toc-docbook \
	  /usr/share/doc/packages/gnome-libs-devel/devel-docs/gnome/html/book1.html > gnome.toc.in ; \
	perl ../tools/toc-docbook \
	  /usr/share/doc/packages/gnome-libs-devel/devel-docs/gnomeui/html/book1.html > gnomeui.toc.in

python-index:
	perl ../tools/index-python \
	  --prefix=ref \
	  /usr/share/doc/packages/python/html/ref/genindex.html > python.index.in
