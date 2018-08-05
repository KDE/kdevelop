/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "checkselection.h"

// plugin
#include "checklistmodel.h"
#include <checkset.h>
#include <debug.h>
// KF
#include <KFilterProxySearchLine>
#include <kconfigwidgets_version.h>
#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5,32,0)
#include <KConfigDialogManager>
#endif
// Qt
#include <QVBoxLayout>
#include <QTreeView>
#include <QSortFilterProxyModel>

namespace ClangTidy
{

CheckSelection::CheckSelection(QWidget* parent)
    : QWidget(parent)
    , m_checkListModel(new CheckListModel(this))
{
    // since 5.32 the signal is by default taken as set for the used property
#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5,32,0)
    KConfigDialogManager::changedMap()->insert(QStringLiteral("ClangTidy::CheckSelection"), SIGNAL(checksChanged(QStringList));
#endif

    auto* layout = new QVBoxLayout;
    layout->setMargin(0);

    auto* checkFilterEdit = new KFilterProxySearchLine(this);
    layout->addWidget(checkFilterEdit);

    auto* checkListView = new QTreeView(this);
    checkListView->setAllColumnsShowFocus(true);
    checkListView->setRootIsDecorated(false);
    checkListView->setHeaderHidden(true);
    layout->addWidget(checkListView);

    setLayout(layout);

    auto* checksFilterProxyModel = new QSortFilterProxyModel(this);
    checkFilterEdit->setProxy(checksFilterProxyModel);
    checksFilterProxyModel->setSourceModel(m_checkListModel);
    checkListView->setModel(checksFilterProxyModel);

    connect(m_checkListModel, &CheckListModel::selectedChecksChanged,
            this, &CheckSelection::checksChanged);
}

CheckSelection::~CheckSelection() = default;

void CheckSelection::setCheckSet(const CheckSet* checkSet)
{
    m_checkListModel->setCheckSet(checkSet);
}


void CheckSelection::setChecks(const QString& checks)
{
    m_checkListModel->setSelectedChecks(checks.split(QLatin1Char(','), QString::SkipEmptyParts));
}

QString CheckSelection::checks() const
{
    return m_checkListModel->selectedChecks().join(QLatin1Char(','));
}

}
