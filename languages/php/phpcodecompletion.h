/*
   Copyright (C) 2005 by Nicolas Escuder <n.escuder@intra-links.com>
   Copyright (C) 2001 by smeier@kdevelop.org

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef PHPCODECOMPLETION_H
#define PHPCODECOMPLETION_H

#include <codemodel.h>

#include <qobject.h>
#include <kregexp.h>
#include <kparts/part.h>

#include <kdevmainwindow.h>
#include <kstatusbar.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/selectioninterface.h>


class KDevCore;
class PHPSupportPart;
class PHPConfigData;

class FunctionCompletionEntry : public KTextEditor::CompletionEntry {
public:
   QString prototype;
};

class PHPCodeCompletion : public QObject {
   Q_OBJECT

public:
   PHPCodeCompletion(PHPSupportPart *phpSupport, PHPConfigData *config);
   ~PHPCodeCompletion();
   void setActiveEditorPart(KParts::Part *part);

public slots:
   void cursorPositionChanged();

protected slots:  
   void argHintHided();
   void completionBoxHided();

protected:
   bool showCompletionBox(QValueList<KTextEditor::CompletionEntry> list, unsigned long max);

   bool checkForVariable(QString line, int col);
   bool checkForStaticFunction(QString line, int col);
   bool checkForNew(QString line, int col);
   bool checkForExtends(QString line, int col);
   bool checkForGlobalFunction(QString line, int col);

   bool checkForArgHint(QString line, int col);

   QValueList<KTextEditor::CompletionEntry> getClasses(QString name);
   QValueList<KTextEditor::CompletionEntry> getFunctionsAndVars(QString classname, QString str);
   QStringList getArguments(QString classname, QString function);
   QString getCurrentClassName();
   QString getClassName(QString varName, QString classname);
   QValueList<ClassDom> getClassByName(QString classname);

   void readGlobalPHPFunctionsFile();
   void setStatusBar(QString expr, QString type);

 private:
   int m_currentLine;
   QValueList<FunctionCompletionEntry> m_globalFunctions;

   PHPSupportPart* m_phpSupport;
   PHPConfigData* m_config;
   CodeModel* m_model;

   bool m_argWidgetShow;
   bool m_completionBoxShow;

   KTextEditor::EditInterface *m_editInterface;
   KTextEditor::CodeCompletionInterface *m_codeInterface;
   KTextEditor::ViewCursorInterface *m_cursorInterface;
   KTextEditor::SelectionInterface *m_selectionInterface;

   QString findDeclaration(QString name, int line = -1);
};

#endif
