#! /bin/sh

# 3 arguments :
# - relative path to the local directory (e.g. ".")
# - module name (e.g. "plop")
# - root repository (e.g. ":ext:me@host:/path/to/cvsroot")

mkcvs() {
    rm -rf $1/CVS
    mkdir -p $1/CVS

    echo $2 > $1/CVS/Repository
    echo $3 > $1/CVS/Root
    
    for i in $1/*; do
	if [ -d $i -a $i != $1/CVS ]; then
	    echo "D/"`basename $i`"////" >> $1/CVS/Entries
	    mkcvs "$i" "$2/"`basename $i` $3
	elif [ -f $i ]; then
	    echo "/"`basename $i`"/1.1.1.1/"`date +"%a %b %d %T %Y//"` >> $1/CVS/Entries
	fi
    done
}

mkcvs $1 $2 $3
