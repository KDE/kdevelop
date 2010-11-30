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
#include <KAction>

class QModelIndex;

class KUrl;

namespace Ui
{
class ProjectManagerView;
}

class ProjectProxyModel;

namespace KDevelop
{
class ProjectBaseItem;
}

class ProjectManagerViewPlugin;

class ProjectManagerFilterAction : public KAction {
    Q_OBJECT

public:
    explicit ProjectManagerFilterAction( QObject* parent );

signals:
    void filterChanged(const QString& filter);

protected:
    virtual QWidget* createWidget( QWidget* parent );
};

class ProjectManagerView: public QWidget
{
    Q_OBJECT
public:
    ProjectManagerView( ProjectManagerViewPlugin*, QWidget *parent );
    virtual ~ProjectManagerView();

    ProjectManagerViewPlugin* plugin() const { return m_plugin; }
    QList<KDevelop::ProjectBaseItem*> selectedItems() const;
private slots:
    void selectionChanged();
    void locateCurrentDocument();
    void updateSyncAction();
    void openUrl( const KUrl& );
    void filterChanged(const QString&);

private:
    QAction* m_syncAction;
    Ui::ProjectManagerView* m_ui;
    QStringList m_cachedFileList;
    ProjectProxyModel* m_modelFilter;
    ProjectManagerViewPlugin* m_plugin;
};

#endif // KDEVPROJECTMANAGER_H

