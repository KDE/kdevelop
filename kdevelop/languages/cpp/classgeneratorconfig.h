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

#ifndef CLASSGENERATORCONFIG_H
#define CLASSGENERATORCONFIG_H

#include "classgeneratorconfigbase.h"

class ClassGeneratorConfig : public ClassGeneratorConfigBase
{
	Q_OBJECT

public:
	enum NameCase { LowerCase, UpperCase, SameAsClassCase, SameAsFileCase };

	ClassGeneratorConfig( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

	ClassGeneratorConfig( QString v_cppHeaderText, QString v_cppSourceText,
	                      QString v_objcHeaderText, QString v_objcSourceText,
	                      QString v_gtkHeaderText, QString v_gtkSourceText,
	                      NameCase v_fileCase, NameCase v_defCase, NameCase v_superCase,
	                      bool v_showAuthor, bool v_genDoc, bool v_reformat,
	                      QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

	~ClassGeneratorConfig();
	/*$PUBLIC_FUNCTIONS$*/

	void readConfig();

	QString cppHeader();
	QString cppSource();
	QString objcHeader();
	QString objcSource();
	QString gtkHeader();
	QString gtkSource();

	NameCase fileCase();
	NameCase defCase();
	NameCase superCase();

	bool showAuthor();
	bool genDoc();

public slots:
	/*$PUBLIC_SLOTS$*/
	void storeConfig();

protected:
	/*$PROTECTED_FUNCTIONS$*/

protected slots:
	/*$PROTECTED_SLOTS$*/
	virtual void templateTypeChanged( int type );

private:
	QString *identifyTemplate( int value );
	QString templateText( QString path );
	void saveTemplateText( QString path, QString content );

	QString cppHeaderText;
	QString cppSourceText;
	QString objcHeaderText;
	QString objcSourceText;
	QString gtkHeaderText;
	QString gtkSourceText;

	QString *currTemplate;
};

#endif 
// kate: indent-mode csands; tab-width 4;


