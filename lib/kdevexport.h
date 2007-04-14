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

#ifndef KDEVPLATFORMINTERFACES_EXPORT
# ifdef MAKE_KDEVPLATFORMINTERFACES_LIB
#  define KDEVPLATFORMINTERFACES_EXPORT KDE_EXPORT
# else
#  define KDEVPLATFORMINTERFACES_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEVPLATFORMPROJECT_EXPORT
# ifdef MAKE_KDEVPLATFORMPROJECT_LIB
#  define KDEVPLATFORMPROJECT_EXPORT KDE_EXPORT
# else
#  define KDEVPLATFORMPROJECT_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEVPLATFORMLANGUAGE_EXPORT
# ifdef MAKE_KDEVPLATFORMLANGUAGE_LIB
#  define KDEVPLATFORMLANGUAGE_EXPORT KDE_EXPORT
# else
#  define KDEVPLATFORMLANGUAGE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEVPLATFORMEDITOR_EXPORT
# ifdef MAKE_KDEVPLATFORMEDITOR_LIB
#  define KDEVPLATFORMEDITOR_EXPORT KDE_EXPORT
# else
#  define KDEVPLATFORMEDITOR_EXPORT KDE_IMPORT
# endif
#endif


#ifndef KDEVPLATFORMSHELL_EXPORT
# ifdef MAKE_KDEVPLATFORMSHELL_LIB
#  define KDEVPLATFORMSHELL_EXPORT KDE_EXPORT
# else
#  define KDEVPLATFORMSHELL_EXPORT KDE_IMPORT
# endif
#endif

#ifndef SUBLIME_EXPORT
# ifdef MAKE_SUBLIME_LIB
#  define SUBLIME_EXPORT KDE_EXPORT
# else
#  define SUBLIME_EXPORT KDE_IMPORT
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
#define KDEVCPPLANGUAGE_EXPORT KDE_EXPORT
#define KDEVCMAKECOMMON_EXPORT KDE_EXPORT
#define SUBLIME_EXPORT KDE_EXPORT
#endif /* KDEVEXPORT_H*/
#endif

