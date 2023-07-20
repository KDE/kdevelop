/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_OUTPUTDELEGATE_H
#define KDEVPLATFORM_OUTPUTDELEGATE_H

#include "outputviewexport.h"

#include <QItemDelegate>

namespace KDevelop
{
class OutputDelegatePrivate;

class KDEVPLATFORMOUTPUTVIEW_EXPORT OutputDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit OutputDelegate( QObject* parent = nullptr );
    ~OutputDelegate() override;
    void paint( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    const QScopedPointer<class OutputDelegatePrivate> d_ptr;
    Q_DECLARE_PRIVATE(OutputDelegate)
};

}
#endif

