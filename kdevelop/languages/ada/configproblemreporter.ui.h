/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <kconfig.h>
#include <kapplication.h>

void ConfigureProblemReporter::init()
{
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    bgParserCheckbox->setChecked( config->readBoolEntry("EnableAdaBgParser", true) );
    delaySlider->setEnabled( bgParserCheckbox->isChecked() );
    delaySlider->setValue( config->readNumEntry("AdaBgParserDelay", 1000) );
}

void ConfigureProblemReporter::destroy()
{
}

void ConfigureProblemReporter::accept()
{
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    config->writeEntry( "EnableAdaBgParser", bgParserCheckbox->isChecked() );
    if( bgParserCheckbox->isChecked() )
	config->writeEntry( "AdaBgParserDelay", delaySlider->value() );
    config->sync();
}


void ConfigureProblemReporter::bgParserCheckbox_toggled( bool b )
{
    delaySlider->setEnabled( b );
}
