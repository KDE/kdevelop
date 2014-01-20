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

#ifndef KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H
#define KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H

#include <outputview/ioutputview.h>
#include <interfaces/iplugin.h>
#include <QtCore/QVariant>

template <typename T> class QList;
class QAbstractItemModel;
class QString;
class QModelIndex;
class QAbstractItemDelegate;
class OutputWidget;
class ToolViewData;

/**
@author Andreas Pakulat
*/

namespace Sublime
{
class View;
}

class StandardOutputView : public KDevelop::IPlugin, public KDevelop::IOutputView
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IOutputView )

public:
    explicit StandardOutputView(QObject *parent = 0, const QVariantList &args = QVariantList());
    virtual ~StandardOutputView();

    int standardToolView( KDevelop::IOutputView::StandardToolView view );
    int registerToolView( const QString& title,
                          KDevelop::IOutputView::ViewType type = KDevelop::IOutputView::OneView,
                          const QIcon& icon = QIcon(),
                          KDevelop::IOutputView::Options option = ShowItemsButton,
                          const QList<QAction*>& actionList = QList<QAction*>());

    int registerOutputInToolView( int toolviewId, const QString& title,
                                  KDevelop::IOutputView::Behaviours behaviour
                                    = KDevelop::IOutputView::AllowUserClose );

    void raiseOutput( int id );

    void setModel( int outputId, QAbstractItemModel* model );
    void setDelegate( int outputId, QAbstractItemDelegate* delegate );

    OutputWidget* outputWidgetForId( int outputId ) const;

    virtual void removeToolView( int toolviewId );
    virtual void removeOutput( int outputId );

    virtual void scrollOutputTo( int outputId, const QModelIndex& idx );

public Q_SLOTS:
    void removeSublimeView( Sublime::View* );

Q_SIGNALS:
    void activated( const QModelIndex& );
    void selectNextItem();
    void selectPrevItem();
    void outputRemoved( int toolviewId, int outputId );
    void toolViewRemoved( int toolviewId );

private:
    QMap<int, ToolViewData*> toolviews;
    QList<int> ids;
    QMap<KDevelop::IOutputView::StandardToolView,int> standardViews;
};

#endif // KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H

