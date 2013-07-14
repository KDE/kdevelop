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

#include "sessionlock.h"

#include "sessioncontroller.h"

#include <KLockFile>
#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>

#include <QDBusConnectionInterface>
#include <QFile>

namespace KDevelop
{

struct SessionLockPrivate
{
    SessionLockPrivate(const QString& sessionId, const KLockFile::Ptr& _lockFile)
    : id(sessionId)
    , lockFile(_lockFile)
    {
        Q_ASSERT(lockFile->isLocked());
    }
    QString id;
    KLockFile::Ptr lockFile;
};

}

using namespace KDevelop;

namespace {

QString lockFileForSession( const QString& id )
{
    return SessionController::sessionDirectory( id ) + "/lock";
}

QString dBusServiceNameForSession( const QString& id )
{
    // We remove starting "{" and ending "}" from the string UUID representation
    // as D-Bus apparently doesn't allow them in service names
    return QString( "org.kdevelop.kdevplatform-lock-" ) + QString( id ).mid( 1, id.size() - 2 );
}

/// Tries to own the lock-file and returns result
KLockFile::LockResult attemptRelock(KLockFile::Ptr lockFile)
{
    return lockFile->lock( KLockFile::ForceFlag | KLockFile::NoBlockFlag );
}

/// Force-removes the lock-file.
void forceRemoveLockfile(const QString& lockFilename)
{
    if( QFile::exists( lockFilename ) ) {
        QFile::remove( lockFilename );
    }
}

}

TryLockSessionResult SessionLock::tryLockSession(const QString& sessionId, bool doLocking)
{
    ///FIXME: if this is hit, someone tried to lock a non-existing session
    ///       this should be fixed by using a proper data type distinct from
    ///       QString for id's, i.e. QUuid or similar.
    Q_ASSERT(QFile::exists(SessionController::sessionDirectory( sessionId )));

    /*
     * We've got two locking mechanisms here: D-Bus unique service name (based on the session id)
     * and a plain lockfile (KLockFile).
     * The latter is required to get the appname/pid of the locking instance
     * in case if it's stale/hanging/crashed (to make user know which PID he needs to kill).
     * D-Bus mechanism is the primary one.
     *
     * Since there is a kind of "logic tree", the code is a bit hard.
     */
    const QString service = dBusServiceNameForSession( sessionId );
    QDBusConnection connection = QDBusConnection::sessionBus();
    QDBusConnectionInterface* connectionInterface = connection.interface();

    const QString lockFilename = lockFileForSession( sessionId );
    KLockFile::Ptr lockFile(new KLockFile( lockFilename ));

    bool canLockDBus = !connectionInterface->isServiceRegistered( service );
    bool lockedDBus = false;

    // Lock D-Bus if we can and we need to
    if( doLocking && canLockDBus ) {
        lockedDBus = connection.registerService( service );
    }

    // Attempt to lock file, despite the possibility to do so and presence of the request (doLocking)
    // This is required as KLockFile::getLockInfo() works only after KLockFile::lock() is called
    KLockFile::LockResult lockResult = attemptRelock(lockFile);
    SessionRunInfo runInfo;
    if( lockResult == KLockFile::LockOK ) {
        // Unlock immediately if we shouldn't have locked it
        if( !lockedDBus ) {
            lockFile->unlock();
        }
    } else {
        // If locking failed, retrieve the lock's metadata
        lockFile->getLockInfo( runInfo.holderPid, runInfo.holderHostname, runInfo.holderApp );

        if( lockedDBus ) {
            // Since the lock-file is secondary, try to force-lock it if D-Bus locking succeeded
            forceRemoveLockfile(lockFilename);
            lockResult = attemptRelock(lockFile);
            Q_ASSERT(lockResult == KLockFile::LockOK);
        }
    }

    // Set the result by D-Bus status
    if (doLocking && lockedDBus) {
        SessionLockPrivate* lockData = new SessionLockPrivate(sessionId, lockFile);
        return TryLockSessionResult(QSharedPointer<ISessionLock>(new SessionLock(lockData)));
    } else {
        runInfo.isRunning = !canLockDBus;
        return TryLockSessionResult(runInfo);
    }
}

QString SessionLock::id()
{
    return d->id;
}

SessionLock::SessionLock(SessionLockPrivate* data)
: d(data)
{
}

SessionLock::~SessionLock()
{
    d->lockFile->unlock();
    bool unregistered = QDBusConnection::sessionBus().unregisterService( dBusServiceNameForSession(d->id) );
    Q_ASSERT(unregistered);
    Q_UNUSED(unregistered);
}

QString SessionLock::handleLockedSession(const QString& sessionName, const QString& sessionId,
                                         const SessionRunInfo& runInfo)
{
    if( !runInfo.isRunning ) {
        return sessionId;
    }

    // try to make the locked session active
    {
        // The timeout for "ensureVisible" call
        // Leave it sufficiently low to avoid waiting for hung instances.
        static const int timeout_ms = 1000;

        QDBusMessage message = QDBusMessage::createMethodCall( dBusServiceNameForSession(sessionId),
                                                                "/kdevelop/MainWindow",
                                                                "org.kdevelop.MainWindow",
                                                                "ensureVisible" );
        QDBusMessage reply = QDBusConnection::sessionBus().call( message,
                                                                    QDBus::Block,
                                                                    timeout_ms );
        if( reply.type() == QDBusMessage::ReplyMessage ) {
            qDebug() << i18nc( "@info:shell", "made running %1 instance (PID: %2) visible", runInfo.holderApp, runInfo.holderPid );
            return QString();
        } else {
            kWarning() << i18nc("@info:shell", "running %1 instance (PID: %2) is apparently hung", runInfo.holderApp, runInfo.holderPid);
        }
    }

    // otherwise ask the user whether we should retry
    QString problemDescription = i18nc("@info",
                                "The given application did not respond to a DBUS call, "
                                "it may have crashed or is hanging.");

    QString problemHeader;
    if( runInfo.holderPid != -1 ) {
        problemHeader = i18nc("@info", "Failed to lock the session <em>%1</em>, "
                              "already locked by %2 on %3 (PID %4).",
                              sessionName, runInfo.holderApp, runInfo.holderHostname, runInfo.holderPid);
    } else {
        problemHeader = i18nc("@info", "Failed to lock the session <em>%1</em> (lock-file unavailable).",
                              sessionName);
    }

    QString problemResolution = i18nc("@info", "<p>Please, close the offending application instance "
                                  "or choose another session to launch.</p>");

    QString errmsg = "<p>" + problemHeader + "<br>" + problemDescription + "</p>" + problemResolution;

    KGuiItem retry = KStandardGuiItem::cont();
    retry.setText(i18nc("@action:button", "Retry startup"));

    KGuiItem choose = KStandardGuiItem::configure();
    choose.setText(i18nc("@action:button", "Choose another session"));

    KGuiItem cancel = KStandardGuiItem::quit();
    int ret = KMessageBox::warningYesNoCancel(0, errmsg, i18nc("@title:window", "Failed to Lock Session %1", sessionName),
                                              retry, choose, cancel);
    switch( ret ) {
    case KMessageBox::Yes:
        return sessionId;
        break;

    case KMessageBox::No: {
        QString errmsg = i18nc("@info", "The session %1 is already active in another running instance.",
                               sessionName);
        return SessionController::showSessionChooserDialog(errmsg);
        break;
    }

    case KMessageBox::Cancel:
    default:
        break;
    }

    return QString();
}
