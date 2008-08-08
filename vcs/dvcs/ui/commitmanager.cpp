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

#include "commitmanager.h"

#include <QtGui/QKeyEvent>
#include <QtCore/QFileInfo>
#include <QtCore/QList>

#include <ktextedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>

#include "../../vcsjob.h"
#include "../../vcsstatusinfo.h"

#include "../idvcsexecutor.h"
#include "../dvcsjob.h"

using KDevelop::VcsStatusInfo;

CommitManager::CommitManager(const QString &_repo, KDevelop::IDVCSexecutor* executor, QWidget *parent)
    : KDialog(parent)
{
    d = executor;

    //we do the same in prepareJob, so actually it isn't required
    QFileInfo repoInfo = QFileInfo(_repo);
    if (repoInfo.isFile())
        repo = repoInfo.path();
    else
        repo = _repo;
//     setButtons(0);
    setupUi(mainWidget());
    setButtons(KDialog::Ok | KDialog::Cancel);

    setWindowTitle(i18n("Commit Dialog"));

    files->resizeColumnToContents(0);
    files->resizeColumnToContents(1);
    message->selectAll();
    setCommitCandidates();
    connect(this, SIGNAL( okClicked() ), SLOT( commit() ) );
}

CommitManager::~CommitManager()
{
}

QString CommitManager::commitMessage() const
{
    return message->toPlainText();
}

void CommitManager::setCommitCandidates()
{
    kDebug() << "Setting file list for commit dialog:";


    insertRow(d->getCachedFiles(repo), QString("C"), Qt::Checked);
//     insertRow("C", file, Qt::black, Qt::Checked);

    insertRow(d->getModifiedFiles(repo), QString(), Qt::Checked);
//     insertRow("M", file, Qt::black, Qt::Checked);

    //files which are not in the repo
    QStringList otherFiles = d->getOtherFiles(repo);
    foreach(const QString &file, otherFiles)
    {
        insertRow("O", file, Qt::green);
    }
}

void CommitManager::insertRow(const QList<VcsStatusInfo> statuses,
                              const QString &stType /*= QString()*/,
                              const Qt::CheckState ifChecked /*= Qt::Unchecked*/)
{
    if (statuses.isEmpty())
        return;
    foreach(const VcsStatusInfo &statusInfo, statuses)
    {
        switch(statusInfo.state() )
        {
            case KDevelop::VcsStatusInfo::ItemAdded:
                insertRow(stType + "A", statusInfo.url().path(), Qt::green, ifChecked);
                break;
            case KDevelop::VcsStatusInfo::ItemDeleted:
                insertRow(stType + "D", statusInfo.url().path(), Qt::red, ifChecked);
                break;
            case KDevelop::VcsStatusInfo::ItemModified:
                insertRow(stType + "M", statusInfo.url().path(), Qt::black, ifChecked);
                break;
            default:
                insertRow(stType + "?", statusInfo.url().path(), Qt::green, ifChecked);
                break;
        }
    }
}

void CommitManager::insertRow(const QString& state, const QString& file,
                              const QColor &foregroundColor/* = Qt::black*/,
                              const Qt::CheckState ifChecked/* = Qt::Unchecked*/)
{
    if (file.isEmpty())
        return;
    QStringList strings;
    strings << " " << state << file;
    QTreeWidgetItem* item = new QTreeWidgetItem(files, strings);
    item->setForeground(2, foregroundColor);
    item->setCheckState(0, ifChecked);
    files->resizeColumnToContents(0);
}

void CommitManager::commit()
{
    kDebug() << "Committing to the " << repo;

    //if indexed file unchecked then reset
    QStringList resetFiles;
    QStringList addFiles;
    QStringList rmFiles;
    QTreeWidgetItemIterator it(files);
    for( ; *it; ++it )
    {
        bool indexed = (*it)->text(1).contains(QChar('C'));
        bool deleted = (*it)->text(1).contains(QChar('D'));
        bool unchecked = (*it)->checkState(0) == Qt::Unchecked;
        if (indexed && unchecked)
            resetFiles << (*it)->text(2);
        if (!indexed && !unchecked)
        {
            if (deleted)
                rmFiles << (*it)->text(2);
            else
                addFiles << (*it)->text(2);
        }
    }
    kDebug() << "filesToReset" << resetFiles;
    kDebug() << "filesToAdd" << addFiles;
    kDebug() << "filesToRm" << rmFiles;
    d->reset(repo, QStringList(QString("--")), resetFiles)->exec();
    d->add(repo, addFiles)->exec();
    d->remove(repo, rmFiles)->exec();
    d->commit(repo, message->toPlainText())->exec();
}

