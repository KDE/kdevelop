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
#include "projectfilterkcm.h"

#include <QLayout>

#include <kgenericfactory.h>
#include <KConfigDialogManager>
#include <KSettings/Dispatcher>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>

#include "ui_projectfiltersettings.h"

#include "projectfilterdebug.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(ProjectFilterKCMFactory, registerPlugin<ProjectFilterKCM>();)
K_EXPORT_PLUGIN(ProjectFilterKCMFactory("kcm_kdevprojectfilter"))

ProjectFilterKCM::ProjectFilterKCM(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<ProjectFilterSettings>(ProjectFilterKCMFactory::componentData(), parent, args)
    , m_preferencesDialog(new Ui::ProjectFilterSettings)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    QWidget *w = new QWidget;

    m_preferencesDialog->setupUi(w);
    l->addWidget(w);

    addConfig( ProjectFilterSettings::self(), w );
    load();
}

ProjectFilterKCM::~ProjectFilterKCM()
{
}

void ProjectFilterKCM::save()
{
    ProjectKCModule<ProjectFilterSettings>::save();

    //FIXME

    KSettings::Dispatcher::reparseConfiguration("kdevprojectfilter");
}

void ProjectFilterKCM::load()
{
    ProjectKCModule<ProjectFilterSettings>::load();

    //FIXME
}

#include "projectfilterkcm.moc"
