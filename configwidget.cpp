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



ConfigWidget::ConfigWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::ConfigWidget ), lastConfiguredActionTool( -1 )
{
    ui->setupUi( this );
    ui->buildAction->insertItem( CustomBuildSystemTool::Build, i18n("Build"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Configure, i18n("Configure"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Install, i18n("Install"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Clean, i18n("Clean"), QVariant() );

    connect( ui->buildAction, SIGNAL(activated(int)), SLOT(changeAction(int)) );
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
    fillEnvironments( cfg.environments );
    fillIncludes( cfg.includes );
    fillDefines( cfg.defines );
    blockSignals( b );
}

void ConfigWidget::fillDefines(const QMap< QString, QHash< QString, QString > >& defines)
{
}

void ConfigWidget::fillEnvironments(const QMap< QString, QString >& envs)
{
}

void ConfigWidget::fillIncludes(const QMap< QString, QStringList >& includes)
{
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
    changeAction( CustomBuildSystemTool::Build );
}

void ConfigWidget::changeAction( int idx )
{
    Q_ASSERT( idx >= 0 && idx < ui->buildAction->count() );

    if( idx == lastConfiguredActionTool ) {
        return;
    }

    if( ui->enableAction->isChecked() ) {
        CustomBuildSystemTool t;
        t.arguments = ui->actionArguments->text();
        t.executable = ui->actionExecutable->url();
        t.envGrp = ui->actionEnvironment->currentText();
        ui->buildAction->setItemData( lastConfiguredActionTool, QVariant::fromValue<CustomBuildSystemTool>( t ) );
    } else {
        ui->buildAction->setItemData( lastConfiguredActionTool, QVariant() );
    }

    emit changed();

    QVariant data = ui->buildAction->itemData( idx );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        ui->enableAction->setChecked( true );
        ui->actionArguments->setText( t.arguments );
        ui->actionExecutable->setUrl( t.executable );
        ui->actionEnvironment->setCurrentIndex( ui->actionEnvironment->findText( t.envGrp ) );
    } else {
        ui->actionArguments->setText( "" );
        ui->actionEnvironment->setCurrentIndex( -1 );
        ui->actionExecutable->setUrl( KUrl() );
        ui->enableAction->setChecked( false );
    }

    lastConfiguredActionTool = idx;
}



#include "configwidget.moc"

