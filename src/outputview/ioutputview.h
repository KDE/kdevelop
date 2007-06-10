/* KDevelop Output View
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef KDEVIOUTPUTVIEW_H
#define KDEVIOUTPUTVIEW_H

#include "iextension.h"

class KUrl;
class QString;
template <typename T1, typename T2> class QMap;
class QAbstractItemModel;
class QModelIndex;

/**
@author Andreas Pakulat
*/
namespace KDevelop
{

class IOutputView
{
public:

    enum CloseBehaviour{
        AllowUserClose = 1,
        AlwaysShowView = 2
    };

    virtual ~IOutputView() {}

    /**
     * Register a new view with the given title
     * @param title the Title to be displayed on the view-switcher
     * @returns an id that identifies the new view and is used in the other
     *          methods
     */
    virtual QString registerView( const QString& title, CloseBehaviour = AllowUserClose ) = 0;

    /**
     * Sets the model of the registered view identified by id to model
     *
     * Does nothing if the id doesn't exist
     */
    virtual void setModel( const QString& id, QAbstractItemModel* model ) = 0;

Q_SIGNALS:
    void activated( const QModelIndex& );
    void viewRemoved( const QString& );
};
}
KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IOutputView, "org.kdevelop.IOutputView" )
Q_DECLARE_INTERFACE( KDevelop::IOutputView, "org.kdevelop.IOutputView" )

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

