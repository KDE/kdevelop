/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PROJECTINFOPAGE_H
#define KDEVPLATFORM_PROJECTINFOPAGE_H

#include <QtGui/QWidget>

namespace Ui
{
    class ProjectInfoPage;
}

class KUrl;

namespace KDevelop
{

class ProjectInfoPage : public QWidget
{
Q_OBJECT
public:
    ProjectInfoPage( QWidget* parent = 0 );
    virtual ~ProjectInfoPage();
    void setProjectName( const QString& );
    void setProjectManager( const QString& );
signals:
    void projectNameChanged( const QString& );
    void projectManagerChanged( const QString& );
private slots:
    void changeProjectManager( int );
private:
    Ui::ProjectInfoPage* page_ui;
};

}

#endif
