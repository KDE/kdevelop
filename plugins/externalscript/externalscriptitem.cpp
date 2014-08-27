/*
    This plugin is part of KDevelop.

    Copyright (C) 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "externalscriptitem.h"

#include "externalscriptplugin.h"

#include <QAction>

#include <KParts/MainWindow>
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

ExternalScriptItem::ExternalScriptItem()
  : m_saveMode(SaveNone), m_outputMode(OutputNone)
  , m_errorMode(ErrorNone), m_inputMode(InputNone)
  , m_action(nullptr)
  , m_showOutput(true)
  , m_filterMode(0) // == no filter
  , m_performReplacements(true)
{

}

QString ExternalScriptItem::command() const
{
  return m_command;
}

void ExternalScriptItem::setCommand( const QString& command )
{
  m_command = command;
}

void ExternalScriptItem::setWorkingDirectory ( const QString& workingDirectory )
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

void ExternalScriptItem::setSaveMode( ExternalScriptItem::SaveMode mode )
{
  m_saveMode = mode;
}

ExternalScriptItem::OutputMode ExternalScriptItem::outputMode() const
{
  return m_outputMode;
}

void ExternalScriptItem::setOutputMode( ExternalScriptItem::OutputMode mode )
{
  m_outputMode = mode;
}

ExternalScriptItem::ErrorMode ExternalScriptItem::errorMode() const
{
  return m_errorMode;
}

void ExternalScriptItem::setErrorMode( ExternalScriptItem::ErrorMode mode )
{
  m_errorMode = mode;
}

ExternalScriptItem::InputMode ExternalScriptItem::inputMode() const
{
  return m_inputMode;
}

void ExternalScriptItem::setInputMode( ExternalScriptItem::InputMode mode )
{
  m_inputMode = mode;
}

int ExternalScriptItem::filterMode() const
{
  return m_filterMode;
}

void ExternalScriptItem::setFilterMode( int mode )
{
  m_filterMode = mode;
}

QAction* ExternalScriptItem::action()
{
  ///TODO: this is quite ugly, or is it? if someone knows how to do it better, please refactor
  if ( !m_action ) {
    static int actionCount = 0;
    m_action = new QAction( QString("executeScript%1").arg(actionCount), ExternalScriptPlugin::self() );
    m_action->setData( QVariant::fromValue<ExternalScriptItem*>(this) );
    ExternalScriptPlugin::self()->connect(
      m_action, SIGNAL(triggered()),
      ExternalScriptPlugin::self(), SLOT(executeScriptFromActionData())
    );
    m_action->setShortcut( QKeySequence() );
    // action needs to be added to a widget before it can work...
    KDevelop::ICore::self()->uiController()->activeMainWindow()->addAction(m_action);
  }

  Q_ASSERT( m_action );
  return m_action;
}

bool ExternalScriptItem::showOutput() const
{
  return m_showOutput;
}

void ExternalScriptItem::setShowOutput( bool show )
{
  m_showOutput = show;
}

bool ExternalScriptItem::performParameterReplacement() const
{
  return m_performReplacements;
}

void ExternalScriptItem::setPerformParameterReplacement ( bool perform )
{
  m_performReplacements = perform;
}

void ExternalScriptItem::save() const
{
  ExternalScriptPlugin::self()->saveItem( this );
}

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
