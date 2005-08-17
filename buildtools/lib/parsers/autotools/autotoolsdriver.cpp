/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   Copyright (c) 2005 by Matt Rogers                                     *
 *   mattr@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "autotoolsdriver.h"
#include "autotoolsast.h"

#include <stdio.h>
#include <q3valuestack.h>
#include <kio/netaccess.h>

extern FILE *yyin, *yyout;
extern int yyparse();
//extern int yydebug;
extern Q3ValueStack<AutoTools::ProjectAST *> projects;

namespace AutoTools {

int Driver::parseFile(const char *fileName, ProjectAST **ast)
{
	//yydebug = 1;
	yyin = fopen(fileName, "r");
	if (yyin == 0)
	{
		ast = 0;
		return 1;
	}
	int ret = yyparse();
	*ast = projects.top();
	fclose(yyin);
	return ret;
}

int Driver::parseFile(QString fileName, ProjectAST **ast)
{
	return parseFile(fileName.ascii(), ast);
}

int Driver::parseFile(KURL fileName, ProjectAST **ast)
{
	QString tmpFile;
	int ret = 0;
	if (KIO::NetAccess::download(fileName, tmpFile, 0))
		ret = parseFile(tmpFile, ast);
	KIO::NetAccess::removeTempFile(tmpFile);
	return ret;
}

}

//kate: indent-mode csands; space-indent off; tab-width 4;

