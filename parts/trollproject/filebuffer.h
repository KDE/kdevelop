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
  void          removeValues(QString variable);
  void          dumpBuffer();

private:
  QStringList   m_buffer;
};

#endif
