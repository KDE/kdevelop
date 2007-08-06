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

#ifndef ENVIRONMENTPREFERENCES_H
#define ENVIRONMENTPREFERENCES_H

#include <kurl.h>
#include <kstandarddirs.h>
#include <kcmodule.h>

namespace KDevelop
{

class EnvironmentPreferences : public KCModule
{
    Q_OBJECT
public:
    explicit EnvironmentPreferences( QWidget *parent, const QStringList &args );
    virtual ~EnvironmentPreferences();

    virtual void save();
    virtual void load();
    virtual void defaults();

    virtual KUrl localNonShareableFile() const;

private slots:
    void settingsChanged();

private:
    class EnvironmentPreferencesPrivate *const d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
