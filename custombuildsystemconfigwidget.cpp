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

#include "custombuildsystemconfigwidget.h"

#include <KConfig>

#include "ui_custombuildsystemconfigwidget.h"
#include "configconstants.h"

CustomBuildSystemConfigWidget::CustomBuildSystemConfigWidget( QWidget* parent )
    : QWidget( parent ), ui( new Ui::CustomBuildSystemConfigWidget )
{
    ui->setupUi( this );
    connect( ui->currentConfig, SIGNAL(activated(int)), SLOT(changeCurrentConfig(int)));

    connect( ui->configWidget, SIGNAL(changed()), SIGNAL(changed()) );
}

void CustomBuildSystemConfigWidget::loadDefaults()
{
}

void CustomBuildSystemConfigWidget::loadFrom( KConfig* cfg )
{
    KConfigGroup grp = cfg->group( ConfigConstants::customBuildSystemGroup );
    foreach( const QString& grpName, grp.groupList() ) {
        KConfigGroup subgrp = grp.group( grpName );
        CustomBuildSystemConfig config;

        config.title = subgrp.readEntry( "Title", "" );
        config.grpName = grpName;
        config.buildDir = subgrp.readEntry( ConfigConstants::buildDirKey, "" );

        {
            QByteArray tmp = subgrp.readEntry( ConfigConstants::definesKey, QByteArray() );
            QDataStream s(tmp);
            s.setVersion( QDataStream::Qt_4_5 );
            s >> config.defines;
        }

        {
            QByteArray tmp = subgrp.readEntry( ConfigConstants::environmentKey, QByteArray() );
            QDataStream s(tmp);
            s.setVersion( QDataStream::Qt_4_5 );
            s >> config.environments;
        }

        {
            QByteArray tmp = subgrp.readEntry( ConfigConstants::includesKey, QByteArray() );
            QDataStream s(tmp);
            s.setVersion( QDataStream::Qt_4_5 );
            s >> config.includes;
        }

        foreach( const QString& toolgrpName, subgrp.groupList() ) {
            if( toolgrpName.startsWith( ConfigConstants::toolGroupPrefix ) ) {
                KConfigGroup toolgrp = subgrp.group( toolgrpName );
                CustomBuildSystemTool tool;
                tool.arguments = toolgrp.readEntry( ConfigConstants::toolArguments, "" );
                tool.executable = toolgrp.readEntry( ConfigConstants::toolExecutable, KUrl() );
                tool.envGrp = toolgrp.readEntry( ConfigConstants::toolEnvironment, "default" );
                tool.type = CustomBuildSystemTool::ActionType( toolgrp.readEntry( ConfigConstants::toolType, 0 ) );
                config.tools.insert( tool.type, tool );
            }
        }
        configs << config;
        ui->currentConfig->addItem( config.title, config.grpName );
    }
    changeCurrentConfig( ui->currentConfig->findData( grp.readEntry( ConfigConstants::currentConfigKey, "" ) ) );
}

void CustomBuildSystemConfigWidget::saveTo( KConfig* )
{
}

void CustomBuildSystemConfigWidget::changeCurrentConfig( int idx )
{
    if( idx == -1 ) {
        return;
    }
    Q_ASSERT( idx >= 0 && idx < configs.size() );
    CustomBuildSystemConfig cfg = configs.at( idx );
    ui->configWidget->loadConfig( cfg );
}

#include "custombuildsystemconfigwidget.moc"

