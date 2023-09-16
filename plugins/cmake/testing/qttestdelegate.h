/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QTTESTDELEGATE_H
#define QTTESTDELEGATE_H

#include <QItemDelegate>
#include <KColorScheme>
#include <KStatefulBrush>

class QtTestDelegate : public QItemDelegate
{
    Q_OBJECT
    
public:
    explicit QtTestDelegate(QObject* parent = nullptr);
    ~QtTestDelegate() override;
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    
private:
    void highlight(QStyleOptionViewItem& option, const KStatefulBrush& brush, bool bold = true) const;
    
    KStatefulBrush passBrush;
    KStatefulBrush failBrush;
    KStatefulBrush xFailBrush;
    KStatefulBrush xPassBrush;
    KStatefulBrush debugBrush;
};

#endif // QTTESTDELEGATE_H
