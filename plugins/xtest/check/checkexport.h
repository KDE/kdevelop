/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2006 Matt Rogers <mattr@kde.org>                            *
 *   Copyright 2004 Jaroslaw Staniek <js@iidea.pl>                         *
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

#ifndef QXCHECKEXPORT_H
#define QXCHECKEXPORT_H

#include <kdemacros.h>

#ifndef QXCHECK_EXPORT
# ifdef MAKE_QXCHECK_LIB
#  define QXCHECK_EXPORT KDE_EXPORT
# else
#  define QXCHECK_EXPORT KDE_IMPORT
# endif
#endif

#endif // QXCHECK_H
