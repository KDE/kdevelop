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

#include <kdebug.h>
#include <qtextstream.h>
#include "filebuffer.h"
#include <qmessagebox.h>
#include <qregexp.h>

/**
 * Destructor
 */
FileBuffer::~FileBuffer()
//=======================
{
  for ( FileBufferList::Iterator it = m_subBuffers.begin(); it != m_subBuffers.end(); ++it )
    delete *it;
  for ( ValuesIgnoreList::Iterator it = m_valuesIgnore.begin(); it != m_valuesIgnore.end(); ++it )
    delete *it;
  m_subBuffers.clear();
}

/**
 * Find next accurance of a substring in the buffer starting at a given caret position
 * Uses searchForVariable flag to determine if we are looking for a variable (like "TARGET")
 * or just a sign (like "=").
 * Returns: a Caret pointing to the position where the substring is found.
 *          nvlToMax ? (rowcount+1,0) : (-1,-1) if the substring is not found.
 */
Caret FileBuffer::findInBuffer(const QString &subString,const Caret& startPos, bool nvlToMax, bool searchForVariable)
//===========================================================================================
{
  // ATTENTION: This method is central for the class. Almost all other methods rely on it
  //            so be careful!
  unsigned int i=startPos.m_row;
  if (!m_buffer.count())
    if (nvlToMax)
      return Caret(m_buffer.count(),0);
    else
      return Caret(-1,-1);  QString line = m_buffer[i++];
  line = line.mid(startPos.m_idx,line.length()-startPos.m_idx);
  for (; i<=m_buffer.count(); ++i)
  {
    int idxSeek = line.find(subString);
    //qWarning("FILEBUFFER: substring %s in line %s idxSeek = %d", subString.ascii(), line.latin1(), idxSeek);
    if ((line.find(subString)!=-1)
        //adymo: do not match substrings if the next character is not a letter or number
        //this is supposed to fix handling of similar words like TARGET and TARGETDEPS
        && ( ! (searchForVariable && line[idxSeek+subString.length()].isLetterOrNumber()) ) )
    {

      //qWarning("FILEBUFFER: next char is %c, index %d", line[idxSeek+subString.length()].latin1(), idxSeek+subString.length());
      if (startPos.m_row == (int) i-1)
        // first line in search so start idx should be added to result idx
        idxSeek = idxSeek + startPos.m_idx;
      return Caret(i-1,idxSeek);
    }
    if (i<m_buffer.count())
      line = m_buffer[i];
  }
  if (nvlToMax)
    return Caret(m_buffer.count(),0);
  else
    return Caret(-1,-1);
}

/**
 * removeComments does as name implies. For the time being this rather drastic
 * cut into manual applied comments is inavoidable.
 */
void FileBuffer::removeComments()
//===============================
{
  for (uint i=0; i<m_buffer.count(); ++i)
  {
    QString tmp = m_buffer[i].simplifyWhiteSpace();
    if (tmp[0]=='#')
    {
      pop(i);
      --i;
    }
  }

}


/**
 * Pop a line from the buffer
 * Returns: the string being popped
 */
QString FileBuffer::pop(int row)
//==============================
{
  if ((unsigned int)row>=m_buffer.count())
    return NULL;
  QStringList::Iterator it=m_buffer.begin();
  for (int i=0; i<row; ++it,++i);
  QString ReturnStr = *it;
  m_buffer.remove(it);
  return ReturnStr;
}

/**
 * Split a scopestring (s1:s2:s3:...:sn) into scopename (s1) and scoperest (s2:s3:...:sn)
 */
void FileBuffer::splitScopeString(QString scopeString,QString &scopeName, QString &scopeRest)
//===========================================================================================
{
  scopeString = scopeString.simplifyWhiteSpace();
  scopeName="";
  scopeRest="";
  if (!scopeString.isEmpty())
  {
    int kolonPos = scopeString.find(':');
    if (kolonPos==-1)
      scopeName = scopeString;
    else
    {
      scopeName = scopeString.left(kolonPos).simplifyWhiteSpace();
      scopeRest = scopeString.right(scopeString.length()-kolonPos-1);
    }
  }
}

/**
 * Get the subBuffer representing a certain scope (s1:s2:s3:...:sn).
 * Returns the subbuffer
 */
FileBuffer* FileBuffer::getSubBuffer(QString scopeString)
//=======================================================
{
  QString nextScopeName,scopeStringRest;
  splitScopeString(scopeString,nextScopeName,scopeStringRest);
  if (nextScopeName.isEmpty())
    return this;
  int idx = findChildBuffer(nextScopeName);
  if (idx==-1)
    return NULL;
  return m_subBuffers[idx]->getSubBuffer(scopeStringRest);
}

/**
 * Set values for a variable. (variable = value_1 value_2 valu_3 ... value_n)
 */
void FileBuffer::setValues(const QString &variable,QStringList values,FileBuffer::ValueSetMode append, int valuesPerRow)
//======================================================================================================================
{
  QStringList temp;
  unsigned int i;
  QString line;
  ValuesIgnore* valIgnore = getValuesIgnore(variable);
  if (append == VSM_APPEND)
  {
    line = variable + " += ";
    values += valIgnore->values;
  }
  else if (append == VSM_RESET)
  {
    line = variable + " = ";
    values += valIgnore->values;
  }
  else if (append == VSM_EXCLUDE)
  {
    line = variable + " -= ";
    values += valIgnore->values_exclude;
  }
  QString spacing;
  spacing.fill(' ',line.length());

  for (i=0; i<values.count(); ++i)
  {
    line = line + values[i] + " ";
    if (!((i+1) % valuesPerRow))
    {
      if (i != values.count()-1)
        line = line + "\\";
      temp.append(line);
      line = spacing;
    }
  }

  if (i % valuesPerRow)
    temp.append( line );

  /* last to first */
  for ( int i = temp.count()-1; i >= 0; i-- )
    m_buffer.prepend( temp[i] );
}

/**
 * Get values for a variable.
 * Returns: values as a whitespace separated list. (value_1 value_2 ... value_n)
 */
/*
bool FileBuffer::getValues(const QString &variable,QStringList &plusList, QStringList &minusList)
//===============================================================================================
{
  Caret curPos(0,0);
  QString valueString="";
  bool finished = false;
  while (!finished)
  {
    Caret variablePos(findInBuffer(variable,curPos));
    if (variablePos==Caret(-1,-1))
    {
      finished=true;
      continue;
    }
    Caret eqSign = findInBuffer("=",variablePos);
    QString line=m_buffer[eqSign.m_row];
    if (line[eqSign.m_idx-1]=='-')
    {
    }
    if (line[eqSign.m_idx-1]!='+')
      valueString = "";
    else
      valueString = valueString + " ";
    int lineNum=eqSign.m_row;
    line = line.mid(eqSign.m_idx+1,line.length()-eqSign.m_idx);
    while (line!="")
    {
      if (line[line.length()-1]=='\\')
      {
        valueString = valueString + line.left(line.length()-1) + " ";
        lineNum++;
        line = m_buffer[lineNum];
        continue;
      }
      valueString = valueString + line;
      line = "";
    }
    curPos = Caret(lineNum+1,0);
  }
  return valueString.simplifyWhiteSpace();
}
*/

/**
 * Get values for a variable.
 * Returns: values as a whitespace separated list. (value_1 value_2 ... value_n)
 */
bool FileBuffer::getValues(const QString &variable, QStringList &plusList, QStringList &minusList)
//=======================================================================================================
{
  Caret curPos(0,0);
  QStringList plusValues;
  QStringList minusValues;
  QStringList curValues;
  QStringList curValuesIgnore;
  bool finished = false;
  while (!finished)
  {
    Caret variablePos(findInBuffer(variable,curPos,false,true));
    if (variablePos==Caret(-1,-1))
    {
      finished=true;
      continue;
    }
    Caret eqSign = findInBuffer("=",variablePos);
    if (eqSign.m_row != variablePos.m_row)
    {
      curPos = Caret(variablePos)+Caret(1,0);
      continue;
    }
    QRegExp functionRegExp("(\\$\\$system\\(|\\$\\$include\\().+[)]");
    QString line=m_buffer[eqSign.m_row];
    QChar effectOperator = line[eqSign.m_idx-1];
    int lineNum=eqSign.m_row;
    curValues.clear();
    curValuesIgnore.clear();
    line = line.mid(eqSign.m_idx+1,line.length()-eqSign.m_idx);
    filterOutIgnoreValues(line,curValuesIgnore);
    while (!line.isEmpty())
    {
      if( functionRegExp.search(line) != -1) // QMake function
      {
      	curValues += functionRegExp.cap(0);
      }
      else
      if (line[line.length()-1]=='\\')
      {
        line = line.left(line.length()-1).simplifyWhiteSpace();
        curValues += QStringList::split(" ",line);
        lineNum++;
        line = m_buffer[lineNum];
        filterOutIgnoreValues(line,curValuesIgnore);
        continue;
      }
      else
      	curValues += QStringList::split(" ",line);
      line = "";
    }
    if (QString("+-*~").find(effectOperator)==-1)
    {
      // operator = resets the variable values

      getValuesIgnore(variable)->values.clear();
      getValuesIgnore(variable)->values_exclude.clear();
      plusValues.clear();
      minusValues.clear();
    }
    if (effectOperator=='-')
    {
      // remove from plus list if in curvalues
      for (uint i=0; i<curValues.count(); ++i)
        plusValues.remove(curValues[i]);
      // remove from plus list if in curvaluesignore
      for (uint i=0; i<curValuesIgnore.count(); ++i)
        getValuesIgnore(variable)->values.remove(curValuesIgnore[i]);
      // add curvalues to minuslist
      getValuesIgnore(variable)->values_exclude += curValuesIgnore;
      minusValues += curValues;
    }
    else
    {
      // remove from minus list if in curvalues
      for (uint i=0; i<curValues.count(); ++i)
        minusValues.remove(curValues[i]);
      // remove from minus list if in curvaluesignore
      for (uint i=0; i<curValuesIgnore.count(); ++i)
        getValuesIgnore(variable)->values_exclude.remove(curValuesIgnore[i]);
      // add curvalues to pluslist
      getValuesIgnore(variable)->values += curValuesIgnore;
      plusValues += curValues;
    }
    curPos = Caret(lineNum+1,0);
  }
  plusList = plusValues;
  minusList = minusValues;
  m_customValueNames.remove(variable);
  return true;
}

void FileBuffer::getValueNames()
{
	QRegExp var("([_.\\d\\w]+)[\\s]*(=|-=|\\+=)");
	m_customValueNames.clear();
	QStringList::ConstIterator idx = m_buffer.begin();
	for( ; idx != m_buffer.end(); ++idx)
	{
		if( var.search(*idx) != -1 )
			m_customValueNames += var.cap(1);
	}
}

/**
 * Get all VariableSetModes for a variable.
 */
void FileBuffer::getVariableValueSetModes(const QString &variable,QPtrList<FileBuffer::ValueSetMode> &modes)
//=======================================================================================================
{
  Caret curPos(0,0);
  bool finished = false;

  for (int i=0; !finished; ++i)
  {
    Caret variablePos(findInBuffer(variable,curPos,false,true));
    if (variablePos==Caret(-1,-1))
    {
      finished=true;
      continue;
    }

    Caret eqSign = findInBuffer("=",variablePos);
    if (eqSign.m_row != variablePos.m_row)
    {
      curPos = Caret(variablePos)+Caret(1,0);
      continue;
    }

    QString line=m_buffer[eqSign.m_row];
    QChar effectOperator = line[eqSign.m_idx-1];
    int lineNum=eqSign.m_row;
    line = line.mid(eqSign.m_idx+1,line.length()-eqSign.m_idx);
    while (!line.isEmpty())
    {
      if (line[line.length()-1]=='\\')
      {
        line = line.left(line.length()-1).simplifyWhiteSpace();
        lineNum++;
        line = m_buffer[lineNum];
        continue;
      }
      line = "";
    }

    if (QString("+-*~").find(effectOperator)==-1)
    {
      modes.append(new ValueSetMode(VSM_RESET));
    }
    if (effectOperator=='-')
    {
      modes.append(new ValueSetMode(VSM_EXCLUDE));
    }
    else
    {
      modes.append(new ValueSetMode(VSM_APPEND));
    }
    curPos = Caret(lineNum+1,0);
  }
}

/**
 * Remove values for a variable.
 */
void FileBuffer::removeValues(const QString &variable)
//===============================================================================
{
  Caret curPos = Caret(0,0);
  bool finished = false;
  while (!finished)
  {
    Caret variablePos = findInBuffer(variable,curPos,false,true);
    if (variablePos==Caret(-1,-1))
    {
      finished = true;
      continue;
    }
    // Check if this is infact a variable decliration and not someone using the damn thing.
    Caret eqSign = findInBuffer("=",variablePos);
    if (eqSign.m_row != variablePos.m_row && eqSign > variablePos )
    {
      curPos = Caret(variablePos)+Caret(1,0);
      finished = false;
      continue;
    }

    QString line = pop(variablePos.m_row);
    while (line[line.length()-1]=='\\')
    {
      line = pop(variablePos.m_row);
      if (line.isNull())
        break;
    }
  }
}

/**
 * Loads a file into the buffer
 */
void FileBuffer::bufferFile(const QString &fileName)
//==================================================
{
  m_buffer.clear();
  QFile dataFile(fileName);
  if (dataFile.open(IO_ReadOnly))
  {
    QTextStream inStream( &dataFile );
    QString inLine;
    while ( !inStream.eof() )
    {
      inLine = inStream.readLine();
      inLine = inLine.simplifyWhiteSpace();
      m_buffer.append(inLine);
    }
  }
  dataFile.close();
  removeComments();
  getValueNames();
}

/**
 * Writes buffer to a file.
 */
void FileBuffer::saveBuffer(const QString &filename,const QString &qmakeHeader)
//==================================================
{
  QFile dataFile(filename);
  QStringList writeBuffer;
  writeBuffer.append(qmakeHeader);
  writeBuffer += getBufferTextInDepth();
  if (dataFile.open(IO_WriteOnly))
  {
    for (unsigned int i=0; i<writeBuffer.count(); ++i)
      if (!writeBuffer[i].simplifyWhiteSpace().isEmpty())
        dataFile.writeBlock((writeBuffer[i]+"\n").ascii(),(writeBuffer[i]+"\n").length());
  }
}

/**
 * Create a new scope triggering the creation of new subBuffers.
 * The method is recursive, thus the scope "n1:n2:n3" will create
 * the subbuffer n1, and n1.makeScope is called with "n2:n3" etc.

 */
void FileBuffer::makeScope(const QString &scopeString)
//====================================================
{
  FileBuffer *subBuffer;
  QString nextScopeName,scopeStringRest;
  splitScopeString(scopeString,nextScopeName,scopeStringRest);
  if (nextScopeName.isEmpty())
    return;
  // next scope in nested scopeString
  int idx = findChildBuffer(nextScopeName);

  if (idx==-1)
  {
    // scope did not exist (create it)
    subBuffer = new FileBuffer();
    subBuffer->setScopeName(nextScopeName);
    m_subBuffers.append(subBuffer);
  }
  else
    subBuffer = m_subBuffers[idx];
  subBuffer->makeScope(scopeStringRest);
}

QStringList FileBuffer::getBufferTextInDepth()
//========================================
{
  QStringList resBuffer = m_buffer;
  for (unsigned int i=0; i<m_subBuffers.count(); ++i)
  {
    resBuffer.append(m_subBuffers[i]->getScopeName() + "{");
    QStringList subBuffer = m_subBuffers[i]->getBufferTextInDepth();
    for (unsigned int j=0; j<subBuffer.count(); j++)
      subBuffer[j] = "  " + subBuffer[j];
    resBuffer += subBuffer;
    resBuffer.append("}");
  }
  return resBuffer;
}

void FileBuffer::dumpBuffer()
//===========================
{
  for ( unsigned int i=0; i<m_buffer.count(); ++i )
    printf("%s\n", m_buffer[i].latin1());
}

Caret FileBuffer::findScopeEnd(Caret pos)
//=======================================
{
  int scopeDepth=1;
  while (scopeDepth)
  {
    Caret nextScopeStart = findInBuffer("{",pos,true);
    Caret nextScopeEnd = findInBuffer("}",pos,true);
    if (nextScopeStart < nextScopeEnd)
    {
      ++scopeDepth;
      pos = nextScopeStart + Caret(0,1);
    }
    else
    {
      --scopeDepth;
      pos = nextScopeEnd + Caret(0,1);
    }
    if (nextScopeStart==nextScopeEnd)
      return Caret(-1,-1);
  }
  return pos - Caret(0,1);
}

bool FileBuffer::findNextScope(const Caret &startSearch, Caret& scopeStart, Caret& scopeEnd)
//====================================================================================
{
  scopeStart = findInBuffer("{",startSearch);
  if (scopeStart==Caret(-1,-1))
    return false;
  scopeEnd = findScopeEnd(scopeStart+Caret(0,1));
  if (scopeEnd==Caret(-1,-1))
    return false;
  return true;
}

QStringList FileBuffer::copyBlock(const Caret &blockStart, const Caret &blockEnd)
//===============================================================================
{
  QStringList result;
  QString tmp = m_buffer[blockStart.m_row];
  result.append(tmp.right(tmp.length()-blockStart.m_idx));
  for (int i=blockStart.m_row+1; i<blockEnd.m_row; ++i)
    result.append(m_buffer[i]);
  tmp = m_buffer[blockEnd.m_row];
  result.append(tmp.left(blockEnd.m_idx+1));
  return result;
}

QStringList FileBuffer::popBlock(const Caret &blockStart, const Caret &blockEnd)
//===============================================================================
{
  QStringList result = copyBlock(blockStart,blockEnd);
  int poprow;
  if (blockStart.m_idx==0)
  {
    pop(blockStart.m_row);
    poprow=blockStart.m_row;
  }
  else
  {
    m_buffer[blockStart.m_row] = m_buffer[blockStart.m_row].left(blockStart.m_idx);
    poprow = blockStart.m_row+1;
  }
  for (int i=0; i<blockEnd.m_row-blockStart.m_row-1; ++i)
    pop(poprow);
  QString tmp = m_buffer[poprow];
  if (blockEnd.m_idx >= (int) tmp.length()-1)
    pop(poprow);
  else
    m_buffer[poprow] = tmp.right(tmp.length()-blockEnd.m_idx-1);
  return result;
}

/**
 * Handling qmake scopes means handling two types of scopes
 * 1. bracket scopes s1:s2:...:sn{ . . . }
 * 2. non-bracketscopes s1:s2:...:sn:v1 = ... [\ ... [\ ...]]
 */
bool FileBuffer::handleScopes()
//=============================
{
  bool parseError = false;
  unsigned int i;
  Caret pos(0,0);
  // Bracket scopes
  while (true)
  {
    Caret startScope,endScope;
    if (!findNextScope(pos,startScope,endScope))
      break;
    pos = Caret(startScope.m_row,0);
    // is it a qmake valid scope ident | ident1:ident2:...:identn
    if (startScope.m_idx==0)
    {
      break;
      parseError = true;
    }

    QStringList subBuffer;
    QString tmp = m_buffer[startScope.m_row];
    QStringList scopeNames = QStringList::split(":",tmp.left(startScope.m_idx));
    // clean-up scopenames
    for (i=0;i<scopeNames.count();++i)
      scopeNames[i]=scopeNames[i].simplifyWhiteSpace();
    if (scopeNames.count()>1)
    {
      // nested scopename
      QString subBufferPrefix = scopeNames[1];
      for (i=2;i<scopeNames.count();++i)
        subBufferPrefix = subBufferPrefix + ":" + scopeNames[i];
      subBuffer = popBlock(startScope,endScope);
      subBuffer[0] = subBufferPrefix + subBuffer[0];
    }
    else
    {
      subBuffer = popBlock(startScope,endScope);
      // remove start and end brakets
      tmp = subBuffer[0];
      subBuffer[0] = tmp.right(tmp.length()-1);
      tmp = subBuffer[subBuffer.count()-1];
      subBuffer[subBuffer.count()-1] = tmp.left(tmp.length()-1);
    }

    // clear scopenesting
    pop(startScope.m_row);
    int subBufferIdx = findChildBuffer(scopeNames[0]);
    FileBuffer *subBufferObject;
    if (subBufferIdx==-1)
    {
      subBufferObject = new FileBuffer();
      m_subBuffers.append(subBufferObject);
    }
    else
      subBufferObject = m_subBuffers[subBufferIdx];
    subBufferObject->setScopeName(scopeNames[0]);
    subBufferObject->appendBufferText(subBuffer);
    subBufferObject->handleScopes();
  }
  //Non bracket scopes
  pos = Caret(0,0);
  while (true)
  {
    Caret kolonPos = findInBuffer(":",pos);
    if (kolonPos == Caret(-1,-1))
      break;
    QString scopeLine = pop(kolonPos.m_row);
    int equalPos = scopeLine.find('=',kolonPos.m_idx);
    if (equalPos == -1)
    {
      // Parse error
      parseError = true;
      break;
    }
    int idxScopeStringEnd = scopeLine.findRev(':',equalPos);
    QString scopeString = scopeLine.left(idxScopeStringEnd);
    scopeLine = scopeLine.right(scopeLine.length()-idxScopeStringEnd-1);
    scopeLine = scopeLine.simplifyWhiteSpace();
    makeScope(scopeString);
    FileBuffer *subBufferObject = getSubBuffer(scopeString);
    subBufferObject->appendBufferText(scopeLine);
    while (scopeLine[scopeLine.length()-1] == '\\')
    {
      scopeLine = pop(kolonPos.m_row);
      scopeLine = scopeLine.simplifyWhiteSpace();
      subBufferObject->appendBufferText(scopeLine);
    }
    pos = Caret(kolonPos.m_row,0);
  }
  return parseError;
}

int FileBuffer::findChildBuffer(const QString &scopeName)
//==============================================================
{
  for (unsigned int i=0; i<m_subBuffers.count(); ++i)
    if (m_subBuffers[i]->getScopeName()==scopeName)
      return i;
  return -1;
}

QStringList FileBuffer::getAllScopeStrings(int depth)
//==================================================
{
  QStringList result;
  unsigned int i;
  for (i=0; i<m_subBuffers.count(); ++i)
    result += m_subBuffers[i]->getAllScopeStrings(depth+1);
  if (depth)
  {
    for (i=0; i<result.count(); ++i)
      result[i] = getScopeName() + ":" + result[i];
    result.append(getScopeName());
  }
  return result;
}

QStringList FileBuffer::getAllScopeNames(int depth)
//================================================
{
  QStringList result;
  unsigned int i;
  for (i=0; i<m_subBuffers.count(); ++i)
    result += m_subBuffers[i]->getAllScopeNames(depth+1);
  if (!depth)
  {
    for (i=0; i<result.count(); ++i)
    {
      QString scopeName = result[0];
      result.remove(scopeName);
      result.append(scopeName);
    }
  }
  else
  {
    QString tmpScopeName = getScopeName();
    // remove negation
    if (tmpScopeName[0]=='!')
      tmpScopeName = tmpScopeName.right(tmpScopeName.length()-1);
    result.append(tmpScopeName);
  }
  return result;

}

QStringList FileBuffer::getChildScopeNames()
//========================================
{
  QStringList result;
  for (unsigned int i=0; i<m_subBuffers.count(); ++i)
    result += m_subBuffers[i]->getScopeName();
  return result;
}

bool FileBuffer::getAllExcludeValues(const QString &variable,QStringList &minusValues, int depth)
//==============================================================================================
{
  unsigned int i;
  QStringList plusDummy,minusTmp;
  for (i=0; i<m_subBuffers.count(); ++i)
    m_subBuffers[i]->getAllExcludeValues(variable,minusValues,depth+1);
  if (depth)
  {
    for (i=0; i<minusValues.count(); ++i)
      minusValues[i] = getScopeName() + ":" + minusValues[i];
  }
  getValues(variable,plusDummy,minusTmp);
  for (i=0; i<minusTmp.count(); ++i)
    minusTmp[i] = getScopeName() + "-" + minusTmp[i];
  minusValues += minusTmp;
  return true;
}


void FileBuffer::filterOutIgnoreValues(QString& line,QStringList& valuesignore)
//=====================================================================================================
{
  QStringList qmakeFunctions/* =
    QStringList::split(',',"join(,member(,find(,contains(,count(,error(,exists(,"
                       "include(,isEmpty(,system(,message(,infile(")*/;

  int len=0;
  int closestMatch = -1;
  for (uint i=0; i<qmakeFunctions.count(); ++i)
  {
    int match = line.find(qmakeFunctions[i],0);
    if (match==-1)
      continue;
    if(closestMatch==-1 ||
       closestMatch>match)
    {
      closestMatch = match;
      len=qmakeFunctions[i].length();
    }
  }
  int startpos = closestMatch;

  while (startpos>-1)
  {
    int bracketCount=1;
    while (bracketCount>0 && startpos+len<(int)line.length())
    {
      if (line[startpos+len]=='(')
        ++bracketCount;
      if (line[startpos+len]==')')
        --bracketCount;
      ++len;
    }

    valuesignore.append(line.mid(startpos,len));
    line = line.left(startpos)+line.right(line.length()-startpos-len);

    closestMatch=-1;
    for (uint i=0; i<qmakeFunctions.count(); ++i)
    {
      int match = line.find(qmakeFunctions[i],startpos);
      if (match==-1)
        continue;
      if(closestMatch==-1 ||
        closestMatch>match)
      {
        closestMatch = match;
        len=qmakeFunctions[i].length();
      }
    }
    startpos = closestMatch;
  }

}

ValuesIgnore* FileBuffer::getValuesIgnore(const QString &variable)
//================================================================
{
  ValuesIgnoreList::iterator it;
  for ( it = m_valuesIgnore.begin(); it != m_valuesIgnore.end(); ++it )
    if ((*it)->variable == variable)
      return (*it);
  ValuesIgnore* newVar = new ValuesIgnore;
  newVar->variable = variable;
  m_valuesIgnore.append(newVar);
  return newVar;
}

void FileBuffer::removeScope( const QString & scopeString, const QString &removeString, QStringList buffer )
{
  FileBuffer *subBuffer;
  QString nextScopeName,scopeStringRest;
  splitScopeString(scopeString,nextScopeName,scopeStringRest);
  if (nextScopeName.isEmpty())
    return;
  // next scope in nested scopeString
  int idx = findChildBuffer(nextScopeName);

  if (idx == -1)
    return;
  else
  {
    buffer.append(nextScopeName);
    subBuffer = m_subBuffers[idx];
    if (buffer.join(":") == removeString)
    {
        m_subBuffers.remove(subBuffer);
        delete subBuffer;
    }
    else
        subBuffer->removeScope(scopeStringRest, removeString, buffer);
  }
}
