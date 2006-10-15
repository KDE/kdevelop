#include "astyle_part.h"
#include "astyle_widget.h"

#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qmultilineedit.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <kapplication.h>
#include <kdebug.h>

#include <kdevcore.h>


AStyleWidget::AStyleWidget( AStylePart * part, QWidget *parent, const char *name )
 : AStyleConfig(parent, name), m_part ( part )
{
  connect(StyleGroup, SIGNAL(clicked(int)), this, SLOT(styleChanged(int)));
  connect(ConfigTabs, SIGNAL(currentChanged(QWidget*)), this, SLOT(pageChanged()) );

  connect(FillingGroup, SIGNAL(clicked(int)), this, SLOT(pageChanged()));
  connect(Fill_ForceTabs, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Fill_TabCount, SIGNAL(valueChanged(int)), this, SLOT(pageChanged()));
  connect(Fill_SpaceCount, SIGNAL(valueChanged(int)), this, SLOT(pageChanged()));

  connect(BracketGroup, SIGNAL(clicked(int)), this, SLOT(pageChanged()));
  connect(Brackets_CloseHeaders, SIGNAL(clicked()), this, SLOT(pageChanged()));

  connect(Indent_Switches, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Indent_Cases, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Indent_Classes, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Indent_Brackets, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Indent_Namespaces, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Indent_Labels, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Indent_Blocks, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Indent_Preprocessors, SIGNAL(clicked()), this, SLOT(pageChanged()));

  connect(Continue_MaxStatement, SIGNAL(valueChanged(int)), this, SLOT(pageChanged()));
  connect(Continue_MinConditional, SIGNAL(valueChanged(int)), this, SLOT(pageChanged()));

  connect(Block_Break, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Block_BreakAll, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Block_IfElse, SIGNAL(clicked()), this, SLOT(pageChanged()));

  connect(Pad_ParenthesesIn, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Pad_ParenthesesOut, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Pad_ParenthesesUn, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Pad_Operators, SIGNAL(clicked()), this, SLOT(pageChanged()));

  connect(Keep_Statements, SIGNAL(clicked()), this, SLOT(pageChanged()));
  connect(Keep_Blocks, SIGNAL(clicked()), this, SLOT(pageChanged()));


  // load settings
  KConfig *config = kapp->config();
  config->setGroup("AStyle");

  // style
  QString s = config->readEntry("Style");
  int id=0;
  if (s == "ANSI") id=1;
  if (s == "KR") id=2;
  if (s == "Linux") id=3;
  if (s == "GNU") id=4;
  if (s == "JAVA") id=5;
  StyleGroup->setButton(id);

  // fill
  Fill_Tabs->setChecked(config->readBoolEntry("FillTabs", true));
  Fill_TabCount->setValue(config->readNumEntry("FillTabCount",2));
  Fill_ForceTabs->setChecked(config->readBoolEntry("FillForceTabs",false));
  Fill_Spaces->setChecked(config->readBoolEntry("FillSpaces",false));
  Fill_SpaceCount->setValue(config->readNumEntry("FillSpaceCount",2));


  Fill_ConvertTabs->setChecked(config->readBoolEntry("FillConvertTabs",false));
  Fill_EmptyLines->setChecked(config->readBoolEntry("FillEmptyLines",false));

  // indent
  Indent_Switches->setChecked(config->readBoolEntry("IndentSwitches", false));
  Indent_Cases->setChecked(config->readBoolEntry("IndentCases", false));
  Indent_Classes->setChecked(config->readBoolEntry("IndentClasses", false));
  Indent_Brackets->setChecked(config->readBoolEntry("IndentBrackets", false));
  Indent_Namespaces->setChecked(config->readBoolEntry("IndentNamespaces", true));
  Indent_Labels->setChecked(config->readBoolEntry("IndentLabels", true));
  Indent_Blocks->setChecked(config->readBoolEntry("IndentBlocks", true));
  Indent_Preprocessors->setChecked(config->readBoolEntry("IndentPreprocessors", true));

  // contiuation
  Continue_MaxStatement->setValue(config->readNumEntry("MaxStatement", 40));
  Continue_MinConditional->setValue(config->readNumEntry("MinConditional", -1));

  // brackets
  Brackets_None->setChecked(config->readBoolEntry("BracketsNone", true));
  Brackets_Break->setChecked(config->readBoolEntry("BracketsBreak", false));
  Brackets_Attach->setChecked(config->readBoolEntry("BracketsAttach", false));
  Brackets_Linux->setChecked(config->readBoolEntry("BracketsLinux", false));
  Brackets_CloseHeaders->setChecked(config->readBoolEntry("BracketsCloseHeaders",false));

  Block_Break->setChecked(config->readBoolEntry("BlockBreak",false));
  Block_BreakAll->setChecked(config->readBoolEntry("BlockBreakAll",false));
  Block_IfElse->setChecked(config->readBoolEntry("BlockIfElse",false));

  // padding
  Pad_ParenthesesIn->setChecked(config->readBoolEntry("PadParenthesesIn", false));
  Pad_ParenthesesOut->setChecked(config->readBoolEntry("PadParenthesesOut", false));
  Pad_ParenthesesUn->setChecked(config->readBoolEntry("PadParenthesesUn", false));

  Pad_Operators->setChecked(config->readBoolEntry("PadOperators", false));

  // oneliner
  Keep_Statements->setChecked(config->readBoolEntry("KeepStatements", false));
  Keep_Blocks->setChecked(config->readBoolEntry("KeepBlocks", false));

  styleChanged(id);
}


AStyleWidget::~AStyleWidget()
{
}


void AStyleWidget::accept()
{
  // save settings
  KConfig *config = kapp->config();
  config->setGroup("AStyle");

  // style
  if (Style_UserDefined->isChecked())
	config->writeEntry("Style", "UserDefined");
  else if (Style_ANSI->isChecked())
	config->writeEntry("Style", "ANSI");
  else if (Style_KR->isChecked())
    config->writeEntry("Style", "KR");
  else if (Style_Linux->isChecked())
	config->writeEntry("Style", "Linux");
  else if (Style_GNU->isChecked())
    config->writeEntry("Style", "GNU");
  else if (Style_JAVA->isChecked())
    config->writeEntry("Style", "JAVA");

  // fill
  config->writeEntry("FillTabs", Fill_Tabs->isChecked());
  config->writeEntry("FillTabCount", Fill_SpaceCount->value());
  config->writeEntry("FillForceTabs", Fill_ForceTabs->isChecked());
  config->writeEntry("FillSpaces", Fill_Spaces->isChecked());
  config->writeEntry("FillSpaceCount", Fill_SpaceCount->value());
  config->writeEntry("FillConvertTabs", Fill_ConvertTabs->isChecked());
  config->writeEntry("FillEmptyLines", Fill_EmptyLines->isChecked());

  // indent
  config->writeEntry("IndentSwitches", Indent_Switches->isChecked());
  config->writeEntry("IndentCases", Indent_Cases->isChecked());
  config->writeEntry("IndentClasses", Indent_Classes->isChecked());
  config->writeEntry("IndentBrackets", Indent_Brackets->isChecked());
  config->writeEntry("IndentNamespaces", Indent_Namespaces->isChecked());
  config->writeEntry("IndentLabels", Indent_Labels->isChecked());
  config->writeEntry("IndentBlocks", Indent_Blocks->isChecked());
  config->writeEntry("IndentPreprocessors", Indent_Preprocessors->isChecked());

  // continuation
  config->writeEntry("MaxStatement", Continue_MaxStatement->value());
  config->writeEntry("MinConditional", Continue_MinConditional->value());

  // brackets
  config->writeEntry("BracketsNone", Brackets_None->isChecked());
  config->writeEntry("BracketsBreak", Brackets_Break->isChecked());
  config->writeEntry("BracketsAttach", Brackets_Attach->isChecked());
  config->writeEntry("BracketsLinux", Brackets_Linux->isChecked());
  config->writeEntry("BracketsCloseHeaders", Brackets_CloseHeaders->isChecked());

  // blocks
  config->writeEntry("BlockBreak", Block_Break->isChecked());
  config->writeEntry("BlockBreakAll", Block_BreakAll->isChecked());
  config->writeEntry("BlockIfElse", Block_IfElse->isChecked());

  // padding
  config->writeEntry("PadParenthesesIn", Pad_ParenthesesIn->isChecked());
  config->writeEntry("PadParenthesesOut", Pad_ParenthesesOut->isChecked());
  config->writeEntry("PadParenthesesUn", Pad_ParenthesesUn->isChecked());

  config->writeEntry("PadOperators", Pad_Operators->isChecked());

  // oneliner
  config->writeEntry("KeepStatements", Keep_Statements->isChecked());
  config->writeEntry("KeepBlocks", Keep_Blocks->isChecked());

  config->sync();
}


void AStyleWidget::styleChanged( int id )
{
	QString sample = "namespace foospace {class Bar { public: int foo(); private: int m_foo;};int Bar::foo(){ switch (x) { case 1:{ALabel:if(true)Foo();else Bar();for(int i=0;i<10;i++){a+=i;}} break; default: break; } if ((isBar)&&(isFoo)){ bar(); return m_foo+1; } else return 0; } }";

	ConfigTabs->setTabEnabled(tab_2, id == 0);
	ConfigTabs->setTabEnabled(tab_3, id == 0);
	ConfigTabs->setTabEnabled(tab_4, id == 0);
	
	StyleExample->clear();
	
	StyleExample->setText( m_part->formatSource( sample, this ) );
}

void AStyleWidget::pageChanged( )
{
	if ( Style_UserDefined->isChecked() )
	{
		styleChanged( 0 );
	}
}


#include "astyle_widget.moc"
