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

#include "projectvcspage.h"
#include "ui_projectvcspage.h"

#include <QStackedWidget>

#include <kcombobox.h>
#include <kplugininfo.h>

#include <iplugincontroller.h>
#include <iplugin.h>

#include <vcs/vcsmapping.h>
#include <vcs/vcsimportmetadatawidget.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

ProjectVcsPage::ProjectVcsPage( KDevelop::IPluginController* controller, QWidget * parent )
    : QWidget( parent ), m_ui( new Ui::ProjectVcsPage )
{
    m_ui->setupUi( this );
    QList<KDevelop::IPlugin*> vcsplugins = controller->allPluginsForExtension (
            "org.kdevelop.IBasicVersionControl", QStringList() );
    int idx = 1;
    m_ui->vcsImportOptions->insertWidget( 0, new QWidget(this) );
    m_ui->vcsTypes->insertItem( 0, i18nc("No Version Control Support chosen", "None") );
    foreach( KDevelop::IPlugin* plugin, vcsplugins )
    {
        KDevelop::IBasicVersionControl* iface = plugin->extension<KDevelop::IBasicVersionControl>();
        if( iface  )
        {
            KDevelop::VcsImportMetadataWidget* widget = iface->createImportMetadataWidget(
                                                    m_ui->vcsImportOptions );
            if( widget )
            {
                m_ui->vcsTypes->insertItem( idx, iface->name() );
                importWidgets.insert( idx, widget );
                vcsPlugins.insert( idx, qMakePair( controller->pluginInfo( plugin ).pluginName(), iface->name() ) );
                m_ui->vcsImportOptions->insertWidget( idx, widget );
                idx++;
            }
        }
    }
    connect( m_ui->vcsTypes, SIGNAL( activated(int) ),
             m_ui->vcsImportOptions, SLOT( setCurrentIndex(int) ) );
}

ProjectVcsPage::~ ProjectVcsPage( )
{
    delete m_ui;
}

void ProjectVcsPage::setSourceLocation( const KUrl& s )
{
    foreach( int idx, importWidgets.keys() )
    {
        importWidgets[idx]->setSourceLocation( KDevelop::VcsLocation( s ) );
    }
}

QString ProjectVcsPage::pluginName() const
{
    int idx = m_ui->vcsTypes->currentIndex();
    if( idx > 0)
        return vcsPlugins[idx].first;
    return "";
}


QString ProjectVcsPage::commitMessage() const
{
    int idx = m_ui->vcsTypes->currentIndex();
    if( idx )
        return importWidgets[idx]->message();
    return "";
}

KDevelop::VcsMapping ProjectVcsPage::mapping() const
{
    int idx = m_ui->vcsTypes->currentIndex();
    if( idx )
        return importWidgets[idx]->mapping();
    return KDevelop::VcsMapping();
}

