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

#ifndef KDEVENVIRONMENT_H
#define KDEVENVIRONMENT_H

#include <QObject>
#include <QMap>
#include "kdevexport.h"

class QProcess;
class KProcess;

typedef QMap<QString, QString> EnvironmentMap;

/**
@file kdevenvironment.h
KDevelop Environment Variable interface.
*/

/**
The interface to KDevelop's environment.
This class is used by any @ref KDevPlugin that needs to sync or interact with
environment variables.  The user can set all environment variables by way of a
KCM module.  All plugins should respect the environment variables set by the user
and sync any spawned QProcess/KProcess with the environment variables found in
this class.
*/
class KDEVINTERFACES_EXPORT KDevEnvironment: public QObject
{
    friend class KDevEnvWidget;
    Q_OBJECT
public:
    KDevEnvironment( QObject *parent = 0 );
    virtual ~KDevEnvironment();

    /**
     * Get the value of an environment variable or QString::null if no
     * value has been specified.
     * @param name The name of the environment variable.
     * @return The value of the environment variable.
     */
    QString variable( const QString &name ) const;

    /**
     * Set an environment variable to a specific value.
     * @param name The name of the environment variable.
     * @param value The value of the environment variable.
     */
    void setVariable( const QString &name, const QString &value );

    /**
     * Unset the environment variable.
     * @param name The name of the environment variable.
     */
    void unsetVariable( const QString &name );

    /**
     * Revert an environment variable to the process default.  The process
     * default was the value of the variable when KDevelop was first initialized.
     * @param name The name of the environment variable.
     * @return true if the variable has a process default. false if no default exists.
     */
    bool revertToProcessDefault( const QString &name );

    /**
     * Save the current environment to disk.  Either the local project file if
     * a project is loaded, or the local configuration file if a project is not
     * loaded.
     */
    void saveSettings();

    /**
     * Sync the @ref QProcess with the environment specified by the user.
     * @param process The process to sync.
     */
    void syncProcess( QProcess *process );

    /**
     * Sync the @ref KProcess with the environment specified by the user.
     * @param process The process to sync.
     */
    void syncProcess( KProcess *process );

public slots:
    /**
     * Populate the enviroment with the process defaults and the settigs specified
     * in the configuration/project files.
     */
    void populate();

private:
    void saveSettings( EnvironmentMap overrides );

    void populateOverrides();
    void populateProcessDefaults();

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

private:
    EnvironmentMap m_overrides;
    EnvironmentMap m_processDefaults;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
