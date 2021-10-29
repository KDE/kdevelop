/*
    SPDX-FileCopyrightText: 2013 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTMODELITEMDELEGATE_H
#define KDEVPLATFORM_PLUGIN_PROJECTMODELITEMDELEGATE_H

#include <QItemDelegate>
#include <QPointer>

namespace KDevelop
{
class NavigationToolTip;
}

class ProjectModelItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ProjectModelItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                             const QRect &rect, const QString &text) const override;
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

private:
    void drawBranchName(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect,
                        const QString& branchName) const;
    void drawStyledBackground(QPainter* painter, const QStyleOptionViewItem& option) const;

private:
    QPointer<KDevelop::NavigationToolTip> m_tooltip;
    QPersistentModelIndex m_tooltippedIndex;
};

#endif // KDEVPLATFORM_PLUGIN_PROJECTMODELITEMDELEGATE_H
