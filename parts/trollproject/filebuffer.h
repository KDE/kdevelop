#ifndef _FILEBUFFER_H_
#define _FILEBUFFER_H_

#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include "caret.h"

class FileBuffer
{
public:
                FileBuffer() {}
                FileBuffer(QString fileName) {bufferFile(fileName);}
  Caret         findInBuffer(QString subString,const Caret& startPos);
  QString       getValues(QString variable);
  void          bufferFile(QString fileName);
  void          saveBuffer(QString filename);
  void          removeValues(QString variable);
  void          setValues(QString variable,QString values,int valuesPerRow=3)
                {setValues(variable,QStringList::split(' ',values),valuesPerRow);}
  void          setValues(QString variable,QStringList values,int valuesPerRow=3);
  void          dumpBuffer();
  QString       pop(int row);

private:
  QStringList   m_buffer;
};

#endif
