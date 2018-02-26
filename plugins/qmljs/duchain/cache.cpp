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
#include "debug.h"

#include <QString>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QCoreApplication>

QmlJS::Cache::Cache()
{
    // qmlplugindump from Qt4 and Qt5. They will be tried in order when dumping
    // a binary QML file.
    m_pluginDumpExecutables
        << PluginDumpExecutable(QStringLiteral("qmlplugindump"), QStringLiteral("1.0"))
        << PluginDumpExecutable(QStringLiteral("qmlplugindump-qt4"), QStringLiteral("1.0"))
        << PluginDumpExecutable(QStringLiteral("qmlplugindump-qt5"), QStringLiteral("2.0"))
        << PluginDumpExecutable(QStringLiteral("qml1plugindump-qt5"), QStringLiteral("1.0"));
}

QmlJS::Cache& QmlJS::Cache::instance()
{
    static Cache *c = nullptr;

    if (!c) {
        c = new Cache();
    }

    return *c;
}

QString QmlJS::Cache::modulePath(const KDevelop::IndexedString& baseFile,
                                 const QString& uri,
                                 const QString& version)
{
    QMutexLocker lock(&m_mutex);
    QString cacheKey = uri + version;
    QString path = m_modulePaths.value(cacheKey, QString());

    if (!path.isNull()) {
        return path;
    }

    // List of the paths in which the modules will be looked for
    KDevelop::Path::List paths;

    for (auto path : QCoreApplication::instance()->libraryPaths()) {
        KDevelop::Path p(path);

        // Change /path/to/qt5/plugins to /path/to/qt5/{qml,imports}
        paths << p.cd(QStringLiteral("../qml"));
        paths << p.cd(QStringLiteral("../imports"));
    }

    paths << m_includeDirs[baseFile];

    // Find the path for which <path>/u/r/i exists
    QString fragment = QString(uri).replace(QLatin1Char('.'), QDir::separator());
    bool isVersion1 = version.startsWith(QLatin1String("1."));
    bool isQtQuick = (uri == QLatin1String("QtQuick"));

    const QStringList modulesWithoutVersionSuffix{"QtQml",
                                                  "QtMultimedia",
                                                  "QtQuick.LocalStorage",
                                                  "QtQuick.XmlListModel"};

    if (!version.isEmpty() && !isVersion1 && !modulesWithoutVersionSuffix.contains(uri)) {
        // Modules having a version greater or equal to 2 are stored in a directory
        // name like QtQuick.2
        fragment += QLatin1Char('.') + version.section(QLatin1Char('.'), 0, 0);
    }

    for (auto p : paths) {
        QString pathString = p.cd(fragment).path();

        // HACK: QtQuick 1.0 is put in $LIB/qt5/imports/builtins.qmltypes. The "QtQuick"
        //       identifier appears nowhere.
        if (isQtQuick && isVersion1) {
            if (QFile::exists(p.cd(QStringLiteral("builtins.qmltypes")).path())) {
                path = p.path();
                break;
            }
        } else if (QFile::exists(pathString + QLatin1String("/plugins.qmltypes"))) {
            path = pathString;
            break;
        }
    }

    m_modulePaths.insert(cacheKey, path);
    return path;
}

QStringList QmlJS::Cache::getFileNames(const QFileInfoList& fileInfos)
{
    QStringList result;

    for (const QFileInfo& fileInfo : fileInfos) {
        QString filePath = fileInfo.canonicalFilePath();

        // If the module directory contains a plugins.qmltypes files, use it
        // and skip everything else
        if (filePath.endsWith(QLatin1String("plugins.qmltypes"))) {
            return QStringList() << filePath;
        } else if (fileInfo.dir().exists(QLatin1String("plugins.qmltypes"))) {
            return {fileInfo.dir().filePath(QLatin1String("plugins.qmltypes"))};
        }

        // Non-so files don't need any treatment
        if (!filePath.endsWith(QLatin1String(".so"))) {
            result.append(filePath);
            continue;
        }

        // Use the cache to speed-up reparses
        {
            QMutexLocker lock(&m_mutex);

            if (m_modulePaths.contains(filePath)) {
                QString cachedFilePath = m_modulePaths.value(filePath);

                if (!cachedFilePath.isEmpty()) {
                    result.append(cachedFilePath);
                }

                continue;
            }
        }

        // Locate an existing dump of the file
        QString dumpFile = QStringLiteral("kdevqmljssupport/%1.qml").arg(
            QString::fromLatin1(QCryptographicHash::hash(filePath.toUtf8(), QCryptographicHash::Md5).toHex())
        );
        QString dumpPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
            dumpFile
        );

        if (!dumpPath.isNull()) {
            QMutexLocker lock(&m_mutex);

            result.append(dumpPath);
            m_modulePaths.insert(filePath, dumpPath);
            continue;
        }

        // Create a dump of the file
        const QStringList args = {QStringLiteral("-noinstantiate"), QStringLiteral("-path"), filePath};

        for (const PluginDumpExecutable& executable : m_pluginDumpExecutables) {
            QProcess qmlplugindump;
            qmlplugindump.setProcessChannelMode(QProcess::SeparateChannels);
            qmlplugindump.start(executable.executable, args, QIODevice::ReadOnly);

            qCDebug(KDEV_QMLJS_DUCHAIN) << "starting qmlplugindump with args:" << executable.executable << args << qmlplugindump.state() << fileInfo.absolutePath();

            if (!qmlplugindump.waitForFinished(3000)) {
                if (qmlplugindump.state() == QProcess::Running) {
                    qCWarning(KDEV_QMLJS_DUCHAIN) << "qmlplugindump didn't finish in time -- killing";
                    qmlplugindump.kill();
                    qmlplugindump.waitForFinished(100);
                } else {
                    qCDebug(KDEV_QMLJS_DUCHAIN) << "qmlplugindump attempt failed" << qmlplugindump.program() << qmlplugindump.arguments() << qmlplugindump.readAllStandardError();
                }
                continue;
            }

            if (qmlplugindump.exitCode() != 0) {
                qCWarning(KDEV_QMLJS_DUCHAIN) << "qmlplugindump finished with exit code:" << qmlplugindump.exitCode();
                continue;
            }

            // Open a file in which the dump can be written
            QFile dumpFile(
                QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                + dumpPath
            );

            if (dumpFile.open(QIODevice::WriteOnly)) {
                qmlplugindump.readLine();   // Skip "import QtQuick.tooling 1.1"

                dumpFile.write("// " + filePath.toUtf8() + "\n");
                dumpFile.write("import QtQuick " + executable.quickVersion.toUtf8() + "\n");
                dumpFile.write(qmlplugindump.readAllStandardOutput());
                dumpFile.close();

                result.append(dumpFile.fileName());

                QMutexLocker lock(&m_mutex);
                m_modulePaths.insert(filePath, dumpFile.fileName());
                break;
            }
        }
    }

    return result;
}

void QmlJS::Cache::setFileCustomIncludes(const KDevelop::IndexedString& file, const KDevelop::Path::List& dirs)
{
    QMutexLocker lock(&m_mutex);

    m_includeDirs[file] = dirs;
}

void QmlJS::Cache::addDependency(const KDevelop::IndexedString& file, const KDevelop::IndexedString& dependency)
{
    QMutexLocker lock(&m_mutex);

    m_dependees[dependency].insert(file);
    m_dependencies[file].insert(dependency);
}

QList<KDevelop::IndexedString> QmlJS::Cache::filesThatDependOn(const KDevelop::IndexedString& file)
{
    QMutexLocker lock(&m_mutex);

    return m_dependees[file].toList();
}

QList<KDevelop::IndexedString> QmlJS::Cache::dependencies(const KDevelop::IndexedString& file)
{
    QMutexLocker lock(&m_mutex);

    return m_dependencies[file].toList();
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
