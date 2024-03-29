/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_EXPANDING_WIDGET_MODEL_H
#define KDEVPLATFORM_PLUGIN_EXPANDING_WIDGET_MODEL_H

#include <QAbstractTableModel>
#include <QPointer>

class ExpandingDelegate;
class ExpandingTree;

class QTreeView;

/**
 * Cares about expanding/un-expanding items in a tree-view together with ExpandingDelegate
 */
class ExpandingWidgetModel
    : public QAbstractTableModel
{
    Q_OBJECT
public:

    explicit ExpandingWidgetModel(QWidget* parent);
    ~ExpandingWidgetModel() override;

    enum ExpandingType {
        NotExpandable = 0,
        Expandable,
        Expanded
    };

    ///The following three are convenience-functions for the current item that could be replaced by the later ones
    ///@return whether the current item can be expanded
    bool canExpandCurrentItem() const;
    ///@return whether the current item can be collapsed
    bool canCollapseCurrentItem() const;
    ///Expand/collapse the current item
    void setCurrentItemExpanded(bool);

    void clearMatchQualities();

    ///Unexpand all rows and clear all cached information about them(this includes deleting the expanding-widgets)
    void clearExpanding();

    ///@return whether the row given through index is expandable
    bool isExpandable(const QModelIndex& index) const;

    enum ExpansionType {
        NotExpanded = 0,
        ExpandDownwards, //The additional(expanded) information is shown UNDER the original information
        ExpandUpwards //The additional(expanded) information is shown ABOVE the original information
    };

    ///Returns whether the given index is currently partially expanded. Does not do any other checks like calling models for data.
    ExpansionType isPartiallyExpanded(const QModelIndex& index) const;

    ///@return whether row is currently expanded
    bool isExpanded(const QModelIndex& row) const;
    ///Change the expand-state of the row given through index. The display will be updated.
    void setExpanded(const QModelIndex& index, bool expanded);

    ///Returns the total height added through all open expanding-widgets
    int expandingWidgetsHeight() const;

    ///@return the expanding-widget for the given row, if available. Expanding-widgets are in best case available for all expanded rows.
    ///This does not return the partially-expand widget.
    QWidget* expandingWidget(const QModelIndex& row) const;

    ///Amount by which the height of a row increases when it is partially expanded
    int partiallyExpandWidgetHeight() const;
    /**
     * Notifies underlying models that the item was selected, collapses any previous partially expanded line,
     * checks whether this line should be partially expanded, and eventually does it.
     * Does nothing when nothing needs to be done.
     * Does NOT show the expanding-widget. That is done immediately when painting by ExpandingDelegate,
     * to reduce flickering. @see showPartialExpandWidget()
     * @param row The row
     * */
    ///
    virtual void rowSelected(const QModelIndex& row);

    /// Returns the rectangle for the partially expanded part of the given row
    /// TODO: Do this via QAIM roles?
    QRect partialExpandRect(const QModelIndex& row) const;

    /// TODO: Do this via QAIM roles?
    QString partialExpandText(const QModelIndex& row) const;

    ///Places and shows the expanding-widget for the given row, if it should be visible and is valid.
    ///Also shows the partial-expanding-widget when it should be visible.
    void placeExpandingWidget(const QModelIndex& row);

    virtual QTreeView* treeView() const = 0;

    ///Should return true if the given row should be painted like a contained item(as opposed to label-rows etc.)
    virtual bool indexIsItem(const QModelIndex& index) const = 0;

    ///Does not request data from index, this only returns local data like highlighting for expanded rows and similar
    QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;

    ///Returns the first row that is currently partially expanded.
    QModelIndex partiallyExpandedRow() const;

    ///Returns the match-color for the given index, or zero if match-quality could not be computed.
    uint matchColor(const QModelIndex& index) const;
public Q_SLOTS:
    ///Place or hides all expanding-widgets to the correct positions. Should be called after the view was scrolled.
    void placeExpandingWidgets();
protected:
    /**
     * @return the context-match quality from 0 to 10 if it could be determined, else -1
     * */
    virtual int contextMatchQuality(const QModelIndex& index) const = 0;

    QModelIndex mapFromSource(const QModelIndex& index) const;
    QModelIndex mapToSource(const QModelIndex& index) const;

    //Does not update the view
    void partiallyUnExpand(const QModelIndex& index);
    //Finds out the basic height of the row represented by the given index. Basic means without respecting any expansion.
    int basicRowHeight(const QModelIndex& index) const;
private:
    friend ExpandingDelegate;
    friend ExpandingTree;

    QMap<QModelIndex, ExpansionType> m_partiallyExpanded;
    // Store expanding-widgets and cache whether items can be expanded
    mutable QMap<QModelIndex, ExpandingType> m_expandState;
    QMap<QModelIndex, QPointer<QWidget> > m_expandingWidgets;  //Map rows to their expanding-widgets
    QMap<QModelIndex, int> m_contextMatchQualities;   //Map rows to their context-match qualities(undefined if unknown, else 0 to 10). Not used yet, eventually remove.
};

/**
 * Helper-function to merge custom-highlighting variant-lists.
 *
 * @param strings A list of strings that should be merged
 * @param highlights One variant-list for highlighting, as described in the kde header ktextedtor/codecompletionmodel.h
 * @param gapBetweenStrings How many signs are inserted between 2 strings?
 * */
QList<QVariant> mergeCustomHighlighting(const QStringList& strings, const QList<QVariantList>& highlights, int gapBetweenStrings = 0);
#endif
