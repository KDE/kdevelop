/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ISESSION_H
#define KDEVPLATFORM_ISESSION_H

#include "interfacesexport.h"

#include <KSharedConfig>

#include <QObject>
#include <QUrl>

class QUuid;
class QString;

namespace KDevelop
{

/**
 * @class ISession
 */
class KDEVPLATFORMINTERFACES_EXPORT ISession : public QObject
{
    Q_OBJECT
public:
    explicit ISession( QObject* parent = nullptr );
    ~ISession() override;

    /**
     * A short string nicely identifying the session, including contained projects
     * 
     * The string is empty if the session is empty and has no name.
     */
    virtual QString description() const = 0;
    virtual QString name() const = 0;
    virtual QList<QUrl> containedProjects() const = 0;
    virtual void setContainedProjects( const QList<QUrl>& projects ) = 0;
    /**
     * @return the directory where persistent session data can be stored, also known as the session directory
     */
    [[nodiscard]] virtual QString dataDirectory() const = 0;
    virtual KSharedConfigPtr config() = 0;
    virtual QUuid id() const = 0;

    /**
     * Mark session as temporary. It will then be deleted on close.
     *
     * This is mainly useful for unit tests etc.
     */
    virtual void setTemporary(bool temp) = 0;
    virtual bool isTemporary() const = 0;

Q_SIGNALS:
    void sessionUpdated( KDevelop::ISession* session );
};

}

#endif

