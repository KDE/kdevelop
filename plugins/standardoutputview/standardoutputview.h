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

template <typename T> class QList;
class QAbstractItemModel;
class KUrl;
class QString;

/**
@author Andreas Pakulat
*/

class StandardOutputView : public KDevelop::IPlugin, public KDevelop::IOutputView
{
Q_OBJECT
Q_INTERFACES( KDevelop::IOutputView )

public:
    explicit StandardOutputView(QObject *parent = 0, const QStringList &args = QStringList());
    virtual ~StandardOutputView();
    QString registerView( const QString& title,
                          KDevelop::IOutputView::CloseBehaviour behaviour = AllowUserClose );
    void setModel( const QString& id, QAbstractItemModel* );

    QAbstractItemModel* registeredModel( const QString& );
    QString registeredTitle( const QString& id );
    QStringList registeredViews();
    KDevelop::IOutputView::CloseBehaviour closeBehaviour( const QString& );

Q_SIGNALS:
    void activated( const QModelIndex& );
    void selectNextItem();
    void selectPrevItem();
    void modelChanged( const QString& );
    void viewRemoved( const QString& );

private:
    class StandardOutputViewPrivate* const d;
};

#endif // STANDARDOUTPUTVIEW_H

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
