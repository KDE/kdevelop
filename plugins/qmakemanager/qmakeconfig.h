/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKECONFIG_H
#define QMAKECONFIG_H

#include <QHash>
#include <QStringList>

namespace KDevelop {
class IProject;
class Path;
}

class QMakeConfig
{
public:
    static const QLatin1String CONFIG_GROUP;

    static const QLatin1String
        QMAKE_EXECUTABLE,
        BUILD_FOLDER,
        INSTALL_PREFIX,
        EXTRA_ARGUMENTS,
        BUILD_TYPE,
        ALL_BUILDS;

    /**
     * Returns true when the given project is sufficiently configured.
     */
    static bool isConfigured(const KDevelop::IProject* project);

    /**
     * Returns the directory where srcDir will be built.
     * srcDir must contain a *.pro file !
     */
    static KDevelop::Path buildDirFromSrc(const KDevelop::IProject* project, const KDevelop::Path& srcDir);

    /**
     * Returns the QMake executable configured for the given @p project.
     */
    static QString qmakeExecutable(const KDevelop::IProject* project);

    /**
     * Query QMake and return the thus obtained QMake variables.
     */
    static QHash<QString, QString> queryQMake(const QString& qmakeExecutable, const QStringList& args = {});

    /**
     * Given the QMake variables, try to find a basic MkSpec.
     */
    static QString findBasicMkSpec( const QHash<QString,QString>& qmakeVars );
};

#endif
