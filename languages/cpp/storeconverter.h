/***************************************************************************
*   Copyright (C) 2003-2004 by Alexander Dymo                             *
*   adymo@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef STORECONVERTER_H
#define STORECONVERTER_H

#include <qstringlist.h>

#include <tag.h>
#include <catalog.h>
#include <codemodel.h>

#include "cpp_tags.h"

class CppSupportPart;

class StoreConverter
{
public:
	StoreConverter( CppSupportPart *part, CodeModel *model );

	void PCSClassToCodeModel( const QString &className, const QStringList &classScope );

protected:
	/**Parses class from PCS and adds it into a file dom.*/
	void parseClass( Tag &classTag, FileDom file );
	/**Parses function declaration from PCS and adds it into a class dom.*/
	void parseFunctionDeclaration( Tag &fun, ClassDom klass );
	/**Parses variable from PCS and adds it into a class dom.*/
	void parseVariable( Tag &var, ClassDom klass );
	/**Parses function arguments from @p cppFun and adds them into a function dom.*/
	void parseArguments( FunctionDom function, CppFunction<Tag> &cppFun );

private:
	CppSupportPart *m_part;
	CodeModel *m_model;
};

#endif 
// kate: indent-mode csands; tab-width 4;

