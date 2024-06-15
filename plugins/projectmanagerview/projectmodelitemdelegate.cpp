/*
    SPDX-FileCopyrightText: 2013 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectmodelitemdelegate.h"

#include "vcsoverlayproxymodel.h"
#include <debug.h>

#include <project/projectmodel.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/util/navigationtooltip.h>
#include <util/path.h>

#include <QHelpEvent>
#include <QToolTip>
#include <QAbstractItemView>
#include <QPainter>

using namespace KDevelop;

ProjectModelItemDelegate::ProjectModelItemDelegate(QObject* parent)
    : QItemDelegate(parent)
{}

static QIcon::Mode IconMode( QStyle::State state )
{
    if (!(state & QStyle::State_Enabled)) {
        return QIcon::Disabled;
    } else if (state & QStyle::State_Selected) {
        return QIcon::Selected;
    } else {
        return QIcon::Normal;
    }
}

static QIcon::State IconState(QStyle::State state)
{
    return  (state & QStyle::State_Open) ? QIcon::On : QIcon::Off;
}

void ProjectModelItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    // Qt5.5 HiDPI Fix part (1/2)
    // This fix is based on how Qt5.5's QItemDelegate::paint implementation deals with the same issue
    // Unfortunately, there doesn't seem to be a clean way to use the base implementation
    // and have the added functionality this class provides
    QPixmap decoData;
    QRect decorationRect;
    QIcon icon;
    QIcon::Mode mode = QIcon::Mode::Disabled;
    QIcon::State state = QIcon::State::Off;
    {
        QVariant value;
        value = index.data(Qt::DecorationRole);
        if (value.isValid()) {
            decoData = decoration(opt, value);

            if (value.typeId() == qMetaTypeId<QIcon>()) {
                icon = qvariant_cast<QIcon>(value);
                mode = IconMode(opt.state);
                state = IconState(opt.state);
                QSize size = icon.actualSize( opt.decorationSize, mode, state );
                if (size.isEmpty()) {
                    // For items with an empty icon, set size to opt.decorationSize to make them have the same indent as items with a valid icon
                    size = opt.decorationSize;
                }
                decorationRect = QRect(QPoint(0, 0), size);
            } else {
                decorationRect = QRect(QPoint(0, 0), decoData.size());
            }
        } else {
            decorationRect = QRect();
        }
    }


    QRect checkRect; //unused in practice

    QRect spaceLeft = opt.rect;
    spaceLeft.setLeft(decorationRect.right());
    QString displayData = index.data(Qt::DisplayRole).toString();
    QRect displayRect = textRectangle(painter, spaceLeft, opt.font, displayData);
    displayRect.setLeft(spaceLeft.left());

    QRect branchNameRect(displayRect.topRight(), opt.rect.bottomRight());

    doLayout(opt, &checkRect, &decorationRect, &displayRect, false);
    branchNameRect.setLeft(branchNameRect.left() + displayRect.left());
    branchNameRect.setTop(displayRect.top());

    drawStyledBackground(painter, opt);
//     drawCheck(painter, opt, checkRect, checkState);

    // Qt5.5 HiDPI Fix part (2/2)
    // use the QIcon from above if possible
    if (!icon.isNull()) {
        icon.paint(painter, decorationRect, opt.decorationAlignment, mode, state );
    } else {
        drawDecoration(painter, opt, decorationRect, decoData);
    }

    drawDisplay(painter, opt, displayRect, displayData);

    /// FIXME: this can apparently trigger a nested eventloop, see
    ///        https://bugs.kde.org/show_bug.cgi?id=355099
    QString branchNameData = index.data(VcsOverlayProxyModel::VcsStatusRole).toString();
    drawBranchName(painter, opt, branchNameRect, branchNameData);
    drawFocus(painter, opt, displayRect);

}

void ProjectModelItemDelegate::drawBranchName(QPainter* painter, const QStyleOptionViewItem& option,
                                              const QRect& rect, const QString& branchName) const
{
    QString text = option.fontMetrics.elidedText(branchName, Qt::ElideRight, rect.width());

    bool selected = option.state & QStyle::State_Selected;
    QPalette::ColorGroup colorGroup = selected ? QPalette::Active : QPalette::Disabled;
    painter->save();
    painter->setPen(option.palette.color(colorGroup, QPalette::Text));
    painter->drawText(rect, text);
    painter->restore();
}

void ProjectModelItemDelegate::drawStyledBackground(QPainter* painter, const QStyleOptionViewItem& option) const
{
    QStyleOptionViewItem opt(option);
    QStyle *style = opt.widget->style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, opt.widget);
}

void ProjectModelItemDelegate::drawDisplay(QPainter* painter, const QStyleOptionViewItem& option,
                                           const QRect& rect, const QString& text) const
{
    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                            ? QPalette::Normal : QPalette::Disabled;
    if (option.state & QStyle::State_Editing) {
        painter->save();
        painter->setPen(option.palette.color(cg, QPalette::Text));
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
        painter->restore();
    }

    if(text.isEmpty()) {
        return;
    }

    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    QFontMetrics fm(painter->fontMetrics());
    painter->drawText(rect, fm.elidedText(text, Qt::ElideRight, rect.width()));
}

bool ProjectModelItemDelegate::helpEvent(QHelpEvent* event,
                                         QAbstractItemView* view, const QStyleOptionViewItem& option,
                                         const QModelIndex& index)
{
    if (!event || !view) {
        return false;
    }

    if (event->type() == QEvent::ToolTip) {
        // explicitly close current tooltip, as its autoclose margins overlap items
        if ((m_tooltippedIndex != index) && m_tooltip) {
            m_tooltip->close();
            m_tooltip.clear();
        }

        const ProjectBaseItem* it = index.data(ProjectModel::ProjectItemRole).value<ProjectBaseItem*>();

        // show navigation tooltip for files
        if (it && it->file()) {
            if (!m_tooltip) {
                m_tooltippedIndex = index;
                KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
                const TopDUContext* top = DUChainUtils::standardContextForUrl(it->file()->path().toUrl());

                if (top) {
                    if (auto* navigationWidget = top->createNavigationWidget()) {
                        // force possible existing normal tooltip for other list item to hide
                        // Seems that is still only done with a small delay though,
                        // but the API seems not to allow more control.
                        QToolTip::hideText();

                        m_tooltip = new KDevelop::NavigationToolTip(view, event->globalPos() + QPoint(40, 0), navigationWidget);
                        m_tooltip->resize(navigationWidget->sizeHint() + QSize(10, 10));
                        auto rect = view->visualRect(m_tooltippedIndex);
                        rect.moveTopLeft(view->mapToGlobal(rect.topLeft()));
                        m_tooltip->setHandleRect(rect);
                        ActiveToolTip::showToolTip(m_tooltip);
                    }
                }
            }

            // tooltip successfully handled by us?
            if (m_tooltip) {
                return true;
            }
        }
    }

    return QItemDelegate::helpEvent(event, view, option, index);
}

#include "moc_projectmodelitemdelegate.cpp"
