/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#ifndef KDEVENV_H
#define KDEVENV_H

#include <QObject>
#include <QMap>
#include "kdevexport.h"

class KDEVINTERFACES_EXPORT KDevEnv: public QObject
{
    Q_OBJECT
public:
    KDevEnv( QObject *parent = 0 );
    virtual ~KDevEnv();

    QMap<QString, QString> variableMap() const { return m_variables; }

    QStringList currentVariables() const;

    QString getenv( const QString &name );

private:
    QMap<QString, QString> m_variables;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
