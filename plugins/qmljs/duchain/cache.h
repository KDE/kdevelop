/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __CACHE_H__
#define __CACHE_H__

#include "duchainexport.h"
#include <serialization/indexedstring.h>
#include <util/path.h>

#include <QHash>
#include <QString>
#include <QFileInfoList>
#include <QList>
#include <QSet>
#include <QMutex>

class QStringList;

namespace QmlJS
{

/**
 * Cache for values that may be slow to compute (search paths, things
 * involving QStandardPaths, etc)
 */
class KDEVQMLJSDUCHAIN_EXPORT Cache
{
private:
    Cache();

public:
    static Cache& instance();

    /**
     * List of the paths in which the modules will be looked for
     */
    KDevelop::Path::List libraryPaths(const KDevelop::IndexedString& baseFile) const;

    /**
     * Path corresponding to a module name
     *
     * The path returned can either be the one of a module file shipped with
     * kdev-qmljs, or the path of a directory containing the module components
     * (one .qml file per component, and possibly .so files if the module has
     * native components)
     *
     * @param version If not null, the file being looked for is "uri_version.qml".
     *                If null, then the untouched uri is used for searching.
     */
    QString modulePath(const KDevelop::IndexedString& baseFile, const QString& uri,
                       const QString& version = QStringLiteral("2.0"));

    /**
     * Return the list of the paths of the given files.
     *
     * Files having a name ending in ".so" are replaced with the path of their
     * qmlplugindump dump.
     */
    QStringList getFileNames(const QFileInfoList& fileInfos);

    /**
     * Set the custom include directories list of a file
     */
    void setFileCustomIncludes(const KDevelop::IndexedString& file,
                               const KDevelop::Path::List& dirs);

    /**
     * Add a dependency between two URLs
     */
    void addDependency(const KDevelop::IndexedString& file, const KDevelop::IndexedString& dependency);

    /**
     * List of the files that depend on a given URL
     */
    QList<KDevelop::IndexedString> filesThatDependOn(const KDevelop::IndexedString& file);

    /**
     * List of the dependencies of a file
     */
    QList<KDevelop::IndexedString> dependencies(const KDevelop::IndexedString& file);

    /**
     * Return whether a file is up to date (all its dependencies are up to date
     * and the file has been freshly parsed)
     */
    bool isUpToDate(const KDevelop::IndexedString& file);
    void setUpToDate(const KDevelop::IndexedString& file, bool upToDate);

private:
    KDevelop::Path::List libraryPaths_internal(const KDevelop::IndexedString& baseFile) const;

    struct PluginDumpExecutable {
        QString executable;
        QString quickVersion;       // Version of QtQuick that should be imported when this qmlplugindump is used

        PluginDumpExecutable(const QString& e, const QString &v)
        : executable(e), quickVersion(v)
        {}
    };

    mutable QMutex m_mutex;
    QHash<QString, QString> m_modulePaths;
    QList<PluginDumpExecutable> m_pluginDumpExecutables;
    QHash<KDevelop::IndexedString, QSet<KDevelop::IndexedString>> m_dependees;
    QHash<KDevelop::IndexedString, QSet<KDevelop::IndexedString>> m_dependencies;
    QHash<KDevelop::IndexedString, bool> m_isUpToDate;
    QHash<KDevelop::IndexedString, KDevelop::Path::List> m_includeDirs;
};

}

#endif
