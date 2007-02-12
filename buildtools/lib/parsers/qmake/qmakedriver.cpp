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

#include <qvaluestack.h>
#include <kio/netaccess.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "qmake_lex.h"
#include "qmake_yacc.hpp"

namespace QMake {

int Driver::parseFile(const char *fileName, ProjectAST **ast, int debug)
{
    std::ifstream inf( fileName, std::ios::in );
    if ( !inf.is_open() )
    {
        *ast = 0;
        return 1;
    }
//     yydebug = 1;
    Lexer l(&inf);
    l.set_debug(debug);
    int depth = 0;
    QValueStack<ProjectAST*> stack;
    Parser p(&l, stack, depth);
    p.set_debug_level(debug);
    int ret = p.parse();
    *ast = stack.top();
    (*ast)->setFileName(fileName);
    switch( l.lineending() )
    {
        case QMake::Lexer::Windows:
            (*ast)->setLineEnding(QMake::ProjectAST::Windows);
            break;
        case QMake::Lexer::MacOS:
            (*ast)->setLineEnding(QMake::ProjectAST::MacOS);
            break;
        case QMake::Lexer::Unix:
        default:
            (*ast)->setLineEnding(QMake::ProjectAST::Unix);
            break;
    }
    return ret;
}

int Driver::parseFile(QString fileName, ProjectAST **ast, int debug)
{
    return parseFile(fileName.ascii(), ast, debug);
}

int Driver::parseFile(KURL fileName, ProjectAST **ast, int debug)
{
    QString tmpFile;
    int ret = 0;
    if (KIO::NetAccess::download(fileName, tmpFile, 0))
        ret = parseFile(tmpFile, ast, debug);
    KIO::NetAccess::removeTempFile(tmpFile);
    return ret;
}

int Driver::parseString( const char* string, ProjectAST **ast, int debug )
{
    std::istringstream ins;
    ins.str(string);
    Lexer l(&ins);
    l.set_debug(debug);
    int depth = 0;
    QValueStack<ProjectAST*> stack;
    Parser p(&l, stack, depth);
    p.set_debug_level(debug);
    int ret = p.parse();
    *ast = stack.top();
    (*ast)->setFileName("");
    switch( l.lineending() )
    {
        case QMake::Lexer::Windows:
            (*ast)->setLineEnding(QMake::ProjectAST::Windows);
            break;
        case QMake::Lexer::MacOS:
            (*ast)->setLineEnding(QMake::ProjectAST::MacOS);
            break;
        case QMake::Lexer::Unix:
        default:
            (*ast)->setLineEnding(QMake::ProjectAST::Unix);
            break;
    }
    return ret;
}

}
