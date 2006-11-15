/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPCSIMPORTER_H
#define KDEVPCSIMPORTER_H

#include <QObject>
#include <qstringlist.h>
#include "kdevexport.h"

class QWidget;

/**
KDevelop persistant class store importer plugin.

These plugins are used by language support plugins to fill symbol stores
with symbol information from certain files. The purpose of the importer
is to provide file selection wizard.
*/
class KDEVPLATFORM_EXPORT KDevPCSImporter: public QObject
{
    Q_OBJECT
public:
    KDevPCSImporter( QObject* parent=0 );
    virtual ~KDevPCSImporter();

    virtual QString dbName() const = 0;
    virtual QStringList includePaths() = 0;
    virtual QStringList fileList() = 0;

    virtual QWidget* createSettingsPage( QWidget* parent, const char* name=0 );
};

#endif // KDEVPCSIMPORTER_H
