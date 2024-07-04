/*
    SPDX-FileCopyrightText: 2010 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcschangesviewplugin.h"

#include <QAction>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include <project/projectchangesmodel.h>
#include <project/projectmodel.h>
#include "vcschangesview.h"

K_PLUGIN_FACTORY_WITH_JSON(VcsProjectIntegrationFactory, "kdevvcschangesview.json", registerPlugin<VcsProjectIntegrationPlugin>();)

using namespace KDevelop;

class VCSProjectToolViewFactory : public KDevelop::IToolViewFactory
{
public:
    explicit VCSProjectToolViewFactory(VcsProjectIntegrationPlugin *plugin): m_plugin(plugin) {}

    QWidget* create(QWidget *parent = nullptr) override
    {
        auto* modif = new VcsChangesView(m_plugin, parent);
        modif->setModel(m_plugin->model());
        QObject::connect(modif, QOverload<const QList<KDevelop::IProject*>&>::of(&VcsChangesView::reload),
                         m_plugin->model(), QOverload<const QList<KDevelop::IProject*>&>::of(&ProjectChangesModel::reload));
        QObject::connect(modif, QOverload<const QList<KDevelop::IProject*>&>::of(&VcsChangesView::reload),
                         m_plugin->model(), QOverload<const QList<KDevelop::IProject*>&>::of(&ProjectChangesModel::reload));
        QObject::connect(modif, &VcsChangesView::activated, m_plugin, &VcsProjectIntegrationPlugin::activated);
        return modif;
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::RightDockWidgetArea;
    }

    QString id() const override
    {
        return QStringLiteral("org.kdevelop.VCSProject");
    }

private:
    VcsProjectIntegrationPlugin *m_plugin;
};

VcsProjectIntegrationPlugin::VcsProjectIntegrationPlugin(QObject* parent, const KPluginMetaData& metaData,
                                                         const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevvcsprojectintegration"), parent, metaData)
    , m_model(nullptr)
{
    ICore::self()->uiController()->addToolView(i18nc("@title:window", "Project Changes"), new VCSProjectToolViewFactory(this));

    QAction* synaction = actionCollection()->addAction(QStringLiteral("locate_document"));
    synaction->setText(i18nc("@action", "Locate Current Document"));
    synaction->setIcon(QIcon::fromTheme(QStringLiteral("dirsync")));
    synaction->setToolTip(i18nc("@info:tooltip", "Locate the current document and select it"));

    QAction* reloadaction = actionCollection()->addAction(QStringLiteral("reload_view"));
    reloadaction->setText(i18nc("@action", "Reload View"));
    reloadaction->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    reloadaction->setToolTip(i18nc("@info:tooltip", "Refresh the view for all projects, in case anything changed"));
}

void VcsProjectIntegrationPlugin::activated(const QModelIndex& /*idx*/)
{

}

ProjectChangesModel* VcsProjectIntegrationPlugin::model()
{
    if(!m_model) {
        m_model = ICore::self()->projectController()->changesModel();
        connect(actionCollection()->action(QStringLiteral("reload_view")), &QAction::triggered, m_model, &ProjectChangesModel::reloadAll);
    }

    return m_model;
}

#include "vcschangesviewplugin.moc"
#include "moc_vcschangesviewplugin.cpp"
