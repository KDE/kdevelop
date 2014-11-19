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

#include <QMutex>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KDebug>
#include <KProcess>

#include <interfaces/iproject.h>
#include <util/path.h>

const char *QMakeConfig::CONFIG_GROUP = "QMake_Builder";

const char *QMakeConfig::QMAKE_BINARY = "QMake_Binary";
const char *QMakeConfig::BUILD_FOLDER = "Build_Folder";
const char *QMakeConfig::INSTALL_PREFIX = "Install_Prefix";
const char *QMakeConfig::EXTRA_ARGUMENTS = "Extra_Arguments";
const char *QMakeConfig::BUILD_TYPE = "Build_Type";
const char *QMakeConfig::ALL_BUILDS = "All_Builds";

using namespace KDevelop;

///NOTE: KConfig is not thread safe
QMutex s_buildDirMutex;

bool QMakeConfig::isConfigured(const IProject* project)
{
    QMutexLocker lock(&s_buildDirMutex);
    KConfigGroup cg(project->projectConfiguration(), CONFIG_GROUP);
    return cg.exists() && cg.hasKey(QMAKE_BINARY) && cg.hasKey(BUILD_FOLDER);
}

Path QMakeConfig::buildDirFromSrc(const IProject* project, const Path& srcDir)
{
    QMutexLocker lock(&s_buildDirMutex);
    KConfigGroup cg(project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    Path buildDir = Path(cg.readEntry(QMakeConfig::BUILD_FOLDER, project->path().toLocalFile()));
    lock.unlock();

    if(buildDir.isValid()) {
        buildDir.addPath(project->path().relativePath(srcDir));
    }
    return buildDir;
}

QString QMakeConfig::qmakeBinary(const IProject* project)
{
    QMutexLocker lock(&s_buildDirMutex);
    QString exe;
    if (project) {
        KSharedConfig::Ptr cfg = project->projectConfiguration();
        KConfigGroup group(cfg.data(), CONFIG_GROUP);
        if (group.hasKey(QMAKE_BINARY)) {
            exe = group.readEntry(QMAKE_BINARY, QString() );
            QFileInfo info(exe);
            if (!info.exists() || !info.isExecutable()) {
                kWarning() << "bad QMake configured for project " << project->folder() << ":" << exe;
                exe.clear();
            }
        }
    }
    if (exe.isEmpty()) {
        exe = QStandardPaths::findExecutable("qmake-qt4");
    }
    if (exe.isEmpty()) {
        exe = QStandardPaths::findExecutable("qmake-qt5");
    }
    if (exe.isEmpty()) {
        exe = QStandardPaths::findExecutable("qmake");
    }
    Q_ASSERT(!exe.isEmpty());
    return exe;
}

QHash<QString, QString> QMakeConfig::queryQMake(const QString& qmakeBinary)
{
    QHash<QString,QString> hash;
    KProcess p;
    p.setOutputChannelMode( KProcess::OnlyStdoutChannel );
    p << qmakeBinary << "-query";
    int execed = p.execute();
    if (execed != 0) {
        kWarning() << "failed to execute qmake query " << p.program().join(" ") << "return code was:" << execed;
        return QHash<QString,QString>();
    }

    foreach( const QByteArray& line, p.readAllStandardOutput().split('\n')) {
        const int colon = line.indexOf(':');
        if (colon == -1) {
            continue;
        }
        const QByteArray key = line.left(colon);
        const QByteArray value = line.mid(colon + 1);
        hash.insert(key, value);
    }
    kDebug(9024) << "Ran qmake (" << p.program().join(" ") << "), found:" << hash;
    return hash;
}

QString QMakeConfig::findBasicMkSpec( const QHash<QString,QString>& qmakeVars )
{
    QString path;
    if (qmakeVars.contains("QMAKE_MKSPECS")) {
        // qt4
        path = qmakeVars["QMAKE_MKSPECS"] + "/default";
    } else if (!qmakeVars.contains("QMAKE_MKSPECS") && qmakeVars.contains("QMAKE_SPEC")) {
        // qt5 doesn't have the MKSPECS nor default anymore
        // let's try to look up the mkspec path ourselves,
        // see QMakeEvaluator::updateMkspecPaths() in QMake source code as reference
        if (qmakeVars.contains("QT_HOST_DATA/src")) {
            // >=qt5.2: since 0d463c05fc4f2e79e5a4e5a5382a1e6ed5d2615e (in Qt5 qtbase repository)
            // mkspecs are no longer copied to the build directory.
            // instead, we have to look them up in the source directory.
            // this commit also introduced the key 'QT_HOST_DATA/src' which we use here
            path = qmakeVars["QT_HOST_DATA/src"];
        } else if (qmakeVars.contains("QT_HOST_DATA")) {
            // cross compilation
            path = qmakeVars["QT_HOST_DATA"];
        } else {
            Q_ASSERT(qmakeVars.contains("QT_INSTALL_PREFIX"));
            path = qmakeVars["QT_INSTALL_PREFIX"];
        }
        path += "/mkspecs/" + qmakeVars["QMAKE_SPEC"];
    }
    path += "/qmake.conf";

    QFileInfo fi( path );
    if( !fi.exists() )
        return QString();

    return fi.absoluteFilePath();
}
