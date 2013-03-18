/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
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

#include "brancheslistmodel.h"
#include <vcs/interfaces/ibranchingversioncontrol.h>
#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <interfaces/iruncontroller.h>
#include <KIcon>
#include <KMessageBox>
#include <KLocalizedString>
#include <KDebug>

using namespace std;
using namespace KDevelop;

class BranchItem : public QStandardItem
{
    public:
        BranchItem(const QString& name, bool current=false) : 
            QStandardItem(name)
        {
            setEditable(true);
            setCurrent(current);
        }
        
        void setCurrent(bool current)
        {
            setData(current, BranchesListModel::CurrentRole);
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
                if (ret == KMessageBox::No) {
                    return; // ignore event
                }

                KDevelop::VcsJob *branchJob = bmodel->interface()->renameBranch(bmodel->repository(), newBranch, text());
                ret = branchJob->exec();
                kDebug() << "Renaming " << text() << " to " << newBranch << ':' << ret;
                if (!ret) {
                    return; // ignore event
                }
            }

            QStandardItem::setData(value, role);
        }
};

static QVariant runSynchronously(KDevelop::VcsJob* job)
{
    job->setVerbosity(KDevelop::OutputJob::Silent);
    QVariant ret;
    if(job->exec() && job->status()==KDevelop::VcsJob::JobSucceeded) {
        ret = job->fetchResults();
    }
    delete job;
    return ret;
}

BranchesListModel::BranchesListModel(QObject* parent)
    : QStandardItemModel(parent), dvcsplugin(dvcsplugin)
{
    QHash< int, QByteArray > roles = roleNames();
    roles.insert(CurrentRole, "isCurrent");
    setRoleNames(roles);
}

void BranchesListModel::createBranch(const QString& baseBranch, const QString& newBranch)
{
    kDebug() << "Creating " << baseBranch << " based on " << newBranch;
    KDevelop::VcsRevision rev;
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

KDevelop::IBranchingVersionControl* BranchesListModel::interface()
{
    return dvcsplugin;
}

void BranchesListModel::initialize(KDevelop::IBranchingVersionControl* branching, const KUrl& r)
{
    dvcsplugin = branching;
    repo = r;
    refresh();
}

void BranchesListModel::refresh()
{
    QStringList branches = runSynchronously(dvcsplugin->branches(repo)).toStringList();
    QString curBranch = runSynchronously(dvcsplugin->currentBranch(repo)).toString();
    
    foreach(const QString& branch, branches)
        appendRow(new BranchItem(branch, branch == curBranch));
}

void BranchesListModel::resetCurrent()
{
    refresh();
    emit currentBranchChanged();
}

QString BranchesListModel::currentBranch() const
{
    return runSynchronously(dvcsplugin->currentBranch(repo)).toString();
}

KDevelop::IProject* BranchesListModel::project() const
{
    return KDevelop::ICore::self()->projectController()->findProjectForUrl(repo);
}

void BranchesListModel::setProject(KDevelop::IProject* p)
{
    if(!p || !p->versionControlPlugin()) {
        qDebug() << "null or invalid project" << p;
        return;
    }

    KDevelop::IBranchingVersionControl* branching = p->versionControlPlugin()->extension<KDevelop::IBranchingVersionControl>();
    if(branching) {
        initialize(branching, p->folder());
    } else
        qDebug() << "not a branching vcs project" << p->name();
}

void BranchesListModel::setCurrentBranch(const QString& branch)
{
    KDevelop::VcsJob* job = dvcsplugin->switchBranch(repo, branch);
    connect(job, SIGNAL(finished(KJob*)), SIGNAL(currentBranchChanged()));
    KDevelop::ICore::self()->runController()->registerJob(job);
}
