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
#include <QListView>
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
#include <QSortFilterProxyModel>
#include <KFilterProxySearchLine>

using namespace KDevelop;

KDEProviderWidget::KDEProviderWidget(QWidget* parent)
    : IProjectProviderWidget(parent)
{
    setLayout(new QVBoxLayout());
    m_projects = new QListView(this);
    QHBoxLayout* topLayout = new QHBoxLayout(this);
    KFilterProxySearchLine* filterLine = new KFilterProxySearchLine(this);
    KDEProjectsModel* model = new KDEProjectsModel(this);
    KDEProjectsReader* reader = new KDEProjectsReader(model, model);
    connect(reader, SIGNAL(downloadDone()), reader, SLOT(deleteLater()));
    connect(m_projects, SIGNAL(clicked(QModelIndex)), this, SLOT(projectIndexChanged(QModelIndex)));

    topLayout->addWidget(filterLine);

    
    QPushButton* settings=new QPushButton(KIcon("configure"), i18n("Settings"), this);
    settings->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    connect(settings, SIGNAL(clicked()), SLOT(showSettings()));

    topLayout->addWidget(settings);
    
    layout()->addItem(topLayout);
    layout()->addWidget(m_projects);
    
    m_dialog = new KConfigDialog(this, "settings", KDEProviderSettings::self());
    m_dialog->setFaceType(KPageDialog::Auto);
    QWidget* page = new QWidget(m_dialog);

    Ui::KDEConfig configUi;
    configUi.setupUi(page);
    configUi.kcfg_gitProtocol->setProperty("kcfg_property", QByteArray("currentText"));
    
    m_dialog->addPage(page, i18n("General") );
    connect(m_dialog, SIGNAL(settingsChanged(QString)), this, SLOT(loadSettings()));
    
    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->sort(0);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_projects->setModel(proxyModel);
    m_projects->setEditTriggers(QAbstractItemView::NoEditTriggers);
    filterLine->setProxy(proxyModel);
}

VcsLocation extractLocation(const QVariantMap& urls)
{
    QString gitUrl=KDEProviderSettings::self()->gitProtocol();
    return VcsLocation(urls[gitUrl].toUrl());
}

VcsJob* KDEProviderWidget::createWorkingCopy(const KUrl& destinationDirectory)
{
    QModelIndex pos = m_projects->currentIndex();
    if(!pos.isValid())
        return 0;
    
    IPlugin* plugin = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IBasicVersionControl", "kdevgit");
    IBasicVersionControl* vcIface = plugin->extension<IBasicVersionControl>();
    VcsJob* ret = vcIface->createWorkingCopy(extractLocation(pos.data(KDEProjectsModel::VcsLocationRole).toMap()), destinationDirectory);
    
    return ret;
}

bool KDEProviderWidget::isCorrect() const
{
    return m_projects->currentIndex().isValid();
}

void KDEProviderWidget::showSettings()
{
    if(KConfigDialog::showDialog("kdesettings"))
        return;
    
    m_dialog->show();
}

void KDEProviderWidget::projectIndexChanged(const QModelIndex& currentIndex)
{
    if (currentIndex.isValid()) {
        QString currentProjectName = currentIndex.data(Qt::DisplayRole).toString();

        emit changed(currentProjectName);
    }
}
