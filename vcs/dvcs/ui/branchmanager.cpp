/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "branchmanager.h"

#include <QListWidget>
#include <QInputDialog>
#include <QStringList>
#include <QFileInfo>

#include <KJob>
#include <KMessageBox>
#include <KDebug>

#include "../dvcsjob.h"
#include "../dvcsplugin.h"

BranchManager::BranchManager(const QString &_repo, KDevelop::DistributedVersionControlPlugin* executor, QWidget *parent)
    : KDialog(parent)
    , d(executor)
{
    //we do the same in prepareJob, so actually it isn't required
    QFileInfo repoInfo = QFileInfo(_repo);
    if (repoInfo.isFile())
        repo = repoInfo.path();
    else
        repo = _repo;
    setButtons(0);
    setupUi(this);

    QStringList branches = d->branches(repo);
    QString curBranch = d->curBranch(repo);

    QStringList::const_iterator constIterator;
    for (constIterator = branches.constBegin(); constIterator != branches.constEnd(); ++constIterator)
    {
        QListWidgetItem *item = new QListWidgetItem(*constIterator, branchWidget);
        item->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        if (*constIterator == curBranch)
            item->setForeground(Qt::red);
    }

    branchWidget->setEditTriggers(/*QAbstractItemView::AnyKeyPressed
                                    | */QAbstractItemView::DoubleClicked);
    branchWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(branchWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&) ),
            this, SLOT(activateButtons(const QItemSelection&, const QItemSelection&) ) );
    connect(branchWidget, SIGNAL(itemActivated(QListWidgetItem *) ), 
            this, SLOT(currentActivatedData(QListWidgetItem *)));
    connect(branchWidget, SIGNAL(itemChanged ( QListWidgetItem *)),
            this, SLOT(renameBranch(QListWidgetItem *)));

    connect(newButton, SIGNAL(clicked()), this, SLOT(createBranch()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(delBranch()));
    connect(checkoutButton, SIGNAL(clicked()), this, SLOT(checkoutBranch()));
}

BranchManager::~BranchManager()
{
}

void BranchManager::createBranch()
{
    //a hack to prevent renameBranch()
    disconnect(branchWidget, SIGNAL(itemChanged ( QListWidgetItem *)),
               this, SLOT(renameBranch(QListWidgetItem *)));
    QString baseBranch = branchWidget->currentIndex().data().toString();
    QString newBranch = QInputDialog::getText(this, i18n("New branch"),
                                              i18n("Name of the new branch:"));

    if (d->branches(repo).contains(newBranch))
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                i18n("Branch \"%1\" already exists.\n"
                                        "Pick another name.",
                                        newBranch));
        connect(branchWidget, SIGNAL(itemChanged ( QListWidgetItem *)),
                this, SLOT(renameBranch(QListWidgetItem *)));
        return;
    }

    kDebug() << "Creating " << baseBranch << " based on " << newBranch;
    KDevelop::VcsJob* branchJob = d->branch(repo, baseBranch, newBranch);

    if (branchJob)
    {
        kDebug() << "Adding new branch";
        branchJob->exec();
    }
    QListWidgetItem *item = new QListWidgetItem(newBranch, branchWidget);
    item->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    connect(branchWidget, SIGNAL(itemChanged ( QListWidgetItem *)),
            this, SLOT(renameBranch(QListWidgetItem *)));
}

void BranchManager::renameBranch(QListWidgetItem * item)
{
    QString baseBranch = lastActivated;
    QString newBranch = item->data(Qt::DisplayRole).toString();

    if (d->branches(repo).contains(newBranch))
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                i18n("Branch \"%1\" already exists.\n"
                                        "Pick another name.",
                                        newBranch));
        //a hack to prevent another one renameBranch() after resetting item to the old value
        disconnect(branchWidget, SIGNAL(itemChanged ( QListWidgetItem *)),
                   this, SLOT(renameBranch(QListWidgetItem *)));
        item->setText(baseBranch);
        connect(branchWidget, SIGNAL(itemChanged ( QListWidgetItem *)),
                this, SLOT(renameBranch(QListWidgetItem *)));
        return;
    }

    int ret = KMessageBox::messageBox(this, KMessageBox::WarningYesNo, 
                                      i18n("The branch is going to be renamed from\n"
                                            "%1 to %2.\nAre you sure?",baseBranch, newBranch));

    if (ret == QMessageBox::Cancel)
        return;

    KDevelop::VcsJob *branchJob = d->branch(repo, newBranch, baseBranch, QStringList("-m"));
    if (branchJob)
    {
        kDebug() << "Renaming " << baseBranch << " to " << newBranch;
        branchJob->exec();
    }
}

void BranchManager::delBranch()
{
    QString baseBranch = branchWidget->currentIndex().data().toString();

    if (baseBranch == d->curBranch(repo))
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                    i18n("Currently at the branch \"%1\".\n"
                                            "To remove it, another branch has to be checked out.",
                                            baseBranch));
        return;
    }

    int ret = KMessageBox::messageBox(this, KMessageBox::WarningYesNo, 
                                      i18n("About to remove the branch \"%1\".\n" 
                                              "Once removed, it cannot be restored.\nAre you sure?",
                                              baseBranch));
    if (ret == QMessageBox::Cancel)
        return;

    KDevelop::VcsJob *branchJob = d->branch(repo, baseBranch, QString(), QStringList("-D"));
    if (branchJob)
    {
        kDebug() << "Removing " << baseBranch;
        branchJob->exec();
    }
    branchWidget->model()->removeRow(branchWidget->currentIndex().row());
}

void BranchManager::checkoutBranch()
{
    QString baseBranch = branchWidget->currentIndex().data().toString();
    if (baseBranch == d->curBranch(repo))
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                i18n("Already on branch \"%1\"\n",
                                        baseBranch));
        return;
    }

    KDevelop::VcsJob *branchJob = d->switchBranch(repo, baseBranch);
    if (branchJob)
    {
        //we don't run here, because we have to save all unsaved files (reload proj), checkout and then reload again
        emit checkedOut(branchJob);
    }
    close();
}

void BranchManager::currentActivatedData(QListWidgetItem * item)
{
    lastActivated = item->data(Qt::DisplayRole).toString();
//     kDebug() << "Last activated item data: " << lastActivated;
}

void BranchManager::activateButtons(const QItemSelection &selected, const QItemSelection&)
{
    Q_UNUSED(selected)
    bool enableButtons = branchWidget->selectionModel()->hasSelection();
    newButton->setEnabled(enableButtons);
    deleteButton->setEnabled(enableButtons);
    checkoutButton->setEnabled(enableButtons);
}
