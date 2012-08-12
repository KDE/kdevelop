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

#ifndef KCM_CUSTOMBUILDSYSTEM_H
#define KCM_CUSTOMBUILDSYSTEM_H

#include <project/projectkcmodule.h>
#include "custombuildsystemplugin.h"

class CustomBuildSystemSettings;
namespace Ui
{
class CustomBuildSystemConfig;
}

class CustomBuildSystemKCModule : public ProjectKCModule<CustomBuildSystemSettings>
{
Q_OBJECT
public:
    CustomBuildSystemKCModule( QWidget* parent, const QVariantList& args = QVariantList() );
    virtual ~CustomBuildSystemKCModule();

    virtual void defaults();
    virtual void load();
    virtual void save();
private:
    class CustomBuildSystemConfigWidget* configWidget;
public slots:
    void dataChanged();
};

#endif
