/***************************************************************************
 *   Copyright (C) 2001 by Jakob Simon-Gaarde                              *
 *   jakob@jsg.dk                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtextstream.h>
#include "filebuffer.h"
#include <qmessagebox.h>


/**
 * Destructor
 */
FileBuffer::~FileBuffer()
//=======================
{
  for ( FileBufferList::Iterator it = m_subBuffers.begin(); it != m_subBuffers.end(); ++it ) 
    delete *it;
  m_subBuffers.clear();
}

/**
 * Find next accurance of a substring in the buffer starting at a given caret position
 * Returns: a Caret pointing to the position where the substring is found.
 *          nvlToMax ? (rowcount+1,0) : (-1,-1) if the substring is not found.
 */
Caret FileBuffer::findInBuffer(const QString &subString,const Caret& startPos, bool nvlToMax)
//===========================================================================================
{
  // ATTENTION: This method is central for the class. It almost all other methods rely on it
  //            so be careful!
  unsigned int i=startPos.m_row;
  QString line = m_buffer[i++];
  line = line.mid(startPos.m_idx,line.length()-startPos.m_idx);
  for (; i<=m_buffer.count(); i++)
  {
    int idxSeek = line.find(subString);
    if (line.find(subString)!=-1)
      return Caret(i-1,idxSeek);
    if (i<m_buffer.count())
      line = m_buffer[i];
  }
  if (nvlToMax)
    return Caret(m_buffer.count(),0);
  else
    return Caret(-1,-1);
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
  for (int i=0; i<row; ++it,i++);
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
  if (scopeString!="")
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
  if (nextScopeName=="")
    return this;
  int idx = findChildBuffer(nextScopeName);
  if (idx==-1)
    return NULL;
  return m_subBuffers[idx]->getSubBuffer(scopeStringRest);
}
  
/**
 * Set values for a variable. (variable = value_1 value_2 valu_3 ... value_n)
 */
void FileBuffer::setValues(const QString &variable,QStringList values,int valuesPerRow)
//=====================================================================================
{
  unsigned int i;
  QString line = variable + " = ";
  QString spacing;
  spacing.fill(' ',line.length());
  for (i=0; i<values.count(); i++)
  {
    line = line + values[i] + " ";
    if (!((i+1) % valuesPerRow))
    {
      if (i != values.count()-1)
        line = line + "\\";
      m_buffer.append(line);
      line = spacing;
    }
  }
  if (i % valuesPerRow)
    m_buffer.append(line);
}

/**
 * Get values for a variable.
 * Returns: values as a whitespace separated list. (value_1 value_2 ... value_n)
 */
QString FileBuffer::getValues(const QString &variable)
//====================================================
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
    if (line[eqSign.m_idx-1]!='+')
      valueString = "";
    else
      valueString = valueString + " ";
    long lineNum=eqSign.m_row;
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
    Caret variablePos = findInBuffer(variable,curPos);
    if (variablePos==Caret(-1,-1))
    {
      finished = true;
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
}

/**
 * Writes buffer to a file.
 */
void FileBuffer::saveBuffer(const QString &filename)
//==================================================
{
  QFile dataFile(filename);
  QStringList writeBuffer = getBufferTextInDepth();
  if (dataFile.open(IO_WriteOnly))
  {
    for (unsigned int i=0; i<writeBuffer.count(); i++)
      if (writeBuffer[i].simplifyWhiteSpace()!="")
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
  if (nextScopeName=="")
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
  for (unsigned int i=0; i<m_subBuffers.count(); i++)
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
  for ( unsigned int i=0; i<m_buffer.count(); i++ )
    printf(m_buffer[i]+"\n");
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
      scopeDepth++;
      pos = nextScopeStart + Caret(0,1);
    }
    else
    {
      scopeDepth--;
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
  for (int i=blockStart.m_row+1; i<blockEnd.m_row; i++)
    result.append(m_buffer[i]);
  tmp = m_buffer[blockEnd.m_row];
  result.append(tmp.left(blockEnd.m_idx+1));
  return result;
}

QStringList FileBuffer::popBlock(const Caret &blockStart, const Caret &blockEnd)
//===============================================================================
{
  QStringList result = copyBlock(blockStart,blockEnd);
  long poprow;
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
  for (int i=0; i<blockEnd.m_row-blockStart.m_row-1; i++)
    pop(poprow);
  QString tmp = m_buffer[poprow];
  if (blockEnd.m_idx >= tmp.length()-1)
    pop(poprow);
  else
    m_buffer[poprow] = tmp.right(tmp.length()-blockEnd.m_idx-1);
  return result;
}

bool FileBuffer::handleScopes()
//=============================
{
  unsigned int i;
  Caret pos(0,0);
  while (true)
  {
    Caret startScope,endScope;
    if (!findNextScope(pos,startScope,endScope))
      break;
    pos = Caret(startScope.m_row,0);
    // is it a qmake valid scope ident | ident1:ident2:...:identn
    if (startScope.m_idx==0)
      return false;

    QStringList subBuffer;
    QString tmp = m_buffer[startScope.m_row];
    QStringList scopeNames = QStringList::split(":",tmp.left(startScope.m_idx));
    // clean scopenames
    for (i=0;i<scopeNames.count();i++)
      scopeNames[i]=scopeNames[i].simplifyWhiteSpace();
    if (scopeNames.count()>1)
    {
      // nested scopename
      QString subBufferPrefix = scopeNames[1];
      for (i=2;i<scopeNames.count();i++)
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

}

int FileBuffer::findChildBuffer(const QString &scopeName)
//==============================================================
{
  for (int i=0; i<m_subBuffers.count(); i++)
    if (m_subBuffers[i]->getScopeName()==scopeName)
      return i;
  return -1;
}

QStringList FileBuffer::getAllScopeStrings(int depth)
//==================================================
{
  QStringList result;
  unsigned int i;
  for (i=0; i<m_subBuffers.count(); i++)
    result += m_subBuffers[i]->getAllScopeStrings(depth+1);
  if (depth)
  {
    for (i=0; i<result.count(); i++)
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
  for (i=0; i<m_subBuffers.count(); i++)
    result += m_subBuffers[i]->getAllScopeNames(depth+1);
  if (!depth)
  {
    for (i=0; i<result.count(); i++)
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
  for (unsigned int i=0; i<m_subBuffers.count(); i++)
    result += m_subBuffers[i]->getScopeName();
  return result;
}


