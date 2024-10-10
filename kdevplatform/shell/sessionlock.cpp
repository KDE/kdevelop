/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "sessionlock.h"

#include "debug.h"
#include "sessioncontroller.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QDBusConnectionInterface>
#include <QFile>
#include <QDir>

namespace KDevelop {
namespace {
QString lockFileForSession( const QString& id )
{
    return SessionController::sessionDirectory( id ) + QLatin1String("/lock");
}

QString dBusServiceNameForSession(QStringView id)
{
    Q_ASSERT(id.startsWith(QLatin1Char{'{'}));
    Q_ASSERT(id.endsWith(QLatin1Char{'}'}));
    // We remove starting "{" and ending "}" from the string UUID representation
    // as D-Bus apparently doesn't allow them in service names
    return QLatin1String("org.kdevelop.kdevplatform-lock-") + id.sliced(1, id.size() - 2);
}

/// Force-removes the lock-file.
void forceRemoveLockfile(const QString& lockFilename)
{
    if( QFile::exists( lockFilename ) ) {
        QFile::remove( lockFilename );
    }
}

} // unnamed namespace

TryLockSessionResult SessionLock::tryLockSession(const QString& sessionId, bool doLocking)
{
    ///FIXME: if this is hit, someone tried to lock a non-existing session
    ///       this should be fixed by using a proper data type distinct from
    ///       QString for id's, i.e. QUuid or similar.
    Q_ASSERT(QFile::exists(SessionController::sessionDirectory( sessionId )));

    /*
     * We've got two locking mechanisms here: D-Bus unique service name (based on the session id)
     * and a plain lockfile (QLockFile).
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
    QSharedPointer<QLockFile> lockFile(new QLockFile( lockFilename ));

    const bool haveDBus = connection.isConnected();
    const bool canLockDBus = haveDBus && connectionInterface && !connectionInterface->isServiceRegistered( service );
    bool lockedDBus = false;

    // Lock D-Bus if we can and we need to
    if( doLocking && canLockDBus ) {
        lockedDBus = connection.registerService( service );
    }

    // Attempt to lock file, despite the possibility to do so and presence of the request (doLocking)
    // This is required as QLockFile::getLockInfo() works only after QLockFile::lock() is called
    bool lockResult = lockFile->tryLock();
    SessionRunInfo runInfo;
    if (lockResult) {
        // Unlock immediately if we shouldn't have locked it
        if( haveDBus && !lockedDBus ) {
            lockFile->unlock();
        }
    } else {
        // If locking failed, retrieve the lock's metadata
        lockFile->getLockInfo(&runInfo.holderPid, &runInfo.holderHostname, &runInfo.holderApp );
        runInfo.isRunning = !haveDBus || !canLockDBus;

        if( haveDBus && lockedDBus ) {
            // Since the lock-file is secondary, try to force-lock it if D-Bus locking succeeded
            forceRemoveLockfile(lockFilename);
            lockResult = lockFile->tryLock();
            Q_ASSERT(lockResult);
        }
    }

    // Set the result by D-Bus status
    if (doLocking && (haveDBus ? lockedDBus : lockResult)) {
        return TryLockSessionResult(QSharedPointer<ISessionLock>(new SessionLock(sessionId, lockFile)));
    } else {
        return TryLockSessionResult(runInfo);
    }
}

QString SessionLock::id()
{
    return m_sessionId;
}

SessionLock::SessionLock(const QString& sessionId, const QSharedPointer<QLockFile>& lockFile)
: m_sessionId(sessionId)
, m_lockFile(lockFile)
{
    Q_ASSERT(lockFile->isLocked());
}

SessionLock::~SessionLock()
{
    m_lockFile->unlock();
    bool unregistered = QDBusConnection::sessionBus().unregisterService( dBusServiceNameForSession(m_sessionId) );
    Q_UNUSED(unregistered);
}

void SessionLock::removeFromDisk()
{
    Q_ASSERT(m_lockFile->isLocked());
    // unlock first to prevent warnings: "Could not remove our own lock file ..."
    m_lockFile->unlock();
    QDir(SessionController::sessionDirectory(m_sessionId)).removeRecursively();
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
                                                                QStringLiteral("/kdevelop/MainWindow"),
                                                                QStringLiteral("org.kdevelop.MainWindow"),
                                                                QStringLiteral("ensureVisible") );
        QDBusMessage reply = QDBusConnection::sessionBus().call( message,
                                                                    QDBus::Block,
                                                                    timeout_ms );
        if( reply.type() == QDBusMessage::ReplyMessage ) {
            QTextStream out(stdout);
            out << i18nc(
                "@info:shell",
                "Running %1 instance (PID: %2) detected, making this one visible instead of starting a new one",
                runInfo.holderApp, runInfo.holderPid)
                << Qt::endl;
            return QString();
        } else {
            qCWarning(SHELL) << i18nc("@info:shell", "Running %1 instance (PID: %2) is apparently hung", runInfo.holderApp, runInfo.holderPid);
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

    QString errmsg = QLatin1String("<p>") + problemHeader + QLatin1String("<br>") + problemDescription + QLatin1String("</p>") + problemResolution;

    KGuiItem retry = KStandardGuiItem::cont();
    retry.setText(i18nc("@action:button", "Retry Startup"));

    KGuiItem choose = KStandardGuiItem::configure();
    choose.setText(i18nc("@action:button", "Choose Another Session"));

    KGuiItem cancel = KStandardGuiItem::quit();
    int ret = KMessageBox::warningTwoActionsCancel(
        nullptr, errmsg, i18nc("@title:window", "Failed to Lock Session %1", sessionName), retry, choose, cancel);
    switch( ret ) {
    case KMessageBox::PrimaryAction:
        return sessionId;

    case KMessageBox::SecondaryAction: {
        QString errmsg = i18nc("@info", "The session %1 is already active in another running instance.",
                               sessionName);
        return SessionController::showSessionChooserDialog(errmsg);
    }

    case KMessageBox::Cancel:
    default:
        break;
    }

    return QString();
}

}
