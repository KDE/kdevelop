/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SESSION_H
#define KDEVPLATFORM_SESSION_H

#include "shellexport.h"
#include <QUuid>
#include <QUrl>
#include <interfaces/isession.h>

namespace KDevelop
{

class SessionPrivate;

struct SessionInfo
{
    QString name;
    QUuid uuid;
    QString description;
    QList<QUrl> projects;
    QString path;
    KSharedConfigPtr config;
};
using SessionInfos = QVector<SessionInfo>;

class KDEVPLATFORMSHELL_EXPORT Session : public ISession
{
    Q_OBJECT
public:
    static const QString cfgSessionNameEntry;
    static const QString cfgSessionDescriptionEntry;
    static const QString cfgSessionProjectsEntry;
    static const QString cfgSessionOptionsGroup;

    explicit Session( const QString& id, QObject * parent = nullptr );
    ~Session() override;

    KSharedConfigPtr config() override;

    QList<QUrl> containedProjects() const override;
    void setContainedProjects( const QList<QUrl>& projects ) override;

    QString name() const override;
    void setName( const QString& );

    QUuid id() const override;

    QString description() const override;

    bool isTemporary() const override;
    void setTemporary(bool temp) override;

    [[nodiscard]] QString dataDirectory() const override;

    /**
     * Generates a @ref SessionInfo by a session @p id.
     * @param mkdir Whether to create a session directory if one does not exist.
     */
    static SessionInfo parse( const QString& id, bool mkdir = false );

private:
    const QScopedPointer<class SessionPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Session)
    friend class SessionPrivate;
};

}

Q_DECLARE_TYPEINFO(KDevelop::SessionInfo, Q_MOVABLE_TYPE);

#endif

