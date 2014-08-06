/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2013 Sven Brauch <svenbrauch@googlemail.com>
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "cache.h"

#include <QString>
#include <QProcess>
#include <QDir>
#include <QCryptographicHash>

#include <kglobal.h>
#include <kstandarddirs.h>

QmlJS::Cache::Cache()
{
    // qmlplugindump from Qt4 and Qt5. They will be tried in order when dumping
    // a binary QML file.
    m_pluginDumpExecutables
        << PluginDumpExecutable("qmlplugindump", "1.0")
        << PluginDumpExecutable("qmlplugindump-qt5", "2.0")
        << PluginDumpExecutable("qml1plugindump-qt5", "1.0");
}

QmlJS::Cache& QmlJS::Cache::instance()
{
    static Cache *c = nullptr;

    if (!c) {
        c = new Cache();
    }

    return *c;
}

QString QmlJS::Cache::modulePath(const QString& uri, const QString& version)
{
    QMutexLocker lock(&m_mutex);
    QString cacheKey = uri + version;
    QString path = m_modulePaths.value(cacheKey, QString());

    if (!path.isNull()) {
        return path;
    }

    // Look for <uri>_<version>.qml in the shipped files
    QString fileName = (version.isNull() ? uri : QString("%1_%2.qml").arg(uri, version));
    path = KGlobal::dirs()->findResource("data",
        QString("kdevqmljssupport/qmlplugins/%1").arg(fileName)
    );

    if (!path.isNull()) {
        m_modulePaths.insert(cacheKey, path);
        return path;
    }

    // Look for <uri> (with the dots replaced with slashes) in the standard KDE
    // QML imports dir.
    QStringList dirs = KGlobal::dirs()->findDirs("module",
        QString("imports/%1").arg(QString(uri).replace(QLatin1Char('.'), QDir::separator()))
    );

    if (dirs.count() != 0) {
        path = dirs.first();
    }

    m_modulePaths.insert(cacheKey, path);
    return path;
}

QStringList QmlJS::Cache::getFileNames(const QFileInfoList& fileInfos)
{
    QMutexLocker lock(&m_mutex);
    QStringList result;
    KStandardDirs d;

    for (const QFileInfo& fileInfo : fileInfos) {
        QString filePath = fileInfo.canonicalFilePath();

        if (!filePath.endsWith(QLatin1String(".so"))) {
            result.append(filePath);
            continue;
        }

        // Use the cache to speed-up reparses
        if (m_modulePaths.contains(filePath)) {
            QString cachedFilePath = m_modulePaths.value(filePath);

            if (!cachedFilePath.isEmpty()) {
                result.append(m_modulePaths.value(filePath));
            }

            continue;
        }

        // Locate an existing dump of the file
        QString dumpHash = QString::fromUtf8(
            QCryptographicHash::hash(filePath.toUtf8(), QCryptographicHash::Md5).toHex()
        ) + QLatin1String("_0.0.qml");
        QString dumpPath = d.findResource("data",
            QString("kdevqmljssupport/%1").arg(dumpHash)
        );

        if (!dumpPath.isNull()) {
            result.append(dumpPath);
            m_modulePaths.insert(filePath, dumpPath);
            continue;
        }

        // Create a dump of the file
        QStringList args;
        args << QLatin1String("-noinstantiate") << QLatin1String("-path") << filePath;

        for (const PluginDumpExecutable& executable : m_pluginDumpExecutables) {
            QProcess qmlplugindump;

            qmlplugindump.setProcessChannelMode(QProcess::SeparateChannels);
            qmlplugindump.setWorkingDirectory(fileInfo.absolutePath());
            qmlplugindump.start(executable.executable, args);

            if (!qmlplugindump.waitForFinished(3000) ||
                qmlplugindump.exitCode() != 0) {
                continue;
            }

            // Open a file in which the dump can be written
            QFile dumpFile(
                d.saveLocation("data", QLatin1String("kdevqmljssupport")) +
                dumpHash
            );

            if (dumpFile.open(QIODevice::WriteOnly)) {
                qmlplugindump.readLine();   // Skip "import QtQuick.tooling 1.1"

                dumpFile.write("// " + filePath.toUtf8() + "\n");
                dumpFile.write("import QtQuick " + executable.quickVersion.toUtf8() + "\n");
                dumpFile.write(qmlplugindump.readAllStandardOutput());
                dumpFile.close();

                result.append(dumpFile.fileName());
                m_modulePaths.insert(filePath, dumpFile.fileName());
                break;
            }
        }
    }

    return result;
}

void QmlJS::Cache::addDependency(const KDevelop::IndexedString& file, const KDevelop::IndexedString& dependency)
{
    QMutexLocker lock(&m_mutex);

    m_dependees[dependency].append(file);
    m_dependencies[file].append(dependency);
}

QList<KDevelop::IndexedString> QmlJS::Cache::filesThatDependOn(const KDevelop::IndexedString& file)
{
    QMutexLocker lock(&m_mutex);

    return m_dependees[file];
}

QList<KDevelop::IndexedString> QmlJS::Cache::dependencies(const KDevelop::IndexedString& file)
{
    QMutexLocker lock(&m_mutex);

    return m_dependencies[file];
}

bool QmlJS::Cache::isUpToDate(const KDevelop::IndexedString& file)
{
    QMutexLocker lock(&m_mutex);

    return m_isUpToDate.value(file, false);
}

void QmlJS::Cache::setUpToDate(const KDevelop::IndexedString& file, bool upToDate)
{
    QMutexLocker lock(&m_mutex);

    m_isUpToDate[file] = upToDate;
}
