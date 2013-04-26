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

#ifndef KDEVPLATFORM_PLUGIN_PROJECTVCSPAGE_H
#define KDEVPLATFORM_PLUGIN_PROJECTVCSPAGE_H

#include "appwizardpagewidget.h"
#include <QMap>
#include <vcs/interfaces/ibasicversioncontrol.h>

namespace Ui
{
class ProjectVcsPage;
}

namespace KDevelop
{
class IPluginController;
class VcsImportMetadataWidget;
}

class KUrl;

class ProjectVcsPage : public AppWizardPageWidget
{
    Q_OBJECT
public:
    ProjectVcsPage( KDevelop::IPluginController*, QWidget* parent = 0 );
    ~ProjectVcsPage();
    bool shouldContinue();
signals:
    void valid();
    void invalid();
public slots:
    void setSourceLocation( const KUrl& );
    void vcsTypeChanged(int);
    void validateData();
public:
    QString pluginName() const;
    KUrl source() const;
    KDevelop::VcsLocation destination() const;
    QString commitMessage() const;
private:
    QList<KDevelop::VcsImportMetadataWidget*> importWidgets;
    QList<QPair<QString,QString> > vcsPlugins;
    Ui::ProjectVcsPage* m_ui;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
