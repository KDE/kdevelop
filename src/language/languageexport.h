/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef kdevlanguageexport_H
#define kdevlanguageexport_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>

/* needed, because e.g. Q_OS_UNIX is so frequently used */
#ifdef __cplusplus
# include <QtCore/qglobal.h>
#endif

#if defined _WIN32 || defined _WIN64

#ifndef KDEVPLATFORMLANGUAGE_EXPORT
# ifdef MAKE_KDEVPLATFORMLANGUAGE_LIB
#  define KDEVPLATFORMLANGUAGE_EXPORT KDE_EXPORT
# else
#  define KDEVPLATFORMLANGUAGE_EXPORT KDE_IMPORT
# endif
#endif

#else //UNIX

#define KDEVPLATFORMLANGUAGE_EXPORT KDE_EXPORT

#endif

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
