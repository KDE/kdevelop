/*
   This file is part of the KDevelop platform
   Copyright 2004 Jaroslaw Staniek <js@iidea.pl>
   Copyright 2006 Matt Rogers <mattr@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef CPPPARSEREXPORT_H
#define CPPPARSEREXPORT_H

/* needed for KDE_EXPORT macros */
#include <QtGlobal>

#ifndef KDEVCPPPARSER_EXPORT
# ifdef MAKE_KDEV4CPPPARSER_LIB
#  define KDEVCPPPARSER_EXPORT Q_DECL_EXPORT
# else
#  define KDEVCPPPARSER_EXPORT Q_DECL_IMPORT
# endif
#endif

#ifndef KDEVCPPRPP_EXPORT
# ifdef MAKE_KDEV4CPPRPP_LIB
#  define KDEVCPPRPP_EXPORT Q_DECL_EXPORT
# else
#  define KDEVCPPRPP_EXPORT Q_DECL_IMPORT
# endif
#endif

#endif /* CPPPARSEREXPORT_H*/
