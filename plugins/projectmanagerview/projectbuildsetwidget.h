/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTBUILDSETWIDGET_H
#define KDEVPLATFORM_PLUGIN_PROJECTBUILDSETWIDGET_H

#include <QWidget>

class ProjectManagerViewPlugin;
class ProjectManagerView;

namespace KDevelop
{
class ProjectBaseItem;
}

namespace Ui
{
    class ProjectBuildSetWidget;
}

class ProjectBuildSetWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectBuildSetWidget( QWidget* parent = nullptr );
    ~ProjectBuildSetWidget() override;
    void setProjectView( ProjectManagerView* view );
public Q_SLOTS:
    void selectionChanged();
private Q_SLOTS:
    void addItems();
    void removeItems();
    void moveUp();
    void moveDown();
    void moveToBottom();
    void moveToTop();
    void showContextMenu( const QPoint& p );
private:
    ProjectManagerView* m_view = nullptr ;

    Ui::ProjectBuildSetWidget* m_ui;
};

#endif
