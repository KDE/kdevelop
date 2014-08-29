/*  This file is part of KDevelop
    Copyright 2010 Aleix Pol <aleixpol@kde.org>

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

#include "vcschangesviewplugin.h"
#include <QDebug>
#include <QPalette>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KAboutData>
#include <KColorScheme>
#include <KDirWatch>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/vcsstatusinfo.h>
#include <vcs/vcsjob.h>
#include <project/projectmodel.h>
#include <QAction>
#include <QTreeView>
#include <interfaces/iplugincontroller.h>
#include "vcschangesview.h"
#include <KActionCollection>
#include <KLocalizedString>
#include <vcs/models/projectchangesmodel.h>

K_PLUGIN_FACTORY_WITH_JSON(VcsProjectIntegrationFactory, "kdevvcschangesview.json", registerPlugin<VcsProjectIntegrationPlugin>();)
// K_EXPORT_PLUGIN(VcsProjectIntegrationFactory(
//     KAboutData("kdevvcsprojectintegration","kdevvcsprojectintegration",
//                ki18n("VCS Project Integration"), "0.1", ki18n("This plugin provides integration between the projects and their VCS infrastructure"), KAboutData::License_GPL)))

using namespace KDevelop;

class VCSProjectToolViewFactory : public KDevelop::IToolViewFactory
{
public:
    VCSProjectToolViewFactory(VcsProjectIntegrationPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        VcsChangesView* modif = new VcsChangesView(m_plugin, parent);
        modif->setModel(m_plugin->model());
        QObject::connect(modif, SIGNAL(reload(QList<KDevelop::IProject*>)), m_plugin->model(), SLOT(reload(QList<KDevelop::IProject*>)));
        QObject::connect(modif, SIGNAL(reload(QList<QUrl>)), m_plugin->model(), SLOT(reload(QList<QUrl>)));
        QObject::connect(modif, SIGNAL(activated(QModelIndex)), m_plugin, SLOT(activated(QModelIndex)));
        return modif;
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QString id() const
    {
        return "org.kdevelop.VCSProject";
    }

private:
    VcsProjectIntegrationPlugin *m_plugin;
};

VcsProjectIntegrationPlugin::VcsProjectIntegrationPlugin(QObject* parent, const QVariantList&)
    : KDevelop::IPlugin("kdevvcsprojectintegration", parent)
    , m_model(0)
{
    ICore::self()->uiController()->addToolView(i18n("VCS Changes"), new VCSProjectToolViewFactory(this));
    
    QAction* synaction = actionCollection()->addAction( "locate_document" );
    synaction->setText(i18n("Locate Current Document"));
    synaction->setIcon(QIcon::fromTheme("dirsync"));
    synaction->setToolTip(i18n("Locates the current document and selects it."));
    
    QAction* reloadaction = actionCollection()->addAction( "reload_view" );
    reloadaction->setText(i18n("Reload View"));
    reloadaction->setIcon(QIcon::fromTheme("view-refresh"));
    reloadaction->setToolTip(i18n("Refreshes the view for all projects, in case anything changed."));
}

void VcsProjectIntegrationPlugin::activated(const QModelIndex& /*idx*/)
{

}

ProjectChangesModel* VcsProjectIntegrationPlugin::model()
{
    if(!m_model) {
        m_model = ICore::self()->projectController()->changesModel();
        connect(actionCollection()->action("reload_view"), SIGNAL(triggered(bool)), m_model, SLOT(reloadAll()));
    }
    
    return m_model;
}

#include "vcschangesviewplugin.moc"
