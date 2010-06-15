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

#include <KProcess>
#include <KShell>
#include <KLocalizedString>
#include <KMessageBox>
#include <QApplication>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <QFileInfo>

ExternalScriptItem::ExternalScriptItem()
  : m_saveMode(SaveNone), m_replaceMode(ReplaceNone), m_inputMode(InputNone)
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

void ExternalScriptItem::execute() const
{
  ///TODO: user feedback
  if ( m_command.isEmpty() ) {
    return;
  }

  KProcess cmd;

  KDevelop::IDocument* active = KDevelop::ICore::self()->documentController()->activeDocument();

  QString command = m_command;

  if ( active ) {
    const KUrl url = active->url();
    if ( url.isLocalFile() ) {
      cmd.setWorkingDirectory( active->url().directory() );
    }
    ///TODO: make those placeholders escapeable
    command.replace( "%u", url.pathOrUrl() );
    ///TODO: does that work with remote files?
    ///TODO: document the available placeholders
    QFileInfo info( url.pathOrUrl() );
    command.replace( "%f", info.filePath() );
    command.replace( "%b", info.baseName() );
    command.replace( "%n", info.fileName() );
    command.replace( "%d", info.path() );
  }


  KShell::Errors err = KShell::NoError;
  const QStringList args = KShell::splitArgs( m_command, KShell::TildeExpand | KShell::AbortOnMeta, &err );

  if ( err != KShell::NoError ) {
    QString errMsg;
    if( err == KShell::BadQuoting ) {
        errMsg = i18n( "There is a quoting error in the command of the external script '%1'.", text() );
    } else {
        errMsg = i18n("A shell meta character was included in the command of the external script '%1'. "
                      "which is currently not supported.", text() );
    }
    KMessageBox::error( QApplication::activeWindow(), errMsg, i18n("error exuting external script") );
    return;
  }
  
  Q_ASSERT( !args.isEmpty() );

//   cmd.execute(args.first(), args.mid(1));
//   TODO: execute async from plugin or something like that
}

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
