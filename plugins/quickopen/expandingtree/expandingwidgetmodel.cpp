/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "expandingwidgetmodel.h"

#include <QTreeView>
#include <QAbstractProxyModel>
#include <QModelIndex>
#include <QBrush>
#include <QApplication>

#include <KTextEditor/CodeCompletionModel>
#include <KTextEdit>
#include <KColorUtils>

#include "expandingdelegate.h"
#include <debug.h>

using namespace KTextEditor;

inline QModelIndex firstColumn(const QModelIndex& index)
{
    return index.sibling(index.row(), 0);
}

ExpandingWidgetModel::ExpandingWidgetModel(QWidget* parent) :
    QAbstractTableModel(parent)
{
}

ExpandingWidgetModel::~ExpandingWidgetModel()
{
    clearExpanding();
}

static QColor doAlternate(const QColor& color)
{
    QColor background = QApplication::palette().window().color();
    return KColorUtils::mix(color, background, 0.15);
}

uint ExpandingWidgetModel::matchColor(const QModelIndex& index) const
{
    int matchQuality = contextMatchQuality(index.sibling(index.row(), 0));

    if (matchQuality > 0) {
        bool alternate = index.row() & 1;

        QColor badMatchColor(0xff00aa44); //Blue-ish green
        QColor goodMatchColor(0xff00ff00); //Green

        QColor background = treeView()->palette().light().color();

        QColor totalColor = KColorUtils::mix(badMatchColor, goodMatchColor, (( float )matchQuality) / 10.0);

        if (alternate) {
            totalColor = doAlternate(totalColor);
        }

        const float dynamicTint = 0.2f;
        const float minimumTint = 0.2f;
        double tintStrength = (dynamicTint * matchQuality) / 10;
        if (tintStrength) {
            tintStrength += minimumTint; //Some minimum tinting strength, else it's not visible any more
        }
        return KColorUtils::tint(background, totalColor, tintStrength).rgb();
    } else {
        return 0;
    }
}

QVariant ExpandingWidgetModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::BackgroundRole:
    {
        if (index.column() == 0) {
            //Highlight by match-quality
            uint color = matchColor(index);
            if (color) {
                return QBrush(color);
            }
        }
        //Use a special background-color for expanded items
        if (isExpanded(index)) {
            if (index.row() & 1) {
                return doAlternate(treeView()->palette().toolTipBase().color());
            } else {
                return treeView()->palette().toolTipBase();
            }
        }
    }
    }
    return QVariant();
}

QModelIndex ExpandingWidgetModel::mapFromSource(const QModelIndex& index) const
{
    const auto proxyModel = qobject_cast<QAbstractProxyModel*>(treeView()->model());
    Q_ASSERT(proxyModel);
    Q_ASSERT(!index.isValid() || index.model() == this);
    return proxyModel->mapFromSource(index);
}

QModelIndex ExpandingWidgetModel::mapToSource(const QModelIndex& index) const
{
    const auto proxyModel = qobject_cast<QAbstractProxyModel*>(treeView()->model());
    Q_ASSERT(proxyModel);
    Q_ASSERT(!index.isValid() || index.model() == proxyModel);
    return proxyModel->mapToSource(index);
}

void ExpandingWidgetModel::clearMatchQualities()
{
    m_contextMatchQualities.clear();
}

QModelIndex ExpandingWidgetModel::partiallyExpandedRow() const
{
    if (m_partiallyExpanded.isEmpty()) {
        return QModelIndex();
    } else {
        return m_partiallyExpanded.constBegin().key();
    }
}

void ExpandingWidgetModel::clearExpanding()
{
    clearMatchQualities();
    QMap<QModelIndex, ExpandingWidgetModel::ExpandingType> oldExpandState = m_expandState;
    for (auto& widget : std::as_const(m_expandingWidgets)) {
        delete widget;
    }

    m_expandingWidgets.clear();
    m_expandState.clear();
    m_partiallyExpanded.clear();

    for (QMap<QModelIndex, ExpandingWidgetModel::ExpandingType>::const_iterator it = oldExpandState.constBegin(); it != oldExpandState.constEnd(); ++it) {
        if (it.value() == Expanded) {
            emit dataChanged(it.key(), it.key());
        }
    }
}

ExpandingWidgetModel::ExpansionType ExpandingWidgetModel::isPartiallyExpanded(const QModelIndex& index) const
{
    const auto expansionIt = m_partiallyExpanded.find(firstColumn(index));
    if (expansionIt != m_partiallyExpanded.end()) {
        return *expansionIt;
    } else {
        return NotExpanded;
    }
}

void ExpandingWidgetModel::partiallyUnExpand(const QModelIndex& idx_)
{
    QModelIndex index(firstColumn(idx_));
    m_partiallyExpanded.remove(index);
    m_partiallyExpanded.remove(idx_);
}

int ExpandingWidgetModel::partiallyExpandWidgetHeight() const
{
    return 60; ///@todo use font-metrics text-height*2 for 2 lines
}

void ExpandingWidgetModel::rowSelected(const QModelIndex& idx_)
{
    Q_ASSERT(idx_.model() == this);

    QModelIndex idx(firstColumn(idx_));
    if (!m_partiallyExpanded.contains(idx)) {
        QModelIndex oldIndex = partiallyExpandedRow();
        //Unexpand the previous partially expanded row
        if (!m_partiallyExpanded.isEmpty()) { ///@todo allow multiple partially expanded rows
            while (!m_partiallyExpanded.isEmpty())
                m_partiallyExpanded.erase(m_partiallyExpanded.begin());
            //partiallyUnExpand( m_partiallyExpanded.begin().key() );
        }
        //Notify the underlying models that the item was selected, and eventually get back the text for the expanding widget.
        if (!idx.isValid()) {
            //All items have been unselected
            if (oldIndex.isValid()) {
                emit dataChanged(oldIndex, oldIndex);
            }
        } else {
            QVariant variant = data(idx, CodeCompletionModel::ItemSelected);

            if (!isExpanded(idx) && variant.typeId() == qMetaTypeId<QString>()) {
                //Either expand upwards or downwards, choose in a way that
                //the visible fields of the new selected entry are not moved.
                if (oldIndex.isValid() && (oldIndex < idx || (!(oldIndex < idx) && oldIndex.parent() < idx.parent()))) {
                    m_partiallyExpanded.insert(idx, ExpandUpwards);
                } else {
                    m_partiallyExpanded.insert(idx, ExpandDownwards);
                }

                //Say that one row above until one row below has changed, so no items will need to be moved(the space that is taken from one item is given to the other)
                if (oldIndex.isValid() && oldIndex < idx) {
                    emit dataChanged(oldIndex, idx);

                    if (treeView()->verticalScrollMode() == QAbstractItemView::ScrollPerItem) {
                        const QModelIndex viewIndex = mapFromSource(idx);
                        //Qt fails to correctly scroll in ScrollPerItem mode, so the selected index is completely visible,
                        //so we do the scrolling by hand.
                        QRect selectedRect = treeView()->visualRect(viewIndex);
                        QRect frameRect = treeView()->frameRect();

                        if (selectedRect.bottom() > frameRect.bottom()) {
                            int diff = selectedRect.bottom() - frameRect.bottom();
                            //We need to scroll down
                            QModelIndex newTopIndex = viewIndex;

                            QModelIndex nextTopIndex = viewIndex;
                            QRect nextRect = treeView()->visualRect(nextTopIndex);
                            while (nextTopIndex.isValid() && nextRect.isValid() && nextRect.top() >= diff) {
                                newTopIndex = nextTopIndex;
                                nextTopIndex = treeView()->indexAbove(nextTopIndex);
                                if (nextTopIndex.isValid()) {
                                    nextRect = treeView()->visualRect(nextTopIndex);
                                }
                            }
                            treeView()->scrollTo(newTopIndex, QAbstractItemView::PositionAtTop);
                        }
                    }

                    //This is needed to keep the item we are expanding completely visible. Qt does not scroll the view to keep the item visible.
                    //But we must make sure that it isn't too expensive.
                    //We need to make sure that scrolling is efficient, and the whole content is not repainted.
                    //Since we are scrolling anyway, we can keep the next line visible, which might be a cool feature.

                    //Since this also doesn't work smoothly, leave it for now
                    //treeView()->scrollTo( nextLine, QAbstractItemView::EnsureVisible );
                } else if (oldIndex.isValid() &&  idx < oldIndex) {
                    emit dataChanged(idx, oldIndex);

                    //For consistency with the down-scrolling, we keep one additional line visible above the current visible.

                    //Since this also doesn't work smoothly, leave it for now
/*            QModelIndex prevLine = idx.sibling(idx.row()-1, idx.column());
            if( prevLine.isValid() )
                treeView()->scrollTo( prevLine );*/
                } else {
                    emit dataChanged(idx, idx);
                }
            } else if (oldIndex.isValid()) {
                //We are not partially expanding a new row, but we previously had a partially expanded row. So signalize that it has been unexpanded.

                emit dataChanged(oldIndex, oldIndex);
            }
        }
    } else {
        qCDebug(PLUGIN_QUICKOPEN) << "ExpandingWidgetModel::rowSelected: Row is already partially expanded";
    }
}

QString ExpandingWidgetModel::partialExpandText(const QModelIndex& idx) const
{
    Q_ASSERT(idx.model() == this);

    if (!idx.isValid()) {
        return QString();
    }

    return data(firstColumn(idx), CodeCompletionModel::ItemSelected).toString();
}

QRect ExpandingWidgetModel::partialExpandRect(const QModelIndex& idx_) const
{
    Q_ASSERT(idx_.model() == this);

    QModelIndex idx(firstColumn(idx_));

    if (!idx.isValid()) {
        return QRect();
    }

    ExpansionType expansion = ExpandDownwards;

    if (m_partiallyExpanded.find(idx) != m_partiallyExpanded.constEnd()) {
        expansion = m_partiallyExpanded[idx];
    }

    //Get the whole rectangle of the row:
    const QModelIndex viewIndex = mapFromSource(idx);
    QModelIndex rightMostIndex = viewIndex;
    QModelIndex tempIndex = viewIndex;
    while ((tempIndex = rightMostIndex.sibling(rightMostIndex.row(), rightMostIndex.column() + 1)).isValid())
        rightMostIndex = tempIndex;

    QRect rect = treeView()->visualRect(viewIndex);
    QRect rightMostRect = treeView()->visualRect(rightMostIndex);

    rect.setLeft(rect.left() + 20);
    rect.setRight(rightMostRect.right() - 5);

    //These offsets must match exactly those used in ExpandingDelegate::sizeHint()
    int top = rect.top() + 5;
    int bottom = rightMostRect.bottom() - 5;

    if (expansion == ExpandDownwards) {
        top += basicRowHeight(viewIndex);
    } else {
        bottom -= basicRowHeight(viewIndex);
    }

    rect.setTop(top);
    rect.setBottom(bottom);

    return rect;
}

bool ExpandingWidgetModel::isExpandable(const QModelIndex& idx_) const
{
    Q_ASSERT(idx_.model() == this);

    QModelIndex idx(firstColumn(idx_));

    auto expandStateIt = m_expandState.find(idx);
    if (expandStateIt == m_expandState.end()) {
        expandStateIt = m_expandState.insert(idx, NotExpandable);
        QVariant v = data(idx, CodeCompletionModel::IsExpandable);
        if (v.canConvert<bool>() && v.toBool()) {
            *expandStateIt = Expandable;
        }
    }

    return *expandStateIt != NotExpandable;
}

bool ExpandingWidgetModel::isExpanded(const QModelIndex& idx_) const
{
    Q_ASSERT(idx_.model() == this);

    QModelIndex idx(firstColumn(idx_));
    return m_expandState.contains(idx) && m_expandState[idx] == Expanded;
}

void ExpandingWidgetModel::setExpanded(const QModelIndex& idx_, bool expanded)
{
    Q_ASSERT(idx_.model() == this);

    QModelIndex idx(firstColumn(idx_));

    qCDebug(PLUGIN_QUICKOPEN) << "Setting expand-state of row " << idx.row() << " to " << expanded;
    if (!idx.isValid()) {
        return;
    }

    if (isExpandable(idx)) {
        if (!expanded && m_expandingWidgets.contains(idx) && m_expandingWidgets[idx]) {
            m_expandingWidgets[idx]->hide();
        }

        m_expandState[idx] = expanded ? Expanded : Expandable;

        if (expanded) {
            partiallyUnExpand(idx);
        }

        if (expanded && !m_expandingWidgets.contains(idx)) {
            QVariant v = data(idx, CodeCompletionModel::ExpandingWidget);

            if (v.canConvert<QWidget*>()) {
                m_expandingWidgets[idx] = v.value<QWidget*>();
            } else if (v.canConvert<QString>()) {
                //Create a html widget that shows the given string
                auto* edit = new KTextEdit(v.toString());
                edit->setReadOnly(true);
                edit->resize(200, 50); //Make the widget small so it embeds nicely.
                m_expandingWidgets[idx] = edit;
            } else {
                m_expandingWidgets[idx] = nullptr;
            }
        }

        //Eventually partially expand the row
        if (!expanded && firstColumn(mapToSource(treeView()->currentIndex())) == idx && (isPartiallyExpanded(idx) == ExpandingWidgetModel::ExpansionType::NotExpanded)) {
            rowSelected(idx); //Partially expand the row.
        }
        emit dataChanged(idx, idx);

        if (treeView()) {
            treeView()->scrollTo(mapFromSource(idx));
        }
    }
}

int ExpandingWidgetModel::basicRowHeight(const QModelIndex& idx_) const
{
    Q_ASSERT(idx_.model() == treeView()->model());

    QModelIndex idx(firstColumn(idx_));

    auto* delegate = qobject_cast<ExpandingDelegate*>(treeView()->itemDelegateForIndex(idx));
    if (!delegate || !idx.isValid()) {
        qCDebug(PLUGIN_QUICKOPEN) << "ExpandingWidgetModel::basicRowHeight: Could not get delegate";
        return 15;
    }
    return delegate->basicSizeHint(idx).height();
}

void ExpandingWidgetModel::placeExpandingWidget(const QModelIndex& idx_)
{
    Q_ASSERT(idx_.model() == this);

    QModelIndex idx(firstColumn(idx_));

    QWidget* w = nullptr;
    const auto widgetIt = m_expandingWidgets.constFind(idx);
    if (widgetIt != m_expandingWidgets.constEnd()) {
        w = *widgetIt;
    }

    if (w && isExpanded(idx)) {
        if (!idx.isValid()) {
            return;
        }

        const QModelIndex viewIndex = mapFromSource(idx_);
        QRect rect = treeView()->visualRect(viewIndex);

        if (!rect.isValid() || rect.bottom() < 0 || rect.top() >= treeView()->height()) {
            //The item is currently not visible
            w->hide();
            return;
        }

        QModelIndex rightMostIndex = viewIndex;
        QModelIndex tempIndex = viewIndex;
        while ((tempIndex = rightMostIndex.sibling(rightMostIndex.row(), rightMostIndex.column() + 1)).isValid())
            rightMostIndex = tempIndex;

        QRect rightMostRect = treeView()->visualRect(rightMostIndex);

        //Find out the basic height of the row
        rect.setLeft(rect.left() + 20);
        rect.setRight(rightMostRect.right() - 5);

        //These offsets must match exactly those used in KateCompletionDeleage::sizeHint()
        rect.setTop(rect.top() + basicRowHeight(viewIndex) + 5);
        rect.setHeight(w->height());

        if (w->parent() != treeView()->viewport() || w->geometry() != rect || !w->isVisible()) {
            w->setParent(treeView()->viewport());

            w->setGeometry(rect);
            w->show();
        }
    }
}

void ExpandingWidgetModel::placeExpandingWidgets()
{
    for (QMap<QModelIndex, QPointer<QWidget> >::const_iterator it = m_expandingWidgets.constBegin(); it != m_expandingWidgets.constEnd(); ++it) {
        placeExpandingWidget(it.key());
    }
}

int ExpandingWidgetModel::expandingWidgetsHeight() const
{
    int sum = 0;
    for (QMap<QModelIndex, QPointer<QWidget> >::const_iterator it = m_expandingWidgets.constBegin(); it != m_expandingWidgets.constEnd(); ++it) {
        if (isExpanded(it.key()) && (*it)) {
            sum += (*it)->height();
        }
    }

    return sum;
}

QWidget* ExpandingWidgetModel::expandingWidget(const QModelIndex& idx_) const
{
    QModelIndex idx(firstColumn(idx_));

    const auto widgetIt = m_expandingWidgets.find(idx);
    if (widgetIt != m_expandingWidgets.end()) {
        return *widgetIt;
    } else {
        return nullptr;
    }
}

QList<QVariant> mergeCustomHighlighting(int leftSize, const QList<QVariant>& left, int rightSize, const QList<QVariant>& right)
{
    QList<QVariant> ret = left;
    if (left.isEmpty()) {
        ret << QVariant(0);
        ret << QVariant(leftSize);
        ret << QTextFormat(QTextFormat::CharFormat);
    }

    if (right.isEmpty()) {
        ret << QVariant(leftSize);
        ret << QVariant(rightSize);
        ret << QTextFormat(QTextFormat::CharFormat);
    } else {
        QList<QVariant>::const_iterator it = right.constBegin();
        while (it != right.constEnd()) {
            {
                QList<QVariant>::const_iterator testIt = it;
                for (int a = 0; a < 2; a++) {
                    ++testIt;
                    if (testIt == right.constEnd()) {
                        qCWarning(PLUGIN_QUICKOPEN) << "Length of input is not multiple of 3";
                        break;
                    }
                }
            }

            ret << QVariant((*it).toInt() + leftSize);
            ++it;
            ret << QVariant((*it).toInt());
            ++it;
            ret << *it;
            if (!(*it).value<QTextFormat>().isValid()) {
                qCDebug(PLUGIN_QUICKOPEN) << "Text-format is invalid";
            }
            ++it;
        }
    }
    return ret;
}

//It is assumed that between each two strings, one space is inserted
QList<QVariant> mergeCustomHighlighting(const QStringList& strings_, const QList<QVariantList>& highlights_, int grapBetweenStrings)
{
    QStringList strings(strings_);
    QList<QVariantList> highlights(highlights_);

    if (strings.isEmpty()) {
        qCWarning(PLUGIN_QUICKOPEN) << "List of strings is empty";
        return QList<QVariant>();
    }

    if (highlights.isEmpty()) {
        qCWarning(PLUGIN_QUICKOPEN) << "List of highlightings is empty";
        return QList<QVariant>();
    }

    if (strings.count() != highlights.count()) {
        qCWarning(PLUGIN_QUICKOPEN) << "Length of string-list is " << strings.count() << " while count of highlightings is " << highlights.count() << ", should be same";
        return QList<QVariant>();
    }

    //Merge them together
    QString totalString = strings[0];
    QVariantList totalHighlighting = highlights[0];

    strings.pop_front();
    highlights.pop_front();

    while (!strings.isEmpty()) {
        const int stringLength = strings[0].length();
        totalHighlighting = mergeCustomHighlighting(totalString.length(), totalHighlighting, stringLength, highlights[0]);
        totalString.reserve(totalString.size() + stringLength + grapBetweenStrings);
        totalString += strings[0];

        for (int a = 0; a < grapBetweenStrings; a++) {
            totalString += QLatin1Char(' ');
        }

        strings.pop_front();
        highlights.pop_front();
    }
    //Combine the custom-highlightings
    return totalHighlighting;
}

#include "moc_expandingwidgetmodel.cpp"
