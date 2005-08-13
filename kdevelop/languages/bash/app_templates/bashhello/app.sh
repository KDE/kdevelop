#!/bin/bash
# $Id$
# %{APPNAME} - Copyright (C) %{YEAR} %{AUTHOR} <%{EMAIL}>


hello()
{
	echo "Hello $NAME"
}

echo "This is a test"

LST="Ian Ben Rook Kaz"
for NAME in $LST
do
	hello
done
