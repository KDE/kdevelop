/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/**
 * The interface to compiler options configuration
 */

#ifndef _KDEVCOMPILEROPTIONS_H_
#define _KDEVCOMPILEROPTIONS_H_

#include <qobject.h>


class KDevCompilerOptions : public QObject
{
    Q_OBJECT

public:
    KDevCompilerOptions( QObject *parent=0, const char *name=0 );
    ~KDevCompilerOptions();

    /**
     * Opens a dialog which allows the user to configure the
     * compiler options. The initial settings in the dialog
     * will be set from the flags argument of this method.
     * After the dialog is accepted, the new settings will
     * be returned as a string. If the dialog was cancelled,
     * QString::null is returned.
     */
    virtual QString exec(QWidget *parent, const QString &flags) = 0;
};

#endif
