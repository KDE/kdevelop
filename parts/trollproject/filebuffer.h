#ifndef _FILEBUFFER_H_
#define _FILEBUFFER_H_

#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include "caret.h"

class FileBuffer;
typedef QValueList<FileBuffer*> FileBufferList;

class FileBuffer
{
public:
                  FileBuffer() {}
                  FileBuffer(const QString &fileName) {bufferFile(fileName);}
                  ~FileBuffer();

  void            SetScopeName(const QString &scopeName) {m_scopeName=scopeName;}
  QString         GetScopeName(const QString &scopeName) {m_scopeName=scopeName;}
  void            SetBuffer(const QStringList &buffer) {m_buffer=buffer;}

  Caret           findNextScope(const Caret &pos, Caret& scopeStart, Caret& scopeEnd);
  Caret           findScopeEnd(Caret pos);
  Caret           findInBuffer(const QString &subString,const Caret& startPos,bool nvlToMax=false);
  
  QStringList     popBlock(const Caret &blockStart, const Caret &blockEnd);
  QStringList     copyBlock(const Caret &blockStart, const Caret &blockEnd);

  void            removeValues(const QString &variable);
  QString         getValues(const QString &variable);
  void            setValues(const QString &variable,QString values,int valuesPerRow=3)
                  {setValues(variable,QStringList::split(' ',values),valuesPerRow);}
  void            setValues(const QString &variable,QStringList values,int valuesPerRow=3);

  void            bufferFile(const QString &fileName);
  void            saveBuffer(const QString &filename);
  void            dumpBuffer();

  QString         pop(int row);

private:
  QString         m_scopeName;
  QStringList     m_buffer;
  FileBufferList  m_subBuffers;
};

#endif
