/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
