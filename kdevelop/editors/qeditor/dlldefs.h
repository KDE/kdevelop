 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DLLDEFS_H
#define DLLDEFS_H

#include <qglobal.h>

#if defined(Q_OS_WIN32)
#  if defined(EDITOR_DLL)
#    define EDITOR_EXPORT __declspec(dllexport)
#  else
#    define EDITOR_EXPORT __declspec(dllimport)
#  endif
#else
#  define EDITOR_EXPORT
#endif

#endif //DLLDEFS_H
