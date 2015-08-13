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
#include <QDebug>

#include "debug.h"
#include "qmakeparser.h"
#include "qmakeast.h"

#include "qmakedebugvisitor.h"
#include "buildastvisitor.h"

#include "ast.h"

#define ifDebug(x)

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
        qCWarning(KDEV_QMAKE) << "Couldn't open project file:" << filename;
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
    KDevPG::TokenStream tokenStream;
    KDevPG::MemoryPool memory_pool;

    Parser qmakeparser;
    qmakeparser.setTokenStream(&tokenStream);
    qmakeparser.setMemoryPool(&memory_pool);
    qmakeparser.setDebug( m_debug );

    qmakeparser.tokenize(m_content);
    ProjectAst* ast = nullptr;
    bool matched = qmakeparser.parseProject(&ast);
    if( matched )
    {
        ifDebug(qCDebug(KDEV_QMAKE) << "Successfully parsed";)
        if( m_debug )
        {
            DebugVisitor d(&qmakeparser);
            d.visitProject(ast);
        }
        *qmast = new ProjectAST();
        BuildASTVisitor d( &qmakeparser, *qmast );
        d.visitProject(ast);
        ifDebug(qCDebug(KDEV_QMAKE) << "Found" << (*qmast)->statements.count() << "Statements";)
    }else
    {
        ast = nullptr;
        qCDebug(KDEV_QMAKE) << "Couldn't parse content";
    }
    return matched;
}

}


