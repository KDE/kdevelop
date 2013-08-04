/* This file is part of KDevelop
    Copyright 2008 Alexander Dymo <adymo@kdevelop.org>

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
#ifndef KDEVPLATFORM_PLUGIN_GENERICPROJECTMANAGERPREFERENCES_H
#define KDEVPLATFORM_PLUGIN_GENERICPROJECTMANAGERPREFERENCES_H

#include <project/projectkcmodule.h>

class KConfigDialogManager;
class GenericProjectManagerSettings;

namespace Ui
{
class GenericProjectManagerSettings;
}

namespace KDevelop
{

class GenericProjectManagerPreferences: public ProjectKCModule<GenericProjectManagerSettings> {
    Q_OBJECT
public:
    GenericProjectManagerPreferences(QWidget *parent, const QVariantList &args);
    virtual ~GenericProjectManagerPreferences();

    virtual void save();
    virtual void load();

private:
    Ui::GenericProjectManagerSettings *preferencesDialog;
};

}
#endif
