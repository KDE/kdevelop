/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QWidget>

#include "custombuildsystemconfig.h"
#include <qabstractitemmodel.h>

namespace Ui
{
class ConfigWidget;
}

namespace KDevelop
{
    class IProject;
}

class ProjectPathsModel;
class IncludesModel;
class DefinesModel;
class QItemSelection;

class ConfigWidget : public QWidget
{
Q_OBJECT
public:
    ConfigWidget( QWidget* parent = 0 );
    void setProject(KDevelop::IProject* w_project);
    void loadConfig( CustomBuildSystemConfig cfg );
    CustomBuildSystemConfig config() const;
    void clear();
signals:
    void changed();
private slots:
    void changeAction( int );
    void toggleActionEnablement( bool );
    void actionArgumentsEdited( const QString& );
    void actionEnvironmentChanged( int );
    void actionExecutableChanged( const KUrl& );
    void actionExecutableChanged( const QString& );
    void projectPathSelected(const QItemSelection&, const QItemSelection& );
    void includesChanged();
    void definesChanged();
    void deleteDefine();
    void deleteIncludePath();
    void deleteProjectPath();
private:
    Ui::ConfigWidget* ui;
    ProjectPathsModel* pathsModel;
    IncludesModel* includesModel;
    DefinesModel* definesModel;
    QVector<CustomBuildSystemTool> m_tools;
    void setTool( const CustomBuildSystemTool& tool );
};

#endif
