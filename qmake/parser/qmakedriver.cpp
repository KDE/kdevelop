/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "qmakedriver.h"
#include "qmakeast.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include <QtCore/QByteArray>
#include <QtCore/QStack>

#include "qmake_lexer.h"
#include "qmake_parser.hpp"

namespace QMake
{
    int Driver::parseFile( char const *filename, ProjectAST* ast, int debug )
    {
        std::ifstream inf( filename, std::ios::in );
        if ( !inf.is_open() )
        {
            std::cerr << filename << ": file not found" << std::endl;
            *ast = 0;
            return 1;
        }
        printf("Parsing\n");
        Lexer l(&inf);
        l.set_debug(debug);
        Parser p(&l, ast);
        p.set_debug_level(debug);
        int ret = p.parse();
        printf("Parsed: %d\n", ret);
        ast->setFilename(QString::fromUtf8( filename ));
        return ret;
    }

    int Driver::parseFile( const QString& filename, ProjectAST* ast, int debug )
    {
        return parseFile( filename.toUtf8().constData(), ast, debug );
    }

    int Driver::parseString( char const *content, ProjectAST* ast, int debug )
    {
        std::istringstream ins;
        ins.str(content);
        Lexer l(&ins);
        l.set_debug(debug);
        Parser p(&l, ast);
        p.set_debug_level(debug);
        int ret = p.parse();
        printf("Parsed: %d\n", ret);
        ast->setFilename( "" );
        return ret;
    }

    int Driver::parseString( const QString& content, ProjectAST* ast, int debug )
    {
        QByteArray qb = content.toUtf8();
        const char* data = qb.data();
        return QMake::Driver::parseString( data, ast, debug );
    }

}


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

