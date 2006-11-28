/*
   This file is part of KDevelop
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

#ifndef SHELLEXPORT_H
#define SHELLEXPORT_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>

/* needed, because e.g. Q_OS_UNIX is so frequently used */
#ifdef __cplusplus
# include <QtCore/qglobal.h>
#endif


#if defined Q_OS_WIN
#ifndef KDEVSHELL_EXPORT
# ifdef MAKE_KDEVSHELL_LIB
#  define KDEVSHELL_EXPORT KDE_EXPORT
# else
#  define KDEVSHELL_EXPORT KDE_IMPORT
# endif
#endif

#else //UNIX

/* export statements for unix */
#define KDEVSHELL_EXPORT KDE_EXPORT

#endif

#endif /* SHELLEXPORT_H*/

