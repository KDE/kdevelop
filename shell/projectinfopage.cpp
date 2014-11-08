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

#include "core.h"
#include "plugincontroller.h"

namespace KDevelop
{

ProjectInfoPage::ProjectInfoPage( QWidget* parent )
        : QWidget( parent )
{
    page_ui = new Ui::ProjectInfoPage;
    page_ui->setupUi( this );
    connect( page_ui->nameEdit, &QLineEdit::textEdited, 
             this, &ProjectInfoPage::projectNameChanged );
    connect( page_ui->managerCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
             this, &ProjectInfoPage::changeProjectManager );
    foreach( const KPluginInfo& info, Core::self()->pluginControllerInternal()->queryExtensionPlugins( "org.kdevelop.IProjectFileManager" ) )
    {
        page_ui->managerCombo->addItem( QIcon::fromTheme(info.icon()), info.name(), info.pluginName() );
    }
}

ProjectInfoPage::~ProjectInfoPage()
{
    delete page_ui;
}

void ProjectInfoPage::changeProjectManager( int idx )
{
    emit projectManagerChanged( page_ui->managerCombo->itemData( idx ).toString() );
}

void ProjectInfoPage::setProjectName( const QString& name )
{
    page_ui->nameEdit->setText( name );
    emit projectNameChanged( page_ui->nameEdit->text() );
}

void ProjectInfoPage::setProjectManager( const QString& name )
{
    int idx = page_ui->managerCombo->findText( name );
    if( idx != -1 )
    {
        page_ui->managerCombo->setCurrentIndex( idx );
        changeProjectManager( idx );
    }
}

}

