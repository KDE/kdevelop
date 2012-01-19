/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
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
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <language/backgroundparser/parseprojectjob.h>

CustomBuildSystemConfigWidget::CustomBuildSystemConfigWidget( QWidget* parent )
    : QWidget( parent ), ui( new Ui::CustomBuildSystemConfigWidget )
{
    ui->setupUi( this );

    connect( ui->currentConfig, SIGNAL(activated(int)), SLOT(changeCurrentConfig(int)));
    connect( ui->configWidget, SIGNAL(changed()), SLOT(configChanged()) );

    connect( ui->addConfig, SIGNAL(clicked(bool)), SLOT(addConfig()));
    connect( ui->removeConfig, SIGNAL(clicked(bool)), SLOT(removeConfig()));
}

void CustomBuildSystemConfigWidget::loadDefaults()
{
}

void CustomBuildSystemConfigWidget::loadFrom( KConfig* cfg )
{
    ui->configWidget->clear();
    ui->currentConfig->clear();
    KConfigGroup grp = cfg->group( ConfigConstants::customBuildSystemGroup );
    foreach( const QString& grpName, grp.groupList() ) {
        KConfigGroup subgrp = grp.group( grpName );
        CustomBuildSystemConfig config;

        config.title = subgrp.readEntry( "Title", "" );
        config.grpName = grpName;
        config.buildDir = subgrp.readEntry( ConfigConstants::buildDirKey, "" );

        foreach( const QString& subgrpName, subgrp.groupList() ) {
            if( subgrpName.startsWith( ConfigConstants::toolGroupPrefix ) ) {
                KConfigGroup toolgrp = subgrp.group( subgrpName );
                CustomBuildSystemTool tool;
                tool.arguments = toolgrp.readEntry( ConfigConstants::toolArguments, "" );
                tool.executable = toolgrp.readEntry( ConfigConstants::toolExecutable, KUrl() );
                tool.envGrp = toolgrp.readEntry( ConfigConstants::toolEnvironment, "default" );
                tool.enabled = toolgrp.readEntry( ConfigConstants::toolEnabled, false );
                tool.type = CustomBuildSystemTool::ActionType( toolgrp.readEntry( ConfigConstants::toolType, 0 ) );
                config.tools.insert( tool.type, tool );
            } else if( subgrpName.startsWith( ConfigConstants::projectPathPrefix ) ) {
                KConfigGroup pathgrp = subgrp.group( subgrpName );
                CustomBuildSystemProjectPathConfig path;
                path.path = pathgrp.readEntry( ConfigConstants::projectPathKey, "" );
                {
                    QByteArray tmp = pathgrp.readEntry( ConfigConstants::definesKey, QByteArray() );
                    QDataStream s(tmp);
                    s.setVersion( QDataStream::Qt_4_5 );
                    s >> path.defines;
                }

                {
                    QByteArray tmp = pathgrp.readEntry( ConfigConstants::includesKey, QByteArray() );
                    QDataStream s(tmp);
                    s.setVersion( QDataStream::Qt_4_5 );
                    s >> path.includes;
                }
                config.projectPaths << path;
            }
        }
        configs << config;
        ui->currentConfig->addItem( config.title, config.grpName );
    }
    int idx = ui->currentConfig->findData( grp.readEntry( ConfigConstants::currentConfigKey, "" ) );
    ui->currentConfig->setCurrentIndex( idx );
    changeCurrentConfig( idx );
}

void CustomBuildSystemConfigWidget::saveConfig( KConfigGroup& cfg, CustomBuildSystemConfig& c )
{
    // First generate a groupname if none exists yet
    if( c.grpName.isEmpty() ) {
        int maxnum = 0;
        foreach( const QString& grpname, cfg.groupList() ) {
            int grpnum = grpname.mid( QString("BuildConifg").length() ).toInt();
            if( grpnum >= maxnum ) {
                maxnum = grpnum + 1;
            }
        }
        c.grpName = QString("BuildConfig%1").arg( maxnum );
    }

    // then access the subgrp
    KConfigGroup subgrp = cfg.group( c.grpName );

    // Clear out all path-groups so we properly delete paths
    // that were removed in the gui.
    foreach( const QString& grpname, subgrp.groupList() ) {
        if( grpname.startsWith( ConfigConstants::projectPathPrefix ) ) {
            subgrp.deleteGroup( grpname );
        }
    }

    subgrp.writeEntry( ConfigConstants::configTitleKey, c.title );
    subgrp.writeEntry( ConfigConstants::buildDirKey, c.buildDir );
    foreach( const CustomBuildSystemTool& tool, c.tools.values() ) {
        KConfigGroup toolgrp;
        switch( tool.type ) {
            case CustomBuildSystemTool::Build: {
                toolgrp = subgrp.group( QString("%1Build").arg( ConfigConstants::toolGroupPrefix ) );
                break;
            }
            case CustomBuildSystemTool::Configure: {
                toolgrp = subgrp.group( QString("%1Configure").arg( ConfigConstants::toolGroupPrefix ) );
                break;
            }
            case CustomBuildSystemTool::Clean: {
                toolgrp = subgrp.group( QString("%1Clean").arg( ConfigConstants::toolGroupPrefix ) );
                break;
            }
            case CustomBuildSystemTool::Install: {
                toolgrp = subgrp.group( QString("%1Install").arg( ConfigConstants::toolGroupPrefix ) );
                break;
            }
            case CustomBuildSystemTool::Prune: {
                toolgrp = subgrp.group( QString("%1Prune").arg( ConfigConstants::toolGroupPrefix ) );
                break;
            }
            default:
                break;
        }
        toolgrp.writeEntry( ConfigConstants::toolType, int(tool.type) );
        toolgrp.writeEntry( ConfigConstants::toolEnvironment , tool.envGrp );
        toolgrp.writeEntry( ConfigConstants::toolEnabled, tool.enabled );
        toolgrp.writeEntry( ConfigConstants::toolExecutable, tool.executable );
        toolgrp.writeEntry( ConfigConstants::toolArguments, tool.arguments );
    }
    for( int i = 0; i < c.projectPaths.count(); i++ ) {
        KConfigGroup pathgrp = subgrp.group( QString("%1%2").arg( ConfigConstants::projectPathPrefix ).arg( i ) );
        CustomBuildSystemProjectPathConfig path = c.projectPaths.at( i );
        pathgrp.writeEntry( ConfigConstants::projectPathKey, path.path );
        {
            QByteArray tmp;
            QDataStream s(&tmp, QIODevice::WriteOnly);
            s.setVersion( QDataStream::Qt_4_5 );
            s << path.includes;
            pathgrp.writeEntry( ConfigConstants::includesKey, tmp );
        }
        {
            QByteArray tmp;
            QDataStream s(&tmp, QIODevice::WriteOnly);
            s.setVersion( QDataStream::Qt_4_5 );
            s << path.defines;
            pathgrp.writeEntry( ConfigConstants::definesKey, tmp );
        }
    }
}

void CustomBuildSystemConfigWidget::saveTo( KConfig* cfg, KDevelop::IProject* project )
{
    KConfigGroup subgrp = cfg->group( ConfigConstants::customBuildSystemGroup );
    for( int i = 0; i < ui->currentConfig->count(); i++ ) {
        CustomBuildSystemConfig c = configs.at( i );
        c.title = ui->currentConfig->itemText( i );
        saveConfig( subgrp, c );
        configs[i] = c;
        ui->currentConfig->setItemData( i, c.grpName );
    }
    subgrp.writeEntry( ConfigConstants::currentConfigKey, ui->currentConfig->itemData( ui->currentConfig->currentIndex() ) );
    cfg->sync();

    if ( KDevelop::IProjectController::parseAllProjectSources()) {
        KJob* parseProjectJob = new KDevelop::ParseProjectJob(project);
        KDevelop::ICore::self()->runController()->registerJob(parseProjectJob);
    }
}

void CustomBuildSystemConfigWidget::configChanged()
{
    int idx = ui->currentConfig->currentIndex();
    if( idx >= 0 && idx < configs.count() ) {
        CustomBuildSystemConfig c = configs[ idx ];
        CustomBuildSystemConfig updated = ui->configWidget->config();
        // Only update the stuff thats configurable inside the configwidget, leave the grp and title alone
        c.buildDir = updated.buildDir;
        c.projectPaths = updated.projectPaths;
        c.tools = updated.tools;
        configs[ idx ] = c;
        emit changed();
    }
}

void CustomBuildSystemConfigWidget::changeCurrentConfig( int idx )
{
    if( idx == -1 ) {
        ui->configWidget->setEnabled( false );
        ui->configWidget->clear();
        return;
    }
    Q_ASSERT( idx >= 0 && idx < configs.size() );
    ui->configWidget->setEnabled( true );
    CustomBuildSystemConfig cfg = configs.at( idx );
    ui->configWidget->loadConfig( cfg );
}

void CustomBuildSystemConfigWidget::addConfig()
{
    CustomBuildSystemConfig c;
    c.title = ui->currentConfig->currentText();
    configs.append( c );
    ui->currentConfig->addItem( c.title );
    ui->currentConfig->setCurrentIndex( ui->currentConfig->count() - 1 );
    changeCurrentConfig( ui->currentConfig->currentIndex() );
}

void CustomBuildSystemConfigWidget::removeConfig()
{
    int curidx = ui->currentConfig->currentIndex();
    Q_ASSERT( curidx < configs.length() && curidx >= 0 );
    configs.removeAt( curidx );
    ui->currentConfig->removeItem( curidx );

    ui->currentConfig->setCurrentIndex( curidx - 1 );
    changeCurrentConfig( ui->currentConfig->currentIndex() );
}


#include "custombuildsystemconfigwidget.moc"

