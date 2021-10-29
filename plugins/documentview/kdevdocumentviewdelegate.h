/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEWDELEGATE_H
#define KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEWDELEGATE_H

#include <QItemDelegate>

class KDevDocumentViewDelegate: public QItemDelegate
{
    Q_OBJECT
public:
    explicit KDevDocumentViewDelegate( QObject *parent = nullptr );
    ~KDevDocumentViewDelegate() override;

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

#endif // KDEVPLATFORM_PLUGIN_KDEVCLASSVIEWDELEGATE_H
