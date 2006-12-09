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

#include "qmakeparser.h"
#include "qmakeast.h"
#include <iostream>
#include <stdio.h>

#include <QtCore/QByteArray>
#include <QtCore/QStack>

extern FILE* yyin, *yyout;
extern int yyparse();
extern int yydebug;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string( const char* );
extern void yy_delete_buffer( YY_BUFFER_STATE );
extern QMake::ProjectAST* project;

namespace QMake
{
    int Parser::parseFile( char const *filename, ProjectAST** ast )
    {
        yyin = fopen( filename, "r" );
        if ( yyin == 0 )
        {
            std::cerr << filename << ": file not found" << std::endl;
            *ast = 0;
            return 1;
        }
        printf("Parsing\n");
        yydebug = 1;
        int ret = yyparse();
        printf("Parsed: %d\n", ret);
        *ast = project;
        (*ast)->setFilename(QString::fromUtf8( filename ));
        return ret;
    }

    int Parser::parseFile( const QString& filename, ProjectAST** ast )
    {
        return parseFile( filename.toUtf8().constData(), ast );
    }

    int Parser::parseString( char const *content, ProjectAST** ast )
    {
        YY_BUFFER_STATE state = yy_scan_string( content );
//         yydebug = 1;
        int ret = yyparse();
        *ast = project;
        (*ast)->setFilename( "" );
        yy_delete_buffer( state );
        return ret;
    }

    int Parser::parseString( const QString& content, ProjectAST** ast )
    {
        QByteArray qb = content.toUtf8();
        const char* data = qb.data();
        return QMake::Parser::parseString( data, ast );
    }

}


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

