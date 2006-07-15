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

#include "cmakesettings.h"

#include <QString>
#include <QDomDocument>

#include <kurl.h>
#include <kdebug.h>

#include "domutil.h"
#include "kdevapi.h"

CMakeSettings* CMakeSettings::s_instance = 0;

CMakeSettings* CMakeSettings::self()
{
    if ( !s_instance )
        s_instance = new CMakeSettings;
    return s_instance;
}

CMakeSettings::CMakeSettings()
{
}


CMakeSettings::~CMakeSettings()
{
}

KUrl CMakeSettings::installationPrefix() const
{
    QDomDocument* dom = KDevApi::self()->projectDom();
    return KUrl(DomUtil::readEntry(*dom, QLatin1String("cmakeproject/prefix")));
}

KUrl CMakeSettings::buildDirectory() const
{
    QDomDocument* dom = KDevApi::self()->projectDom();
    QString value = DomUtil::readEntry(*dom, QLatin1String("cmakeproject/builddir"));
    kDebug( 9020 ) << k_funcinfo << "build dir is " << value << endl;
    return KUrl(value);
}

QString CMakeSettings::buildType() const
{
    QDomDocument* dom = KDevApi::self()->projectDom();
    QString value = DomUtil::readEntry(*dom , QLatin1String("cmakeproject/buildtype"));
    kDebug( 9020 ) << k_funcinfo << "build type is " << value << endl;
    return value;
}

void CMakeSettings::setInstallationPrefix(const KUrl& newPrefix)
{
    QDomDocument* dom = KDevApi::self()->projectDom();
    DomUtil::writeEntry(*dom, QLatin1String("cmakeproject/prefix"), newPrefix.path());
}

void CMakeSettings::setBuildDirectory(const KUrl& newBuildDir)
{
    QDomDocument* dom = KDevApi::self()->projectDom();
    DomUtil::writeEntry(*dom, QLatin1String("cmakeproject/builddir"), newBuildDir.path());
}

void CMakeSettings::setBuildType(const QString& newType)
{
    QDomDocument* dom = KDevApi::self()->projectDom();
    DomUtil::writeEntry(*dom, QLatin1String("cmakeproject/buildtype"), newType);
}


//kate: space-indent on; indent-width 4; replace-tabs on;
