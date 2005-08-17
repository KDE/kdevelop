#include "astyle_part.h"
#include "astyle_widget.h"

#include <qradiobutton.h>
#include <qtabwidget.h>
#include <q3multilineedit.h>
#include <q3buttongroup.h>
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
  if (config->readEntry("Fill", "Tabs") == "Tabs")
	Fill_Tabs->setChecked(true);
  else
	Fill_Spaces->setChecked(true);
  Fill_SpaceCount->setValue(config->readNumEntry("FillSpaces",2));

  // indent
  Indent_Switches->setChecked(config->readBoolEntry("IndentSwitches", false));
  Indent_Cases->setChecked(config->readBoolEntry("IndentCases", false));
  Indent_Classes->setChecked(config->readBoolEntry("IndentClasses", false));
  Indent_Brackets->setChecked(config->readBoolEntry("IndentBrackets", false));
  Indent_Namespaces->setChecked(config->readBoolEntry("IndentNamespaces", true));
  Indent_Labels->setChecked(config->readBoolEntry("IndentLabels", true));  

  // contiuation
  Continue_MaxStatement->setValue(config->readNumEntry("MaxStatement", 40));
  Continue_MinConditional->setValue(config->readNumEntry("MinConditional", -1));

  // brackets
  s = config->readEntry("Brackets", "Break");
  Brackets_Break->setChecked(s == "Break");
  Brackets_Attach->setChecked(s == "Attach");
  Brackets_Linux->setChecked(s == "Linux");

  // padding
  Pad_Parentheses->setChecked(config->readBoolEntry("PadParentheses", false));
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
  if (Fill_Tabs->isChecked())
	config->writeEntry("Fill", "Tabs");
  else
	config->writeEntry("Fill", "Spaces");
  config->writeEntry("FillSpaces", Fill_SpaceCount->value());

  // indent
  config->writeEntry("IndentSwitches", Indent_Switches->isChecked());
  config->writeEntry("IndentCases", Indent_Cases->isChecked());
  config->writeEntry("IndentClasses", Indent_Classes->isChecked());
  config->writeEntry("IndentBrackets", Indent_Brackets->isChecked());
  config->writeEntry("IndentNamespaces", Indent_Namespaces->isChecked());
  config->writeEntry("IndentLabels", Indent_Labels->isChecked());

  // continuation
  config->writeEntry("MaxStatement", Continue_MaxStatement->value());
  config->writeEntry("MinConditional", Continue_MinConditional->value());

  // brackets
  if (Brackets_Break->isChecked())
	config->writeEntry("Brackets", "Break");
  else if (Brackets_Attach->isChecked())
	config->writeEntry("Brackets", "Attach");
  else
	config->writeEntry("Brackets", "Linux");

  // padding
  config->writeEntry("PadParentheses", Pad_Parentheses->isChecked());
  config->writeEntry("PadOperators", Pad_Operators->isChecked());

  // oneliner
  config->writeEntry("KeepStatements", Keep_Statements->isChecked());
  config->writeEntry("KeepBlocks", Keep_Blocks->isChecked());  

  config->sync();
}


void AStyleWidget::styleChanged( int id )
{
	QString sample = "namespace foospace { class Bar { public: int foo(); private: int m_foo; }; int Bar::foo() { switch (x) { case 1: break; default: break; } if (isBar) { bar(); return m_foo+1; } else return 0; } }";

	ConfigTabs->setTabEnabled(tab_2, id == 0);
	ConfigTabs->setTabEnabled(tab_3, id == 0);
	
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
