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
#include <interfaces/iruncontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/widgets/vcslocationwidget.h>
#include <vcs/vcsjob.h>
#include <QVBoxLayout>
#include <QDebug>

using namespace KDevelop;

ProjectSourcePage::ProjectSourcePage(const KUrl& initial, QWidget* parent)
    : QWidget(parent)
{
    m_ui = new Ui::ProjectSourcePage;
    m_ui->setupUi(this);
    
    m_ui->workingDir->setUrl(initial);
    m_ui->workingDir->setMode(KFile::Directory);
    connect(m_ui->workingDir, SIGNAL(textChanged(QString)), SLOT(reevaluateCorrection()));
    m_ui->remoteWidget->setLayout(new QVBoxLayout(m_ui->remoteWidget));
    
    m_ui->sources->addItem(KIcon("folder"), i18n("Local"));
    m_plugins.append(0);
    
    IPluginController* pluginManager = ICore::self()->pluginController();
    foreach( IPlugin* p, pluginManager->allPluginsForExtension( "org.kdevelop.IBasicVersionControl" ) )
    {
        m_plugins.append(p);
        m_ui->sources->addItem(KIcon(pluginManager->pluginInfo(p).icon()), pluginManager->pluginInfo(p).name());
    }
    connect(m_ui->sources, SIGNAL(currentIndexChanged(int)), SLOT(sourceChanged(int)));
    connect(m_ui->get, SIGNAL(clicked()), SLOT(getVcsProject()));
    
    sourceChanged(0);
}

void ProjectSourcePage::sourceChanged(int index)
{
    m_locationWidget=0;
    QLayout* remoteWidgetLayout = m_ui->remoteWidget->layout();
    QLayoutItem *child;
    while ((child = remoteWidgetLayout->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }
    
    IBasicVersionControl* vcIface = vcsPerIndex(index);
    if(vcIface) {
        m_locationWidget=vcIface->vcsLocation(m_ui->sourceBox);
        connect(m_locationWidget, SIGNAL(changed()), SLOT(sourceLocationChanged()));
        
        remoteWidgetLayout->addWidget(m_locationWidget);
    }
    reevaluateCorrection();
    
    m_ui->sourceBox->setVisible(vcIface!=0);
}

IBasicVersionControl* ProjectSourcePage::vcsPerIndex(int index)
{
    IPlugin* p=m_plugins[index];
    if(!p)
        return 0;
    else
        return p->extension<KDevelop::IBasicVersionControl>();
}

void ProjectSourcePage::getVcsProject()
{
    IBasicVersionControl* iface=vcsPerIndex(m_ui->sources->currentIndex());
    Q_ASSERT(iface && m_locationWidget);

    emit isCorrect(false);
    
    VcsJob* job=iface->createWorkingCopy(m_locationWidget->location(), m_ui->workingDir->url());
    m_ui->creationProgress->setValue(m_ui->creationProgress->minimum());
    
    connect(job, SIGNAL(result(KJob*)), SLOT(projectReceived(KJob*)));
    ICore::self()->runController()->registerJob(job);
}

void ProjectSourcePage::projectReceived(KJob* job)
{
    m_ui->creationProgress->setValue(m_ui->creationProgress->maximum());
    if(job->error()==0)
        reevaluateCorrection();
}

void ProjectSourcePage::sourceLocationChanged()
{
    m_ui->creationProgress->setEnabled(m_locationWidget->isCorrect());
    m_ui->get->setEnabled(m_locationWidget->isCorrect());
}

void ProjectSourcePage::reevaluateCorrection()
{
    KUrl cwd=m_ui->workingDir->url();
    bool correct=!cwd.isEmpty() && (!cwd.isLocalFile() || QFile::exists(cwd.toLocalFile()));
    
    if(correct) {
        QDir d(cwd.toLocalFile());
        correct &= d.exists() && !d.entryList().isEmpty();
    }
    
    emit isCorrect(correct);
}

KUrl ProjectSourcePage::workingDir() const
{
    return m_ui->workingDir->url();
}
