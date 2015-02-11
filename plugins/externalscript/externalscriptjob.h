/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTJOB_H
#define KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTJOB_H

#include <QtCore/QProcess>
#include <outputview/outputjob.h>

#include "externalscriptitem.h"

#include <KTextEditor/Range>
#include <QUrl>

namespace KDevelop
{
class ProcessLineMaker;
class OutputModel;
class Document;
}

namespace KTextEditor
{
class Document;
}

class KProcess;
class ExternalScriptPlugin;

class ExternalScriptJob : public KDevelop::OutputJob
{
  Q_OBJECT

public:
  ExternalScriptJob( ExternalScriptItem* item, const QUrl &url, ExternalScriptPlugin* parent );
  virtual void start() override;
  KDevelop::OutputModel* model();

protected:
  virtual bool doKill() override;

private slots:
  void processError( QProcess::ProcessError );
  void processFinished( int, QProcess::ExitStatus );

  void receivedStdoutLines(const QStringList& lines);
  void receivedStderrLines(const QStringList& lines);
private:
  void appendLine( const QString &l );

  KProcess* m_proc;
  KDevelop::ProcessLineMaker* m_lineMaker;
  ExternalScriptItem::OutputMode m_outputMode;
  ExternalScriptItem::InputMode m_inputMode;
  ExternalScriptItem::ErrorMode m_errorMode;
  int m_filterMode;
  KTextEditor::Document* m_document;
  QUrl m_url;
  /// invalid when whole doc should be replaced
  KTextEditor::Range m_selectionRange;
  KTextEditor::Cursor m_cursorPosition;
  bool m_showOutput;

  QStringList m_stdout;
  QStringList m_stderr;
};

#endif // KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTJOB_H

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on;
