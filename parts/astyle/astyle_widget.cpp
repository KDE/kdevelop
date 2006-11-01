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
	// which style changed - disable the other pages.
	connect(StyleGroup, SIGNAL(clicked(int)), this, SLOT(styleChanged()));
  connect(ConfigTabs, SIGNAL(currentChanged(QWidget*)), this, SLOT(styleChanged()) );

  connect(FillingGroup, SIGNAL(clicked(int)), this, SLOT(styleChanged()));
  connect(Fill_ForceTabs, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Fill_TabCount, SIGNAL(valueChanged(int)), this, SLOT(styleChanged()));
  connect(Fill_SpaceCount, SIGNAL(valueChanged(int)), this, SLOT(styleChanged()));

  connect(BracketGroup, SIGNAL(clicked(int)), this, SLOT(styleChanged()));
  connect(Brackets_CloseHeaders, SIGNAL(clicked()), this, SLOT(styleChanged()));

  connect(Indent_Switches, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Indent_Cases, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Indent_Classes, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Indent_Brackets, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Indent_Namespaces, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Indent_Labels, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Indent_Blocks, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Indent_Preprocessors, SIGNAL(clicked()), this, SLOT(styleChanged()));

  connect(Continue_MaxStatement, SIGNAL(valueChanged(int)), this, SLOT(styleChanged()));
  connect(Continue_MinConditional, SIGNAL(valueChanged(int)), this, SLOT(styleChanged()));

  connect(Block_Break, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Block_BreakAll, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Block_IfElse, SIGNAL(clicked()), this, SLOT(styleChanged()));

  connect(Pad_ParenthesesIn, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Pad_ParenthesesOut, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Pad_ParenthesesUn, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Pad_Operators, SIGNAL(clicked()), this, SLOT(styleChanged()));

  connect(Keep_Statements, SIGNAL(clicked()), this, SLOT(styleChanged()));
  connect(Keep_Blocks, SIGNAL(clicked()), this, SLOT(styleChanged()));


  // load settings
  KConfig *config = kapp->config();
  config->setGroup("AStyle");

  // style
  QString s = config->readEntry("FStyle");
  // fake the id so we disable the other pages.
  int id=0;
  if (s == "ANSI") id=1;
  if (s == "KR") id=2;
  if (s == "Linux") id=3;
  if (s == "GNU") id=4;
  if (s == "JAVA") id=5;
  StyleGroup->setButton(id);

  if ( s == "UserDefined" ){

	  int wsCount = config->readNumEntry("FillCount",2);
	  if (config->readEntry("Fill", "Tabs") == "Tabs")
	  {
		  Fill_Tabs->setChecked(true);
		  Fill_TabCount->setValue(wsCount);
		  Fill_TabCount->setEnabled(true);
		  Fill_ForceTabs->setChecked(config->readBoolEntry("FillForce",false));
		  Fill_ForceTabs->setEnabled(true);

	  } else
	  {
		  Fill_Spaces->setChecked(true);
		  Fill_SpaceCount->setValue(wsCount);
		  Fill_SpaceCount->setEnabled(true);
		  Fill_ConvertTabs->setChecked(config->readBoolEntry("FillForce",false));
		  Fill_ConvertTabs->setEnabled(true);
	  }

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
	 // brackets
	s = config->readEntry("Brackets", "None");
	if (s == "Break")
		Brackets_Break->setChecked(true);
	else if (s == "Attach")
		Brackets_Attach->setChecked(true);
	else if (s == "Linux")
		Brackets_Linux->setChecked(true);
	else
		Brackets_None->setChecked(true);

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
  }


  styleChanged();
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
  if (Style_ANSI->isChecked())
	config->writeEntry("FStyle", "ANSI");
  else if (Style_KR->isChecked())
    config->writeEntry("FStyle", "KR");
  else if (Style_Linux->isChecked())
	config->writeEntry("FStyle", "Linux");
  else if (Style_GNU->isChecked())
    config->writeEntry("FStyle", "GNU");
  else if (Style_JAVA->isChecked())
    config->writeEntry("FStyle", "JAVA");
  else if (Style_UserDefined->isChecked()){
	config->writeEntry("FStyle", "UserDefined");

	// fill
	if ( Fill_Tabs->isChecked()){
		config->writeEntry("Fill", "Tabs");
		config->writeEntry("FillCount", Fill_TabCount->value());
		config->writeEntry("FillForce", Fill_ForceTabs->isChecked());
	}
	else{
		config->writeEntry("Fill", "Spaces");
		config->writeEntry("FillCount", Fill_SpaceCount->value());
		config->writeEntry("FillForce", Fill_ConvertTabs->isChecked());
	}

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
	if ( Brackets_None->isChecked())
		config->writeEntry("Brackets", "None");
	else if (Brackets_Break->isChecked())
		config->writeEntry("Brackets","Break" );
	else if (Brackets_Attach->isChecked())
		config->writeEntry("Brackets", "Attach" );
	else if (Brackets_Linux->isChecked())
		config->writeEntry("Brackets","Linux");

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
  }
  config->sync();
}


/**
 * Change the sample formatting text depending on what tab is selected.
 *
 * @param id Tab selected
 */
void AStyleWidget::styleChanged(  )
{

	// diable the tabs for predefined sample
	// only user define id==0, enables the other tabs
	ConfigTabs->setTabEnabled(tab_2, Style_UserDefined->isChecked());
	ConfigTabs->setTabEnabled(tab_3, Style_UserDefined->isChecked());
	ConfigTabs->setTabEnabled(tab_4, Style_UserDefined->isChecked());

	int id = ConfigTabs->currentPageIndex();


	StyleExample->clear();

	QString bracketSample = "if (isFoo){\n\tbar();\n} else {\n\tbar();\n}\n";

	QString indentSample = "#define foobar(A)\\\n{Foo();Bar();}\n#define anotherFoo(B)\\\nreturn Bar()\n\nnamespace Bar\n{\nclass Foo\n{public:\nFoo();\nvirtual !Foo();\n};\nswitch (foo)\n{\ncase 1:\na+=1;\nbreak;\ncase 2:\n{\na += 2;\n break;\n}\n}\nif (isFoo)\n{\nbar();\n{\nelse\n{\nanotherBar();\n}\n}\nint foo()\n\twhile(isFoo)\n\t\t{\n\t\t\t...\n\t\t\tgoto error;\n\t\t....\n\t\terror:\n\t\t\t...\n\t\t}\n\t}\nfooArray[]={ red,\n\tgreen,\n\tdarkblue};\nfooFunction(barArg1,\n\tbarArg2,\n\tbarArg3);\n";

	QString formattingSample = "if(isFoo(a,b))\n\tbar(a,b);\nif(isFoo)\n\ta=bar((b-c)*a,*d--);\nif(  isFoo( a,b ) )\n\tbar(a, b);\nif (isFoo) {isFoo=false;cat << isFoo <<endl;}\nif(isFoo)DoBar();if (isFoo){\n\tbar();\n\telse if(isBar()){\n\tannotherBar();\n}\n";

	QString styleSample = "\t//Tabs & Brackets\nnamespace foo{\n" + bracketSample + "}\n\t// Indentation\n" + indentSample + "\t// Formatting\n" + formattingSample;


	switch(id){
		case 1:
			StyleExample->setText( m_part->formatSource( bracketSample, this ) );
			break;
		case 2:
			StyleExample->setText( m_part->formatSource( indentSample, this ) );
			break;
		case 3:
			StyleExample->setText( m_part->formatSource( formattingSample, this ) );
			break;
		default:
		StyleExample->setText( m_part->formatSource( styleSample, this ) );
	}
}

#include "astyle_widget.moc"
