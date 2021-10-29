/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTINFOPAGE_H
#define KDEVPLATFORM_PROJECTINFOPAGE_H

#include <QWidget>

struct ProjectFileChoice {
    QString text;
    QString pluginId;
    QString iconName;
    QString fileName;
};

namespace Ui
{
    class ProjectInfoPage;
}

namespace KDevelop
{

class ProjectInfoPage : public QWidget
{
Q_OBJECT
public:
    explicit ProjectInfoPage( QWidget* parent = nullptr );
    ~ProjectInfoPage() override;
    void setProjectName( const QString& );
    void populateProjectFileCombo( const QVector<ProjectFileChoice>& choices );
Q_SIGNALS:
    void projectNameChanged( const QString& );
    void projectManagerChanged( const QString& pluginId, const QString & fileName);
private Q_SLOTS:
    void changeProjectManager( int );
private:
    Ui::ProjectInfoPage* page_ui;
};

}

Q_DECLARE_TYPEINFO(ProjectFileChoice, Q_MOVABLE_TYPE);

#endif
