/***************************************************************************
                          phpcodecompletion.h  -  description
                             -------------------
    begin                : Tue Jul 17 2001
    copyright            : (C) 2001 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PHPCODECOMPLETION_H
#define PHPCODECOMPLETION_H

#include <qobject.h>
#include <kregexp.h>
#include "keditor/editor.h"
#include "keditor/edit_iface.h"
#include "keditor/cursor_iface.h"
#include "keditor/codecompletion_iface.h"

class KDevCore;
class ClassStore;
/**
 *@author Sandy Meier
 */

class FunctionCompletionEntry : public KEditor::CompletionEntry {
 public:
  QString prototype;
};

class PHPCodeCompletion : public QObject {
  Q_OBJECT

public: 
  PHPCodeCompletion(KDevCore* core,ClassStore* store);
  ~PHPCodeCompletion();
protected slots:  
  void documentActivated(KEditor::Document* doc);
  void cursorPositionChanged(KEditor::Document *doc, int line, int col);
  void argHintHided();
  void completionBoxHided();

 protected:
  bool checkForVariable(KEditor::Document *doc,QString lineStr,int col,int line);
  bool checkForArgHint(KEditor::Document *doc,QString lineStr,int col,int line);
  bool checkForMethodArgHint(KEditor::Document *doc,QString lineStr,int col,int line);
  bool checkForGlobalFunction(KEditor::Document *doc,QString lineStr,int col);
  bool checkForNewInstance(KEditor::Document *doc,QString lineStr,int col,int line);
  QValueList<KEditor::CompletionEntry> getClassMethodsAndVariables(QString className);
  QString getClassName(QString varName,QString maybeInstanceOf);
  QString searchCurrentClassName();
  QString searchClassNameForVariable(QString varName);
 private:
  int m_currentLine;
  KEditor::Editor* m_editor;
  QValueList<FunctionCompletionEntry> m_globalFunctions;
  KDevCore* m_core;
  ClassStore* m_classStore;
  KEditor::CursorDocumentIface* m_cursorInterface;
  KEditor::EditDocumentIface* m_editInterface;
  KEditor::CodeCompletionDocumentIface* m_codeInterface;
  bool m_argWidgetShow;
  bool m_completionBoxShow;
};

#endif
