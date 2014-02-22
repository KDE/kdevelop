/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#ifndef CUSTOMBUILDSYSTEMCONFIGWIDGET_H
#define CUSTOMBUILDSYSTEMCONFIGWIDGET_H

#include <QWidget>
#include "custombuildsystemconfig.h"
#include "custombuildjob.h"

class KConfig;
class KConfigGroup;

namespace KDevelop
{
class IProject;
}

namespace Ui
{
class CustomBuildSystemConfigWidget;
}

class CustomBuildSystemConfigWidget : public QWidget
{
Q_OBJECT
public:
    CustomBuildSystemConfigWidget( QWidget* parent, KDevelop::IProject* w_project = 0 );
    void loadFrom( KConfig* );
    void saveTo( KConfig*, KDevelop::IProject* project );
    void loadDefaults();
signals:
    void changed();
private slots:
    void configChanged();
    void changeCurrentConfig(int);
    void renameCurrentConfig(const QString& name);
    void addConfig();
    void removeConfig();
    void verify();
private:
    void saveConfig( KConfigGroup& grp, CustomBuildSystemConfig& c, int index );
    Ui::CustomBuildSystemConfigWidget* ui;
    QList<CustomBuildSystemConfig> configs;
};

#endif
