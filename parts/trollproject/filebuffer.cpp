#include <qtextstream.h>
#include "filebuffer.h"
#include <qmessagebox.h>

Caret FileBuffer::findInBuffer(QString subString,const Caret& startPos)
//=====================================================================
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
  return Caret(-1,-1);
}

QString FileBuffer::pop(int row)
//===========================
{
  if ((unsigned int)row>=m_buffer.count())
    return NULL;
  QStringList::Iterator it=m_buffer.begin();
  for (int i=0; i<row; ++it,i++);
  QString ReturnStr = *it;
  m_buffer.remove(it);
  return ReturnStr;
}

void FileBuffer::setValues(QString variable,QStringList values,int valuesPerRow)
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

QString FileBuffer::getValues(QString variable)
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

void FileBuffer::removeValues(QString variable)
//=============================================
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

void FileBuffer::bufferFile(QString fileName)
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

void FileBuffer::saveBuffer(QString filename)
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
