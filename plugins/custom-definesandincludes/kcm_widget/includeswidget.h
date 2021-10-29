/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_INCLUDESWIDGET_H
#define KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_INCLUDESWIDGET_H

#include <QWidget>

namespace Ui
{
class IncludesWidget;
}

namespace KDevelop
{
    class IProject;
}
class QUrl;
class ProjectPathsModel;
class IncludesModel;

class IncludesWidget : public QWidget
{
Q_OBJECT
public:
    explicit IncludesWidget( QWidget* parent = nullptr );
    void setProject(KDevelop::IProject* w_project);
    void setIncludes( const QStringList& );
    void clear();
Q_SIGNALS:
    void includesChanged( const QStringList& );
private Q_SLOTS:
    // Handling of include-path url-requester, add and remove buttons
    void includePathSelected( const QModelIndex& selected );
    void includePathEdited();
    void includePathUrlSelected(const QUrl&);
    void addIncludePath();
    // Handles action and also Del-key in list
    void deleteIncludePath();
    void checkIfIncludePathExist();

    // Forward includes model changes
    void includesChanged();
private:
    Ui::IncludesWidget* ui;
    IncludesModel* includesModel;
    QString makeIncludeDirAbsolute( const QUrl &url ) const;
    // Enables/Disables widgets based on UI state/selection
    void updateEnablements();
    void updatePathsModel( const QVariant& newData, int role );
};

#endif
