/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVDOCUMENTSELECTION_H
#define KDEVPLATFORM_PLUGIN_KDEVDOCUMENTSELECTION_H

#include <QItemSelectionModel>

class KDevDocumentSelection: public QItemSelectionModel
{
    Q_OBJECT
public:
    explicit KDevDocumentSelection( QAbstractItemModel* model );
    ~KDevDocumentSelection() override;

public Q_SLOTS:
    void select( const QModelIndex & index,
                         QItemSelectionModel::SelectionFlags command ) override;
    void select( const QItemSelection & selection,
                         QItemSelectionModel::SelectionFlags command ) override;
};

#endif // KDEVPLATFORM_PLUGIN_KDEVDOCUMENTSELECTION_H

