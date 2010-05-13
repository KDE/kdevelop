/***************************************************************************
 *   Copyright (C) 2010 by Aleix Pol Gonzalez <aleixpol@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "projectsourcepage.h"
#include "ui_projectsourcepage.h"
#include <interfaces/iplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/widgets/vcslocationwidget.h>
#include <QVBoxLayout>

using namespace KDevelop;

ProjectSourcePage::ProjectSourcePage(const KUrl& initial, QWidget* parent)
    : QWidget(parent)
{
    m_ui = new Ui::ProjectSourcePage;
    m_ui->setupUi(this);
    
    m_ui->workingDir->setUrl(initial);
    
    m_ui->sources->addItem(KIcon("folder"), i18n("Local"));
    m_plugins.append(0);
    
    IPluginController* pluginManager = ICore::self()->pluginController();
    foreach( IPlugin* p, pluginManager->allPluginsForExtension( "org.kdevelop.IBasicVersionControl" ) )
    {
        m_plugins.append(p);
        m_ui->sources->addItem(KIcon(pluginManager->pluginInfo(p).icon()), pluginManager->pluginInfo(p).name());
    }
    connect(m_ui->sources, SIGNAL(currentIndexChanged(int)), SLOT(sourceChanged(int)));

    sourceChanged(0);
}

void ProjectSourcePage::sourceChanged(int index)
{
    IPlugin* p=m_plugins[index];
    if(p) {
        QLayout* remoteWidgetLayout = m_ui->remoteWidget->layout();
        if(remoteWidgetLayout) {
            QLayoutItem *child;
            while ((child = remoteWidgetLayout->takeAt(0)) != 0) {
                delete child->widget();
                delete child;
            }
        } else
            remoteWidgetLayout = new QVBoxLayout(m_ui->remoteWidget);
            
        IBasicVersionControl* iface = p->extension<KDevelop::IBasicVersionControl>();
        if(iface) {
            VcsLocationWidget* w=iface->vcsLocation(m_ui->sourceBox);
            remoteWidgetLayout->addWidget(w);
        }
    }
    m_ui->sourceBox->setVisible(p!=0);
    
}
