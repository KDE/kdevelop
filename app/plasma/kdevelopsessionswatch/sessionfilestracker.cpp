/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessionfilestracker.h"

// library
#include <kdevelopsessionsobserver.h>
// KF
#include <KDirWatch>
#include <KConfig>
#include <KConfigGroup>
// Qt
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>
#include <QMutexLocker>
#include <QMetaObject>
#include <QCoreApplication>
// Std
#include <algorithm>

namespace {
namespace Strings {
QString sessionConfigFileName() { return QStringLiteral("sessionrc"); }
}
}

Q_GLOBAL_STATIC(SessionFilesTracker, s_SessionFilesTrackerInstance)

SessionFilesTracker *SessionFilesTracker::instance()
{
    return s_SessionFilesTrackerInstance();
}

static void setSessionDataList(QObject* observer, const QVector<KDevelopSessionData>& sessionDataList)
{
    QMetaObject::invokeMethod(observer, "setSessionDataList", Qt::AutoConnection,
                              Q_ARG(QVector<KDevelopSessionData>, sessionDataList));
}

static void cleanupSessionFilesTracker()
{
    if (s_SessionFilesTrackerInstance.exists()) {
        s_SessionFilesTrackerInstance->cleanup();
    }
}


SessionFilesTracker::SessionFilesTracker()
    : QObject()
    , m_dirWatch(new KDirWatch(this))
{
    // KDirWatch might have QFileSystemWatcher,
    // which wants to be deleted before qApp is gone - bug 261541
    qAddPostRoutine(cleanupSessionFilesTracker);

    m_sessionDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kdevelop/sessions");
    m_dirWatch->stopScan();
    m_dirWatch->addDir(m_sessionDir, KDirWatch::WatchSubDirs);

    connect(m_dirWatch, &KDirWatch::dirty, this, &SessionFilesTracker::sessionSourceChanged);

    updateSessions();
}

SessionFilesTracker::~SessionFilesTracker() = default;

void SessionFilesTracker::cleanup()
{
    delete m_dirWatch;
    m_dirWatch = nullptr;
}


void SessionFilesTracker::registerObserver(QObject* observer)
{
    if (!qobject_cast<KDevelopSessionsObserver*>(observer)) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    const bool isFirstObserver = m_observers.isEmpty();

    m_observers.append(observer);
    setSessionDataList(observer, m_sessionDataList);

    if (isFirstObserver) {
        m_dirWatch->startScan(true);
    }
}

void SessionFilesTracker::unregisterObserver(QObject* observer)
{
    if (!qobject_cast<KDevelopSessionsObserver*>(observer)) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    m_observers.removeOne(observer);

    if (m_observers.isEmpty()) {
        m_dirWatch->stopScan();
    }
}

QVector<KDevelopSessionData> SessionFilesTracker::readSessionDataList() const
{
    QVector<KDevelopSessionData> sessions;

    QDir sessionBaseDir(m_sessionDir);
    const auto dirEntries = sessionBaseDir.entryList(QDir::Dirs);
    sessions.reserve(dirEntries.size());
    for (const QString& sessionDirName : dirEntries) {
        if (QUuid(sessionDirName).isNull()) {
            continue;
        }

        QDir sessionDir(sessionBaseDir.absoluteFilePath(sessionDirName));
        const QString sessionConfigFilePath = sessionDir.filePath(Strings::sessionConfigFileName());
        if (!QFile::exists(sessionConfigFilePath)) {
            continue;
        }

        KConfig sessionDataStorage(sessionConfigFilePath, KConfig::SimpleConfig);
        const KConfigGroup mainSessionData = sessionDataStorage.group(QString());

        const KDevelopSessionData sessionData {
            sessionDirName,
            mainSessionData.readEntry("SessionName"),
            mainSessionData.readEntry("SessionPrettyContents"),
        };

        sessions.append(sessionData);
    }
    sessions.squeeze();
    std::sort(sessions.begin(), sessions.end(), [](const KDevelopSessionData& a, const KDevelopSessionData& b) {
        return a.id < b.id;   
    });

    return sessions;
}

void SessionFilesTracker::sessionSourceChanged(const QString& path)
{
    // session data is not changed too often, so we are fine to simply rescan all the data
    // in case any relevant files (or the main dir) have been touched

    // session dirs removed or added?
    if (m_sessionDir == path) {
        updateSessions();
    } else {
        // session config file?
        QFileInfo pathInfo(path);
        if (pathInfo.fileName() == Strings::sessionConfigFileName()) {
            updateSessions();
        }
    }
}

void SessionFilesTracker::updateSessions()
{
    QMutexLocker lock(&m_mutex);

    const auto newSessionDataList = readSessionDataList();

    if (m_sessionDataList == newSessionDataList) {
        return;
    }

    m_sessionDataList = newSessionDataList;

    for (auto* observer : std::as_const(m_observers)) {
        setSessionDataList(observer, m_sessionDataList);
    }
}

#include "moc_sessionfilestracker.cpp"
