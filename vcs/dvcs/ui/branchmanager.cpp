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
#include "ui_branchmanager.h"
#include <QStandardItemModel>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

using namespace KDevelop;

BranchManager::BranchManager(const QString &repo, KDevelop::DistributedVersionControlPlugin* executor, QWidget *parent)
    : KDialog(parent)
    , repo(repo), d(executor)
{
    setButtons(KDialog::Close);
    
    m_ui = new Ui::BranchDialogBase;
    QWidget* w = new QWidget(this);
    m_ui->setupUi(w);
    setMainWidget(w);

    m_model = new BranchesListModel(d, repo, this);
    m_ui->branchView->setModel(m_model);
    
    QString branchname = d->curBranch(repo);
    m_valid = !branchname.isEmpty();
    if(m_valid) {
        QList< QStandardItem* > items = m_model->findItems(branchname);
        m_ui->branchView->setCurrentIndex(items.first()->index());
    }
    
    connect(m_ui->newButton, SIGNAL(clicked()), this, SLOT(createBranch()));
    connect(m_ui->deleteButton, SIGNAL(clicked()), this, SLOT(delBranch()));
    connect(m_ui->checkoutButton, SIGNAL(clicked()), this, SLOT(checkoutBranch()));
}

BranchManager::~BranchManager()
{
    delete m_ui;
}

void BranchManager::createBranch()
{
    const QModelIndex currentBranchIdx = m_ui->branchView->currentIndex();
    if (!currentBranchIdx.isValid()) {
        KMessageBox::messageBox(this, KMessageBox::Error,
                                i18n("You must select a base branch from the list before creating a new branch."));
        return;
    }
    QString baseBranch = currentBranchIdx.data().toString();
    bool branchNameEntered = false;
    QString newBranch = QInputDialog::getText(this, i18n("New branch"), i18n("Name of the new branch:"),
            QLineEdit::Normal, QString(), &branchNameEntered);
    if (!branchNameEntered)
        return;

    if (!m_model->findItems(newBranch).isEmpty())
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                i18n("Branch \"%1\" already exists.\n"
                                        "Please, choose another name.",
                                        newBranch));
    }
    else
        m_model->createBranch(baseBranch, newBranch);
}

void BranchManager::delBranch()
{
    QString baseBranch = m_ui->branchView->selectionModel()->selection().indexes().first().data().toString();

    if (baseBranch == d->curBranch(repo))
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                    i18n("Currently at the branch \"%1\".\n"
                                            "To remove it, please change to another branch.",
                                            baseBranch));
        return;
    }

    int ret = KMessageBox::messageBox(this, KMessageBox::WarningYesNo, 
                                      i18n("Are you sure you want to irreversibly remove the branch '%1'?", baseBranch));
    if (ret == KMessageBox::Yes)
        m_model->removeBranch(baseBranch);
}

void BranchManager::checkoutBranch()
{
    QString branch = m_ui->branchView->currentIndex().data().toString();
    if (branch == d->curBranch(repo))
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                i18n("Already on branch \"%1\"\n",
                                        branch));
        return;
    }

    kDebug() << "Switching to" << branch << "in" << repo;
    KDevelop::VcsJob *branchJob = d->switchBranch(repo, branch);
//     connect(branchJob, SIGNAL(finished(KJob*)), m_model, SIGNAL(resetCurrent()));
    
    ICore::self()->runController()->registerJob(branchJob);
    close();
}

///////// BranchesListModel ////////

class BranchItem : public QStandardItem
{
    public:
        BranchItem(const QString& name, bool current=false) : 
            QStandardItem(KIcon( current ? "arrow-right" : ""), name)
        {
            setEditable(true);
        }
        
        void setCurrent(bool current)
        {
            setIcon(KIcon( current ? "arrow-right" : ""));
        }
        
        void setData(const QVariant& value, int role = Qt::UserRole + 1)
        {
            if(role==Qt::EditRole && value.toString()!=text()) {
                QString newBranch = value.toString();
                
                BranchesListModel* bmodel = qobject_cast<BranchesListModel*>(model());
                if(!bmodel->findItems(newBranch).isEmpty())
                {
                    KMessageBox::messageBox(0, KMessageBox::Sorry, i18n("Branch \"%1\" already exists.", newBranch));
                    return;
                }

                int ret = KMessageBox::messageBox(0, KMessageBox::WarningYesNo, 
                                                i18n("Are you sure you want to rename \"%1\" to \"%2\"?", text(), newBranch));

                if (ret == KMessageBox::Yes ) {
                    KDevelop::VcsJob *branchJob = bmodel->dvcsPlugin()->renameBranch(bmodel->repository(), newBranch, text());

                    bool ret = branchJob->exec();
                    kDebug() << "Renaming " << text() << " to " << newBranch << ':' << ret;
                    if(ret) {
                        setText(newBranch);
                        QStandardItem::setData(value, Qt::DisplayRole);
                    }
                }
            }
        }
};

BranchesListModel::BranchesListModel(DistributedVersionControlPlugin* dvcsplugin, const KUrl& repo, QObject* parent)
    : QStandardItemModel(parent), dvcsplugin(dvcsplugin), repo(repo)
{
    QStringList branches = dvcsplugin->listBranches(repo);
    QString curBranch = dvcsplugin->curBranch(repo);

    foreach(const QString& branch, branches)
        appendRow(new BranchItem(branch, branch == curBranch));
}

void BranchesListModel::createBranch(const QString& baseBranch, const QString& newBranch)
{
    kDebug() << "Creating " << baseBranch << " based on " << newBranch;
    VcsRevision rev;
    rev.setRevisionValue(baseBranch, KDevelop::VcsRevision::GlobalNumber);
    KDevelop::VcsJob* branchJob = dvcsplugin->branch(repo, rev, newBranch);

    kDebug() << "Adding new branch";
    if (branchJob->exec())
        appendRow(new BranchItem(newBranch));
}

void BranchesListModel::removeBranch(const QString& branch)
{
    KDevelop::VcsJob *branchJob = dvcsplugin->deleteBranch(KUrl(repo), branch);
    
    kDebug() << "Removing branch:" << branch;
    if (branchJob->exec()) {
        QList< QStandardItem* > items = findItems(branch);
        foreach(QStandardItem* item, items)
            removeRow(item->row());
    }
}

void BranchesListModel::resetCurrent()
{
    QString cur = dvcsplugin->curBranch(repo);
    
    for(int i=0; i<rowCount(); ++i) {
        BranchItem* it=static_cast<BranchItem*>(item(i, 0));
        it->setCurrent(it->text()==cur);
    }
}
