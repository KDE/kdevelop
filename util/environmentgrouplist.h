/* This file is part of KDevelop
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_ENVIRONMENTGROUPLIST_H
#define KDEVPLATFORM_ENVIRONMENTGROUPLIST_H

#include "utilexport.h"
#include <ksharedconfig.h>

class KConfig;
template <typename T1, typename T2> class QMap;
class QString;
class QStringList;

namespace KDevelop
{

/**
 * This class manages a list of environment groups, each group containing a number
 * of environment variables and their values.
 *
 * The class is constructed from a KConfig object for easy usage in the plugins.
 *
 * The methods to change the environments is protected to disallow access to those methods
 * from plugins, only the environment widget is allowed to change them.
 *
 * Example Usage
 * \code
 *   KSharedConfigPtr config = KGlobal::config();
 *   EnvironmentGroupList env(config);
 *   KConfigGroup cfg(config, "QMake Builder");
 *   QMap<QString,QString> myenvVars = env.variables( cfg.readEntry("QMake Environment") );
 * \endcode
 *
 * Two entries are used by this class:
 * "Default Environment Group" and "Environment Variables".
 *
 * "Default Environment Variables" stores the default group that should be used if the
 * user didn't select a group via a plugins configuration dialog.
 *
 * "Environment Variables" entry stores the actual list of
 * <groupname%varname=varvalue>. The groupname can't contain '%' or '='.
 * For example, suppose that two configuration, say "release" and "debug" exist.
 * Then the actual contents of .kdev4 project file will be
 *
 * \code
 * [Environment Settings]
 * Default Environment Group=debug
 * Environment Variables=debug_PATH=/home/kde-devel/usr/bin,release_PATH=/usr/bin
 * \endcode
 *
 */
class KDEVPLATFORMUTIL_EXPORT EnvironmentGroupList
{
public:
    /**
     * Creates an a list of EnvironmentGroups from a KConfig object
     * @param config the KConfig object to read the environment groups from
     */
    EnvironmentGroupList( KSharedConfigPtr config );
    EnvironmentGroupList( KConfig* config );
    ~EnvironmentGroupList();

    /**
     * Creates a merged environment between the defaults specified by
     * \a defaultEnvironment and those saved in \a group
     */
    QStringList createEnvironment(const QString& group, const QStringList& defaultEnvironment ) const;

    /**
     * returns the variables that are set for a given group.
     * This function provides read-only access to the environment
     * @param group  the name of the group for which the environment should be returned
     * @return a map containing the environment variables for this group, or an empty map if the group doesn't exist in this list
     */
    const QMap<QString, QString> variables( const QString& group ) const;

    /**
     * returns the default group
     * The default group should be used by plugins unless the user chooses a different group
     * @return the name of the default group, defaults to "default"
     */
    QString defaultGroup() const;

    /**
     * Fetch the list of known groups from the list
     * @return the list of groups
     */
    QStringList groups() const;

protected:
    EnvironmentGroupList();
    /**
     * returns the variables that are set for a given group.
     * This function provides write access to the environment, so new variables can be inserted, existing ones changed or deleted
     *
     * If a non-existing group is specified this returns a new empty map and that way this function can be used to add a new group
     * to the list of environment groups
     * @param group  the name of the group for which the environment should be returned
     * @return a map containing the environment variables for this group, or an empty map if the group doesn't exist in this list
     */
    QMap<QString, QString>& variables( const QString& group );

    /**
     * Changes the default group.
     * @param group a new groupname, if a group of this name doesn't exist the default group is not changed
     */
    void setDefaultGroup( const QString& group );

    /**
     * Stores the environment groups in this list to the given KConfig object
     * @param config a KConfig object to which the environment settings should be stored
     */
    void saveSettings( KConfig* config ) const;

    void loadSettings( KConfig* config );
    void removeGroup( const QString& group );
private:
    class EnvironmentGroupListPrivate* const d;

};

}

#endif
