/*
    SPDX-FileCopyrightText: 2010 Aleix Pol <aleixpol@kde.org>

    Split into separate class
    SPDX-FileCopyrightText: 2011 Andrey Batyiev <batyiev@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsfilechangesmodel.h"

#include "debug.h"

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
            return i18nc("@item file was added to versioncontrolsystem", "Added");
        case KDevelop::VcsStatusInfo::ItemDeleted:
            return i18nc("@item file was deleted from versioncontrolsystem", "Deleted");
        case KDevelop::VcsStatusInfo::ItemHasConflicts:
            return i18nc("@item file is conflicting (versioncontrolsystem)", "Has Conflicts");
        case KDevelop::VcsStatusInfo::ItemModified:
            return i18nc("@item version controlled file was modified", "Modified");
        case KDevelop::VcsStatusInfo::ItemUpToDate:
            return i18nc("@item file is up to date in versioncontrolsystem", "Up To Date");
        case KDevelop::VcsStatusInfo::ItemUnknown:
        case KDevelop::VcsStatusInfo::ItemUserState:
            return i18nc("@item file is not known to versioncontrolsystem", "Unknown");
    }
    return i18nc("@item Unknown VCS file status, probably a backend error", "?");
}

static QIcon stateToIcon(KDevelop::VcsStatusInfo::State state)
{
    switch(state)
    {
        case KDevelop::VcsStatusInfo::ItemAdded:
            return QIcon::fromTheme(QStringLiteral("vcs-added"));
        case KDevelop::VcsStatusInfo::ItemDeleted:
            return QIcon::fromTheme(QStringLiteral("vcs-removed"));
        case KDevelop::VcsStatusInfo::ItemHasConflicts:
            return QIcon::fromTheme(QStringLiteral("vcs-conflicting"));
        case KDevelop::VcsStatusInfo::ItemModified:
            return QIcon::fromTheme(QStringLiteral("vcs-locally-modified"));
        case KDevelop::VcsStatusInfo::ItemUpToDate:
            return QIcon::fromTheme(QStringLiteral("vcs-normal"));
        case KDevelop::VcsStatusInfo::ItemUnknown:
        case KDevelop::VcsStatusInfo::ItemUserState:
            return QIcon::fromTheme(QStringLiteral("unknown"));
    }
    return QIcon::fromTheme(QStringLiteral("dialog-error"));
}

VcsFileChangesSortProxyModel::VcsFileChangesSortProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool VcsFileChangesSortProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    const auto leftStatus = source_left.data(VcsFileChangesModel::StateRole).value<VcsStatusInfo::State>();
    const auto rightStatus = source_right.data(VcsFileChangesModel::StateRole).value<VcsStatusInfo::State>();
    if (leftStatus != rightStatus) {
        return leftStatus < rightStatus;
    }

    const QString leftPath = source_left.data(VcsFileChangesModel::UrlRole).toString();
    const QString rightPath = source_right.data(VcsFileChangesModel::UrlRole).toString();
    return QString::localeAwareCompare(leftPath, rightPath) < 0;
}

class VcsStatusInfoItem : public QStandardItem
{
public:
    explicit VcsStatusInfoItem(const VcsStatusInfo& info)
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
            case VcsFileChangesModel::StateRole:
                return QVariant::fromValue(m_info.state());
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

QModelIndex VcsFileChangesModel::statusIndexForUrl(const QAbstractItemModel& model, const QModelIndex& parent,
                                                   const QUrl& url)
{
    return model.match(model.index(0, StatusColumn, parent), UrlRole, url, 1, Qt::MatchExactly).value(0);
}

VcsFileChangesModel::VcsFileChangesModel(QObject *parent, bool allowSelection)
    : QStandardItemModel(parent)
    , d_ptr(new VcsFileChangesModelPrivate{allowSelection})
{
    setColumnCount(2);
    setHeaderData(0, Qt::Horizontal, i18nc("@title:column", "Filename"));
    setHeaderData(1, Qt::Horizontal, i18nc("@title:column", "Status"));
}

VcsFileChangesModel::~VcsFileChangesModel()
{
}

int VcsFileChangesModel::updateState(QStandardItem *parent, const KDevelop::VcsStatusInfo &status)
{
     Q_D(VcsFileChangesModel);

    if(status.state()==VcsStatusInfo::ItemUnknown || status.state()==VcsStatusInfo::ItemUpToDate) {
        removeUrl(parent->index(), status.url());
        return -1;
    } else {
        QStandardItem* item = fileItemForUrl(parent, status.url());
        if(!item) {
            QString path = ICore::self()->projectController()->prettyFileName(status.url(), KDevelop::IProjectController::FormatPlain);
            QMimeType mime = status.url().isLocalFile()
                ? QMimeDatabase().mimeTypeForFile(status.url().toLocalFile(), QMimeDatabase::MatchExtension)
                : QMimeDatabase().mimeTypeForUrl(status.url());
            QIcon icon = QIcon::fromTheme(mime.iconName());
            item = new QStandardItem(icon, path);
            auto itStatus = new VcsStatusInfoItem(status);

            if(d->allowSelection) {
                item->setCheckable(true);
                item->setCheckState(status.state() == VcsStatusInfo::ItemUnknown ? Qt::Unchecked : Qt::Checked);
            }

            parent->appendRow({ item, itStatus });
        } else {
            QStandardItem *parent = item->parent();
            if(parent == nullptr)
                parent = invisibleRootItem();
            auto statusInfoItem = static_cast<VcsStatusInfoItem*>(parent->child(item->row(), 1));
            statusInfoItem->setStatus(status);
        }

        return item->row();
    }
}

QVariant VcsFileChangesModel::data(const QModelIndex &index, int role) const
{
    if (role >= VcsStatusInfoRole && role < LastItemRole && index.column() == 0) {
        return QStandardItemModel::data(index.sibling(index.row(), 1), role);
    }
    return QStandardItemModel::data(index, role);
}

QStandardItem* VcsFileChangesModel::fileItemForUrl(QStandardItem* parent, const QUrl& url) const
{
    Q_ASSERT(parent);
    if (!parent) {
        qCWarning(VCS) << "null QStandardItem passed to" << Q_FUNC_INFO;
        return nullptr;
    }

    for(int i=0, c=parent->rowCount(); i<c; i++) {
        QStandardItem* item = parent->child(i);
        if(indexFromItem(item).data(UrlRole).toUrl() == url) {
            return parent->child(i);
        }
    }
    return nullptr;
}

void VcsFileChangesModel::setAllChecked(bool checked)
{
    Q_D(VcsFileChangesModel);

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
    Q_D(const VcsFileChangesModel);

    Q_ASSERT(parent);
    if (!parent) {
        qCWarning(VCS) << "null QStandardItem passed to" << Q_FUNC_INFO;
        return {};
    }

    QList<QUrl> ret;
    for(int i = 0, c = parent->rowCount(); i < c; i++) {
        QStandardItem* item = parent->child(i);
        if(!d->allowSelection || item->checkState() == Qt::Checked) {
            ret << indexFromItem(item).data(UrlRole).toUrl();
        }
    }
    return ret;
}

QList<QUrl> VcsFileChangesModel::urls(QStandardItem *parent) const
{
    Q_ASSERT(parent);
    if (!parent) {
        qCWarning(VCS) << "null QStandardItem passed to" << Q_FUNC_INFO;
        return {};
    }

    QList<QUrl> ret;
    const int c = parent->rowCount();
    ret.reserve(c);
    for (int i = 0; i < c; i++) {
        QStandardItem* item = parent->child(i);
        ret << indexFromItem(item).data(UrlRole).toUrl();
    }
    return ret;
}

void VcsFileChangesModel::checkUrls(QStandardItem *parent, const QList<QUrl>& urls) const
{
    Q_D(const VcsFileChangesModel);

    Q_ASSERT(parent);
    if (!parent) {
        qCWarning(VCS) << "null QStandardItem passed to" << Q_FUNC_INFO;
        return;
    }

    if(!d->allowSelection)
        return;

    const QSet<QUrl> urlSet(urls.begin(), urls.end());
    for(int i = 0, c = parent->rowCount(); i < c; i++) {
        QStandardItem* item = parent->child(i);
        item->setCheckState(urlSet.contains(indexFromItem(item).data(UrlRole).toUrl()) ?
            Qt::Checked :
            Qt::Unchecked);
    }
}

void VcsFileChangesModel::setIsCheckbable(bool checkable)
{
    Q_D(VcsFileChangesModel);

    d->allowSelection = checkable;
}

bool VcsFileChangesModel::isCheckable() const
{
    Q_D(const VcsFileChangesModel);

    return d->allowSelection;
}

bool VcsFileChangesModel::removeUrl(const QModelIndex& parent, const QUrl& url)
{
    const auto index = statusIndexForUrl(*this, parent, url);
    return index.isValid() ? removeRow(index.row(), index.parent()) : false;
}

}

#include "moc_vcsfilechangesmodel.cpp"
