/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#ifndef CMAKESETTINGS_H
#define CMAKESETTINGS_H

class QString;
class KUrl;

/**
 * @author Matt Rogers <mattr@kde.org>
 */
class CMakeSettings
{
public:
    ~CMakeSettings();

    static CMakeSettings* self();

    KUrl installationPrefix() const;
    KUrl buildDirectory() const;
    QString buildType() const;

    void setInstallationPrefix( const KUrl& );
    void setBuildDirectory( const KUrl& );
    void setBuildType( const QString& );

private:
    CMakeSettings();

    static CMakeSettings* s_instance;

};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on;
