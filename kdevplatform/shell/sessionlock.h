/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SESSIONLOCK_H
#define SESSIONLOCK_H

#include <interfaces/isessionlock.h>
#include <QLockFile>

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

    QString id() override;

    ~SessionLock() override;

    /**
     * Release the lock file and remove the session directory from disk.
     */
    void removeFromDisk();

private:
    SessionLock(const QString& sessionId, const QSharedPointer<QLockFile>& lockFile);
    const QString m_sessionId;
    QSharedPointer<QLockFile> m_lockFile;
};

}

#endif // SESSIONLOCK_H
