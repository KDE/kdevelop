#    (C) 1999 The KDevelop Team.
#    Modified from a Makefile.am of the kdelibs package.
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this library; see the file COPYING.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.


INCLUDES= $(QT_INCLUDES) $(X_INCLUDES) $(KDE_INCLUDES)

lib_LTLIBRARIES  = libkdlgloader.la

SOMAJOR = 1
SOMINOR = 0

include_HEADERS = kdlgloader.h

noinst_HEADERS = kdlgldr.h kdlgptrdb.h

libkdlgloader_la_SOURCES = kdlgldr.cpp kdlgldr_itemstuff.cpp kdlgptrdb.cpp

EXTRA_DIST = README

LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS)
libkdlgloader_la_LDFLAGS = -version-info $(SOMAJOR):$(SOMINOR) $(LDFLAGS)
libkdlgloader_la_LIBADD =  -lXext  $(LIB_QT)
