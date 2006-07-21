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

typedef QMap<QString, QString> EnvironmentMap;

class KDEVINTERFACES_EXPORT KDevEnv: public QObject
{
    Q_OBJECT
public:
    KDevEnv( QObject *parent = 0 );
    virtual ~KDevEnv();

    void saveSettings( EnvironmentMap overrides );

    void populateOverrides();
    void populateProcessDefaults();

    QString variable( const QString &name ) const;
    void setVariable( const QString &name, const QString &value );
    void unsetVariable( const QString &name );

    bool isOverride( const QString &name ) const;
    QString override( const QString &name ) const;
    bool removeOverride( const QString &name, bool unset = false );
    void clearOverrides();

    bool isProcessDefault( const QString &name ) const;
    QString processDefault( const QString &name ) const;
    bool removeProcessDefault( const QString &name, bool unset = false );
    void clearProcessDefaults();

    EnvironmentMap overrideMap() const
    {
        return m_overrides;
    }
    EnvironmentMap processDefaultMap() const
    {
        return m_processDefaults;
    }

public slots:
    void populate();

private:
    EnvironmentMap m_overrides;
    EnvironmentMap m_processDefaults;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
