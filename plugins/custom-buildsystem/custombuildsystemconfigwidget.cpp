/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include "custombuildsystemconfigwidget.h"

#include <KConfig>
#include <KConfigGroup>

#include "ui_custombuildsystemconfigwidget.h"
#include "configconstants.h"

namespace
{

QString generateToolGroupName( CustomBuildSystemTool::ActionType type )
{
    static const int toolTypeCount = CustomBuildSystemTool::Undefined + 1;
    // Static Qt-based objects are discouraged (MSVC-incompatible), so use raw strings
    static const char* const toolTypes[toolTypeCount] = {
        "Build",
        "Configure",
        "Install",
        "Clean",
        "Prune",
        "Undefined"
    };

    Q_ASSERT( type >= 0 && type < toolTypeCount );
    Q_UNUSED( toolTypeCount );

    return ConfigConstants::toolGroupPrefix() + QLatin1String(toolTypes[type]);
}

}

CustomBuildSystemConfigWidget::CustomBuildSystemConfigWidget( QWidget* parent )
    : QWidget( parent ), ui( new Ui::CustomBuildSystemConfigWidget )
{
    ui->setupUi( this );

    // hack taken from kurlrequester, make the buttons a bit less in height so they better match the url-requester
    ui->addConfig->setFixedHeight( ui->currentConfig->sizeHint().height() );
    ui->removeConfig->setFixedHeight( ui->currentConfig->sizeHint().height() );

    connect( ui->currentConfig, QOverload<int>::of(&QComboBox::activated), this, &CustomBuildSystemConfigWidget::changeCurrentConfig);
    connect( ui->configWidget, &ConfigWidget::changed, this, &CustomBuildSystemConfigWidget::configChanged );

    connect( ui->addConfig, &QPushButton::clicked, this, &CustomBuildSystemConfigWidget::addConfig);
    connect( ui->removeConfig, &QPushButton::clicked, this, &CustomBuildSystemConfigWidget::removeConfig);
    connect( ui->currentConfig, &QComboBox::editTextChanged, this, &CustomBuildSystemConfigWidget::renameCurrentConfig );

    connect( this, &CustomBuildSystemConfigWidget::changed, this, &CustomBuildSystemConfigWidget::verify );
}

void CustomBuildSystemConfigWidget::loadDefaults()
{
}

void CustomBuildSystemConfigWidget::loadFrom( KConfig* cfg )
{
    ui->currentConfig->clear();
    configs.clear();
    QStringList groupNameList;
    KConfigGroup grp = cfg->group(ConfigConstants::customBuildSystemGroup());
    const auto groupList = grp.groupList();
    groupNameList.reserve(groupList.size());
    configs.reserve(groupList.size());
    for (const auto& grpName : groupList) {
        KConfigGroup subgrp = grp.group( grpName );
        CustomBuildSystemConfig config;

        config.title = subgrp.readEntry(ConfigConstants::configTitleKey(), QString());
        config.buildDir = subgrp.readEntry(ConfigConstants::buildDirKey(), QUrl());

        const auto subgrpNames = subgrp.groupList();
        for (const QString& subgrpName : subgrpNames) {
            if (subgrpName.startsWith(ConfigConstants::toolGroupPrefix())) {
                KConfigGroup toolgrp = subgrp.group( subgrpName );
                CustomBuildSystemTool tool;
                tool.arguments = toolgrp.readEntry(ConfigConstants::toolArguments(), QString());
                tool.executable = toolgrp.readEntry(ConfigConstants::toolExecutable(), QUrl());
                tool.envGrp = toolgrp.readEntry(ConfigConstants::toolEnvironment(), QString());
                tool.enabled = toolgrp.readEntry(ConfigConstants::toolEnabled(), false);
                tool.type = CustomBuildSystemTool::ActionType(toolgrp.readEntry(ConfigConstants::toolType(), 0));
                config.tools[tool.type] = tool;
            }
        }
        configs << config;
        ui->currentConfig->addItem( config.title );
        groupNameList << grpName;
    }
    int idx = groupNameList.indexOf(grp.readEntry(ConfigConstants::currentConfigKey(), QString()));
    if( !groupNameList.isEmpty() && idx < 0 )
        idx = 0;

    ui->currentConfig->setCurrentIndex( idx );
    changeCurrentConfig( idx );
}

void CustomBuildSystemConfigWidget::saveConfig( KConfigGroup& grp, const CustomBuildSystemConfig& c, int index )
{
    // Generate group name, access and clear it
    KConfigGroup subgrp = grp.group(ConfigConstants::buildConfigPrefix() + QString::number(index));
    subgrp.deleteGroup();

    // Write current configuration key, if our group is current
    if( ui->currentConfig->currentIndex() == index )
        grp.writeEntry(ConfigConstants::currentConfigKey(), subgrp.name());

    subgrp.writeEntry(ConfigConstants::configTitleKey(), c.title);
    subgrp.writeEntry<QUrl>(ConfigConstants::buildDirKey(), c.buildDir);
    for (const CustomBuildSystemTool& tool : c.tools) {
        KConfigGroup toolgrp = subgrp.group( generateToolGroupName( tool.type ) );
        toolgrp.writeEntry(ConfigConstants::toolType(), int(tool.type));
        toolgrp.writeEntry(ConfigConstants::toolEnvironment(), tool.envGrp);
        toolgrp.writeEntry(ConfigConstants::toolEnabled(), tool.enabled);
        toolgrp.writeEntry<QUrl>(ConfigConstants::toolExecutable(), tool.executable);
        toolgrp.writeEntry(ConfigConstants::toolArguments(), tool.arguments);
    }
}

void CustomBuildSystemConfigWidget::saveTo(KConfig* cfg)
{
    KConfigGroup subgrp = cfg->group(ConfigConstants::customBuildSystemGroup());
    subgrp.deleteGroup();
    for( int i = 0; i < ui->currentConfig->count(); i++ ) {
        configs[i].title = ui->currentConfig->itemText(i);
        saveConfig( subgrp, configs[i], i );
    }
    cfg->sync();
}

void CustomBuildSystemConfigWidget::configChanged()
{
    int idx = ui->currentConfig->currentIndex();
    if( idx >= 0 && idx < configs.count() ) {
        configs[idx] = ui->configWidget->config();
        emit changed();
    }
}

void CustomBuildSystemConfigWidget::changeCurrentConfig( int idx )
{
    if( idx < 0 || idx >= configs.size() ) {
        ui->configWidget->clear();
        emit changed();
        return;
    }
    CustomBuildSystemConfig cfg = configs.at( idx );
    ui->configWidget->loadConfig( cfg );
    emit changed();
}

void CustomBuildSystemConfigWidget::addConfig()
{
    CustomBuildSystemConfig c;
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

    // Make sure the new index is valid if possible, removing the first index
    // would otherwise set the index to -1 and that will cause havoc in changeCurrentConfig
    // since it clears the config if a negative index comes in. I'm not sure that is actually
    // correct, but well easier to fix and understand here right now and the combobox should
    // be dropped ultimately anyway I guess - or at least not be editable anymore.
    int newidx = curidx > 0 ? curidx - 1 : 0;
    ui->currentConfig->setCurrentIndex( newidx );
    changeCurrentConfig( ui->currentConfig->currentIndex() );
}

void CustomBuildSystemConfigWidget::verify() {
    Q_ASSERT( ui->currentConfig->count() == configs.count() );


    const bool hasAnyConfigurations = (configs.count() != 0);

    Q_ASSERT( !hasAnyConfigurations || (ui->currentConfig->currentIndex() >= 0) );

    ui->configWidget->setEnabled( hasAnyConfigurations );
    ui->removeConfig->setEnabled( hasAnyConfigurations );
    ui->currentConfig->setEditable( hasAnyConfigurations );
}

void CustomBuildSystemConfigWidget::renameCurrentConfig(const QString& name) {
    int idx = ui->currentConfig->currentIndex();
    if( idx >= 0 && idx < configs.count() ) {
        ui->currentConfig->setItemText( idx, name );
        emit changed();
    }
}

#include "moc_custombuildsystemconfigwidget.cpp"
