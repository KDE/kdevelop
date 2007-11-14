/* KDevelop Standard OutputView
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
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

#ifndef STANDARDOUTPUTVIEW_H
#define STANDARDOUTPUTVIEW_H

#include "ioutputview.h"
#include "iplugin.h"
#include <QtCore/QVariant>

template <typename T> class QList;
class QAbstractItemModel;
class QString;
class QModelIndex;
class QAbstractItemDelegate;

/**
@author Andreas Pakulat
*/

class StandardOutputView : public KDevelop::IPlugin, public KDevelop::IOutputView
{
Q_OBJECT
Q_INTERFACES( KDevelop::IOutputView )

public:
    explicit StandardOutputView(QObject *parent = 0, const QVariantList &args = QVariantList());
    virtual ~StandardOutputView();
    int registerView( const QString& title,
                          KDevelop::IOutputView::CloseBehaviour behaviour = AllowUserClose );
    void setModel( int id, QAbstractItemModel* );

    void setDelegate( int id, QAbstractItemDelegate* );

    QAbstractItemModel* registeredModel( int ) const;
    QAbstractItemDelegate* registeredDelegate( int ) const;
    QString registeredTitle( int id ) const;
    QList<int> registeredViews() const;
    KDevelop::IOutputView::CloseBehaviour closeBehaviour( int id ) const;
public Q_SLOTS:
    void removeViewData( int );
Q_SIGNALS:
    void activated( const QModelIndex& );
    void selectNextItem();
    void selectPrevItem();
    void modelChanged( int id );
    void delegateChanged( int id );
    void viewRemoved( int id );
    /**
     * Signal to inform the view to remove one output view
     */
    void removeView( int id );

private:
    class StandardOutputViewPrivate* const d;
    friend class StandardOutputViewViewFactory;
};

#endif // STANDARDOUTPUTVIEW_H

