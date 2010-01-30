/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "configwidget.h"

#include "ui_configwidget.h"
#include "projectpathsmodel.h"


ConfigWidget::ConfigWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::ConfigWidget ), pathsModel( new ProjectPathsModel( this ) )
{
    ui->setupUi( this );
    ui->buildAction->insertItem( CustomBuildSystemTool::Build, i18n("Build"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Configure, i18n("Configure"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Install, i18n("Install"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Clean, i18n("Clean"), QVariant() );

    connect( ui->buildAction, SIGNAL(activated(int)), SLOT(changeAction(int)) );

    connect( ui->enableAction, SIGNAL(toggled(bool)), SLOT(toggleActionEnablement(bool)) );
    connect( ui->actionArguments, SIGNAL(textEdited(QString)), SLOT(actionArgumentsEdited(QString)) );
    connect( ui->actionEnvironment, SIGNAL(activated(int)), SLOT(actionEnvironmentChanged(int)) );
    connect( ui->actionExecutable, SIGNAL(urlSelected(KUrl)), SLOT(actionExecutableChanged(KUrl)) );
    connect( ui->actionExecutable, SIGNAL(textChanged(QString)), SLOT(actionExecutableChanged(QString)) );

    ui->projectPaths->setModel( pathsModel );
}

CustomBuildSystemConfig ConfigWidget::config() const
{
    return m_config;
}

void ConfigWidget::loadConfig( CustomBuildSystemConfig cfg )
{
    bool b = blockSignals( true );
    ui->buildDir->setUrl( cfg.buildDir );
    fillTools( cfg.tools );
    pathsModel->setPaths( cfg.projectPaths );
    blockSignals( b );
}

void ConfigWidget::fillTools(const QHash< CustomBuildSystemTool::ActionType, CustomBuildSystemTool>& tools)
{
    for( int i = 0; i < ui->buildAction->count(); i++ ) {
        QHash< CustomBuildSystemTool::ActionType, CustomBuildSystemTool>::const_iterator it = tools.find( CustomBuildSystemTool::ActionType( i ) );
        if( it != tools.end() ) {
            ui->buildAction->setItemData( i, QVariant::fromValue<CustomBuildSystemTool>( *it ) );
        } else {
            ui->buildAction->setItemData( i, QVariant() );
        }
    }
    ui->buildAction->setCurrentIndex( CustomBuildSystemTool::Build );
}



void ConfigWidget::changeAction( int idx )
{
    Q_ASSERT( idx >= 0 && idx < ui->buildAction->count() );

    QVariant data = ui->buildAction->itemData( idx );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        bool b = ui->enableAction->blockSignals( true );
        ui->enableAction->setChecked( true );
        ui->enableAction->blockSignals( b );
        ui->actionArguments->setText( t.arguments );
        ui->actionExecutable->setUrl( t.executable );
        ui->actionEnvironment->setCurrentIndex( ui->actionEnvironment->findText( t.envGrp ) );
    } else {
        ui->actionArguments->setText( "" );
        ui->actionEnvironment->setCurrentIndex( -1 );
        ui->actionExecutable->setUrl( KUrl() );
        bool b = ui->enableAction->blockSignals( true );
        ui->enableAction->setChecked( false );
        ui->enableAction->blockSignals( b );
    }
}

void ConfigWidget::toggleActionEnablement( bool enable )
{
    if( enable ) {
        CustomBuildSystemTool t;
        t.arguments = ui->actionArguments->text();
        t.envGrp = ui->actionEnvironment->currentProfile();
        t.executable = ui->actionExecutable->url();
        t.type = CustomBuildSystemTool::ActionType( ui->buildAction->currentIndex() );
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
    } else {
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant() );
    }
    emit changed();
}

void ConfigWidget::actionArgumentsEdited( const QString& txt )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.arguments = txt;
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
    }
    emit changed();
}

void ConfigWidget::actionEnvironmentChanged( int )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.envGrp = ui->actionEnvironment->currentProfile();
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
    }
    emit changed();
}

void ConfigWidget::actionExecutableChanged(const KUrl& url )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.executable = url;
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
    }
    emit changed();
}

void ConfigWidget::actionExecutableChanged(const QString& txt )
{
    actionExecutableChanged( KUrl( txt ) );
}

#include "configwidget.moc"

