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

#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QFile>
#include <kdebug.h>
#include "qmake_parser.h"
#include "qmake_ast.h"

#include "qmakedebugvisitor.h"
#include "buildastvisitor.h"

#include "qmakeast.h"

namespace QMake
{

Driver::Driver()
    : m_debug(false)
{
}

bool Driver::readFile( const QString& filename, const char* codec )
{
    QFile f(filename);
    if( !f.open(QIODevice::ReadOnly) )
    {
        kDebug(9024) << "Couldn't open project file: " << filename << endl;
        return false;
    }
    QTextStream s(&f);
    if( codec )
        s.setCodec( QTextCodec::codecForName(codec) );
    m_content = s.readAll();
    return true;
}
void Driver::setContent( const QString& content )
{
    m_content = content;
}
void Driver::setDebug( bool debug )
{
    m_debug = debug;
}
bool Driver::parse( ProjectAST** qmast )
{
    parser::token_stream_type token_stream;
    parser::memory_pool_type memory_pool;

    parser qmakeparser;
    qmakeparser.set_token_stream(&token_stream);
    qmakeparser.set_memory_pool(&memory_pool);
    qmakeparser.setDebug( m_debug );

    qmakeparser.tokenize(m_content);
    project_ast* ast = 0;
    bool matched = qmakeparser.parse_project(&ast);
    if( matched )
    {
        qDebug() << "Sucessfully parsed";
        if( m_debug )
        {
            DebugVisitor d(&qmakeparser);
            d.visit_project(ast);
        }
        *qmast = new ProjectAST();
        BuildASTVisitor d( &qmakeparser, *qmast );
        d.visit_project(ast);
        kDebug(9024) << "found stmts:" << (*qmast)->statements().count() << endl;
    }else
    {
        ast = 0;
        qmakeparser.yy_expected_symbol(ast_node::Kind_project, "project");
    }
    return matched;
}

}


// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
