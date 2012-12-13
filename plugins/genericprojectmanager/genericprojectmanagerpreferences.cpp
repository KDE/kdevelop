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
#include "genericprojectmanagerpreferences.h"

#include <QLayout>

#include <kgenericfactory.h>
#include <KConfigDialogManager>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>

#include "genericprojectmanagersettings.h"
#include "ui_genericprojectmanagersettings.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(GenericProjectManagerPreferencesFactory, registerPlugin<GenericProjectManagerPreferences>();)
K_EXPORT_PLUGIN(GenericProjectManagerPreferencesFactory("kcm_kdev_genericprojectmanagersettings"))

GenericProjectManagerPreferences::GenericProjectManagerPreferences(QWidget *parent, const QVariantList &args)
    :ProjectKCModule<GenericProjectManagerSettings>(GenericProjectManagerPreferencesFactory::componentData(),
        parent, args)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    QWidget *w = new QWidget;
    preferencesDialog = new Ui::GenericProjectManagerSettings;
    preferencesDialog->setupUi(w);
    l->addWidget(w);

    addConfig( GenericProjectManagerSettings::self(), w );
    load();
}

GenericProjectManagerPreferences::~GenericProjectManagerPreferences()
{
    delete preferencesDialog;
}

void GenericProjectManagerPreferences::save()
{
    ProjectKCModule<GenericProjectManagerSettings>::save();
    GenericProjectManagerSettings::self()->writeConfig();

    IProject* proj = project();
    if (proj && proj->projectFileManager()) {
        proj->projectFileManager()->reload(proj->projectItem());
    }
}

void GenericProjectManagerPreferences::load()
{
    ProjectKCModule<GenericProjectManagerSettings>::load();
    GenericProjectManagerSettings::self()->readConfig();
}

#include "genericprojectmanagerpreferences.moc"
