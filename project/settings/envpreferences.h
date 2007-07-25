/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>

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

#ifndef ENVPREFERENCES_H
#define ENVPREFERENCES_H

// #include "projectkcmodule.h"

#include <kurl.h>
#include <kstandarddirs.h>
#include <kcmodule.h>

class EnvSettings;
class EnvWidget;

namespace KDevelop
{

// class EnvPreferences : public ProjectKCModule<EnvSettings>
class EnvPreferences : public KCModule
{
    Q_OBJECT
public:
    explicit EnvPreferences( QWidget *parent, const QStringList &args );
    virtual ~EnvPreferences();

    virtual void save();
    virtual void load();
    virtual void defaults();

    virtual KUrl localNonShareableFile() const
    {
        return KUrl::fromPath(
                   KStandardDirs::locate( "data", "kdevelop/data.kdev4" ) );
    }

private slots:
    void settingsChanged(/* bool changed */);

private:
    class Private;
    Private *const d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
