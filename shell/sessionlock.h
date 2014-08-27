/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SESSIONLOCK_H
#define SESSIONLOCK_H

#include <interfaces/isessionlock.h>
#include <KLockFile>

namespace KDevelop {

struct SessionRunInfo;
struct TryLockSessionResult;

class SessionLock : public ISessionLock
{
public:
    /**
     * \copydoc SessionController::tryLockSession
     *
     * @param doLocking whether to actually try to lock or whether to only get
     *                  information about whether the session could be locked
     */
    static TryLockSessionResult tryLockSession(const QString& sessionId, bool doLocking);

    /**
     * \copydoc SessionController::handleLockedSession
     */
    static QString handleLockedSession( const QString& sessionName,
                                        const QString& sessionId,
                                        const SessionRunInfo& runInfo );

    virtual QString id();

    virtual ~SessionLock();

private:
    SessionLock(const QString& sessionId, const QSharedPointer<KLockFile>& lockFile);
    QString m_sessionId;
    QSharedPointer<KLockFile> m_lockFile;
};

}

#endif // SESSIONLOCK_H
