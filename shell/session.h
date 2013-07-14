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
#include <interfaces/isession.h>
#include <interfaces/isessionlock.h>
#include <kurl.h>

namespace KDevelop
{

struct SessionInfo
{
    QString name;
    QUuid uuid;
    QString description;
    KUrl::List projects;
    QString path;
    KSharedConfig::Ptr config;
};

class KDEVPLATFORMSHELL_EXPORT Session : public ISession
{
    Q_OBJECT
public:
    static const QString cfgSessionNameEntry;
    static const QString cfgSessionPrettyContentsEntry;
    Session( const QUuid& id, QObject * parent = 0 );
    virtual ~Session();

    virtual KUrl pluginDataArea( const IPlugin* );
    virtual KSharedConfig::Ptr config();

    KUrl::List containedProjects() const;

    void updateDescription();

    virtual QString description() const;
    virtual QString name() const;
    void setName( const QString& );
    QUuid id() const;

    virtual void setTemporary(bool temp);
    virtual bool isTemporary() const;

    QString path() const;

    /**
     * Generates session's pretty contents from project list in @p info.
     * @return session's pretty contents
     */
    static QString generatePrettyContents( const SessionInfo& info );

    /**
     * Generates session's description field using provided pretty contents.
     * @return session's description field
     */
    static QString generateDescription( const KDevelop::SessionInfo& info, const QString& prettyContents );

    /**
     * Generates a @ref SessionInfo by a session @p id.
     * @param mkdir Whether to create a session directory if one does not exist.
     */
    static SessionInfo parse( const QUuid& id, bool mkdir = false );

Q_SIGNALS:
    void nameChanged( const QString& newname, const QString& oldname );
public slots:
    virtual void updateContainedProjects();
private:
    class SessionPrivate* const d;

};

}
#endif

