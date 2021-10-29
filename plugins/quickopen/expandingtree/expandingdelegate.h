/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_EXPANDINGDELEGATE_H
#define KDEVPLATFORM_PLUGIN_EXPANDINGDELEGATE_H

#include <QItemDelegate>
#include <QTextLayout>
#include <QModelIndex>

class KateRenderer;
class KateCompletionWidget;
class KateDocument;
class KateTextLine;
class ExpandingWidgetModel;
class QVariant;
class QStyleOptionViewItem;

/**
 * This is a delegate that cares, together with ExpandingWidgetModel, about embedded widgets in tree-view.
 * */

class ExpandingDelegate
    : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ExpandingDelegate(ExpandingWidgetModel* model, QObject* parent = nullptr);

    // Overridden to create highlighting for current index
    void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    // Returns the basic size-hint as reported by QItemDelegate
    QSize basicSizeHint(const QModelIndex& index) const;

    ExpandingWidgetModel* model() const;
protected:
    //Called right before paint to allow last-minute changes to the style
    virtual void adjustStyle(const QModelIndex& index, QStyleOptionViewItem& option) const;
    void drawDisplay (QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text) const override;
    QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent (QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    virtual void drawBackground (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawDecoration(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QPixmap& pixmap) const override;
    //option can be changed
    virtual QVector<QTextLayout::FormatRange> createHighlighting(const QModelIndex& index, QStyleOptionViewItem& option) const;

    void adjustRect(QRect& rect) const;

    /**
     * Creates a list of FormatRanges as should be returned by createHighlighting from a list of QVariants as described in the kde header ktexteditor/codecompletionmodel.h
     * */
    QVector<QTextLayout::FormatRange> highlightingFromVariantList(const QList<QVariant>& customHighlights) const;

    //Called when an item was expanded/unexpanded and the height changed
    virtual void heightChanged() const;

    //Initializes the style options from the index
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const;

    mutable int m_currentColumnStart; //Text-offset for custom highlighting, will be applied to m_cachedHighlights(Only highlights starting after this will be used). Should be zero of the highlighting is not taken from kate.
    mutable QList<int> m_currentColumnStarts;
    mutable QVector<QTextLayout::FormatRange> m_cachedHighlights;

    mutable Qt::Alignment m_cachedAlignment;
    mutable QColor m_backgroundColor;
    mutable QModelIndex m_currentIndex;
private:

    ExpandingWidgetModel* m_model;
};

#endif // KDEVPLATFORM_PLUGIN_EXPANDINGDELEGATE_H
