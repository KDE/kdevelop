/* KDevelop QMake Support
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "qmakeconfig.h"

#include "qtcompat_p.h"

#include <QMutex>
#include <QFileInfo>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KProcess>

#include <interfaces/iproject.h>
#include <util/path.h>
#include <debug.h>

const char QMakeConfig::CONFIG_GROUP[] = "QMake_Builder";

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
    KConfigGroup cg(project->projectConfiguration(), CONFIG_GROUP);
    return cg.exists() && cg.hasKey(QMAKE_EXECUTABLE) && cg.hasKey(BUILD_FOLDER);
}

Path QMakeConfig::buildDirFromSrc(const IProject* project, const Path& srcDir)
{
    QMutexLocker lock(&s_buildDirMutex);
    KConfigGroup cg(project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
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
        KConfigGroup group(cfg.data(), CONFIG_GROUP);
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
        exe = QStandardPaths::findExecutable(QStringLiteral("qmake"));
    }
    if (exe.isEmpty()) {
        exe = QStandardPaths::findExecutable(QStringLiteral("qmake-qt5"));
    }
    if (exe.isEmpty()) {
        exe = QStandardPaths::findExecutable(QStringLiteral("qmake-qt4"));
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
        qCWarning(KDEV_QMAKE) << "failed to execute qmake query " << p.program().join(QStringLiteral(" ")) << "return code was:" << rc;
        return QHash<QString, QString>();
    }

    // TODO: Qt 5.5: Use QTextStream::readLineInto
    QTextStream stream(&p);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const int colon = line.indexOf(':');
        if (colon == -1) {
            continue;
        }

        const auto key = line.left(colon);
        const auto value = line.mid(colon + 1);
        hash.insert(key, value);
    }
    qCDebug(KDEV_QMAKE) << "Ran qmake (" << p.program().join(QStringLiteral(" ")) << "), found:" << hash;
    return hash;
}

QString QMakeConfig::findBasicMkSpec(const QHash<QString, QString>& qmakeVars)
{
    QStringList paths;
    if (qmakeVars.contains(QStringLiteral("QMAKE_MKSPECS"))) {
        // qt4
        foreach (const QString& dir, qmakeVars["QMAKE_MKSPECS"].split(QtCompat::listSeparator())) {
            paths << dir + "/default/qmake.conf";
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
            Q_ASSERT(qmakeVars.contains("QT_INSTALL_PREFIX"));
            path = qmakeVars[QStringLiteral("QT_INSTALL_PREFIX")];
        }
        path += "/mkspecs/" + qmakeVars[QStringLiteral("QMAKE_SPEC")] + "/qmake.conf";
        paths << path;
    }

    foreach (const QString& path, paths) {
        QFileInfo fi(path);
        if (fi.exists())
            return fi.absoluteFilePath();
    }

    return QString();
}
