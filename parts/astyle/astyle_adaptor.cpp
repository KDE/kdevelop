#include <string>


#include <kapp.h>
#include <kconfig.h>


#include "astyle_adaptor.h"


ASStringIterator::ASStringIterator(const QString &text)
  : ASSourceIterator(), _content(text)
{
  _is = new QTextStream(&_content, IO_ReadOnly);
}


ASStringIterator::~ASStringIterator()
{
  delete _is;
}


bool ASStringIterator::hasMoreLines() const
{
  return !_is->eof();
}


string ASStringIterator::nextLine()
{
  return _is->readLine().latin1();
}




KDevFormatter::KDevFormatter()
{
  KConfig *config = kapp->config();
  config->setGroup("AStyle");

  // style
  QString s = config->readEntry("Style");
  if (s == "ANSI") 
    {
      setBracketIndent(false);
      setSpaceIndentation(4);
      setBracketFormatMode(BREAK_MODE);
      setClassIndent(false);
      setSwitchIndent(false);
      setNamespaceIndent(false);
      return;
    }
  if (s == "KR")
    {        
      setBracketIndent(false);
      setSpaceIndentation(4);
      setBracketFormatMode(ATTACH_MODE);
      setClassIndent(false);
      setSwitchIndent(false);
      setNamespaceIndent(false);
      return;
    }
  if (s == "Linux")
    {
      setBracketIndent(false);
      setSpaceIndentation(8);
      setBracketFormatMode(BDAC_MODE);
      setClassIndent(false);
      setSwitchIndent(false);
      setNamespaceIndent(false);
      return;
    }
  if (s == "GNU")
    {       
      setBlockIndent(true);
      setSpaceIndentation(2);
      setBracketFormatMode(BREAK_MODE);
      setClassIndent(false);
      setSwitchIndent(false);
      setNamespaceIndent(false);
      return;
    }
  if (s == "JAVA")
    {
      setJavaStyle();
      setBracketIndent(false);
      setSpaceIndentation(4);
      setBracketFormatMode(ATTACH_MODE);
      setSwitchIndent(false);
      return;
    }

  // fill
  if (config->readEntry("Fill", "Tabs") != "Tabs")
    setSpaceIndentation(config->readNumEntry("FillSpaces",2));

  
}
