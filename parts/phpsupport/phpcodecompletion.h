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
#include "keditor/codecompletion_iface.h"

class KDevCore;
class ClassStore;
/**
 *@author Sandy Meier
 */

class PHPCodeCompletion : public QObject {
  Q_OBJECT

public: 
  PHPCodeCompletion(KDevCore* core,ClassStore* store);
  ~PHPCodeCompletion();
protected slots:  
  void documentActivated(KEditor::Document* doc);
  void cursorPositionChanged(KEditor::Document *doc, int line, int col);

 protected:
  bool checkForGlobalFunction(KEditor::Document *doc,QString lineStr,int col);
  bool checkForClassMember(KEditor::Document *doc,QString lineStr,int col,int line);
  bool checkForNewInstance(KEditor::Document *doc,QString lineStr,int col,int line);
  QValueList<KEditor::CompletionEntry> getClassMethodsAndVariables(QString className);
  
 private:
  KEditor::Editor* m_editor;
  QValueList<KEditor::CompletionEntry> m_globalFunctions;
  KDevCore* m_core;
  ClassStore* m_classStore;
};

#endif
