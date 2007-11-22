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

#ifndef KDEVPROJECTMANAGERVIEW_H
#define KDEVPROJECTMANAGERVIEW_H

#include <QtGui/QWidget>

namespace KDevelop
{
class ProjectBaseItem;
}

class ProjectManagerViewPlugin;

class ProjectManagerView: public QWidget
{
    Q_OBJECT
public:
    ProjectManagerView( ProjectManagerViewPlugin *plugin, QWidget *parent );
    virtual ~ProjectManagerView();

    ProjectManagerViewPlugin *plugin() const;

    QList<KDevelop::ProjectBaseItem*> selectedItems() const;
private slots:
    void switchDetailView();
private:
    Q_PRIVATE_SLOT( d, void openUrl( const KUrl& ) )
    Q_PRIVATE_SLOT( d, void fileCreated( const QString &fileName ) )
    Q_PRIVATE_SLOT( d, void fileDirty( const QString &fileName ) )
    Q_PRIVATE_SLOT( d, void fileDeleted( const QString &fileName ) )
    class ProjectManagerPrivate* const d;
};

#endif // KDEVPROJECTMANAGER_H

