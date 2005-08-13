/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>

void ConfigureProblemReporter::init()
{
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    bgParserCheckbox->setChecked( config->readBoolEntry("EnableJavaBgParser", true) );
    delaySlider->setEnabled( bgParserCheckbox->isChecked() );
    delaySlider->setValue( config->readNumEntry("JavaBgParserDelay", 250) );
    setDelayLabel( delaySlider->value() );
}

void ConfigureProblemReporter::destroy()
{
}

void ConfigureProblemReporter::accept()
{
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    config->writeEntry( "EnableJavaBgParser", bgParserCheckbox->isChecked() );
    if( bgParserCheckbox->isChecked() )
	config->writeEntry( "JavaBgParserDelay", delaySlider->value() );
    config->sync();
}


void ConfigureProblemReporter::bgParserCheckbox_toggled( bool b )
{
    delaySlider->setEnabled( b );
    if ( b == TRUE )
	delayLabel->show();
    else
	delayLabel->hide();
}


void ConfigureProblemReporter::setDelayLabel( int delay )
{
    delayLabel->setText( i18n( "delay: %1 msec" ).arg( delay ) );
}


void ConfigureProblemReporter::addSpecialHeader()
{

}


void ConfigureProblemReporter::removeSpecialHeader()
{

}


void ConfigureProblemReporter::moveUpSpecialHeader()
{

}


void ConfigureProblemReporter::moveDownSpecialHeader()
{

}
