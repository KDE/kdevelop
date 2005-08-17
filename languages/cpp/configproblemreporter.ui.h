/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "cppsupportpart.h"

#include <kdevproject.h>

#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>

void ConfigureProblemReporter::init()
{
	m_part = 0;
	KConfig* config = kapp->config();
	config->setGroup( "General Options" );
	bgParserCheckbox->setChecked( config->readBoolEntry( "EnableCppBgParser", true ) );
	delaySlider->setEnabled( bgParserCheckbox->isChecked() );
	delaySlider->setValue( config->readNumEntry( "CppBgParserDelay", 500 ) );
	setDelayLabel( delaySlider->value() );
}

void ConfigureProblemReporter::destroy()
{}

void ConfigureProblemReporter::setPart( CppSupportPart* part )
{
	m_part = part;
	if ( !m_part )
		return ;

	QString conf_file_name = m_part->specialHeaderName();
	if ( QFile::exists( conf_file_name ) )
	{
		QFile f( conf_file_name );
		if ( f.open( QIODevice::ReadOnly ) )
		{
			QTextStream stream( &f );
			specialHeader->setText( stream.read() );
			f.close();
		}
	}
}

void ConfigureProblemReporter::accept()
{
	KConfig * config = kapp->config();
	config->setGroup( "General Options" );
	config->writeEntry( "EnableCppBgParser", bgParserCheckbox->isChecked() );
	if ( bgParserCheckbox->isChecked() )
		config->writeEntry( "CppBgParserDelay", delaySlider->value() );
	config->sync();

	if ( m_part && specialHeader->isModified() )
	{
		QString conf_file_name = m_part->specialHeaderName( true );
		QFile f( conf_file_name );
		if ( f.open( QIODevice::WriteOnly ) )
		{
			QTextStream stream( &f );
			stream << specialHeader->text();
			f.close();

			m_part->updateParserConfiguration();
		}
	}
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
// kate: indent-mode csands; tab-width 4;

