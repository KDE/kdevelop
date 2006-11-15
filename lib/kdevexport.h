/*
   This file is part of the KDevelop platform
   Copyright (c) 2006 Matt Rogers <mattr@kde.org>
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KDEVEXPORT_H
#define KDEVEXPORT_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>

/* needed, because e.g. Q_OS_UNIX is so frequently used */
#ifdef __cplusplus
# include <QtCore/qglobal.h>
#endif


#if defined _WIN32 || defined _WIN64

#ifndef KDEVPLATFORM_EXPORT
# ifdef MAKE_KDEVPLATFORM_LIB
#  define KDEVPLATFORM_EXPORT KDE_EXPORT
# else
#  define KDEVPLATFORM_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEVPROJECTMANAGER_EXPORT
# ifdef MAKE_KDEVPROJECTMANAGER_LIB
#  define KDEVPROJECTMANAGER_EXPORT KDE_EXPORT
# else
#  define KDEVPROJECTMANAGER_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEVUTIL_EXPORT
# ifdef MAKE_KDEVUTIL_LIB
#  define KDEVUTIL_EXPORT KDE_EXPORT
# else
#  define KDEVUTIL_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEVWIDGETS_EXPORT
# ifdef MAKE_KDEVWIDGETS_LIB
#  define KDEVWIDGETS_EXPORT KDE_EXPORT
# else
#  define KDEVWIDGETS_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEVWEAVER_EXPORT
# ifdef MAKE_KDEVWEAVER_LIB
#  define KDEVWEAVER_EXPORT KDE_EXPORT
# else
#  define KDEVWEAVER_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEVPROFILES_EXPORT
# ifdef MAKE_KDEVPROFILES_LIB
#  define KDEVPROFILES_EXPORT KDE_EXPORT
# else
#  define KDEVPROFILES_EXPORT KDE_IMPORT
# endif
#endif



#else //UNIX


/* export statements for unix */
#define KDEVPLATFORM_EXPORT KDE_EXPORT
#define KDEVPROJECTMANAGER_EXPORT KDE_EXPORT
#define KDEVUTIL_EXPORT KDE_EXPORT
#define KDEVWIDGETS_EXPORT KDE_EXPORT
#define KDEVWEAVER_EXPORT KDE_EXPORT
#define KDEVPROFILES_EXPORT KDE_EXPORT

#endif /* KDEVEXPORT_H*/
#endif

