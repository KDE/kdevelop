/* This file is part of the KDE libraries
   Copyright (C) 2001 Harald Fernengel <harry@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KNEDIT_DOCUMENT_INCLUDE_
#define _KNEDIT_DOCUMENT_INCLUDE_

#include <qptrlist.h>
#include <kaction.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/undointerface.h>
#include <ktexteditor/cursorinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/blockselectioninterface.h>
#include <ktexteditor/searchinterface.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/wordwrapinterface.h>
#include <ktexteditor/printinterface.h>

class QProcess;
class QTimer;
class KWinModule;

namespace KNEdit 
{

class View;

/** This interface provides access to the NEdit Document class.
*/
class Document : public KTextEditor::Document, KTextEditor::HighlightingInterface
{
Q_OBJECT

public:
  Document(bool bReadOnly=false, bool bSingleView=false, QWidget *parentWidget = 0, const char *widgetName = 0, QObject *parent = 0, const char *name = 0);
  virtual ~Document();

  // KTextEditor stuff
  KTextEditor::View *createView( QWidget *parent, const char *name );
  QPtrList<KTextEditor::View> views() const { return _views; }

  // KParts::ReadWrite stuff
  bool openFile();
  bool saveFile();
 
  QString filename() const { return m_file; }
  QString serverName() const { return m_serverName; }
  bool isReady() const { return m_isReady; }
  void invokeNC( const QString& command, bool appendFilename = true );
  void addView(KTextEditor::View *view);
  void removeView(KTextEditor::View *view);

  // KTextEditor::HighlightingInterface stuff
  uint hlMode ();
  bool setHlMode (uint mode);
  uint hlModeCount ();
  QString hlModeName (uint mode);
  QString hlModeSectionName (uint mode);

public slots:
  void processExited();
  void launchFinished(WId);

signals:
  void hlChanged();

private slots:
  void checkForNEditServer();

private:
  void processCommands();
  void serverReady( WId wid );

private:
  QValueList<WId>             m_widCache;
  QPtrList<KTextEditor::View> _views;
  KTextEditor::View           *activeView;
  QProcess                    *proc;
  bool                        m_singleView, m_readOnly;
  bool                        m_isReady;
  QString                     m_serverName;
  KWinModule                  *m_kwm;
  WId                         m_wid;
  QStringList                 m_commands;
  QTimer                      *m_timer;
};

};

#endif

