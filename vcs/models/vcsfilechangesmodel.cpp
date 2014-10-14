/*  This file is part of KDevelop
    Copyright 2010 Aleix Pol <aleixpol@kde.org>

    Split into separate class
    Copyright 2011 Andrey Batyiev <batyiev@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "vcsfilechangesmodel.h"

#include <QIcon>

#include <KLocalizedString>
#include <QMimeDatabase>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include <vcs/vcsstatusinfo.h>


namespace KDevelop
{

static QString stateToString(KDevelop::VcsStatusInfo::State state)
{
    switch(state)
    {
        case KDevelop::VcsStatusInfo::ItemAdded:
            return i18nc("file was added to versioncontrolsystem", "Added");
        case KDevelop::VcsStatusInfo::ItemDeleted:
            return i18nc("file was deleted from versioncontrolsystem", "Deleted");
        case KDevelop::VcsStatusInfo::ItemHasConflicts:
            return i18nc("file is confilicting (versioncontrolsystem)", "Has Conflicts");
        case KDevelop::VcsStatusInfo::ItemModified:
            return i18nc("version controlled file was modified", "Modified");
        case KDevelop::VcsStatusInfo::ItemUpToDate:
            return i18nc("file is up to date in versioncontrolsystem", "Up To Date");
        case KDevelop::VcsStatusInfo::ItemUnknown:
        case KDevelop::VcsStatusInfo::ItemUserState:
            return i18nc("file is not known to versioncontrolsystem", "Unknown");
    }
    return i18nc("Unknown VCS file status, probably a backend error", "?");
}

static QIcon stateToIcon(KDevelop::VcsStatusInfo::State state)
{
    switch(state)
    {
        case KDevelop::VcsStatusInfo::ItemAdded:
            return QIcon::fromTheme("vcs-added");
        case KDevelop::VcsStatusInfo::ItemDeleted:
            return QIcon::fromTheme("vcs-removed");
        case KDevelop::VcsStatusInfo::ItemHasConflicts:
            return QIcon::fromTheme("vcs-conflicting");
        case KDevelop::VcsStatusInfo::ItemModified:
            return QIcon::fromTheme("vcs-locally-modified");
        case KDevelop::VcsStatusInfo::ItemUpToDate:
            return QIcon::fromTheme("vcs-normal");
        case KDevelop::VcsStatusInfo::ItemUnknown:
        case KDevelop::VcsStatusInfo::ItemUserState:
            return QIcon::fromTheme("unknown");
    }
    return QIcon::fromTheme("dialog-error");
}

class VcsFileChangesModelPrivate
{
public:
    bool allowSelection;
};

VcsFileChangesModel::VcsFileChangesModel(QObject *parent, bool allowSelection)
    : QStandardItemModel(parent), d(new VcsFileChangesModelPrivate)
{
    setColumnCount(2);
    setHeaderData(0, Qt::Horizontal, i18n("Filename"));
    setHeaderData(1, Qt::Horizontal, i18n("Status"));
    d->allowSelection = allowSelection;
}

int VcsFileChangesModel::updateState(QStandardItem *parent, const KDevelop::VcsStatusInfo &status)
{
    QStandardItem* it1=fileItemForUrl(parent, status.url());
    QStandardItem* itStatus;

    if(status.state()==VcsStatusInfo::ItemUnknown || status.state()==VcsStatusInfo::ItemUpToDate) {
        if(it1)
            parent->removeRow(it1->row());
        return -1;
    } else {
        if(!it1) {
            QString path = ICore::self()->projectController()->prettyFileName(status.url(), KDevelop::IProjectController::FormatPlain);
            QMimeType mime = status.url().isLocalFile()
                ? QMimeDatabase().mimeTypeForFile(status.url().toLocalFile(), QMimeDatabase::MatchExtension)
                : QMimeDatabase().mimeTypeForUrl(status.url());
            QIcon icon = QIcon::fromTheme(mime.iconName());
            it1 = new QStandardItem(icon, path);
            itStatus = new QStandardItem;

            if(d->allowSelection) {
                it1->setCheckable(true);
                it1->setCheckState(status.state() == VcsStatusInfo::ItemUnknown ? Qt::Unchecked : Qt::Checked);
            }

            parent->appendRow(QList<QStandardItem*>() << it1 << itStatus);
        } else {
            QStandardItem *parent = it1->parent();
            if(parent == 0)
                parent = invisibleRootItem();
            itStatus = parent->child(it1->row(), 1);
        }

        QString text = stateToString(status.state());
        if(itStatus->text()!=text) {
            itStatus->setText(text);
            itStatus->setIcon(stateToIcon(status.state()));
        }
        it1->setData(qVariantFromValue<VcsStatusInfo>(status), VcsStatusInfoRole);
        return it1->row();
    }
}

QStandardItem* VcsFileChangesModel::fileItemForUrl(QStandardItem* parent, const QUrl& url)
{
    for(int i=0; i<parent->rowCount(); i++) {
        QStandardItem* curr=parent->child(i);

        if(curr->data(VcsStatusInfoRole).value<VcsStatusInfo>().url()==url) {
            return curr;
        }
    }

    return 0;
}

QList<VcsStatusInfo> VcsFileChangesModel::checkedStatuses(QStandardItem *parent) const
{
    QList<VcsStatusInfo> ret;

    if(!d->allowSelection)
        return ret;

    for(int i = 0; i < parent->rowCount(); i++) {
        QStandardItem* item = parent->child(i);
        if(item->checkState() == Qt::Checked) {
            ret << statusInfo(item);
        }
    }

    return ret;
}

void VcsFileChangesModel::setAllChecked(bool checked)
{
    if(!d->allowSelection)
        return;
    QStandardItem* parent = invisibleRootItem();
    for(int i = 0; i < parent->rowCount(); i++) {
        QStandardItem* item = parent->child(i);
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}

QList<QUrl> VcsFileChangesModel::checkedUrls(QStandardItem *parent) const
{
    QList<QUrl> ret;

    for(int i = 0; i < parent->rowCount(); i++) {
        QStandardItem* item = parent->child(i);
        if(!d->allowSelection || item->checkState() == Qt::Checked) {
            ret << statusInfo(item).url();
        }
    }

    return ret;
}

QList<QUrl> VcsFileChangesModel::urls(QStandardItem *parent) const
{
    QList<QUrl> ret;

    for(int i = 0; i < parent->rowCount(); i++) {
        ret << statusInfo(parent->child(i)).url();
    }

    return ret;
}

void VcsFileChangesModel::checkUrls(QStandardItem *parent, const QList<QUrl>& urls) const
{
    QSet<QUrl> urlSet(urls.toSet());

    if(!d->allowSelection)
        return;

    for(int i = 0; i < parent->rowCount(); i++) {
        QStandardItem* item = parent->child(i);
        item->setCheckState(urlSet.contains(statusInfo(item).url()) ? Qt::Checked : Qt::Unchecked);
    }
}

void VcsFileChangesModel::setIsCheckbable(bool checkable)
{
    d->allowSelection = checkable;
}

}
