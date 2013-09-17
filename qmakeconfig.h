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

#ifndef QMAKECONFIG_H
#define QMAKECONFIG_H

#include <KUrl>

namespace KDevelop {
class IProject;
}

class QMakeConfig
{
public:
    static const char *CONFIG_GROUP;

    static const char 
        *QMAKE_BINARY,
        *BUILD_FOLDER,
        *INSTALL_PREFIX,
        *EXTRA_ARGUMENTS,
        *BUILD_TYPE,
        *ALL_BUILDS;

    /**
     * Returns the directory where srcDir will be built.
     * srcDir must contain a *.pro file !
     */
    static KUrl buildDirFromSrc(const KDevelop::IProject* project, const KUrl& srcDir);

    /**
     * Returns the QMake binary configured for the given @p project.
     */
    static QString qmakeBinary(const KDevelop::IProject* project);

    /**
     * Query QMake and return the thus obtained QMake variables.
     */
    static QHash<QString, QString> queryQMake(const QString& qmakeBinary);

    /**
     * Given the QMake variables, try to find a basic MkSpec.
     */
    static QString findBasicMkSpec( const QHash<QString,QString>& qmakeVars );
};

#endif
