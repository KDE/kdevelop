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

#include "stashmanagerdialog.h"
#include "ui_stashmanagerdialog.h"
#include "gitplugin.h"
#include "stashpatchsource.h"
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iplugincontroller.h>
#include <KMessageBox>
#include <QInputDialog>
#include <vcs/dvcs/dvcsjob.h>
#include <shell/core.h>
#include <KLocalizedString>

StashManagerDialog::StashManagerDialog(const QDir& stashed, GitPlugin* plugin, QWidget* parent)
    : KDialog(parent), m_plugin(plugin), m_dir(stashed)
{
    setWindowTitle(KDialog::makeStandardCaption(i18n("Stash Manager")));
    setButtons(KDialog::Close);
    
    QWidget* w = new QWidget(this);
    m_ui = new Ui::StashManager;
    m_ui->setupUi(w);
    
    StashModel* m = new StashModel(stashed, plugin, this);
    m_ui->stashView->setModel(m);
    
    connect(m_ui->show,   SIGNAL(clicked(bool)), SLOT(showStash()));
    connect(m_ui->apply,  SIGNAL(clicked(bool)), SLOT(applyClicked()));
    connect(m_ui->branch, SIGNAL(clicked(bool)), SLOT(branchClicked()));
    connect(m_ui->pop,    SIGNAL(clicked(bool)), SLOT(popClicked()));
    connect(m_ui->drop,   SIGNAL(clicked(bool)), SLOT(dropClicked()));
    connect(m, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(stashesFound()));
    
    setMainWidget(w);
    w->setEnabled(false); //we won't enable it until we have the model with data and selection
}

StashManagerDialog::~StashManagerDialog()
{
    delete m_ui;
}

void StashManagerDialog::stashesFound()
{
    QModelIndex firstIdx=m_ui->stashView->model()->index(0, 0);
    m_ui->stashView->setCurrentIndex(firstIdx);
    mainWidget()->setEnabled(true);
}

QString StashManagerDialog::selection() const
{
    QModelIndex idx = m_ui->stashView->currentIndex();
    Q_ASSERT(idx.isValid());
    return idx.data().toString();
}

void StashManagerDialog::runStash(const QStringList& arguments)
{
    KDevelop::VcsJob* job = m_plugin->gitStash(m_dir, arguments, KDevelop::OutputJob::Verbose);
    connect(job, SIGNAL(result(KJob*)), SLOT(accept()));
    
    mainWidget()->setEnabled(false);
    
    KDevelop::ICore::self()->runController()->registerJob(job);
}

void StashManagerDialog::showStash()
{
    KDevelop::IPatchReview * review = KDevelop::ICore::self()->pluginController()->extensionForPlugin<KDevelop::IPatchReview>();
    KDevelop::IPatchSource::Ptr stashPatch(new StashPatchSource(selection(), m_plugin, m_dir));

    review->startReview(stashPatch);
    accept();
}

void StashManagerDialog::applyClicked()
{
    runStash(QStringList("apply") << selection());
}

void StashManagerDialog::popClicked()
{
    runStash(QStringList("pop") << selection());
}

void StashManagerDialog::dropClicked()
{
    QString sel = selection();
    int ret = KMessageBox::questionYesNo(this, i18n("Are you sure you want to drop the stash '%1'?", sel));
    
    if(ret == KMessageBox::Yes)
        runStash(QStringList("drop") << sel);
}

void StashManagerDialog::branchClicked()
{
    QString branchName = QInputDialog::getText(this, i18n("KDevelop - Git Stash"), i18n("Select a name for the new branch"));
    
    if(!branchName.isEmpty())
        runStash(QStringList("branch") << branchName << selection());
}

//////////////////StashModel

StashModel::StashModel(const QDir& dir, GitPlugin* git, QObject* parent)
    : QStandardItemModel(parent)
{
    KDevelop::VcsJob* job=git->gitStash(dir, QStringList("list"), KDevelop::OutputJob::Silent);
    connect(job, SIGNAL(finished(KJob*)), SLOT(stashListReady(KJob*)));
    
    KDevelop::ICore::self()->runController()->registerJob(job);
}

void StashModel::stashListReady(KJob* _job)
{
    KDevelop::DVcsJob* job = qobject_cast<KDevelop::DVcsJob*>(_job);
    QList< QByteArray > output = job->rawOutput().split('\n');
    
    foreach(const QByteArray& line, output) {
        QList< QByteArray > fields = line.split(':');
        
        QList<QStandardItem*> elements;
        foreach(const QByteArray& field, fields)
            elements += new QStandardItem(QString(field.trimmed()));
        
        appendRow(elements);
    }
}
