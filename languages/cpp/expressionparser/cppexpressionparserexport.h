/*
   This file is part of the KDevelop platform
   Copyright 2004 Jaroslaw Staniek <js@iidea.pl>
   Copyright 2006 Matt Rogers <mattr@kde.org>
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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
#ifndef CPPEXPRESSIONPARSEREXPORT_H
#define CPPEXPRESSIONPARSEREXPORT_H

//krazy:excludeall=cpp

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>

/* needed, because e.g. Q_OS_UNIX is so frequently used */
#ifdef __cplusplus
# include <QtCore/qglobal.h>
#endif

#if defined _WIN32 || defined _WIN64

#ifndef KDEVCPPEXPRESSIONPARSER_EXPORT
# ifdef MAKE_KDEVCPPEXPRESSIONPARSER_LIB
#  define KDEVCPPEXPRESSIONPARSER_EXPORT KDE_EXPORT
# else
#  define KDEVCPPEXPRESSIONPARSER_EXPORT KDE_IMPORT
# endif
#endif

#else //UNIX
#define KDEVCPPEXPRESSIONPARSER_EXPORT KDE_EXPORT
#endif

#endif /* CPPEXPRESSIONPARSEREXPORT_H*/
