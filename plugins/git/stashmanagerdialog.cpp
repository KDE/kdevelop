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
#include <KLocalizedString>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KDevelop;

StashManagerDialog::StashManagerDialog(const QDir& stashed, GitPlugin* plugin, QWidget* parent)
    : QDialog(parent), m_plugin(plugin), m_dir(stashed)
{
    setWindowTitle(i18n("Stash Manager"));

    m_mainWidget = new QWidget(this);
    m_ui = new Ui::StashManager;
    m_ui->setupUi(m_mainWidget);
    
    StashModel* m = new StashModel(stashed, plugin, this);
    m_ui->stashView->setModel(m);
    
    connect(m_ui->show,   &QPushButton::clicked, this, &StashManagerDialog::showStash);
    connect(m_ui->apply,  &QPushButton::clicked, this, &StashManagerDialog::applyClicked);
    connect(m_ui->branch, &QPushButton::clicked, this, &StashManagerDialog::branchClicked);
    connect(m_ui->pop,    &QPushButton::clicked, this, &StashManagerDialog::popClicked);
    connect(m_ui->drop,   &QPushButton::clicked, this, &StashManagerDialog::dropClicked);
    connect(m, &StashModel::rowsInserted, this, &StashManagerDialog::stashesFound);
    
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &StashManagerDialog::reject);

    m_mainWidget->setEnabled(false); //we won't enable it until we have the model with data and selection
}

StashManagerDialog::~StashManagerDialog()
{
    delete m_ui;
}

void StashManagerDialog::stashesFound()
{
    QModelIndex firstIdx=m_ui->stashView->model()->index(0, 0);
    m_ui->stashView->setCurrentIndex(firstIdx);
    m_mainWidget->setEnabled(true);
}

QString StashManagerDialog::selection() const
{
    QModelIndex idx = m_ui->stashView->currentIndex();
    Q_ASSERT(idx.isValid());
    return idx.data().toString();
}

void StashManagerDialog::runStash(const QStringList& arguments)
{
    VcsJob* job = m_plugin->gitStash(m_dir, arguments, OutputJob::Verbose);
    connect(job, &VcsJob::result, this, &StashManagerDialog::accept);
    
    m_mainWidget->setEnabled(false);
    
    ICore::self()->runController()->registerJob(job);
}

void StashManagerDialog::showStash()
{
    IPatchReview * review = ICore::self()->pluginController()->extensionForPlugin<IPatchReview>();
    IPatchSource::Ptr stashPatch(new StashPatchSource(selection(), m_plugin, m_dir));

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
    QString branchName = QInputDialog::getText(this, i18n("KDevelop - Git Stash"), i18n("Select a name for the new branch:"));
    
    if(!branchName.isEmpty())
        runStash(QStringList("branch") << branchName << selection());
}

//////////////////StashModel

StashModel::StashModel(const QDir& dir, GitPlugin* git, QObject* parent)
    : QStandardItemModel(parent)
{
    VcsJob* job=git->gitStash(dir, QStringList("list"), OutputJob::Silent);
    connect(job, &VcsJob::finished, this, &StashModel::stashListReady);
    
    ICore::self()->runController()->registerJob(job);
}

void StashModel::stashListReady(KJob* _job)
{
    DVcsJob* job = qobject_cast<DVcsJob*>(_job);
    QList< QByteArray > output = job->rawOutput().split('\n');
    
    foreach(const QByteArray& line, output) {
        QList< QByteArray > fields = line.split(':');
        
        QList<QStandardItem*> elements;
        foreach(const QByteArray& field, fields)
            elements += new QStandardItem(QString(field.trimmed()));
        
        appendRow(elements);
    }
}
