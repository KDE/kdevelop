/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#include "qmakedriver.h"
#include "qmakeast.h"

#include <stdio.h>
#include <qvaluestack.h>
#include <kio/netaccess.h>

extern FILE *yyin, *yyout;
extern int yyparse();
extern int yydebug;
extern QValueStack<QMake::ProjectAST *> projects;

namespace QMake {

int Driver::parseFile(const char *fileName, ProjectAST **ast)
{
    yyin = fopen(fileName, "r");
    if (yyin == 0)
    {
        ast = 0;
        return 1;
    }
//     yydebug = 1;
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
