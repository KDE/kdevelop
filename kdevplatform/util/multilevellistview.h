/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_MULTILEVELLISTVIEW_H
#define KDEVPLATFORM_MULTILEVELLISTVIEW_H

#include <QWidget>
#include "utilexport.h"

class QTreeView;
class QModelIndex;
class QAbstractItemModel;

namespace KDevelop {
class MultiLevelListViewPrivate;

/**
 * A view for displaying a tree structure in a series of list views.
 *
 * A MultiLevelListView can have any number of levels, with one list view for each level.
 * Selecting an item at one level causes that item to become the root of the next level.
 *
 * For compatibility and convenience, this class has methods and signals similar to those of
 * QAbstractItemView, such as setModel(), setRootIndex() and currentIndexChanged().
 */
class KDEVPLATFORMUTIL_EXPORT MultiLevelListView : public QWidget
{
    Q_OBJECT

public:
    enum LastLevelViewMode {
        SubTrees,      ///< Shows complete subtree for each child. Only leafs are selectable.
        DirectChildren ///< Shows only the direct childs.
    };
    Q_ENUM(LastLevelViewMode)

    /**
     * Creates a new MultiLevelListView with parent @p parent.
     *
     * Call setLevels() afterwards to set the number of list views.
     *
     * @param parent parent widget
     * @param f window flags, passed to QWidget
     */
    explicit MultiLevelListView(QWidget* parent = nullptr, Qt::WindowFlags f = {});
    /**
     * Default destructor
     */
    ~MultiLevelListView() override;

    /**
     * @return the number of list view
     */
    int levels() const;
    /**
     * Sets the number of levels, i.e. the number of list views visible, to @p levels
     * @param levels the new number of levels
     */
    void setLevels(int levels);

    /**
     * @return the model displayed by this view, or 0 if none was set
     * @sa QAbstractItemView::model()
     */
    QAbstractItemModel* model() const;
    /**
     * Sets the model to be displayed by this view.
     *
     * @param model the model to be displayed
     * @sa QAbstractItemView::setModel()
     */
    void setModel(QAbstractItemModel* model);
    /**
     * Provides access to the QTreeView objects used internally.
     * Returns the view for level @p level of the tree structure.
     *
     * @param level the level of the tree structure shown by the returned view
     */
    QTreeView* viewForLevel(int level) const;

    /**
     * The current index of the view.
     *
     * The current index is determined as the current index of the last list view.
     *
     * @sa QAbstractItemView::currentIndex()
     */
    QModelIndex currentIndex() const;

    /**
     * Adds the widget @p widget under the list view for level @p level.
     * This function can be used to insert custom widgets into the view hierarchy.
     *
     * @param level specifies where to place the widget
     * @param widget the widget to add
    */
    void addWidget(int level, QWidget* widget);

    void setHeaderLabels(const QStringList& labels);

    /**
     * Set the view mode of the view for the last level.
     * Default is @c SubTrees.
     */
    void setLastLevelViewMode(LastLevelViewMode mode);

Q_SIGNALS:
    /**
     * Notified that the current index has changed from @p previous to @p current
     *
     * @param current the new current index
     * @param previous the previous index
     *
     * @note This signal is *not* emitted when the current index becomes
     *       invalid, which can happen if the model becomes empty.
     *
     * @sa currentIndex(), QItemSelectionModel::currentChanged()
     */
    void currentIndexChanged(const QModelIndex& current, const QModelIndex& previous);

public Q_SLOTS:
    /**
     * Sets the root index of the entire view to @p index.
     *
     * @sa QAbstractItemView::setRootIndex()
     */
    void setRootIndex(const QModelIndex& index);
    /**
     * Sets the current index to @p index.
     *
     * @sa currentIndex(), QAbstractItemView::setCurrentIndex()
     */
    void setCurrentIndex(const QModelIndex& index);

private:
    const QScopedPointer<class MultiLevelListViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE(MultiLevelListView)
    friend class MultiLevelListViewPrivate;
    Q_PRIVATE_SLOT(d_func(), void ensureViewSelected(QTreeView * view))
};

}

#endif // KDEVPLATFORM_MULTILEVELLISTVIEW_H
