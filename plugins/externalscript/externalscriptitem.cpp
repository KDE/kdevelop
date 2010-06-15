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

#include <KAction>
#include <KParts/MainWindow>
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

ExternalScriptItem::ExternalScriptItem()
  : m_saveMode(SaveNone), m_replaceMode(ReplaceNone), m_inputMode(InputNone), m_action( 0 )
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

ExternalScriptItem::SaveMode ExternalScriptItem::saveMode() const
{
  return m_saveMode;
}

void ExternalScriptItem::setSaveMode( ExternalScriptItem::SaveMode mode )
{
  m_saveMode = mode;
}

ExternalScriptItem::ReplaceMode ExternalScriptItem::replaceMode() const
{
  return m_replaceMode;
}

void ExternalScriptItem::setReplaceMode( ExternalScriptItem::ReplaceMode mode )
{
  m_replaceMode = mode;
}

ExternalScriptItem::InputMode ExternalScriptItem::inputMode() const
{
  return m_inputMode;
}

void ExternalScriptItem::setInputMode( ExternalScriptItem::InputMode mode )
{
  m_inputMode = mode;
}

KAction* ExternalScriptItem::action()
{
  ///TODO: this is quite ugly, or is it? if someone knows how to do it better, please refactor
  if ( !m_action ) {
    static int actionCount = 0;
    m_action = new KAction( QString("executeScript%1").arg(actionCount), ExternalScriptPlugin::self() );
    m_action->setData( QVariant::fromValue<ExternalScriptItem*>(this) );
    ExternalScriptPlugin::self()->connect(
      m_action, SIGNAL(triggered()),
      ExternalScriptPlugin::self(), SLOT(executeScriptFromActionData())
    );
    m_action->setShortcutConfigurable( true );
    ///TODO: load from settings
    m_action->setShortcut( KShortcut() );
    // action needs to be added to a widget before it can work...
    KDevelop::ICore::self()->uiController()->activeMainWindow()->addAction(m_action);
  }

  Q_ASSERT( m_action );
  return m_action;
}

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
