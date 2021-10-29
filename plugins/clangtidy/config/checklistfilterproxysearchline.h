/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_CHECKLISTFILTERPROXYSEARCHLINE_H
#define CLANGTIDY_CHECKLISTFILTERPROXYSEARCHLINE_H

// Qt
#include <QLineEdit>

class QTimer;
class QSortFilterProxyModel;

namespace ClangTidy
{

class CheckListFilterProxySearchLine : public QLineEdit
{
    Q_OBJECT

public:
    explicit CheckListFilterProxySearchLine(QWidget* parent = nullptr);

public:
    void setFilterProxyModel(QSortFilterProxyModel* filterProxyModel);

private:
    void updateFilter();

private:
    QTimer* m_delayTimer;
    QSortFilterProxyModel* m_filterProxyModel = nullptr;
};

}

#endif
