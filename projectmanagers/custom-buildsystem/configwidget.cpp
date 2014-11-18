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

#include "configwidget.h"

#include <QToolButton>
#include <QLayout>

#include <KLocalizedString>
#include <KLineEdit>
#include <QAction>

#include "ui_configwidget.h"
#include <util/environmentgrouplist.h>
#include <interfaces/iproject.h>

using namespace KDevelop;

ConfigWidget::ConfigWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::ConfigWidget )
{
    ui->setupUi( this );

    ui->buildAction->insertItem( CustomBuildSystemTool::Build, i18n("Build"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Configure, i18n("Configure"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Install, i18n("Install"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Clean, i18n("Clean"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Prune, i18n("Prune"), QVariant() );

    connect( ui->buildAction, static_cast<void(KComboBox::*)(int)>(&KComboBox::activated), this, &ConfigWidget::changeAction );

    connect( ui->enableAction, &QCheckBox::toggled, this, &ConfigWidget::toggleActionEnablement );
    connect( ui->actionArguments, &QLineEdit::textEdited, this, &ConfigWidget::actionArgumentsEdited );
    connect( ui->actionEnvironment, &EnvironmentSelectionWidget::currentProfileChanged, this, &ConfigWidget::actionEnvironmentChanged );
    connect( ui->actionExecutable, &KUrlRequester::urlSelected, this, static_cast<void(ConfigWidget::*)(const QUrl&)>(&ConfigWidget::actionExecutableChanged) );
    connect( ui->actionExecutable->lineEdit(), &KLineEdit::textEdited, this, static_cast<void(ConfigWidget::*)(const QString&)>(&ConfigWidget::actionExecutableChanged) );
}

CustomBuildSystemConfig ConfigWidget::config() const
{
    CustomBuildSystemConfig c;
    c.buildDir = ui->buildDir->url();
    c.tools = m_tools;
    return c;
}

void ConfigWidget::loadConfig( CustomBuildSystemConfig cfg )
{
    bool b = blockSignals( true );
    clear();
    ui->buildDir->setUrl( cfg.buildDir );
    m_tools = cfg.tools;
    blockSignals( b );
    changeAction( ui->buildAction->currentIndex() );
    m_tools = cfg.tools;
}

void ConfigWidget::setTool(const CustomBuildSystemTool& tool)
{
    bool b = ui->enableAction->blockSignals( true );
    ui->enableAction->setChecked( tool.enabled );
    ui->enableAction->blockSignals( b );

    ui->actionArguments->setText( tool.arguments );
    ui->actionArguments->setEnabled( tool.enabled );
    ui->actionExecutable->setUrl( tool.executable );
    ui->actionExecutable->setEnabled( tool.enabled );
    ui->actionEnvironment->setCurrentProfile( tool.envGrp );
    ui->actionEnvironment->setEnabled( tool.enabled );
    ui->execLabel->setEnabled( tool.enabled );
    ui->argLabel->setEnabled( tool.enabled );
    ui->envLabel->setEnabled( tool.enabled );
}

void ConfigWidget::changeAction( int idx )
{
    if (idx < 0 || idx >= m_tools.count() ) {
        CustomBuildSystemTool emptyTool;
        emptyTool.type = CustomBuildSystemTool::Build;
        emptyTool.enabled = false;
        setTool(emptyTool);
    } else {
        CustomBuildSystemTool& selectedTool = m_tools[idx];
        setTool(selectedTool);
    }
}

void ConfigWidget::toggleActionEnablement( bool enable )
{
    m_tools[ ui->buildAction->currentIndex() ].enabled = enable;
    emit changed();
}

void ConfigWidget::actionArgumentsEdited( const QString& txt )
{
    m_tools[ ui->buildAction->currentIndex() ].arguments = txt;
    emit changed();
}

void ConfigWidget::actionEnvironmentChanged(const QString& profile)
{
    m_tools[ui->buildAction->currentIndex()].envGrp = profile;
    emit changed();
}

void ConfigWidget::actionExecutableChanged( const QUrl &url )
{
    m_tools[ ui->buildAction->currentIndex() ].executable = url;
    emit changed();
}

void ConfigWidget::actionExecutableChanged(const QString& txt )
{
    m_tools[ ui->buildAction->currentIndex() ].executable = QUrl::fromLocalFile(txt);
    emit changed();
}

void ConfigWidget::clear()
{
    ui->buildAction->setCurrentIndex( int( CustomBuildSystemTool::Build ) );
    changeAction( ui->buildAction->currentIndex() );
    ui->buildDir->setText("");
}


