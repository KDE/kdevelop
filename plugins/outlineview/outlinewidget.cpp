/*
    SPDX-FileCopyrightText: 2010, 2015 Alex Richardson <alex.richardson@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "outlinewidget.h"

#include <QAction>
#include <QVBoxLayout>
#include <QTreeView>
#include <QLineEdit>
#include <QIcon>
#include <QWidgetAction>
#include <QSortFilterProxyModel>

#include <KLocalizedString>

#include "outlineviewplugin.h"
#include "outlinemodel.h"

using namespace KDevelop;

OutlineWidget::OutlineWidget(QWidget* parent, OutlineViewPlugin* plugin)
    : QWidget(parent)
    , m_plugin(plugin)
    , m_model(new OutlineModel(this))
    , m_tree(new QTreeView(this))
    , m_proxy(new QSortFilterProxyModel(this))
    , m_filter(new QLineEdit(this))
{
    setObjectName(QStringLiteral("Outline View"));
    setWindowTitle(i18nc("@title:window", "Outline"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("code-class"), windowIcon())); //TODO: better icon?

    m_proxy->setRecursiveFilteringEnabled(true);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setDynamicSortFilter(false);

    m_tree->setModel(m_proxy);
    m_tree->setHeaderHidden(true);

    // sort action
    m_sortAlphabeticallyAction = new QAction(QIcon::fromTheme(QStringLiteral("view-sort-ascending")),
                                             i18nc("@action", "Sort Alphabetically"), this);
    m_sortAlphabeticallyAction->setToolTip(i18nc("@info:tooltip", "Sort items alphabetically"));
    m_sortAlphabeticallyAction->setCheckable(true);
    connect(m_sortAlphabeticallyAction, &QAction::triggered, this, [this](bool sort) {
        // calling sort with -1 will restore the original order
        m_proxy->sort(sort ? 0 : -1, Qt::AscendingOrder);
        m_sortAlphabeticallyAction->setChecked(sort);
    });
    addAction(m_sortAlphabeticallyAction);

    // filter
    connect(m_filter, &QLineEdit::textChanged, m_proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(m_tree, &QTreeView::activated, this, &OutlineWidget::activated);
    m_filter->setPlaceholderText(i18nc("@info:placeholder", "Filter..."));
    auto filterAction = new QWidgetAction(this);
    filterAction->setDefaultWidget(m_filter);
    addAction(filterAction);

    setFocusProxy(m_filter);

    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(m_tree);
    setLayout(vbox);
    expandFirstLevel();
    connect(m_model, &QAbstractItemModel::modelReset, this, &OutlineWidget::expandFirstLevel);
}

void OutlineWidget::activated(const QModelIndex& index)
{
    QModelIndex realIndex = m_proxy->mapToSource(index);
    m_model->activate(realIndex);
}

OutlineWidget::~OutlineWidget()
{
}

void OutlineWidget::expandFirstLevel()
{
    for (int i = 0; i < m_proxy->rowCount(); i++) {
        m_tree->expand(m_proxy->index(i, 0));
    }
}
