/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "noprojectincludepathsmanager.h"

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <serialization/indexedstring.h>

#include "noprojectcustomincludepaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include <utility>

namespace
{
inline QString includePathsFile() { return QStringLiteral(".kdev_include_paths"); }

QDir absoluteParentDirForPath(const QString& path)
{
    return QFileInfo{path}.absoluteDir();
}

QString findConfigurationFileForDir(QDir dir)
{
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

struct ConfigurationFile
{
    QString filePath;
    QString fileContents;
};

ConfigurationFile readConfigurationFileForDir(QDir dir)
{
    ConfigurationFile ret;

    ret.filePath = findConfigurationFileForDir(std::move(dir));
    if (ret.filePath.isEmpty()) {
        return ret;
    }

    QFile file(ret.filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ret.fileContents = QString::fromUtf8(file.readAll());
    }

    return ret;
}

std::pair<Path::List, QHash<QString, QString>> includesAndDefines(const QString& path, bool gatherIncludes,
                                                                  bool gatherDefines)
{
    Path::List includes;
    QHash<QString, QString> defines;

    const auto configurationFile = readConfigurationFileForDir(absoluteParentDirForPath(path));
    if (!configurationFile.fileContents.isEmpty()) {
        const auto lines = QStringView{configurationFile.fileContents}.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        const QFileInfo dir(configurationFile.filePath);
        const QChar dirSeparator = QDir::separator();
        for (auto line : lines) {
            line = line.trimmed();
            if (line.empty()) {
                continue;
            }

            // Do not support whitespace between parameters of function-like macros for simplicity.
            // Though such whitespace usually still works in practice, because KDevelop joins the macro
            // identifier and replacement-list into a #define directive line and feeds it to libclang.
            static const QRegularExpression defineRegex(QStringLiteral("^#define\\s+(\\S+)(?:\\s+(.*))?$"));
            if (const auto match = defineRegex.matchView(line); match.hasMatch()) {
                if (gatherDefines) {
                    defines.insert(match.captured(1), match.capturedView(2).trimmed().toString());
                }
                continue;
            }

            if (!gatherIncludes) {
                continue;
            }
            const auto textLine = line.toString();
            QFileInfo pathInfo(textLine);
            if (pathInfo.isRelative()) {
                includes << Path(dir.canonicalPath() + dirSeparator + textLine);
            } else {
                includes << Path(textLine);
            }
        }
    }
    return std::make_pair(includes, defines);
}

} // unnamed namespace

Path::List NoProjectIncludePathsManager::includes(const QString& path)
{
    return includesAndDefines(path, true, false).first;
}

QHash<QString, QString> NoProjectIncludePathsManager::defines(const QString& path)
{
    return includesAndDefines(path, false, true).second;
}

static bool writeIncludePaths(const QString& storageDirectory, QStringView includePaths)
{
    QDir dir(storageDirectory);
    QFileInfo customIncludePaths(dir, includePathsFile());
    QFile f(customIncludePaths.filePath());

    includePaths = includePaths.trimmed();
    if (includePaths.empty()) {
        return f.exists() ? f.remove() : true;
    }
    return f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)
        && f.write(includePaths.toUtf8()) != -1;
}

void NoProjectIncludePathsManager::openConfigurationDialog(const QString& path)
{
    auto cip = new NoProjectCustomIncludePaths;
    cip->setAttribute(Qt::WA_DeleteOnClose);
    cip->setModal(true);

    {
        auto dir = absoluteParentDirForPath(path);
        cip->setStorageDirectory(dir.path());
        cip->setCustomIncludePaths(readConfigurationFileForDir(std::move(dir)).fileContents);
    }

    QObject::connect(cip, &QDialog::accepted, cip, [cip, path] {
        if (!writeIncludePaths(cip->storageDirectory(), cip->customIncludePaths())) {
            qWarning() << i18n("Failed to save custom include paths in directory: %1", cip->storageDirectory());
        }
        KDevelop::ICore::self()->languageController()->backgroundParser()->addDocument(KDevelop::IndexedString(path));
    });

    cip->show();
}
