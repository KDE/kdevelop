
#include "simpleparser.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qregexp.h>

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
    QRegExp comment( "//[^\\n]*" );
    QRegExp preproc( "^\\s*#[^\\n]*$" );
    QRegExp rx( "[\n|&|\\*]" );
    QRegExp strconst( "\\\"([^\"]|\\\\\\\")*\\\"" );
    QRegExp chrconst( "'[^']*'" );
    QRegExp keywords( "\\b(public|protected|private|mutable|typename|case|new|delete|enum|class|virtual|const|extern|static|struct|if|else|return|while|for|do)\\b" ); // etc...
    QRegExp assign( "=[^,;]*" );

    contents = remove_comment( contents );
//    contents = remove( contents, '(', ')' );
    contents = remove( contents, '[', ']' );

    contents
        .replace( ws, " " )
        .replace( qt, "" )
        .replace( preproc, "" )
        .replace( comment, "" )
        .replace( rx, "" )
        .replace( strconst, "" )
        .replace( chrconst, "" )
        .replace( keywords, "" )
        .replace( QRegExp("\\{"), "{;" )
        .replace( QRegExp("\\}"), ";};" )
        ;

    QStringList lines = QStringList::split( ";", contents );

    QRegExp decl_rx( "^\\s*(([\\w_]|::)+)\\s+([\\w_]+)\\b[^{]*$" );
    QRegExp method_rx( "^\\s*((?:[\\w_]|::)+).*{$" );
    QRegExp ide_rx( "\\b(([\\w_]|::)+)\\s*\\(" );

    int lev = 0;
    QStringList::Iterator it = lines.begin();
    while( it != lines.end() ){
        QString line = *it++;
        line = line.simplifyWhiteSpace();

        QString simplifyLine = remove( line, '(', ')' );
        simplifyLine.replace( assign, "" );

        if( line.find("{") != -1 ){
            ++lev;
        } else if( line.find("}") != -1 ){
            --lev;
        }

        if( line.startsWith("(") ){
            // pass
#if QT_VERSION >= 300
        } else if( decl_rx.exactMatch(simplifyLine) ){
            // parse a declaration
            QString type = decl_rx.cap( 1 );
            QString rest = simplifyLine.mid( decl_rx.pos(2) + 1 )
                           .replace( ws, "" );

            QStringList vlist = QStringList::split( ",", rest);
            for( QStringList::Iterator it=vlist.begin(); it!=vlist.end(); ++it ){
                SimpleVariable var;
                var.scope = lev;
                var.type = type;
                var.name = *it;
                vars.append( var );
            }
//            qDebug( "lev = %d - type = %s - vars = %s",
//                    lev,
//                    type.latin1(),
//                    vlist.join(", ").latin1() );
#endif
        }
    }
    return vars;
}

QValueList<SimpleVariable> SimpleParser::parseFile( const QString& filename ){
    QValueList<SimpleVariable> vars;
    qDebug( "-----------------------------------------------------------" );
    QFile f( filename );
    if( f.open(IO_ReadOnly) ){
        QTextStream in( &f );
        QString contents = in.read();
        vars = localVariables( contents );
        f.close();
    }
    qDebug( "-----------------------------------------------------------" );
    return vars;
}
