#include "astyle_adaptor.h"

#include <string>

#include <kapplication.h>
#include <kconfig.h>



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
  else
    setTabIndentation();

  // indent
  setSwitchIndent(config->readBoolEntry("IndentSwitches", false));
  setClassIndent(config->readBoolEntry("IndentClasses", false));
  setCaseIndent(config->readBoolEntry("IndentCases", false));
  setBracketIndent(config->readBoolEntry("IndentBrackets", false));
  setNamespaceIndent(config->readBoolEntry("IndentNamespaces", true));
  setLabelIndent(config->readBoolEntry("IndentLabels", true));
    
  // continuation
  setMaxInStatementIndentLength(config->readNumEntry("MaxStatement", 40));
  if (config->readNumEntry("MinConditional", -1) != -1)
    setMinConditionalIndentLength(config->readNumEntry("MinConditional"));

  // brackets
  s = config->readEntry("Brackets", "Break");
  if (s == "Break")
    setBracketFormatMode(BREAK_MODE);
  if (s == "Attach")
    setBracketFormatMode(ATTACH_MODE);
  if (s == "Linux")
    setBracketFormatMode(BDAC_MODE);

  // padding
  setOperatorPaddingMode(config->readBoolEntry("PadOperators", false));
  setParenthesisPaddingMode(config->readBoolEntry("PadOperators", false));

  // oneliner
  setBreakOneLineBlocksMode(config->readBoolEntry("KeepBlocks", false)); 
  setSingleStatementsMode(config->readBoolEntry("KeepStatements", false));
}
