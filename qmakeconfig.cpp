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

#include <KConfigGroup>

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
