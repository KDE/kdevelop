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

#ifndef PROJECTVCSPAGE_H
#define PROJECTVCSPAGE_H

#include <QWidget>
#include <QMap>

namespace Ui
{
class ProjectVcsPage;
}

namespace KDevelop
{
class IPluginController;
class VcsMapping;
class VcsImportMetadataWidget;
}

class KUrl;

class ProjectVcsPage : public QWidget
{
    Q_OBJECT
public:
    ProjectVcsPage( KDevelop::IPluginController*, QWidget* parent = 0 );
    ~ProjectVcsPage();
signals:
    void valid();
    void invalid();
public slots:
    void setSourceLocation( const KUrl& );
public:
    QString pluginName() const;
    KDevelop::VcsMapping mapping() const;
    QString commitMessage() const;
private:
    QMap<int, KDevelop::VcsImportMetadataWidget*> importWidgets;
    QMap<int, QPair<QString,QString> > vcsPlugins;
    Ui::ProjectVcsPage* m_ui;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
