/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <kconfig.h>
#include <kapplication.h>

void ConfigureJavaSupport::init()
{
    KConfig* config = kapp->config();
    bgParserCheckbox->setChecked( config->readBoolEntry("EnableJavaBgParser") );
    delaySlider->setEnabled( bgParserCheckbox->isChecked() );
    delaySlider->setValue( config->readNumEntry("JavaBgParserDelay") );
}

void ConfigureJavaSupport::destroy()
{
}

void ConfigureJavaSupport::accept()
{
    KConfig* config = kapp->config();
    config->writeEntry( "EnableJavaBgParser", bgParserCheckbox->isChecked() );
    if( bgParserCheckbox->isChecked() )
	config->writeEntry( "JavaBgParserDelay", delaySlider->value() );
    config->sync();
}


void ConfigureJavaSupport::bgParserCheckbox_toggled( bool b )
{
    delaySlider->setEnabled( b );
}
