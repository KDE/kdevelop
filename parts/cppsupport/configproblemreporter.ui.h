/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include <qaccel.h>
#include <kconfig.h>
#include <kapplication.h>

void ConfigureProblemReporter::init()
{
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    bgParserCheckbox->setChecked( config->readBoolEntry("EnableCppBgParser", true) );
    delaySlider->setEnabled( bgParserCheckbox->isChecked() );
    delaySlider->setValue( config->readNumEntry("CppBgParserDelay", 250) );
    loadSpecialWords();
    addSpecialWord();
    
    QAccel* a = new QAccel( specialWordsTable );
    a->connectItem( a->insertItem(Key_Insert), this, SLOT(addSpecialWord()) );
    a->connectItem( a->insertItem(Key_Delete), this, SLOT(removeCurrentSpecialWord()) );
}

void ConfigureProblemReporter::destroy()
{
    storeSpecialWords();
}

void ConfigureProblemReporter::loadSpecialWords()
{
}

void ConfigureProblemReporter::storeSpecialWords()
{
}

void ConfigureProblemReporter::addSpecialWord()
{
    int row = specialWordsTable->numRows();
    specialWordsTable->insertRows( row );
    specialWordsTable->setItem( row, 1, 
				new QComboTableItem( specialWordsTable, 
						     QStringList() << 
						     "False" << "True") );						       
}

void ConfigureProblemReporter::removeCurrentSpecialWord()
{
    if( specialWordsTable->numRows() > 0 )
    specialWordsTable->removeRow( specialWordsTable->currentRow() );
}

void ConfigureProblemReporter::accept()
{
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    config->writeEntry( "EnableCppBgParser", bgParserCheckbox->isChecked() );
    if( bgParserCheckbox->isChecked() )
	config->writeEntry( "CppBgParserDelay", delaySlider->value() );
    config->sync();
}


void ConfigureProblemReporter::bgParserCheckbox_toggled( bool b )
{
    delaySlider->setEnabled( b );
}
