/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_PROJECTPATHSWIDGET_H
#define KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_PROJECTPATHSWIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "../compilerprovider/icompiler.h"
#include "../compilerprovider/settingsmanager.h"

namespace Ui
{
class ProjectPathsWidget;
}

namespace KDevelop
{
    class IProject;
}

class ProjectPathsModel;

class ProjectPathsWidget : public QWidget
{
Q_OBJECT
public:
    explicit ProjectPathsWidget( QWidget* parent = nullptr );
    ~ProjectPathsWidget() override;

    void setProject(KDevelop::IProject* w_project);
    void setPaths( const QVector<ConfigEntry>& );
    QVector<ConfigEntry> paths() const;
    void clear();

Q_SIGNALS:
    void changed();

private:
    void setCurrentCompiler(const QString& name);

    CompilerPointer currentCompiler() const;

private Q_SLOTS:
    // Handling of project-path combobox, add and remove buttons
    void projectPathSelected( int index );
    void addProjectPath();
    void deleteProjectPath();
    void batchEdit();
    void tabChanged(int);
    void changeCompilerForPath();

    // Forward includes model changes into the pathsModel
    void includesChanged( const QStringList& includes );

    // Forward defines model changes into the pathsModel
    void definesChanged( const KDevelop::Defines& defines );

    void parserArgumentsChanged();

private:
    QScopedPointer<Ui::ProjectPathsWidget> ui;
    ProjectPathsModel* pathsModel;
    // Enables/Disables widgets based on UI state/selection
    void updateEnablements();
    void updatePathsModel( const QVariant& newData, int role );
};

#endif

