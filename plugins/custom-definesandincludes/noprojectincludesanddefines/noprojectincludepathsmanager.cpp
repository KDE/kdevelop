/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "noprojectincludepathsmanager.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <serialization/indexedstring.h>

#include "noprojectcustomincludepaths.h"

namespace
{
inline QString includePathsFile() { return QStringLiteral(".kdev_include_paths"); }


bool removeSettings(const QString& storageDirectory)
{
    const QString file = storageDirectory + QDir::separator() + includePathsFile();
    return QFile::remove(file);
}

QStringList pathListToStringList(const Path::List& paths)
{
    QStringList sl;
    sl.reserve(paths.size());
    for (const auto& p : paths) {
        sl << p.path();
    }
    return sl;
}
}

QString NoProjectIncludePathsManager::findConfigurationFile(const QString& path)
{
    QDir dir(path);
    while (dir.exists()) {
        QFileInfo customIncludePathsFile(dir, includePathsFile());
        if (customIncludePathsFile.exists()) {
            return customIncludePathsFile.absoluteFilePath();
        }

        if (!dir.cdUp()) {
            break;
        }
    }
    return {};
}

std::pair<Path::List, QHash<QString, QString>> 
    NoProjectIncludePathsManager::includesAndDefines(const QString& path)
{
    QFileInfo fi(path);

    auto pathToFile = findConfigurationFile(fi.absoluteDir().absolutePath());
    if (pathToFile.isEmpty()) {
        return {};
    }
    Path::List includes;
    QHash<QString, QString> defines;

    QFile f(pathToFile);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString fileContent = QString::fromLocal8Bit(f.readAll());
        const auto lines = QStringView(fileContent).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QFileInfo dir(pathToFile);
        const QChar dirSeparator = QDir::separator();
        for (const auto& line : lines) {
            const auto textLine = line.trimmed().toString();
            if (textLine.startsWith(QLatin1String("#define "))) {
                QStringList items = textLine.split(QLatin1Char(' '));
                if (items.length() > 1)
                {
                    defines[items[1]] = QStringList(items.mid(2)).join(QLatin1Char(' '));
                }else{
                    qWarning() << i18n("Bad #define directive in %1: %2", pathToFile, textLine);
                }
                continue;
            }
            if (!textLine.isEmpty()) {
                QFileInfo pathInfo(textLine);
                if (pathInfo.isRelative()) {
                    includes << Path(dir.canonicalPath() + dirSeparator + textLine);
                } else {
                    includes << Path(textLine);
                }
            }
        }
        f.close();
    }
    return std::make_pair(includes, defines);
}

bool NoProjectIncludePathsManager::writeIncludePaths(const QString& storageDirectory, const QStringList& includePaths)
{
    QDir dir(storageDirectory);
    QFileInfo customIncludePaths(dir, includePathsFile());
    QFile f(customIncludePaths.filePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&f);
        for (const auto& customPath : includePaths) {
            out << customPath << QLatin1Char('\n');
        }
        if (includePaths.isEmpty()) {
            removeSettings(storageDirectory);
        }
        return true;
    } else {
        return false;
    }
}

void NoProjectIncludePathsManager::openConfigurationDialog(const QString& path)
{
    auto cip = new NoProjectCustomIncludePaths;
    cip->setAttribute(Qt::WA_DeleteOnClose);
    cip->setModal(true);

    QFileInfo fi(path);
    auto dir = fi.absoluteDir().absolutePath();
    cip->setStorageDirectory(dir);

    auto paths = includesAndDefines(path).first;

    cip->setCustomIncludePaths(pathListToStringList(paths));

    QObject::connect(cip, &QDialog::accepted, cip, [this, cip, &path]() {
        if (!writeIncludePaths(cip->storageDirectory(), cip->customIncludePaths())) {
            qWarning() << i18n("Failed to save custom include paths in directory: %1", cip->storageDirectory());
        }
        KDevelop::ICore::self()->languageController()->backgroundParser()->addDocument(KDevelop::IndexedString(path));
    });
}
