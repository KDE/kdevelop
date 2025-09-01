/*
    SPDX-FileCopyrightText: 2010 Aleix Pol <aleixpol@kde.org>

    Split into separate class
    SPDX-FileCopyrightText: 2011 Andrey Batyiev <batyiev@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSFILECHANGESMODEL_H
#define KDEVPLATFORM_VCSFILECHANGESMODEL_H

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include <vcs/vcsstatusinfo.h>

#include <vcs/vcsexport.h>

class QUrl;

namespace KDevelop
{
class VcsStatusInfo;
class VcsFileChangesModelPrivate;

class KDEVPLATFORMVCS_EXPORT VcsFileChangesSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit VcsFileChangesSortProxyModel(QObject* parent = nullptr);

    bool lessThan(const QModelIndex& rLeft, const QModelIndex& rRight) const override;
};

/**
 * This class holds and represents information about changes in files.
 * Also it is possible to provide tree like models by inheriting this class, see protected members.
 * All stuff should be pulled in by @p updateState.
 */
class KDEVPLATFORMVCS_EXPORT VcsFileChangesModel : public QStandardItemModel
{
    Q_OBJECT

public:
    enum ItemRoles { VcsStatusInfoRole = Qt::UserRole+1, UrlRole, StateRole, LastItemRole };
    enum Column { PathColumn = 0, StatusColumn = 1 };

    /**
     * Search for an index whose data under UrlRole equals a given URL.
     *
     * @return the searched for child of a given parent index in the StatusColumn, or an invalid index if no such child
     *
     * @note When @p model represents a list rather than a tree (in other words, when all
     *       items are children of invisibleRootItem()), @p parent is an invalid index and
     *       @p parent.model() returns @c nullptr. So the @p model parameter is not redundant.
     */
    [[nodiscard]] static QModelIndex statusIndexForUrl(const QAbstractItemModel& model, const QModelIndex& parent,
                                                       const QUrl& url);

    /**
     * Constructor for class.
     * @param isCheckable if true, model will show checkboxes on items.
     */
    explicit VcsFileChangesModel(QObject *parent = nullptr, bool isCheckable = false);
    ~VcsFileChangesModel() override;

    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * Returns list of currently checked urls.
     */
    QList<QUrl> checkedUrls() const {
        return checkedUrls(invisibleRootItem());
    }

    /**
     * Returns urls of all files
     * */
    QList<QUrl> urls() const {
        return urls(invisibleRootItem());
    }
    
    /**
     * Set the checked urls
     * */
    void setCheckedUrls(const QList<QUrl>& urls) const {
        checkUrls(invisibleRootItem(), urls);
    }

    /** 
     * Returns the item for the specified url
     * */
    QStandardItem* itemForUrl(const QUrl &url) const {
        return fileItemForUrl(invisibleRootItem(), url);
    }

    /**
     * Changes the check-state of all files to the given state
     * */
    void setAllChecked(bool checked);

    void setIsCheckbable(bool checkable);
    bool isCheckable() const;

    /**
     * Remove the row that corresponds to a given URL from the child items of a given parent index.
     *
     * @return whether the row was found and removed successfully
     */
    bool removeUrl(const QModelIndex& parent, const QUrl& url);

public Q_SLOTS:
    /**
     * Used to post update of status of some file. Any status except UpToDate
     * and Unknown will update (or add) item representation.
     */
    void updateState(const KDevelop::VcsStatusInfo &status) {
        updateState(invisibleRootItem(), status);
    }

protected:
    /**
     * Post update of status of some file.
     * @return changed row or -1 if row is deleted
     */
    int updateState(QStandardItem *parent, const KDevelop::VcsStatusInfo &status);

    /**
     * Returns list of currently checked urls.
     */
    QList<QUrl> checkedUrls(QStandardItem *parent) const;
    
    /**
     * Checks the given urls, unchecks all others.
     * */
    void checkUrls(QStandardItem *parent, const QList<QUrl>& urls) const;
    
    /**
     * Returns all urls
     * */
    QList<QUrl> urls(QStandardItem *parent) const;

    /**
     * Returns item for particular url.
     */
    QStandardItem* fileItemForUrl(QStandardItem *parent, const QUrl &url) const;

private:
    const QScopedPointer<class VcsFileChangesModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsFileChangesModel)
};
}

Q_DECLARE_METATYPE(KDevelop::VcsStatusInfo::State)

#endif // KDEVPLATFORM_VCSFILECHANGESMODEL_H
