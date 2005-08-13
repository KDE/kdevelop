#!/bin/sh

set -e

if [ ! -x /usr/bin/htdig -o ! -x /usr/bin/htmerge ]; then
  exit 0
fi

QTDOC_PACKAGE="qt3-doc"
QTDOC_DIR="/usr/share/qt3/doc/html"
KDELIBSDOC_PACKAGE="kdelibs4-doc"
KDELIBSDOC_DIR="/usr/share/doc/kde/HTML/en/kdelibs-apidocs"
KDEVELOPDOC_PACKAGE="kdevelop3-doc"
KDEVELOPDOC_DIR="/usr/share/doc/kde/HTML/en/kdevelop"

# Checks if a package is installed
check_package_status() {
  local package=${1}

  if dpkg --status $package  2> /dev/null | grep '^Status: .* installed$' > /dev/null; then
    echo "yes"
  else
    echo "no"
  fi
}

GLOBAL_KDEVDOCTREEVIEW_DIR="/var/lib/kdevelop3"
GLOBAL_HTDIG_DIR="$GLOBAL_KDEVDOCTREEVIEW_DIR/helpindex"
HTDIG_DATA_DIR="/usr/share/apps/kdevdoctreeview"

get_indexed_versions() {
  if [ -e "$GLOBAL_HTDIG_DIR/package_versions" ]; then
    . "$GLOBAL_HTDIG_DIR/package_versions"
  else
    QTDOC_VERSION="0.0"
    KDEDOC_VERSION="0.0"
    KDEVELOPDOC_VERSION="0.0"
  fi
}

update_files_list() {
  local pkgname=${1}
  local pkgdir=${2}

  find "$pkgdir" -name "*.html" | \
    awk 'OFS=""; {print "file://", $0, " \\"}' > "$GLOBAL_HTDIG_DIR/$pkgname""_files"
}

doc_dirs="$QTDOC_DIR $KDELIBSDOC_DIR $KDEVELOPDOC_DIR"

# Create the directory global htdig directory if necessary
if [ ! -d "$GLOBAL_KDEVDOCTREEVIEW_DIR" ]; then
  mkdir "$GLOBAL_KDEVDOCTREEVIEW_DIR"
fi

if [ ! -d "$GLOBAL_HTDIG_DIR" ]; then
  mkdir "$GLOBAL_HTDIG_DIR"
  initial="-i"
else
  if [ ! -e "$GLOBAL_HTDIG_DIR/db.docdb" ]; then
    initial="-i"
  else
    initial=""
  fi
fi

get_indexed_versions

need_update=0

# Check if the QT documentation needs to be (re)indexed
qtdoc_status=`check_package_status "$QTDOC_PACKAGE"`
version=`dpkg --status $QTDOC_PACKAGE  2> /dev/null | grep '^Version: ' | cut -d ' ' -f 2`
if [ "$qtdoc_status" == "yes" ] &&
   ( [ ! -e "$GLOBAL_HTDIG_DIR/qtdoc_files" ] ||
   dpkg --compare-versions $version gt $QTDOC_VERSION ); then
  QTDOC_VERSION="$version"
  `update_files_list "qtdoc" "$QTDOC_DIR"`
  need_update=1
elif [ "$qtdoc_status" == "no" -a -e "$GLOBAL_HTDIG_DIR/qtdoc_files" ]; then
  rm "$GLOBAL_HTDIG_DIR/qtdoc_files"
fi

# Check if the KDE documentation needs to be (re)indexed
kdedoc_status=`check_package_status "$KDELIBSDOC_PACKAGE"`
version=`dpkg --status $KDELIBSDOC_PACKAGE  2> /dev/null | grep '^Version: ' | cut -d ' ' -f 2`
if [ "$kdedoc_status" == "yes" ] &&
   ( [ ! -e "$GLOBAL_HTDIG_DIR/kdelibsdoc_files" ] ||
   dpkg --compare-versions $version gt $KDEDOC_VERSION ); then
  KDEDOC_VERSION="$version"
  `update_files_list "kdelibsdoc" "$KDELIBSDOC_DIR"`
  need_update=1
elif [ "$kdedoc_status" == "no" -a -e "$GLOBAL_HTDIG_DIR/kdelibsdoc_files" ]; then
  rm "$GLOBAL_HTDIG_DIR/kdelibsdoc_files"
fi

# Check if the KDevelop documentation needs to be (re)indexed
kdevelopdoc_status=`check_package_status "$KDEVELOPDOC_PACKAGE"`
version=`dpkg --status $KDEVELOPDOC_PACKAGE  2> /dev/null | grep '^Version: ' | cut -d ' ' -f 2`
if [ "$kdevelopdoc_status" == "yes" ] &&
   ( [ ! -e "$GLOBAL_HTDIG_DIR/kdevelopdoc_files" ] ||
   dpkg --compare-versions $version gt $KDEVELOPDOC_VERSION ); then
  KDEVELOPDOC_VERSION="$version"
  `update_files_list "kdevelopdoc" "$KDEVELOPDOC_DIR"`
  need_update=1
elif [ "$kdevelopdoc_status" == "no" -a -e "$GLOBAL_HTDIG_DIR/kdevelopdoc_files" ]; then
  rm "$GLOBAL_HTDIG_DIR/kdelibsdoc_files"
fi

if [ $need_update == 0 ]; then
  exit 0
fi

# Create the htdig config file if necessary
if [ ! -e "$GLOBAL_HTDIG_DIR/htdig.conf" ]; then
  echo "# For htdig 3.2.x" >"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "database_dir:		$GLOBAL_HTDIG_DIR" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "include:		$GLOBAL_HTDIG_DIR/files" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "limit_urls_to:		file:///" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "local_urls:		file:///=/" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "local_urls_only:	true" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "maximum_pages:		1" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "image_url_prefix:	$HTDIG_DATA_DIR/pics/" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "star_image:		$HTDIG_DATA_DIR/pics/star.png" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "star_blank:		$HTDIG_DATA_DIR/pics/star_blank.png" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "compression_level:	6" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "max_hop_count:		1" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "search_results_wrapper:	$HTDIG_DATA_DIR/en/wrapper.html" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "nothing_found_file:	$HTDIG_DATA_DIR/en/nomatch.html" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "syntax_error_file:	$HTDIG_DATA_DIR/en/syntax.html" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
  echo "bad_word_list:		$HTDIG_DATA_DIR/en/bad_words" >>"$GLOBAL_HTDIG_DIR/htdig.conf"
fi

# Create the list of files to index
rm -f "$GLOBAL_HTDIG_DIR/files" 2> /dev/null
echo "start_url: \\" > "$GLOBAL_HTDIG_DIR/files"
if [ -e "$GLOBAL_HTDIG_DIR/qtdoc_files" ]; then
  cat "$GLOBAL_HTDIG_DIR/qtdoc_files" >> "$GLOBAL_HTDIG_DIR/files"
fi
if [ -e "$GLOBAL_HTDIG_DIR/kdelibsdoc_files" ]; then
  cat "$GLOBAL_HTDIG_DIR/kdelibsdoc_files" >> "$GLOBAL_HTDIG_DIR/files"
fi
if [ -e "$GLOBAL_HTDIG_DIR/kdevelopdoc_files" ]; then
  cat "$GLOBAL_HTDIG_DIR/kdevelopdoc_files" >> "$GLOBAL_HTDIG_DIR/files"
fi
# Strip the backslash from the last line
cat "$GLOBAL_HTDIG_DIR/files" | sed '$s/ \\$//' > "$GLOBAL_HTDIG_DIR/files.new"
mv "$GLOBAL_HTDIG_DIR/files.new" "$GLOBAL_HTDIG_DIR/files"

# Index the files
/usr/bin/htdig $initial -s -c "$GLOBAL_HTDIG_DIR/htdig.conf" > /dev/null 2>&1
/usr/bin/htmerge -s -c "$GLOBAL_HTDIG_DIR/htdig.conf" > /dev/null 2>&1

# Mark the current package versions as indexed
echo "QTDOC_VERSION=\"$QTDOC_VERSION\"" > "$GLOBAL_HTDIG_DIR/package_versions"
echo "KDEDOC_VERSION=\"$KDEDOC_VERSION\"" >> "$GLOBAL_HTDIG_DIR/package_versions"
echo "KDEVELOPDOC_VERSION=\"$KDEVELOPDOC_VERSION\"" >> "$GLOBAL_HTDIG_DIR/package_versions"
chmod 0764 "$GLOBAL_HTDIG_DIR/package_versions"
