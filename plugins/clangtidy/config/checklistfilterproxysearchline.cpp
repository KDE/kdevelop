/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checklistfilterproxysearchline.h"

// KF
#include <KLocalizedString>
// Qt
#include <QTimer>
#include <QSortFilterProxyModel>

namespace ClangTidy
{

CheckListFilterProxySearchLine::CheckListFilterProxySearchLine(QWidget *parent)
    : QLineEdit(parent)
    , m_delayTimer(new QTimer(this))
{
    setClearButtonEnabled(true);
    setPlaceholderText(i18nc("@info:placeholder", "Search..."));

    m_delayTimer->setSingleShot(true);
    m_delayTimer->setInterval(300);
    connect(m_delayTimer, &QTimer::timeout,
            this, &CheckListFilterProxySearchLine::updateFilter);
    connect(this, &CheckListFilterProxySearchLine::textChanged,
            m_delayTimer, QOverload<>::of(&QTimer::start));
}


void CheckListFilterProxySearchLine::setFilterProxyModel(QSortFilterProxyModel* filterProxyModel)
{
    m_filterProxyModel = filterProxyModel;
}

void CheckListFilterProxySearchLine::updateFilter()
{
    if (!m_filterProxyModel) {
        return;
    }

    m_filterProxyModel->setFilterFixedString(text());
}

}
