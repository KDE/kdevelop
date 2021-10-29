/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include "configwidget.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <QAction>

#include "ui_configwidget.h"
#include <interfaces/iproject.h>

using namespace KDevelop;

ConfigWidget::ConfigWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::ConfigWidget )
{
    ui->setupUi( this );

    ui->buildDir->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    ui->buildAction->insertItem( CustomBuildSystemTool::Build, i18nc("@item:inlistbox build action", "Build"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Configure, i18nc("@item:inlistbox build action", "Configure"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Install, i18nc("@item:inlistbox build action", "Install"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Clean, i18nc("@item:inlistbox build action", "Clean"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Prune, i18nc("@item:inlistbox build action", "Prune"), QVariant() );

    connect( ui->buildAction, QOverload<int>::of(&KComboBox::activated), this, &ConfigWidget::changeAction );

    connect( ui->enableAction, &QCheckBox::toggled, this, &ConfigWidget::toggleActionEnablement );
    connect( ui->actionArguments, &QLineEdit::textEdited, this, &ConfigWidget::actionArgumentsEdited );
    connect( ui->actionEnvironment, &EnvironmentSelectionWidget::currentProfileChanged, this, &ConfigWidget::actionEnvironmentChanged );
    connect( ui->buildDir, &KUrlRequester::urlSelected, this, QOverload<>::of(&ConfigWidget::changed) );
    connect( ui->buildDir->lineEdit(), &KLineEdit::textEdited, this, QOverload<>::of(&ConfigWidget::changed) );
    connect( ui->actionExecutable, &KUrlRequester::urlSelected, this, QOverload<const QUrl&>::of(&ConfigWidget::actionExecutableChanged) );
    connect( ui->actionExecutable->lineEdit(), &KLineEdit::textEdited, this, QOverload<const QString&>::of(&ConfigWidget::actionExecutableChanged) );
}

CustomBuildSystemConfig ConfigWidget::config() const
{
    CustomBuildSystemConfig c;
    c.buildDir = ui->buildDir->url();
    c.tools = m_tools;
    return c;
}

void ConfigWidget::loadConfig(const CustomBuildSystemConfig& cfg)
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
        // create copy to avoid crash, see https://bugs.kde.org/show_bug.cgi?id=335470
        const CustomBuildSystemTool selectedTool = m_tools[idx];
        setTool(selectedTool);
    }
}

void ConfigWidget::toggleActionEnablement( bool enable )
{
    applyChange([=] (CustomBuildSystemTool* tool) {
        tool->enabled = enable;
    });
}

void ConfigWidget::actionArgumentsEdited( const QString& txt )
{
    applyChange([=] (CustomBuildSystemTool* tool) {
        tool->arguments = txt;
    });
}

void ConfigWidget::actionEnvironmentChanged(const QString& profile)
{
    applyChange([=] (CustomBuildSystemTool* tool) {
        tool->envGrp = profile;
    });
}

void ConfigWidget::actionExecutableChanged( const QUrl &url )
{
    applyChange([=] (CustomBuildSystemTool* tool) {
        tool->executable = url;
    });
}

void ConfigWidget::actionExecutableChanged(const QString& txt )
{
    applyChange([=] (CustomBuildSystemTool* tool) {
        tool->executable = QUrl::fromLocalFile(txt);
    });
}

void ConfigWidget::clear()
{
    ui->buildAction->setCurrentIndex( int( CustomBuildSystemTool::Build ) );
    changeAction( ui->buildAction->currentIndex() );
    ui->buildDir->setText({});
}

template<typename F>
void ConfigWidget::applyChange(F toolChanger)
{
    const auto idx = ui->buildAction->currentIndex();
    if (idx < 0 || idx >= m_tools.count()) {
        // happens for the empty tool
        return;
    }
    toolChanger(&m_tools[idx]);
    emit changed();
}
