 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "cindent.h"
#include "qregexp.h"
#include "qeditor_part.h"
#include "qeditor.h"
#include "indentconfigpage.h"

#include <qvbox.h>
#include <kdialogbase.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>

extern int indentForBottomLine( const QStringList& program, QChar typedIn );
extern void configureCIndent( const QMap<QString, QVariant>& values );

using namespace std;

CIndent::CIndent( QEditor* ed )
    : QEditorIndenter( ed )
{
    KConfig* config = QEditorPartFactory::instance()->config();
    config->setGroup( "C++ Indent" );

    m_values[ "TabSize" ] = ed->tabStop();
    m_values[ "IndentSize" ] = config->readNumEntry( "IndentSize", 4 );
    m_values[ "ContinuationSize" ] = config->readNumEntry( "ContinuationSize", 4 );
    m_values[ "CommentOffset" ] = config->readNumEntry( "CommentOffset", 2 );

    configureCIndent( m_values );
}

CIndent::~CIndent()
{
    KConfig* config = QEditorPartFactory::instance()->config();
    config->setGroup( "C++ Indent" );

    config->writeEntry( "IndentSize", m_values[ "IndentSize" ].toInt() );
    config->writeEntry( "ContinuationSize", m_values[ "ContinuationSize" ].toInt() );
    config->writeEntry( "CommentOffset", m_values[ "CommentOffset" ].toInt() );

    config->sync();
}

static int indentation( const QString &s )
{
    if ( s.simplifyWhiteSpace().length() == 0 )
	return 0;
    int i = 0;
    int ind = 0;
    while ( i < (int)s.length() ) {
	QChar c = s.at( i );
	if ( c == ' ' )
	    ind++;
	else if ( c == '\t' )
	    ind += 8;
	else
	    break;
	++i;
    }
    return ind;
}

static void tabify( QString &s )
{
    int i = 0;
    for ( ;; ) {
	for ( int j = i; j < (int)s.length(); ++j ) {
	    if ( s[ j ] != ' ' && s[ j ] != '\t' ) {
		if ( j > i ) {
		    QString t  = s.mid( i, j - i );
		    int spaces = 0;
		    for ( int k = 0; k < (int)t.length(); ++k )
			spaces += ( t[ k ] == ' ' ? 1 : 8 );
		    s.remove( i, t.length() );
		    int tabs = spaces / 8;
		    spaces = spaces - ( 8 * tabs );
		    QString tmp;
		    tmp.fill( ' ', spaces );
		    if ( spaces > 0 )
			s.insert( i, tmp );
		    tmp.fill( '\t', tabs );
		    if ( tabs > 0 )
			s.insert( i, tmp );
		}
		break;
	    }
	}
	i = s.find( '\n', i );
	if ( i == -1 )
	    break;
	++i;
    }
}

static void indentLine( QTextParagraph *p, int &oldIndent, int &newIndent )
{
    QString indentString;
    indentString.fill( ' ', newIndent );
    indentString.append( "a" );
    tabify( indentString );
    indentString.remove( indentString.length() - 1, 1 );
    newIndent = indentString.length();
    oldIndent = 0;
    while ( p->length() > 0 && ( p->at( 0 )->c == ' ' || p->at( 0 )->c == '\t' ) ) {
	++oldIndent;
	p->remove( 0, 1 );
    }
    if ( p->string()->length() == 0 )
	p->append( " " );
    if ( !indentString.isEmpty() )
	p->insert( 0, indentString );
}


void CIndent::indent( QTextDocument *doc, QTextParagraph *p, int *oldIndent, int *newIndent )
{
    int oi = indentation( p->string()->toString() );
    QStringList code;
    QTextParagraph *parag = doc->firstParagraph();
    while ( parag ) {
	code << parag->string()->toString();
	if ( parag == p )
	    break;
	parag = parag->next();
    }

    int ind = indentForBottomLine( code, QChar::null );
    indentLine( p, oi, ind );
    if ( oldIndent )
	*oldIndent = oi;
    if ( newIndent )
	*newIndent = ind;
}

QWidget* CIndent::createConfigPage( QEditorPart* part, KDialogBase* parentWidget,
                                    const char* widgetName )
{
    QVBox* vbox = parentWidget->addVBoxPage( i18n("Indent") );

    IndentConfigPage* page = new IndentConfigPage( vbox, widgetName );
    page->setPart( part );
    QObject::connect( parentWidget, SIGNAL(okClicked()),
                      page, SLOT(accept()) );
    return page;
}

void CIndent::updateValues( const QMap<QString, QVariant>& values )
{
    QEditorIndenter::updateValues( values );
    configureCIndent( values );
}
