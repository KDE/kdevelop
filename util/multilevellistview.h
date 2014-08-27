/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#ifndef KDEVPLATFORM_MULTILEVELLISTVIEW_H
#define KDEVPLATFORM_MULTILEVELLISTVIEW_H

#include <QWidget>
#include <KSelectionProxyModel>
#include "utilexport.h"

class QTreeView;
class QModelIndex;
class QAbstractItemModel;

namespace KDevelop
{

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
    /**
     * Creates a new MultiLevelListView with parent @p parent.
     *
     * Call setLevels() afterwards to set the number of list views.
     *
     * @param parent parent widget
     * @param f window flags, passed to QWidget
     */
    explicit MultiLevelListView(QWidget* parent = 0, Qt::WindowFlags f = 0);
    /**
     * Default destructor
     */
    virtual ~MultiLevelListView();

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
     * Set the filter behavior of the last model. By default, SubTreesWithoutRoots
     * is used and only leafs are selectable in the view for that model.
     */
    void setLastModelsFilterBehavior(KSelectionProxyModel::FilterBehavior filter);
signals:
    /**
     * Notified that the current index has changed from @p previous to @p current
     *
     * @param current the new current index
     * @param previous the previous index
     *
     * @sa currentIndex(), QItemSelectionModel::currentChanged()
     */
    void currentIndexChanged(const QModelIndex& current, const QModelIndex& previous);

public slots:
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
    friend class MultiLevelListViewPrivate;
    class MultiLevelListViewPrivate* const d;
    Q_PRIVATE_SLOT(d, void viewSelectionChanged(const QModelIndex& current,
                                                const QModelIndex& previous))
    Q_PRIVATE_SLOT(d, void lastViewsContentsChanged())
    Q_PRIVATE_SLOT(d, void ensureViewSelected(QTreeView* view))
};

}

#endif // KDEVPLATFORM_MULTILEVELLISTVIEW_H
