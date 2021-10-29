/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GREPOUTPUTDELEGATE_H
#define KDEVPLATFORM_PLUGIN_GREPOUTPUTDELEGATE_H

#include <QStyledItemDelegate>

class GrepOutputDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GrepOutputDelegate(QObject* parent);
    ~GrepOutputDelegate() override;

    static GrepOutputDelegate* self();
    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
private:
    static GrepOutputDelegate* m_self;
};

#endif

