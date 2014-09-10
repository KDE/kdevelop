/* This file is part of KDevelop
Copyright 2008 Andreas Pakulat <apaku@gmx.de>

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

#ifndef KDEVPLATFORM_SESSION_H
#define KDEVPLATFORM_SESSION_H

#include "shellexport.h"
#include <QtCore/QObject>
#include <QtCore/QUuid>
#include <QUrl>
#include <interfaces/isession.h>
#include <interfaces/isessionlock.h>

namespace KDevelop
{

struct SessionInfo
{
    QString name;
    QUuid uuid;
    QString description;
    QList<QUrl> projects;
    QString path;
    KSharedConfig::Ptr config;
};

class KDEVPLATFORMSHELL_EXPORT Session : public ISession
{
    Q_OBJECT
public:
    static const QString cfgSessionNameEntry;
    static const QString cfgSessionDescriptionEntry;
    static const QString cfgSessionProjectsEntry;
    static const QString cfgSessionOptionsGroup;

    Session( const QString& id, QObject * parent = 0 );
    virtual ~Session();

    virtual QUrl pluginDataArea( const IPlugin* );
    virtual KSharedConfig::Ptr config();

    virtual QList<QUrl> containedProjects() const;
    virtual void setContainedProjects( const QList<QUrl>& projects );

    virtual QString name() const;
    void setName( const QString& );

    virtual QUuid id() const;

    virtual QString description() const;

    virtual bool isTemporary() const;
    virtual void setTemporary(bool temp);

    QString path() const;

    /**
     * Generates a @ref SessionInfo by a session @p id.
     * @param mkdir Whether to create a session directory if one does not exist.
     */
    static SessionInfo parse( const QString& id, bool mkdir = false );

private:
    class SessionPrivate* const d;
    friend class SessionPrivate;
};

}

Q_DECLARE_METATYPE( KDevelop::Session* )

#endif

