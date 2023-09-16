/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ENVIRONMENTPROFILELIST_H
#define KDEVPLATFORM_ENVIRONMENTPROFILELIST_H

#include "utilexport.h"

#include <KSharedConfig>

class QProcessEnvironment;
class KConfig;
template<typename T1, typename T2> class QMap;
class QString;
#include <QStringList>

namespace KDevelop {
class EnvironmentProfileListPrivate;

/**
 * This class manages a list of environment profiles, each profile containing a number
 * of environment variables and their values.
 *
 * The class is constructed from a KConfig object for easy usage in the plugins.
 *
 * The methods to change the environments is protected to disallow access to those methods
 * from plugins, only the environment widget is allowed to change them.
 *
 * Example Usage
 * \code
 *   KSharedConfigPtr config = KSharedConfig::openConfig();
 *   EnvironmentProfileList env(config);
 *   KConfigGroup cfg(config, "QMake Builder");
 *   QMap<QString,QString> myenvVars = env.variables( cfg.readEntry("QMake Environment") );
 * \endcode
 *
 * Two entries are used by this class:
 * "Default Environment Group" and "Environment Variables".
 *
 * "Default Environment Variables" stores the default profile that should be used if the
 * user didn't select a profile via a plugins configuration dialog.
 *
 * "Environment Variables" entry stores the actual list of
 * <profilename%varname=varvalue>. The profilename can't contain '%' or '='.
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
class KDEVPLATFORMUTIL_EXPORT EnvironmentProfileList
{
public:
    EnvironmentProfileList(const EnvironmentProfileList& rhs);
    EnvironmentProfileList& operator=(const EnvironmentProfileList& rhs);

    /**
     * Creates an a list of environment profiles from a KConfig object
     * @param config the KConfig object to read the environment profiles from
     */
    explicit EnvironmentProfileList(const KSharedConfigPtr& config);
    explicit EnvironmentProfileList(KConfig* config);
    ~EnvironmentProfileList();

    /**
     * Creates a merged environment between the defaults specified by
     * \a defaultEnvironment and those saved in \a profileName
     */
    QStringList createEnvironment(const QString& profileName, const QStringList& defaultEnvironment) const;

    /**
     * returns the variables that are set for a given profile.
     * This function provides read-only access to the environment
     * @param profileName  the name of the profile for which the environment should be returned
     * @return a map containing the environment variables for this profile, or an empty map if the profile doesn't exist in this list
     */
    QMap<QString, QString> variables(const QString& profileName) const;

    /**
     * returns the name of the default profile
     * The default profile should be used by plugins unless the user chooses a different profile
     * @return the name of the default profile, defaults to "default"
     */
    QString defaultProfileName() const;

    /**
     * Fetch the list of names of known profiles from the list
     * @return the list of profile names
     */
    QStringList profileNames() const;

protected:
    EnvironmentProfileList();
    /**
     * returns the variables that are set for a given profile.
     * This function provides write access to the environment, so new variables can be inserted, existing ones changed or deleted
     *
     * If a non-existing profile is specified this returns a new empty map and that way this function can be used to add a new profile
     * to the list of environment profiles
     * @param profileName  the name of the profile for which the environment should be returned
     * @return a map containing the environment variables for this profile, or an empty map if the profile doesn't exist in this list
     */
    QMap<QString, QString>& variables(const QString& profileName);

    /**
     * Changes the default profile.
     * @param profileName the name of the new default profile, if a profile of this name doesn't exist the default profile is not changed
     */
    void setDefaultProfile(const QString& profileName);

    /**
     * Stores the environment profiles in this list to the given KConfig object
     * @param config a KConfig object to which the environment settings should be stored
     */
    void saveSettings(KConfig* config) const;

    void loadSettings(KConfig* config);
    void removeProfile(const QString& profileName);

private:
    const QScopedPointer<class EnvironmentProfileListPrivate> d_ptr;
    Q_DECLARE_PRIVATE(EnvironmentProfileList)
};

KDEVPLATFORMUTIL_EXPORT void expandVariables(QMap<QString, QString>& variables, const QProcessEnvironment& environment);

}

#endif
