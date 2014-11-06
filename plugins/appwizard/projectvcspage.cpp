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
#include <KI18n/KLocalizedString>
#include <kplugininfo.h>

#include <interfaces/iplugincontroller.h>
#include <interfaces/iplugin.h>

#include <vcs/widgets/vcsimportmetadatawidget.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

ProjectVcsPage::ProjectVcsPage( KDevelop::IPluginController* controller, QWidget * parent )
    : AppWizardPageWidget( parent ), m_ui( new Ui::ProjectVcsPage )
{
    m_ui->setupUi( this );
    QList<KDevelop::IPlugin*> vcsplugins = controller->allPluginsForExtension ( "org.kdevelop.IBasicVersionControl" );
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
                widget->setSourceLocationEditable( false );
                widget->setUseSourceDirForDestination( true );
                m_ui->vcsTypes->insertItem( idx, iface->name() );
                importWidgets.push_back( widget );
                vcsPlugins.push_back( qMakePair( controller->pluginInfo( plugin ).pluginName(), iface->name() ) );
                m_ui->vcsImportOptions->insertWidget( idx, widget );
                idx++;
            }
        }
    }
    connect( m_ui->vcsTypes, static_cast<void(KComboBox::*)(int)>(&KComboBox::activated),
             m_ui->vcsImportOptions, &QStackedWidget::setCurrentIndex );
    connect( m_ui->vcsTypes, static_cast<void(KComboBox::*)(int)>(&KComboBox::activated),
             this, &ProjectVcsPage::vcsTypeChanged );
    validateData();
}


void ProjectVcsPage::vcsTypeChanged( int idx )
{
    validateData();
    int widgetidx = idx - 1;
    disconnect( this, static_cast<void(ProjectVcsPage::*)()>(nullptr), this, &ProjectVcsPage::validateData );
    if ( widgetidx < 0 || widgetidx >= importWidgets.size())
        return;
    connect( importWidgets[widgetidx], &KDevelop::VcsImportMetadataWidget::changed, this, &ProjectVcsPage::validateData );
}

void ProjectVcsPage::validateData()
{
    if( shouldContinue() ) {
        emit valid();
    } else {
        emit invalid();
    }
}


ProjectVcsPage::~ProjectVcsPage( )
{
    delete m_ui;
}

void ProjectVcsPage::setSourceLocation( const QUrl& s )
{
    foreach(KDevelop::VcsImportMetadataWidget* widget, importWidgets)
    {
        widget->setSourceLocation( KDevelop::VcsLocation( s ) );
    }
}

QString ProjectVcsPage::pluginName() const
{
    int idx = m_ui->vcsTypes->currentIndex() - 1;
    if ( idx < 0 || idx >= vcsPlugins.size())
    return "";

    return vcsPlugins[idx].first;
}


QString ProjectVcsPage::commitMessage() const
{
    int idx = m_ui->vcsTypes->currentIndex() - 1;
    if ( idx < 0 || idx >= importWidgets.size())
        return QString();

        return importWidgets[idx]->message();
}

QUrl ProjectVcsPage::source() const
{
    int idx = m_ui->vcsTypes->currentIndex() - 1;
    if ( idx < 0 || idx >= importWidgets.size())
        return QUrl();

    return importWidgets[idx]->source();
}

KDevelop::VcsLocation ProjectVcsPage::destination() const
{
    int idx = m_ui->vcsTypes->currentIndex() - 1;
    if ( idx < 0 || idx >= importWidgets.size())
        return KDevelop::VcsLocation();

    return importWidgets[idx]->destination();
}


bool ProjectVcsPage::shouldContinue()
{
    int idx = m_ui->vcsTypes->currentIndex() - 1;
    if ( idx < 0 || idx >= importWidgets.size())
        return true;

    KDevelop::VcsImportMetadataWidget* widget = importWidgets[idx];
    
    return widget->hasValidData();
}

