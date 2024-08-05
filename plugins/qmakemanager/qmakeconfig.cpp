/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakeconfig.h"

#include <QDir>
#include <QMutex>
#include <QFileInfo>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KProcess>

#include <interfaces/iproject.h>
#include <util/path.h>
#include <debug.h>

namespace {
QString systemQmakeExecutable()
{
    for (const auto& candidate : {QStringLiteral("qmake"), QStringLiteral("qmake-qt5"), QStringLiteral("qmake-qt4")}) {
        auto executable = QStandardPaths::findExecutable(candidate);
        if (!executable.isEmpty()) {
            return executable;
        }
    }
    return {};
}
} // unnamed namespace

QString QMakeConfig::CONFIG_GROUP()
{
    return QStringLiteral("QMake_Builder");
}

// TODO: migrate to more generic & consistent key term "QMake_Executable"
const char QMakeConfig::QMAKE_EXECUTABLE[] = "QMake_Binary";
const char QMakeConfig::BUILD_FOLDER[] = "Build_Folder";
const char QMakeConfig::INSTALL_PREFIX[] = "Install_Prefix";
const char QMakeConfig::EXTRA_ARGUMENTS[] = "Extra_Arguments";
const char QMakeConfig::BUILD_TYPE[] = "Build_Type";
const char QMakeConfig::ALL_BUILDS[] = "All_Builds";

using namespace KDevelop;

/// NOTE: KConfig is not thread safe
QMutex s_buildDirMutex;

bool QMakeConfig::isConfigured(const IProject* project)
{
    QMutexLocker lock(&s_buildDirMutex);
    KConfigGroup cg(project->projectConfiguration(), CONFIG_GROUP());
    return cg.exists() && cg.hasKey(QMAKE_EXECUTABLE) && cg.hasKey(BUILD_FOLDER);
}

Path QMakeConfig::buildDirFromSrc(const IProject* project, const Path& srcDir)
{
    QMutexLocker lock(&s_buildDirMutex);
    KConfigGroup cg(project->projectConfiguration(), QMakeConfig::CONFIG_GROUP());
    Path buildDir = Path(cg.readEntry(QMakeConfig::BUILD_FOLDER, QString()));
    lock.unlock();

    if (buildDir.isValid()) {
        buildDir.addPath(project->path().relativePath(srcDir));
    }
    return buildDir;
}

QString QMakeConfig::qmakeExecutable(const IProject* project)
{
    QMutexLocker lock(&s_buildDirMutex);
    QString exe;
    if (project) {
        KSharedConfig::Ptr cfg = project->projectConfiguration();
        KConfigGroup group(cfg.data(), CONFIG_GROUP());
        if (group.hasKey(QMAKE_EXECUTABLE)) {
            exe = group.readEntry(QMAKE_EXECUTABLE, QString());
            QFileInfo info(exe);
            if (!info.exists() || !info.isExecutable()) {
                qCWarning(KDEV_QMAKE) << "bad QMake configured for project " << project->path().toUrl() << ":" << exe;
                exe.clear();
            }
        }
    }
    if (exe.isEmpty()) {
        static const auto fallbackQmakeExecutable = systemQmakeExecutable();
        exe = fallbackQmakeExecutable;
    }
    Q_ASSERT(!exe.isEmpty());
    return exe;
}

QHash<QString, QString> QMakeConfig::queryQMake(const QString& qmakeExecutable, const QStringList& args)
{
    QHash<QString, QString> hash;
    KProcess p;
    p.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    p << qmakeExecutable << QStringLiteral("-query") << args;

    const int rc = p.execute();
    if (rc != 0) {
        qCWarning(KDEV_QMAKE) << "failed to execute qmake query " << p.program().join(QLatin1Char(' ')) << "return code was:" << rc;
        return QHash<QString, QString>();
    }

    // TODO: Qt 5.5: Use QTextStream::readLineInto
    QTextStream stream(&p);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const int colon = line.indexOf(QLatin1Char(':'));
        if (colon == -1) {
            continue;
        }

        const auto key = line.left(colon);
        const auto value = line.mid(colon + 1);
        hash.insert(key, value);
    }
    qCDebug(KDEV_QMAKE) << "Ran qmake (" << p.program().join(QLatin1Char(' ')) << "), found:" << hash;
    return hash;
}

QString QMakeConfig::findBasicMkSpec(const QHash<QString, QString>& qmakeVars)
{
    QStringList paths;
    if (qmakeVars.contains(QStringLiteral("QMAKE_MKSPECS"))) {
        // qt4
        const auto mkspecDirs = qmakeVars[QStringLiteral("QMAKE_MKSPECS")].split(QDir::listSeparator());
        for (const auto& dir : mkspecDirs) {
            paths << dir + QLatin1String("/default/qmake.conf");
        }
    } else if (!qmakeVars.contains(QStringLiteral("QMAKE_MKSPECS")) && qmakeVars.contains(QStringLiteral("QMAKE_SPEC"))) {
        QString path;
        // qt5 doesn't have the MKSPECS nor default anymore
        // let's try to look up the mkspec path ourselves,
        // see QMakeEvaluator::updateMkspecPaths() in QMake source code as reference
        if (qmakeVars.contains(QStringLiteral("QT_HOST_DATA/src"))) {
            // >=qt5.2: since 0d463c05fc4f2e79e5a4e5a5382a1e6ed5d2615e (in Qt5 qtbase repository)
            // mkspecs are no longer copied to the build directory.
            // instead, we have to look them up in the source directory.
            // this commit also introduced the key 'QT_HOST_DATA/src' which we use here
            path = qmakeVars[QStringLiteral("QT_HOST_DATA/src")];
        } else if (qmakeVars.contains(QStringLiteral("QT_HOST_DATA"))) {
            // cross compilation
            path = qmakeVars[QStringLiteral("QT_HOST_DATA")];
        } else {
            Q_ASSERT(qmakeVars.contains(QStringLiteral("QT_INSTALL_PREFIX")));
            path = qmakeVars[QStringLiteral("QT_INSTALL_PREFIX")];
        }
        path += QLatin1String("/mkspecs/") + qmakeVars[QStringLiteral("QMAKE_SPEC")] + QLatin1String("/qmake.conf");
        paths << path;
    }

    for (const auto& path : std::as_const(paths)) {
        QFileInfo fi(path);
        if (fi.exists())
            return fi.absoluteFilePath();
    }

    return QString();
}
