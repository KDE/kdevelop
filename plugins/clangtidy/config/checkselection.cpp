/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checkselection.h"

// plugin
#include "checklistfilterproxysearchline.h"
#include "checklistitemproxystyle.h"
#include "checklistmodel.h"
#include <debug.h>
// Qt
#include <QEvent>
#include <QVBoxLayout>
#include <QTreeView>
#include <QHeaderView>
#include <QSortFilterProxyModel>

namespace ClangTidy
{

CheckSelection::CheckSelection(QWidget* parent)
    : QWidget(parent)
    , m_checkListModel(new CheckListModel(this))
{
    auto* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    auto* checkFilterEdit = new CheckListFilterProxySearchLine(this);
    layout->addWidget(checkFilterEdit);

    m_checkListView = new QTreeView(this);
    m_checkListView->setAllColumnsShowFocus(true);
    m_checkListView->setRootIsDecorated(true);
    m_checkListView->setHeaderHidden(true);
    m_checkListView->setUniformRowHeights(true);
    m_proxyStyle = new CheckListItemProxyStyle;
    m_proxyStyle->setParent(this);
    m_checkListView->setStyle(m_proxyStyle);
    layout->addWidget(m_checkListView);

    setLayout(layout);

    m_checksFilterProxyModel = new QSortFilterProxyModel(this);
    m_checksFilterProxyModel->setRecursiveFilteringEnabled(true);
    checkFilterEdit->setFilterProxyModel(m_checksFilterProxyModel);
    m_checksFilterProxyModel->setSourceModel(m_checkListModel);
    m_checksFilterProxyModel->setFilterKeyColumn(CheckListModel::NameColumnId);
    m_checksFilterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_checkListView->setModel(m_checksFilterProxyModel);
    auto* header = m_checkListView->header();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(CheckListModel::NameColumnId, QHeaderView::Stretch);
    header->setSectionResizeMode(CheckListModel::CountColumnId, QHeaderView::ResizeToContents);

    connect(m_checkListModel, &CheckListModel::enabledChecksChanged,
            this, &CheckSelection::onEnabledChecksChanged);
}

CheckSelection::~CheckSelection() = default;

void CheckSelection::setEditable(bool editable)
{
    m_checkListModel->setEditable(editable);
}

void CheckSelection::setCheckSet(const CheckSet* checkSet)
{
    m_checkListModel->setCheckSet(checkSet);
    expandSubGroupsWithExplicitlyEnabledStates();
}

void CheckSelection::expandSubGroupsWithExplicitlyEnabledStates()
{
    const QModelIndex allChecksIndex = m_checksFilterProxyModel->index(0, 0, QModelIndex());
    expandSubGroupsWithExplicitlyEnabledStates(allChecksIndex);
}

void CheckSelection::expandSubGroupsWithExplicitlyEnabledStates(const QModelIndex& groupIndex)
{
    if (groupIndex.data(CheckListModel::HasExplicitEnabledStateRole).toBool()) {
        m_checkListView->setExpanded(groupIndex, true);
        const int rowCount = m_checksFilterProxyModel->rowCount(groupIndex);
        for (int c = 0; c < rowCount; ++c) {
            const auto childIndex = m_checksFilterProxyModel->index(c, 0, groupIndex);
            if (m_checksFilterProxyModel->hasChildren(childIndex)) {
                expandSubGroupsWithExplicitlyEnabledStates(childIndex);
            }
        }
    }
}

void CheckSelection::setChecks(const QString& checks)
{
    m_checkListModel->setEnabledChecks(checks.split(QLatin1Char(','), Qt::SkipEmptyParts));
    expandSubGroupsWithExplicitlyEnabledStates();
}

QString CheckSelection::checks() const
{
    return m_checkListModel->enabledChecks().join(QLatin1Char(','));
}


bool CheckSelection::event(QEvent* event)
{
    if (event->type() == QEvent::StyleChange) {
        // no recursion protection needed as the style is set on the subchild only
        m_checkListView->setStyle(nullptr);
        delete m_proxyStyle;
        m_proxyStyle = new CheckListItemProxyStyle;
        m_proxyStyle->setParent(this);
        m_checkListView->setStyle(m_proxyStyle);
    }

    return QWidget::event(event);
}

void CheckSelection::onEnabledChecksChanged()
{
    emit checksChanged(checks());
}

}

#include "moc_checkselection.cpp"
