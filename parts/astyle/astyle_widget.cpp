#include "astyle_part.h"
#include "astyle_widget.h"

#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qmultilineedit.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdevcore.h>


AStyleWidget::AStyleWidget(AStylePart * part, bool global, QWidget *parent, const char *name)
    : AStyleConfig(parent, name), m_part(part), isGlobal(global)
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


  QMap<QString, QVariant> option;
  if ( isGlobal){
	Style_Global->setEnabled(false);
	Style_Global->hide();
	GeneralExtension->hide();
	FrameFilesFormat->hide();
	option = m_part->getGlobalOptions();
  }
  else{
	Style_Global->setEnabled(true);
	Style_Global->show();
	GeneralExtension->show();
	FrameFilesFormat->show();
	option = m_part->getProjectOptions();
    QString ext = m_part->getExtensions();
	  if ( !ext.isEmpty())
  			GeneralExtension->setText(m_part->getExtensions());
  }

  // style
  QString s = option["FStyle"].toString();
  // fake the id so we disable the other pages.
  int id=0;
  if (s == "ANSI") id=1;
  if (s == "KR") id=2;
  if (s == "Linux") id=3;
  if (s == "GNU") id=4;
  if (s == "JAVA") id=5;
  if (s == "GLOBAL") id = 6;
  StyleGroup->setButton(id);

  if (s == "UserDefined" || s =="GLOBAL" )
  {
    int wsCount = option["FillCount"].toInt();
    if (option["Fill"].toString() == "Tabs")
	  {
		  Fill_Tabs->setChecked(true);
		  Fill_TabCount->setValue(wsCount);
		  Fill_TabCount->setEnabled(true);
      Fill_ForceTabs->setChecked(option["FillForce"].toBool());
		  Fill_ForceTabs->setEnabled(true);
    }
    else
	  {
		  Fill_Spaces->setChecked(true);
		  Fill_SpaceCount->setValue(wsCount);
		  Fill_SpaceCount->setEnabled(true);
      Fill_ConvertTabs->setChecked(option["FillForce"].toBool());
		  Fill_ConvertTabs->setEnabled(true);
	  }

    Fill_EmptyLines->setChecked(option["FillEmptyLines"].toBool());

	// indent
    Indent_Switches->setChecked(option["IndentSwitches"].toBool());
    Indent_Cases->setChecked(option["IndentCases"].toBool());
    Indent_Classes->setChecked(option["IndentClasses"].toBool());
    Indent_Brackets->setChecked(option["IndentBrackets"].toBool());
    Indent_Namespaces->setChecked(option["IndentNamespaces"].toBool());
    Indent_Labels->setChecked(option["IndentLabels"].toBool());
    Indent_Blocks->setChecked(option["IndentBlocks"].toBool());
    Indent_Preprocessors->setChecked(option["IndentPreprocessors"].toBool());

	// contiuation
    Continue_MaxStatement->setValue(option["MaxStatement"].toInt());
    Continue_MinConditional->setValue(option["MinConditional"].toInt());

	// brackets
    s = option["Brackets"].toString();
	if (s == "Break")
		Brackets_Break->setChecked(true);
	else if (s == "Attach")
		Brackets_Attach->setChecked(true);
	else if (s == "Linux")
		Brackets_Linux->setChecked(true);
	else
		Brackets_None->setChecked(true);

    Brackets_CloseHeaders->setChecked(option["BracketsCloseHeaders"].toBool());

    Block_Break->setChecked(option["BlockBreak"].toBool());
    Block_BreakAll->setChecked(option["BlockBreakAll"].toBool());
    Block_IfElse->setChecked(option["BlockIfElse"].toBool());

	// padding
    Pad_ParenthesesIn->setChecked(option["PadParenthesesIn"].toBool());
    Pad_ParenthesesOut->setChecked(option["PadParenthesesOut"].toBool());
    Pad_ParenthesesUn->setChecked(option["PadParenthesesUn"].toBool());

    Pad_Operators->setChecked(option["PadOperators"].toBool());

	// oneliner
    Keep_Statements->setChecked(option["KeepStatements"].toBool());
    Keep_Blocks->setChecked(option["KeepBlocks"].toBool());
  }


  styleChanged();
}


AStyleWidget::~AStyleWidget()
{
}


void AStyleWidget::accept()
{
  QMap<QString, QVariant>* m_option;
  if ( isGlobal){
	m_option = &(m_part->getGlobalOptions());
  }
  else{
	m_option = &(m_part->getProjectOptions());
	m_part->setExtensions(GeneralExtension->text());
  }


  // style
  if (Style_ANSI->isChecked())
    (*m_option)["FStyle"] = "ANSI";
  else if (Style_KR->isChecked())
    (*m_option)["FStyle"] = "KR";
  else if (Style_Linux->isChecked())
    (*m_option)["FStyle"] = "Linux";
  else if (Style_GNU->isChecked())
    (*m_option)["FStyle"] = "GNU";
  else if (Style_JAVA->isChecked())
    (*m_option)["FStyle"] = "JAVA";
  else if (Style_Global->isChecked()){
	  QMap<QString,QVariant>& global = m_part->getGlobalOptions();
	  QMap<QString,QVariant>& project = m_part->getProjectOptions();
	  project=global;
	  project["FStyle"]="GLOBAL";
  }
  else if (Style_UserDefined->isChecked())
  {
    (*m_option)["FStyle"] = "UserDefined";

	// fill
    if (Fill_Tabs->isChecked())
    {
      (*m_option)["Fill"] = "Tabs";
      (*m_option)["FillCount"] = Fill_TabCount->value();
      (*m_option)["FillForce"] = Fill_ForceTabs->isChecked();
	}
    else
    {
      (*m_option)["Fill"] = "Spaces";
      (*m_option)["FillCount"] = Fill_SpaceCount->value();
      (*m_option)["FillForce"] = Fill_ConvertTabs->isChecked();
	}

    (*m_option)["FillEmptyLines"] = Fill_EmptyLines->isChecked();

	// indent
    (*m_option)["IndentSwitches"] = Indent_Switches->isChecked();
    (*m_option)["IndentCases"] = Indent_Cases->isChecked();
    (*m_option)["IndentClasses"] = Indent_Classes->isChecked();
    (*m_option)["IndentBrackets"] = Indent_Brackets->isChecked();
    (*m_option)["IndentNamespaces"] = Indent_Namespaces->isChecked();
    (*m_option)["IndentLabels"] = Indent_Labels->isChecked();
    (*m_option)["IndentBlocks"] = Indent_Blocks->isChecked();
    (*m_option)["IndentPreprocessors"] = Indent_Preprocessors->isChecked();

	// continuation
    (*m_option)["MaxStatement"] = Continue_MaxStatement->value();
    (*m_option)["MinConditional"] = Continue_MinConditional->value();

	// brackets
	if ( Brackets_None->isChecked())
      (*m_option)["Brackets"] = "None";
	else if (Brackets_Break->isChecked())
      (*m_option)["Brackets"] = "Break";
	else if (Brackets_Attach->isChecked())
      (*m_option)["Brackets"] = "Attach";
	else if (Brackets_Linux->isChecked())
      (*m_option)["Brackets"] = "Linux";

    (*m_option)["BracketsCloseHeaders"] = Brackets_CloseHeaders->isChecked();

	// blocks
    (*m_option)["BlockBreak"] = Block_Break->isChecked();
    (*m_option)["BlockBreakAll"] = Block_BreakAll->isChecked();
    (*m_option)["BlockIfElse"] = Block_IfElse->isChecked();

	// padding
    (*m_option)["PadParenthesesIn"] = Pad_ParenthesesIn->isChecked();
    (*m_option)["PadParenthesesOut"] = Pad_ParenthesesOut->isChecked();
    (*m_option)["PadParenthesesUn"] = Pad_ParenthesesUn->isChecked();

    (*m_option)["PadOperators"] = Pad_Operators->isChecked();

	// oneliner
    (*m_option)["KeepStatements"] = Keep_Statements->isChecked();
    (*m_option)["KeepBlocks"] = Keep_Blocks->isChecked();
  }

  if ( isGlobal){
	  QMap<QString, QVariant>& project = m_part->getProjectOptions();
	  if ( project["FStyle"] == "GLOBAL"){
		  project = m_part->getGlobalOptions();
		  project["FStyle"] = "GLOBAL";
	  }
	  m_part->saveGlobal();
  }

//   for ( QMap<QString, QVariant>::ConstIterator iter = m_option->begin();iter != m_option->end();iter++ )
// 	{
// 		kdDebug ( 9009 ) << "widget: " << iter.key() << "=" << iter.data()  << endl;
// 	}

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

	QString bracketSample = "namespace foospace {\n\tint Foo(){\n\tif (isBar)\n{\nbar(); \n\treturn 1; } else \nreturn 0;}}\n\nvoid test(){\n\tif (isFoo){\n\tbar();\n} else\n{\n\tbar();\n}\n}\n";

	QString indentSample = "#define foobar(A)\\\n{Foo();Bar();}\n#define anotherFoo(B)\\\nreturn Bar()\n\nnamespace Bar\n{\nclass Foo\n{public:\nFoo();\nvirtual !Foo();\n};\nswitch (foo)\n{\ncase 1:\na+=1;\nbreak;\ncase 2:\n{\na += 2;\n break;\n}\n}\nif (isFoo)\n{\nbar();\n}\nelse\n{\nanotherBar();\n}\nint foo()\n\twhile(isFoo)\n\t\t{\n\t\t\t...\n\t\t\tgoto error;\n\t\t....\n\t\terror:\n\t\t\t...\n\t\t}\n\t}\nfooArray[]={ red,\n\tgreen,\n\tdarkblue};\nfooFunction(barArg1,\n\tbarArg2,\n\tbarArg3);\n";

	QString formattingSample = "void func(){\n\tif(isFoo(a,b))\n\tbar(a,b);\nif(isFoo)\n\ta=bar((b-c)*a,*d--);\nif(  isFoo( a,b ) )\n\tbar(a, b);\nif (isFoo) {isFoo=false;cat << isFoo <<endl;}\nif(isFoo)DoBar();if (isFoo){\n\tbar();\n}\n\telse if(isBar()){\n\tannotherBar();\n}\n}\n";

	QString styleSample = "\t//Tabs & Brackets\nnamespace foo{\n" + bracketSample + "}\n\t// Indentation\n" + indentSample + "\t// Formatting\n" + formattingSample;



	switch(id){
		case 1:
			StyleExample->setText( m_part->formatSource( bracketSample, this,m_part->getProjectOptions() ) );
			break;
		case 2:
			StyleExample->setText( m_part->formatSource( indentSample, this,m_part->getProjectOptions() ) );
			break;
		case 3:
			StyleExample->setText( m_part->formatSource( formattingSample, this,m_part->getProjectOptions() ) );
			break;
		default:
		{
			if ( Style_Global->isChecked())
				StyleExample->setText( m_part->formatSource( styleSample, 0, m_part->getGlobalOptions()) );
			else
			{
				StyleExample->setText( m_part->formatSource( styleSample, this, m_part->getProjectOptions() ) );
			}
		}
	}
}

#include "astyle_widget.moc"
