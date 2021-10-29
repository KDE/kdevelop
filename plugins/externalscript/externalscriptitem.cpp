/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "externalscriptitem.h"

#include "externalscriptplugin.h"

#include <QAction>

#include <KParts/MainWindow>
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

ExternalScriptItem::ExternalScriptItem()
{
}

QString ExternalScriptItem::key() const
{
    return m_key;
}

void ExternalScriptItem::setKey(const QString& key)
{
    m_key = key;
}

QString ExternalScriptItem::command() const
{
    return m_command;
}

void ExternalScriptItem::setCommand(const QString& command)
{
    m_command = command;
}

void ExternalScriptItem::setWorkingDirectory(const QString& workingDirectory)
{
    m_workingDirectory = workingDirectory;
}

QString ExternalScriptItem::workingDirectory() const
{
    return m_workingDirectory;
}

ExternalScriptItem::SaveMode ExternalScriptItem::saveMode() const
{
    return m_saveMode;
}

void ExternalScriptItem::setSaveMode(ExternalScriptItem::SaveMode mode)
{
    m_saveMode = mode;
}

ExternalScriptItem::OutputMode ExternalScriptItem::outputMode() const
{
    return m_outputMode;
}

void ExternalScriptItem::setOutputMode(ExternalScriptItem::OutputMode mode)
{
    m_outputMode = mode;
}

ExternalScriptItem::ErrorMode ExternalScriptItem::errorMode() const
{
    return m_errorMode;
}

void ExternalScriptItem::setErrorMode(ExternalScriptItem::ErrorMode mode)
{
    m_errorMode = mode;
}

ExternalScriptItem::InputMode ExternalScriptItem::inputMode() const
{
    return m_inputMode;
}

void ExternalScriptItem::setInputMode(ExternalScriptItem::InputMode mode)
{
    m_inputMode = mode;
}

int ExternalScriptItem::filterMode() const
{
    return m_filterMode;
}

void ExternalScriptItem::setFilterMode(int mode)
{
    m_filterMode = mode;
}

QAction* ExternalScriptItem::action()
{
    ///TODO: this is quite ugly, or is it? if someone knows how to do it better, please refactor
    if (!m_action) {
        static int actionCount = 0;
        m_action = new QAction(QStringLiteral("executeScript%1").arg(actionCount), ExternalScriptPlugin::self());
        m_action->setData(QVariant::fromValue<ExternalScriptItem*>(this));
        ExternalScriptPlugin::self()->connect(
            m_action, &QAction::triggered,
            ExternalScriptPlugin::self(), &ExternalScriptPlugin::executeScriptFromActionData
        );
        m_action->setShortcut(QKeySequence());
        // action needs to be added to a widget before it can work...
        KDevelop::ICore::self()->uiController()->activeMainWindow()->addAction(m_action);
    }

    Q_ASSERT(m_action);
    return m_action;
}

bool ExternalScriptItem::showOutput() const
{
    return m_showOutput;
}

void ExternalScriptItem::setShowOutput(bool show)
{
    m_showOutput = show;
}

bool ExternalScriptItem::performParameterReplacement() const
{
    return m_performReplacements;
}

void ExternalScriptItem::setPerformParameterReplacement(bool perform)
{
    m_performReplacements = perform;
}

void ExternalScriptItem::save() const
{
    ExternalScriptPlugin::self()->saveItem(this);
}
