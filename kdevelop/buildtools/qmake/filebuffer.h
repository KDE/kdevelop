/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILEBUFFER_H_
#define _FILEBUFFER_H_

#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qfile.h>
#include "caret.h"

class ValuesIgnore
{
public:
  QString           variable;
  QStringList       values;
  QStringList       values_exclude;
};

typedef QValueList<ValuesIgnore*> ValuesIgnoreList;

class FileBuffer;
typedef QValueList<FileBuffer*> FileBufferList;

class FileBuffer
{
public:
  enum ValueSetMode {VSM_RESET,VSM_APPEND,VSM_EXCLUDE};
  // Constructor/destructor
                    FileBuffer() {}
                    FileBuffer(const QString &fileName) {bufferFile(fileName);}
                    ~FileBuffer();

  // basic methods
  void              bufferFile(const QString &fileName);
  void              appendBufferText(const QStringList &buffer) {m_buffer+=buffer;}
  void              removeComments();
  Caret             findInBuffer(const QString &subString,const Caret& startPos,bool nvlToMax=false, bool searchForVariable = false);
  void              saveBuffer(const QString &filename,const QString &qmakeHeader="");
  void              dumpBuffer();
  QString           pop(int row);
  QStringList       popBlock(const Caret &blockStart, const Caret &blockEnd);
  QStringList       copyBlock(const Caret &blockStart, const Caret &blockEnd);

  // Scopes
  void              setScopeName(const QString &scopeName) {m_scopeName=scopeName;}
  QString           getScopeName() {return m_scopeName;}
  QStringList       getChildScopeNames();
  bool              findNextScope(const Caret &pos, Caret& scopeStart, Caret& scopeEnd);
  Caret             findScopeEnd(Caret pos);

  // Recursive scope methods
  bool              handleScopes();
  int               findChildBuffer(const QString &scopeName);
  void              makeScope(const QString &scopeString);
  void              removeScope(const QString &scopeString, const QString &removeString, QStringList buffer = QStringList());
  QStringList       getBufferTextInDepth();
  FileBuffer*       getSubBuffer(QString scopeString="");
  void              splitScopeString(QString scopeString,QString &scopeName, QString &scopeRest);
  QStringList       getAllScopeStrings(int depth=0);
  QStringList       getAllScopeNames(int depth=0);

  // Variable value handling
  void              removeValues(const QString &variable);
  bool              getValues(const QString &variable,QStringList &plusValues, QStringList &minusValues);
  void              setValues(const QString &variable,QString values,FileBuffer::ValueSetMode append=VSM_APPEND, int valuesPerRow=3)
                    {setValues(variable,QStringList::split(' ',values),append,valuesPerRow);}
  void              setValues(const QString &variable,QStringList values,FileBuffer::ValueSetMode append=VSM_APPEND, int valuesPerRow=3);
  bool              getAllExcludeValues(const QString &variable,QStringList &minusValues,int depth=0);
  void              getVariableValueSetModes(const QString &variable,QPtrList<FileBuffer::ValueSetMode> &modes);
  void              filterOutIgnoreValues(QString& line,QStringList& valuesignore);
  ValuesIgnore*     getValuesIgnore(const QString &variable);
  QStringList       getCustomValueNames() const { return m_customValueNames; }

private:
  void       getValueNames();
  QString           m_scopeName;
  QStringList       m_buffer;
  FileBufferList    m_subBuffers;
  ValuesIgnoreList  m_valuesIgnore;
  QStringList       m_customValueNames;

};

#endif

