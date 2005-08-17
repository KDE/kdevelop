/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qcheckbox.h>
#include <qcombobox.h>
#include <q3textedit.h>
#include <qfile.h>
#include <qfileinfo.h>
//Added by qt3to4:
#include <QTextStream>

#include <kstandarddirs.h>
#include <kconfig.h>

#include "cppsupportfactory.h"
#include "classgeneratorconfig.h"

ClassGeneratorConfig::ClassGeneratorConfig( QWidget* parent, const char* name, Qt::WFlags fl )
		: ClassGeneratorConfigBase( parent, name, fl )
{
	readConfig();
	currTemplate = &cppHeaderText;
	template_edit->setText( *currTemplate );
}

ClassGeneratorConfig::ClassGeneratorConfig( QString v_cppHeaderText, QString v_cppSourceText,
        QString v_objcHeaderText, QString v_objcSourceText,
        QString v_gtkHeaderText, QString v_gtkSourceText,
        NameCase v_fileCase, NameCase v_defCase, NameCase v_superCase,
        bool v_showAuthor, bool v_genDoc, bool v_reformat,
        QWidget* parent, const char* name, Qt::WFlags fl )
		: ClassGeneratorConfigBase( parent, name, fl ),
		cppHeaderText( v_cppHeaderText ), cppSourceText( v_cppSourceText ),
		objcHeaderText( v_objcHeaderText ), objcSourceText( v_objcSourceText ),
		gtkHeaderText( v_gtkHeaderText ), gtkSourceText( v_gtkSourceText )
{
	filecase_box->setCurrentItem( ( int ) v_fileCase );
	defcase_box->setCurrentItem( ( int ) v_defCase );
	supercase_box->setCurrentItem( ( int ) v_superCase );
	author_box->setChecked( v_showAuthor );
	doc_box->setChecked( v_genDoc );
	reformat_box->setChecked( v_reformat );

	currTemplate = &cppHeaderText;
}

ClassGeneratorConfig::~ClassGeneratorConfig()
{}

/*$SPECIALIZATION$*/
void ClassGeneratorConfig::templateTypeChanged( int type )
{
	*currTemplate = template_edit->text();

	currTemplate = identifyTemplate( type );
	template_edit->setText( *currTemplate );
}

QString ClassGeneratorConfig::cppHeader()
{
	if ( currTemplate == &cppHeaderText )
		* currTemplate = template_edit->text();
	return cppHeaderText;
}

QString ClassGeneratorConfig::cppSource()
{
	if ( currTemplate == &cppSourceText )
		* currTemplate = template_edit->text();
	return cppSourceText;
}

QString ClassGeneratorConfig::objcHeader()
{
	if ( currTemplate == &objcHeaderText )
		* currTemplate = template_edit->text();
	return objcHeaderText;
}

QString ClassGeneratorConfig::objcSource()
{
	if ( currTemplate == &objcSourceText )
		* currTemplate = template_edit->text();
	return objcSourceText;
}

QString ClassGeneratorConfig::gtkHeader()
{
	if ( currTemplate == &gtkHeaderText )
		* currTemplate = template_edit->text();
	return gtkHeaderText;
}

QString ClassGeneratorConfig::gtkSource()
{
	if ( currTemplate == &gtkSourceText )
		* currTemplate = template_edit->text();
	return gtkSourceText;
}

ClassGeneratorConfig::NameCase ClassGeneratorConfig::fileCase()
{
	return ( NameCase ) filecase_box->currentItem();
}

ClassGeneratorConfig::NameCase ClassGeneratorConfig::defCase()
{
	return ( NameCase ) defcase_box->currentItem();
}

ClassGeneratorConfig::NameCase ClassGeneratorConfig::superCase()
{
	return ( NameCase ) supercase_box->currentItem();
}

bool ClassGeneratorConfig::showAuthor()
{
	return author_box->isChecked();
}

bool ClassGeneratorConfig::genDoc()
{
	return doc_box->isChecked();
}

QString *ClassGeneratorConfig::identifyTemplate( int value )
{
	switch ( value )
	{
	case 0:
		return & cppHeaderText;
	case 1:
		return &cppSourceText;
	case 2:
		return &objcHeaderText;
	case 3:
		return &objcSourceText;
	case 4:
		return &gtkHeaderText;
	case 5:
		return &gtkSourceText;
	}
	return 0;
}

void ClassGeneratorConfig::readConfig()
{
	KConfig * config = CppSupportFactory::instance() ->config();
	if ( config )
	{
		config->setGroup( "Class Generator" );

		filecase_box->setCurrentItem( config->readNumEntry( "File Name Case", 0 ) );
		defcase_box->setCurrentItem( config->readNumEntry( "Defines Case", 1 ) );
		supercase_box->setCurrentItem( config->readNumEntry( "Superclasss Name Case", 0 ) );

		author_box->setChecked( config->readBoolEntry( "Show Author Name", 1 ) );
		doc_box->setChecked( config->readBoolEntry( "Generate Empty Documentation", 1 ) );

		reformat_box->setChecked( config->readBoolEntry( "Reformat Source", 0 ) );

		KStandardDirs *dirs = CppSupportFactory::instance() ->dirs();

		cppHeaderText = templateText( dirs->findResource( "newclasstemplates", "cpp_header" ) );
		cppSourceText = templateText( dirs->findResource( "newclasstemplates", "cpp_source" ) );
		objcHeaderText = templateText( dirs->findResource( "newclasstemplates", "objc_header" ) );
		objcSourceText = templateText( dirs->findResource( "newclasstemplates", "objc_source" ) );
		gtkHeaderText = templateText( dirs->findResource( "newclasstemplates", "gtk_header" ) );
		gtkSourceText = templateText( dirs->findResource( "newclasstemplates", "gtk_source" ) );
	}
}


QString ClassGeneratorConfig::templateText( QString path )
{
	QFileInfo f( path );
	if ( f.exists() )
	{
		QFile file( path );
		if ( file.open( QIODevice::ReadOnly ) )
		{
			QTextStream stream( &file );
			return stream.read();
		}
		else
			return "";
	}
	else
		return "";
}

void ClassGeneratorConfig::storeConfig()
{
	KConfig * config = CppSupportFactory::instance() ->config();
	if ( config )
	{
		config->setGroup( "Class Generator" );

		config->writeEntry( "File Name Case", filecase_box->currentItem() );
		config->writeEntry( "Defines Case", defcase_box->currentItem() );
		config->writeEntry( "Superclasss Name Case", supercase_box->currentItem() );

		config->writeEntry( "Show Author Name", author_box->isChecked() );
		config->writeEntry( "Generate Empty Documentation", doc_box->isChecked() );

		config->writeEntry( "Reformat Source", reformat_box->isChecked() );

		KStandardDirs *dirs = CppSupportFactory::instance() ->dirs();

		saveTemplateText( dirs->saveLocation( "newclasstemplates" ) + "cpp_header", cppHeader() );
		saveTemplateText( dirs->saveLocation( "newclasstemplates" ) + "cpp_source", cppSource() );
		saveTemplateText( dirs->saveLocation( "newclasstemplates" ) + "objc_header", objcHeader() );
		saveTemplateText( dirs->saveLocation( "newclasstemplates" ) + "objc_source", objcSource() );
		saveTemplateText( dirs->saveLocation( "newclasstemplates" ) + "gtk_header", gtkHeader() );
		saveTemplateText( dirs->saveLocation( "newclasstemplates" ) + "gtk_source", gtkSource() );
	}
}

void ClassGeneratorConfig::saveTemplateText( QString path, QString content )
{
	QFile f( path );
	if ( f.open( QIODevice::WriteOnly ) )
	{
		QTextStream stream( &f );
		stream << content;
		f.close();
	}
}

#include "classgeneratorconfig.moc"
