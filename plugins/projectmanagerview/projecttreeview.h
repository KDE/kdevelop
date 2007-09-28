/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPROJECTTREEVIEW_H
#define KDEVPROJECTTREEVIEW_H

#include <QtGui/QTreeView>


class KUrl;
class QItemSelectionModel;


class ProjectManagerViewPart;
namespace KDevelop
{
class ProjectModel;
class ProjectFolderItem;
class ProjectFileItem;
class ProjectTargetItem;
class ProjectBaseItem;
}

class ProjectTreeView: public QTreeView
{
        Q_OBJECT
    public:
        ProjectTreeView( ProjectManagerViewPart *part, QWidget *parent );
        virtual ~ProjectTreeView();

        ProjectManagerViewPart *part() const;
        KDevelop::ProjectModel *projectModel() const;

        KDevelop::ProjectFolderItem *currentFolderItem() const;
        KDevelop::ProjectFileItem *currentFileItem() const;
        KDevelop::ProjectTargetItem *currentTargetItem() const;

        void setSelectionModel( QItemSelectionModel* );

        virtual void reset();

    Q_SIGNALS:
        void activateURL( const KUrl &url );
        void currentChanged( KDevelop::ProjectBaseItem *item );

    protected Q_SLOTS:
        void slotActivated( const QModelIndex &index );
        void slotCurrentChanged( const QModelIndex &index );
        void popupContextMenu( const QPoint &pos );

    private:
        class ProjectTreeViewPrivate* const d;
};

#endif // KDEVPROJECTMANAGER_H

//kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
