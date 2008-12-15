/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "projectinfopage.h"
#include "ui_projectinfopage.h"

#include <kplugininfo.h>
#include <kdebug.h>

#include "core.h"
#include "plugincontroller.h"

namespace KDevelop
{

ProjectInfoPage::ProjectInfoPage( QWidget* parent )
        : QWidget( parent )
{
    page_ui = new Ui::ProjectInfoPage;
    page_ui->setupUi( this );
    connect( page_ui->nameEdit, SIGNAL( textEdited( const QString& ) ), 
             this, SIGNAL( projectNameChanged( const QString& ) ) );
    connect( page_ui->managerCombo, SIGNAL( activated( int ) ),
             this, SLOT( changeProjectManager( int ) ) );
    foreach( const KPluginInfo& info, Core::self()->pluginControllerInternal()->queryExtensionPlugins( "org.kdevelop.IProjectFileManager", QStringList() ) )
    {
        page_ui->managerCombo->addItem( info.name(), info.pluginName() );
    }
}

void ProjectInfoPage::changeProjectManager( int idx )
{
    emit projectManagerChanged( page_ui->managerCombo->itemData( idx ).toString() );
}

void ProjectInfoPage::setProjectDir( const KUrl& url )
{
    kDebug() << "setting project dir to:" << url;
    page_ui->nameEdit->setText( url.fileName() );
    emit projectManagerChanged( page_ui->managerCombo->itemData( page_ui->managerCombo->currentIndex() ).toString() );
    emit projectNameChanged( page_ui->nameEdit->text() );
}

}

#include "projectinfopage.moc"
