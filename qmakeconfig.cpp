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

#include <KConfigGroup>
#include <KStandardDirs>
#include <KDebug>
#include <KProcess>

#include <interfaces/iproject.h>

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

KUrl QMakeConfig::buildDirFromSrc(const IProject* project, const KUrl& srcDir)
{
    QMutexLocker lock(&s_buildDirMutex);
    KConfigGroup cg(project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    KUrl buildDir = cg.readEntry(QMakeConfig::BUILD_FOLDER, project->folder());
    lock.unlock();

    QString relative = KUrl::relativeUrl(project->folder(), srcDir);
    if(buildDir.isValid()) {
        buildDir.addPath(relative);
        buildDir.cleanPath();
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
        exe = group.readEntry(QMAKE_BINARY, KUrl() ).toLocalFile();
        QFileInfo info(exe);
        if (!info.exists() || !info.isExecutable()) {
            kWarning() << "bad QMake configured for project " << project->folder() << ":" << exe;
            exe.clear();
        }
    }
    if (exe.isEmpty()) {
        exe = KStandardDirs::findExe("qmake");
    }
    if (exe.isEmpty()) {
        exe = KStandardDirs::findExe("qmake-qt4");
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
        if (qmakeVars.contains("QT_HOST_PREFIX")) {
            // cross compilation
            path = qmakeVars["QT_HOST_PREFIX"];
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
