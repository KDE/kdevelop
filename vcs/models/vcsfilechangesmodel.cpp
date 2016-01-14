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
#include <QMimeDatabase>

#include <KLocalizedString>

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

class VcsStatusInfoItem : public QStandardItem
{
public:
    VcsStatusInfoItem(const VcsStatusInfo& info)
        : QStandardItem()
        , m_info(info) {}

    void setStatus(const VcsStatusInfo& info) {
        m_info = info;
        emitDataChanged();
    }

    QVariant data(int role) const override
    {
        switch(role) {
            case Qt::DisplayRole:
                return stateToString(m_info.state());
            case Qt::DecorationRole:
                return stateToIcon(m_info.state());
            case VcsFileChangesModel::VcsStatusInfoRole:
                return QVariant::fromValue(m_info);
            case VcsFileChangesModel::UrlRole:
                return m_info.url();
        }
        return {};
    }

private:
    VcsStatusInfo m_info;
};

class VcsFileChangesModelPrivate
{
public:
    bool allowSelection;
};

VcsFileChangesModel::VcsFileChangesModel(QObject *parent, bool allowSelection)
    : QStandardItemModel(parent), d(new VcsFileChangesModelPrivate {allowSelection} )
{
    setColumnCount(2);
    setHeaderData(0, Qt::Horizontal, i18n("Filename"));
    setHeaderData(1, Qt::Horizontal, i18n("Status"));
}

int VcsFileChangesModel::updateState(QStandardItem *parent, const KDevelop::VcsStatusInfo &status)
{
    if(status.state()==VcsStatusInfo::ItemUnknown || status.state()==VcsStatusInfo::ItemUpToDate) {
        removeUrl(status.url());
        return -1;
    } else {
        QStandardItem* it1 = fileItemForUrl(parent, status.url());
        if(!it1) {
            QString path = ICore::self()->projectController()->prettyFileName(status.url(), KDevelop::IProjectController::FormatPlain);
            QMimeType mime = status.url().isLocalFile()
                ? QMimeDatabase().mimeTypeForFile(status.url().toLocalFile(), QMimeDatabase::MatchExtension)
                : QMimeDatabase().mimeTypeForUrl(status.url());
            QIcon icon = QIcon::fromTheme(mime.iconName());
            it1 = new QStandardItem(icon, path);
            auto itStatus = new VcsStatusInfoItem(status);

            if(d->allowSelection) {
                it1->setCheckable(true);
                it1->setCheckState(status.state() == VcsStatusInfo::ItemUnknown ? Qt::Unchecked : Qt::Checked);
            }

            parent->appendRow({ it1, itStatus });
        } else {
            QStandardItem *parent = it1->parent();
            if(parent == 0)
                parent = invisibleRootItem();
            auto itStatus = static_cast<VcsStatusInfoItem*>(parent->child(it1->row(), 1));
            itStatus->setStatus(status);
        }

        return it1->row();
    }
}

QVariant VcsFileChangesModel::data(const QModelIndex &index, int role) const
{
    if (role == UrlRole && index.column()==0) {
        return QStandardItemModel::data(index.sibling(index.row(), 1), role);
    }
    return QStandardItemModel::data(index, role);
}

QStandardItem* VcsFileChangesModel::fileItemForUrl(QStandardItem* parent, const QUrl& url) const
{
    for(int i=0, c=parent->rowCount(); i<c; i++) {
        if(statusInfo(i, parent->index()).url()==url) {
            return parent->child(i);
        }
    }
    return 0;
}

QList<VcsStatusInfo> VcsFileChangesModel::checkedStatuses(QStandardItem *parent) const
{
    QList<VcsStatusInfo> ret;

    if(!d->allowSelection)
        return ret;

    for(int i = 0, c = parent->rowCount(); i < c; i++) {
        QStandardItem* item = parent->child(i);
        if(item->checkState() == Qt::Checked) {
            ret << statusInfo(i, parent->index());
        }
    }

    return ret;
}

void VcsFileChangesModel::setAllChecked(bool checked)
{
    if(!d->allowSelection)
        return;
    QStandardItem* parent = invisibleRootItem();
    for(int i = 0, c = parent->rowCount(); i < c; i++) {
        QStandardItem* item = parent->child(i);
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}

QList<QUrl> VcsFileChangesModel::checkedUrls(QStandardItem *parent) const
{
    QList<QUrl> ret;

    for(int i = 0, c = parent->rowCount(); i < c; i++) {
        QStandardItem* item = parent->child(i);
        if(!d->allowSelection || item->checkState() == Qt::Checked) {
            ret << statusInfo(i, parent->index()).url();
        }
    }

    return ret;
}

VcsStatusInfo VcsFileChangesModel::statusInfo(int row, const QModelIndex &parent) const
{
    return index(row, 1, parent).data(VcsStatusInfoRole).value<VcsStatusInfo>();
}

QList<QUrl> VcsFileChangesModel::urls(QStandardItem *parent) const
{
    QList<QUrl> ret;

    for(int i = 0, c = parent->rowCount(); i < c; i++) {
        ret << statusInfo(i, parent->index()).url();
    }

    return ret;
}

void VcsFileChangesModel::checkUrls(QStandardItem *parent, const QList<QUrl>& urls) const
{
    QSet<QUrl> urlSet(urls.toSet());

    if(!d->allowSelection)
        return;

    for(int i = 0, c = parent->rowCount(); i < c; i++) {
        QStandardItem* item = parent->child(i);
        item->setCheckState(urlSet.contains(statusInfo(i, parent->index()).url()) ? Qt::Checked : Qt::Unchecked);
    }
}

void VcsFileChangesModel::setIsCheckbable(bool checkable)
{
    d->allowSelection = checkable;
}

bool VcsFileChangesModel::isCheckable() const
{
    return d->allowSelection;
}

QModelIndex VcsFileChangesModel::indexForUrl(const QUrl& url) const
{
    return indexForUrl(QModelIndex(), url);
}

QModelIndex VcsFileChangesModel::indexForUrl(const QModelIndex& parent, const QUrl& url) const
{
    for (int i=0, c=rowCount(parent); i<c; ++i) {
        QModelIndex idx = index(i, 0, parent);
        if (hasChildren(idx)) {
            QModelIndex cidx = indexForUrl(idx, url);
            if (cidx.isValid())
                return cidx;
        } else {
            if (statusInfo(idx.row(), idx.parent()).url() == url) {
                return idx;
            }
        }
    }
    return {};
}

bool VcsFileChangesModel::removeUrl(const QUrl& url)
{
    const QModelIndex idx = indexForUrl(url);
    if(idx.isValid()) {
        removeRow(idx.row(), idx.parent());
    }
    return idx.isValid();
}

}
