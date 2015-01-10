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
#include "../debug.h"
#include <vcs/interfaces/ibranchingversioncontrol.h>
#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <interfaces/iruncontroller.h>

#include <QScopedPointer>
#include <QIcon>

#include <KMessageBox>
#include <KLocalizedString>

using namespace std;
using namespace KDevelop;

class KDevelop::BranchesListModelPrivate
{
    public:
        BranchesListModelPrivate()
        {
        }

        QScopedPointer<IBranchingVersionControl> dvcsplugin;
        QUrl repo;
};

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
            setIcon(QIcon::fromTheme( current ? "arrow-right" : ""));
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
                qCDebug(VCS) << "Renaming " << text() << " to " << newBranch << ':' << ret;
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
    : QStandardItemModel(parent), d(new BranchesListModelPrivate())
{
}

BranchesListModel::~BranchesListModel()
{
}

QHash<int, QByteArray> BranchesListModel::roleNames() const
{
    auto roles = QAbstractItemModel::roleNames();
    roles.insert(CurrentRole, "isCurrent");
    return roles;
}

void BranchesListModel::createBranch(const QString& baseBranch, const QString& newBranch)
{
    qCDebug(VCS) << "Creating " << baseBranch << " based on " << newBranch;
    KDevelop::VcsRevision rev;
    rev.setRevisionValue(baseBranch, KDevelop::VcsRevision::GlobalNumber);
    KDevelop::VcsJob* branchJob = d->dvcsplugin->branch(d->repo, rev, newBranch);

    qCDebug(VCS) << "Adding new branch";
    if (branchJob->exec())
        appendRow(new BranchItem(newBranch));
}

void BranchesListModel::removeBranch(const QString& branch)
{
    KDevelop::VcsJob *branchJob = d->dvcsplugin->deleteBranch(d->repo, branch);

    qCDebug(VCS) << "Removing branch:" << branch;
    if (branchJob->exec()) {
        QList< QStandardItem* > items = findItems(branch);
        foreach(QStandardItem* item, items)
            removeRow(item->row());
    }
}

QUrl BranchesListModel::repository() const
{
    return d->repo;
}

KDevelop::IBranchingVersionControl* BranchesListModel::interface()
{
    return d->dvcsplugin.data();
}

void BranchesListModel::initialize(KDevelop::IBranchingVersionControl* branching, const QUrl& r)
{
    d->dvcsplugin.reset(branching);
    d->repo = r;
    refresh();
}

void BranchesListModel::refresh()
{
    QStringList branches = runSynchronously(d->dvcsplugin->branches(d->repo)).toStringList();
    QString curBranch = runSynchronously(d->dvcsplugin->currentBranch(d->repo)).toString();

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
    return runSynchronously(d->dvcsplugin->currentBranch(d->repo)).toString();
}

KDevelop::IProject* BranchesListModel::project() const
{
    return KDevelop::ICore::self()->projectController()->findProjectForUrl(d->repo);
}

void BranchesListModel::setProject(KDevelop::IProject* p)
{
    if(!p || !p->versionControlPlugin()) {
        qCDebug(VCS) << "null or invalid project" << p;
        return;
    }

    KDevelop::IBranchingVersionControl* branching = p->versionControlPlugin()->extension<KDevelop::IBranchingVersionControl>();
    if(branching) {
        initialize(branching, p->folder());
    } else
        qCDebug(VCS) << "not a branching vcs project" << p->name();
}

void BranchesListModel::setCurrentBranch(const QString& branch)
{
    KDevelop::VcsJob* job = d->dvcsplugin->switchBranch(d->repo, branch);
    connect(job, &VcsJob::finished, this, &BranchesListModel::currentBranchChanged);
    KDevelop::ICore::self()->runController()->registerJob(job);
}
