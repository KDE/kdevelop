/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "simpleparser.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qregexp.h>
#include <kregexp.h>
#include <kdebug.h>

static QString remove( QString text, const QChar& l, const QChar& r )
{
    QString s;

    unsigned int index = 0;
    int count = 0;
    while( index < text.length() ){
        if( text[index] == l ){
            ++count;
        } else if( text[index] == r ){
            --count;
        } else if( count == 0 ){
            s += text[ index ];
        }
        ++index;
    }
    return s;
}

static QString remove_comment( QString text ){
    QString s;
    unsigned int index = 0;
    bool skip = FALSE;
    while( index < text.length() ){
        if( text.mid(index, 2) == "/*" ){
            skip = TRUE;
            index += 2;
            continue;
        } else if( text.mid(index, 2) == "*/" ){
            skip = FALSE;
            index += 2;
            continue;
        } else if( !skip ){
            s += text[ index ];
        }
        ++index;
    }
    return s;
}

QValueList<SimpleVariable> SimpleParser::localVariables( QString contents ){
    QValueList<SimpleVariable> vars;

    QRegExp ws( "[ \t]+" );
    QRegExp qt( "Q_[A-Z]+" );
    QRegExp rx( "[\n|&|\\*]" );
    QRegExp strconst( "\"[^\"]*\"" );
    QRegExp chrconst( "'[^']*'" );
    QRegExp assign( "=[^,;]*" );
    QStringList keywords = QStringList::split( "|", "case|new|delete|const|static|struct|if|else|return|while|for|do" );

//    contents = remove_comment( contents );
//    contents = remove( contents, '[', ']' );

    contents
        .replace( ws, " " )
        .replace( rx, "" )
        .replace( strconst, "" )
        .replace( chrconst, "" )
        .replace( QRegExp("static"), "" )
        .replace( QRegExp("\\{"), "{;" )
        .replace( QRegExp("\\}"), ";};" )
        ;

    kdDebug() << "contents = " << contents << endl;

    QStringList lines = QStringList::split( ";", contents );

    QRegExp decl_rx( "^[ \t]*([a-zA-Z0-9_<>:]+)[ \t]+([a-zA-Z0-9_]+)[^{]*$" );

    int lev = 0;
    QStringList::Iterator it = lines.begin();
    while( it != lines.end() ){
        QString line = *it++;
        line = line.simplifyWhiteSpace();

        QString simplifyLine = remove( line, '(', ')' );
        simplifyLine.replace( assign, "" );

        kdDebug() << "line = |" << line << "|" << endl;

        if( line.find("{") != -1 ){
            ++lev;
        } else if( line.find("}") != -1 ){
            --lev;
        }

        if( line.startsWith("(") || line.isEmpty() ){
            // pass
        }
        else if( decl_rx.exactMatch(simplifyLine) ){
            // parse a declaration
            QString type = QString::fromLatin1( decl_rx.cap( 1 ) );
            QString rest = simplifyLine.mid( decl_rx.pos(2) )
                           .replace( ws, "" );
            if( keywords.findIndex(type) == -1 ){

                QStringList vlist = QStringList::split( ",", rest);
                for( QStringList::Iterator it=vlist.begin(); it!=vlist.end(); ++it ){
                    SimpleVariable var;
                    var.scope = lev;
                    var.type = type;
                    var.name = *it;
                    vars.append( var );
                }
//                qDebug( "lev = %d - type = %s - vars = %s",
//                        lev,
//                        type.latin1(),
//                        vlist.join(", ").latin1() );
            }
        }
    }
    return vars;
}

QValueList<SimpleVariable> SimpleParser::parseFile( const QString& filename ){
    QValueList<SimpleVariable> vars;
    kdDebug(9007) << "-----------------------------------------------------------" << endl;
    QFile f( filename );
    if( f.open(IO_ReadOnly) ){
        QTextStream in( &f );
        QString contents = in.read();
        vars = localVariables( contents );
        f.close();
    }
    kdDebug(9007) << "-----------------------------------------------------------" << endl;
    return vars;
}


SimpleVariable SimpleParser::findVariable( const QValueList<SimpleVariable>& vars,
                                           const QString& varname )
{
    for( int i=vars.count() - 1; i>=0; --i ){
        SimpleVariable v = vars[ i ];
        if( v.name == varname ) // TODO: check variable's scope
            return v;
    }
    return SimpleVariable();
}
