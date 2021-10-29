/*
    SPDX-FileCopyrightText: 2013 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLACEHOLDERITEMPROXYMODEL_H
#define KDEVPLATFORM_PLACEHOLDERITEMPROXYMODEL_H

#include "utilexport.h"

#include <QIdentityProxyModel>
#include <QScopedPointer>

namespace KDevelop {
class PlaceholderItemProxyModelPrivate;

/**
 * Proxy model adding a placeholder item for new entries
 *
 * This is mostly a QIdentityProxyModel, with one additional row added at the end
 *
 * Example use:
 *
 * @code
 * PlaceholderItemProxyModel* proxyModel = new PlaceholderItemProxyModel;
 * proxyModel->setSourceModel(new MyItemModel);
 * proxyModel->setColumnHint(0, "(Add new entry)");
 * connect(proxyModel, SIGNAL(dataInserted(...), SLOT(handleDataInserted(...));
 * @endcode
 *
 * In this case MyItemModel has exactly two entries, "Item1" and "Item2"
 *
 * This will end up in PlaceholderItemProxyModel holding the following indices:
 * - "Item1" (from source model)
 * - "Item2" (from source model)
 * - "(Add new entry)" (from PlaceholderItemProxyModel)
 *
 * In case the last entry is edited, and a non-empty value is supplied,
 * dataInserted() is emitted to notify the user about newly created rows.
 * The user then has to make sure the signal is handled accordingly and
 * new items are added to the source model.
 *
 * @see dataInserted
 *
 * @note WARNING: This implementation is only suitable for flat models
 * It will fall apart when you use a tree model as source
 */
class KDEVPLATFORMUTIL_EXPORT PlaceholderItemProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    explicit PlaceholderItemProxyModel(QObject* parent = nullptr);
    ~PlaceholderItemProxyModel() override;

    QVariant columnHint(int column) const;

    /**
     * Set the hint value for @p column to @p hint
     *
     * This text is going to be displayed in the place holder item row
     *
     * Only columns with non-empty hints are clickable and editable and
     * eventually cause the dataInserted() signal to be triggered
     */
    void setColumnHint(int column, const QVariant& hint);

    void setSourceModel(QAbstractItemModel* sourceModel) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QModelIndex parent(const QModelIndex& child) const override;
    QModelIndex sibling(int row, int column, const QModelIndex& idx) const override;
    QModelIndex buddy(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

    /**
     * Implement in subclass.
     *
     * @return True in case the input was valid, and the filter should notify
     *   external observers via the dataInserted signal.
     *
     * By default, this method returns true only in case @p value is non-empty
     *
     * @sa dataInserted()
     */
    virtual bool validateRow(const QModelIndex& index, const QVariant& value) const;

Q_SIGNALS:
    void dataInserted(int column, const QVariant& values);

private:
    const QScopedPointer<class PlaceholderItemProxyModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PlaceholderItemProxyModel)
};

}

#endif // KDEVPLATFORM_PLACEHOLDERITEMPROXYMODEL_H
