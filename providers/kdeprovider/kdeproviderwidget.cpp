/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kdeproviderwidget.h"
#include <QVBoxLayout>
#include <QComboBox>
#include <KIcon>
#include <KPushButton>
#include <KConfigDialog>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include "ui_kdeconfig.h"
#include "kdeconfig.h"
#include <QStandardItemModel>
#include "kdeprojectsmodel.h"
#include "kdeprojectsreader.h"

using namespace KDevelop;

KDEProviderWidget::KDEProviderWidget(QWidget* parent)
    : IProjectProviderWidget(parent)
{
    setLayout(new QHBoxLayout(this));
    m_projects = new QComboBox(this);
    KDEProjectsModel* model = new KDEProjectsModel(this);
    KDEProjectsReader* reader = new KDEProjectsReader(model, model);
    connect(reader, SIGNAL(downloadDone()), reader, SLOT(deleteLater()));
    connect(m_projects, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(changed(QString)));
    m_projects->setModel(model);
    
    layout()->addWidget(m_projects);
    
    QPushButton* settings=new QPushButton(KIcon("configure"), i18n("Settings"), this);
    settings->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    connect(settings, SIGNAL(clicked()), SLOT(showSettings()));
    
    layout()->addWidget(settings);
    
    m_dialog = new KConfigDialog(this, "settings", KDEProviderSettings::self());
    m_dialog->setFaceType(KPageDialog::Auto);
    QWidget* page = new QWidget(m_dialog);
    Ui::KDEConfig().setupUi(page);
    
    m_dialog->addPage(page, i18n("General") );
    connect(m_dialog, SIGNAL(settingsChanged(QString)), this, SLOT(loadSettings()));
}

VcsLocation extractLocation(const QVariantMap& urls)
{
    if(urls.contains("svn")) {
        QString svnPrefix=KDEProviderSettings::self()->svnPrefix();
        QString path = QString(urls["ssh"].toString()).replace("%PREFIX", svnPrefix);
        return VcsLocation(path);
    } else {
        QString gitUrl=KDEProviderSettings::self()->gitProtocol();
        return VcsLocation(urls["git"].toUrl());
    }
}

VcsJob* KDEProviderWidget::createWorkingCopy(const KUrl& destinationDirectory)
{
    int pos = m_projects->currentIndex();
    if(pos<0)
        return 0;
    
    QModelIndex idx = m_projects->model()->index(pos, 0);
    Q_ASSERT(idx.isValid());
    
    IPlugin* plugin = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IBasicVersionControl", idx.data(KDEProjectsModel::PluginRole).toString());
    IBasicVersionControl* vcIface = plugin->extension<IBasicVersionControl>();
    VcsJob* ret = vcIface->createWorkingCopy(extractLocation(idx.data(KDEProjectsModel::VcsLocationRole).toMap()), destinationDirectory);
    
    return ret;
}

void KDEProviderWidget::showSettings()
{
    if(KConfigDialog::showDialog("kdesettings"))
        return;
    
    m_dialog->show();
}
