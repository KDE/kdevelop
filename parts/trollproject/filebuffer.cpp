#include <qtextstream.h>
#include "filebuffer.h"
#include <qmessagebox.h>


FileBuffer::~FileBuffer()
//=======================
{
  for ( FileBufferList::Iterator it = m_subBuffers.begin(); it != m_subBuffers.end(); ++it ) 
    delete *it;
  m_subBuffers.clear();
}

Caret FileBuffer::findInBuffer(const QString &subString,const Caret& startPos, bool nvlToMax)
//====================================================================================
{
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

void FileBuffer::setValues(const QString &variable,QStringList values,int valuesPerRow)
//==============================================================================
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

QString FileBuffer::getValues(const QString &variable)
//=============================================
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

void FileBuffer::removeValues(const QString &variable)
//====================================================
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

void FileBuffer::bufferFile(const QString &fileName)
//===========================================
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

void FileBuffer::saveBuffer(const QString &filename)
//===========================================
{
  QFile dataFile(filename);
  if (dataFile.open(IO_WriteOnly))
  {
    for (unsigned int i=0; i<m_buffer.count(); i++)
      dataFile.writeBlock((m_buffer[i]+"\n").ascii(),(m_buffer[i]+"\n").length());
  }
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

Caret FileBuffer::findNextScope(const Caret &startSearch, Caret& scopeStart, Caret& scopeEnd)
//====================================================================================
{
  scopeStart = findInBuffer("{",startSearch);
  if (scopeStart==Caret(-1,-1))
    return Caret(-1,-1);
  scopeEnd = findScopeEnd(scopeStart+Caret(0,1));
  if (scopeEnd==Caret(-1,-1))
    return scopeEnd;
  QString scopeIdentStr = m_buffer[scopeStart.m_row].left(scopeStart.m_idx-1);
  QStringList scopeIdentifiers = QStringList::split(":",scopeIdentStr);
  return scopeEnd+Caret(0,1);
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