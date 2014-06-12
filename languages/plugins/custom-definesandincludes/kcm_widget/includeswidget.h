/************************************************************************
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

#ifndef KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_INCLUDESWIDGET_H
#define KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_INCLUDESWIDGET_H

#include <QWidget>

#include <qabstractitemmodel.h>

class KFileDialog;
class KUrlRequester;
namespace Ui
{
class IncludesWidget;
}

namespace KDevelop
{
    class IProject;
}
class KUrl;
class ProjectPathsModel;
class IncludesModel;
class QItemSelection;

class IncludesWidget : public QWidget
{
Q_OBJECT
public:
    IncludesWidget( QWidget* parent = 0 );
    void setProject(KDevelop::IProject* w_project);
    void setIncludes( const QStringList& );
    void clear();
signals:
    void includesChanged( const QStringList& );
private slots:
    // Handling of include-path url-requester, add and remove buttons
    void includePathSelected( const QModelIndex& selected );
    void includePathEdited();
    void includePathUrlSelected(const KUrl&);
    void addIncludePath();
    // Handles action and also Del-key in list
    void deleteIncludePath();
    void checkIfIncludePathExist();

    // Forward includes model changes
    void includesChanged();
private:
    Ui::IncludesWidget* ui;
    IncludesModel* includesModel;
    QString makeIncludeDirAbsolute( const KUrl& url ) const;
    // Enables/Disables widgets based on UI state/selection
    void updateEnablements();
    void updatePathsModel( const QVariant& newData, int role );
};

#endif
