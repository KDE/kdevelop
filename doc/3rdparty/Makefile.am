tocdir = ${kde_datadir}/kdevdoctreeview/tocs
toc_DATA = python.toc perl.toc php.toc 

indexdir = ${kde_datadir}/kdevdoctreeview/indices
index_DATA = python.index

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

python.index: $(srcdir)/python.index.in
	echo "<!DOCTYPE gideonindex>" > python.index ; \
	echo "<gideonindex>" >> python.index
	echo "<title>Python</title>" >> python.index ; \
	echo "<base href=\"$(PYTHONDOCDIR)\"/>" >> python.index ; \
	cat $(srcdir)/python.index.in >> python.index ; \
	echo "</gideonindex>" >> python.index ; \
	echo "" >> python.index

EXTRA_DIST = python.toc.in perl.toc.in php.toc.in

python-index:
	perl ../tools/index-python \
	  --prefix=ref \
	  /usr/share/doc/packages/python/html/ref/genindex.html > python.index.in