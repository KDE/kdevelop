#include "astyle_adaptor.h"

#include "astyle_widget.h"

#include <string>

#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>

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
  return _is->readLine().utf8().data();
}




KDevFormatter::KDevFormatter()
{
	KConfig *config = kapp->config();
	config->setGroup("AStyle");
	
	// style
	QString s = config->readEntry("Style");
	
	if ( predefinedStyle( s ) )
	{
		return;
	}

  // fill
  if (config->readEntry("Fill", "Tabs") != "Tabs")
  {
		int wsCount = config->readNumEntry("FillSpaces",2);
		setSpaceIndentation(wsCount);
		m_indentString = "";
		m_indentString.fill(' ', wsCount);
  } else
  {
	setTabIndentation();
	m_indentString = "\t";
  }

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

KDevFormatter::KDevFormatter( AStyleWidget * widget )
{	
	if ( widget->Style_ANSI->isChecked() )
	{
		predefinedStyle( "ANSI" );
		return;
	}
	if ( widget->Style_GNU->isChecked() )
	{
		predefinedStyle( "GNU" );
		return;
	}
	if ( widget->Style_JAVA->isChecked() )
	{
		predefinedStyle( "JAVA" );
		return;
	}
	if ( widget->Style_KR->isChecked() )
	{
		predefinedStyle( "KR" );
		return;
	}
	if ( widget->Style_Linux->isChecked() )
	{
		predefinedStyle( "Linux" );
		return;
	}

	// fill
	if ( widget->Fill_Tabs->isChecked() )
	{
		setTabIndentation();
		m_indentString = "\t";
	}
	else
	{
		setSpaceIndentation( widget->Fill_SpaceCount->value() );
		m_indentString = "";
		m_indentString.fill(' ', widget->Fill_SpaceCount->value());
	}
	
	// indent
	setSwitchIndent( widget->Indent_Switches->isChecked() );
	setClassIndent( widget->Indent_Classes->isChecked() );
	setCaseIndent( widget->Indent_Cases->isChecked() );
	setBracketIndent( widget->Indent_Brackets->isChecked() );
	setNamespaceIndent( widget->Indent_Namespaces->isChecked() );
	setLabelIndent( widget->Indent_Labels->isChecked() );
	
	// continuation
	setMaxInStatementIndentLength( widget->Continue_MaxStatement->value() );
	setMinConditionalIndentLength( widget->Continue_MinConditional->value() );
	
	// brackets
	if ( widget->Brackets_Break->isChecked() )
	{
		setBracketFormatMode( BREAK_MODE );
	}
	else if ( widget->Brackets_Attach->isChecked() )
	{
		setBracketFormatMode( ATTACH_MODE );
	}
	else 
	{
		setBracketFormatMode( BDAC_MODE );
	}
	
	// padding
	setOperatorPaddingMode( widget->Pad_Operators->isChecked() );
	setParenthesisPaddingMode( widget->Pad_Parentheses->isChecked() );
	
	// oneliner
	setBreakOneLineBlocksMode( widget->Keep_Blocks->isChecked() );
	setSingleStatementsMode( widget->Keep_Statements->isChecked() );
}

bool KDevFormatter::predefinedStyle( const QString & style )
{
	if (style == "ANSI")
	{
		setBracketIndent(false);
		setSpaceIndentation(4);
		setBracketFormatMode(BREAK_MODE);
		setClassIndent(false);
		setSwitchIndent(false);
		setNamespaceIndent(false);
		return true;
	}
	if (style == "KR")
	{
		setBracketIndent(false);
		setSpaceIndentation(4);
		setBracketFormatMode(ATTACH_MODE);
		setClassIndent(false);
		setSwitchIndent(false);
		setNamespaceIndent(false);
		return true;
	}
	if (style == "Linux")
	{
		setBracketIndent(false);
		setSpaceIndentation(8);
		setBracketFormatMode(BDAC_MODE);
		setClassIndent(false);
		setSwitchIndent(false);
		setNamespaceIndent(false);
		return true;
	}
	if (style == "GNU")
	{
		setBlockIndent(true);
		setSpaceIndentation(2);
		setBracketFormatMode(BREAK_MODE);
		setClassIndent(false);
		setSwitchIndent(false);
		setNamespaceIndent(false);
		return true;
	}
	if (style == "JAVA")
	{
		setJavaStyle();
		setBracketIndent(false);
		setSpaceIndentation(4);
		setBracketFormatMode(ATTACH_MODE);
		setSwitchIndent(false);
		return true;
	}
	return false;
}
