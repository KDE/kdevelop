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
#include "kdevcodetemplate.h"
#include "kdevcodetemplate.moc"
#include "docviewman.h"

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kapplication.h>

#include <kate/document.h>
#include <kate/view.h>

using namespace std;

static KStaticDeleter<KDevCodeTemplate> sd;

KDevCodeTemplate* KDevCodeTemplate::m_pSelf = 0;

KDevCodeTemplate::KDevCodeTemplate()
{

}

KDevCodeTemplate::~KDevCodeTemplate()
{

}

void KDevCodeTemplate::expandTemplate( Kate::View* pView )
{
    kdDebug() << "KDevCodeTemplate::expandTemplate()" << endl;

    if( pView ){
        unsigned int line, col;
        pView->cursorPositionReal( &line, &col );
        QString word = pView->currentWord();

        Template* templ = m_templates.find( word );
        if( templ ){
            pView->getDoc()->removeText( line, col-word.length(), line, col );
            insertChars( pView, templ->code );
        }

    }
}

void KDevCodeTemplate::insertChars( Kate::View* pView, const QString& chars )
{
    bool bMoveCursor = false;
    unsigned int line, col;
    QStringList l = QStringList::split( "\n", chars, true );
    for( int i=0; i<l.count(); ++i ){
        QString s = l[ i ];
        int idx = s.find( '|' );
        if( idx != -1 ){
            pView->insertText( s.left(idx) );
            bMoveCursor = true;
            pView->cursorPositionReal( &line, &col );
            pView->insertText( s.mid(idx+1) );
        } else {
            pView->insertText( s );
        }

        if( i != l.count()-1 ){
            pView->keyReturn();
        }
    }

    if( bMoveCursor ){
        pView->setCursorPositionReal( line, col );
    }
}

void KDevCodeTemplate::addTemplate( const QString& templ,
                                    const QString& descr,
                                    const QString& code )
{
    Template* t = m_templates.find( templ );
    if( !t ){
        t = new Template();
        m_templates.insert( templ, t );
    }
    t->description = descr;
    t->code = code;
}

void KDevCodeTemplate::removeTemplate( const QString& templ )
{
    m_templates.remove( templ );
}

void KDevCodeTemplate::clearTemplates()
{
    m_templates.clear();
}

void KDevCodeTemplate::save()
{
    kdDebug() << "KDevCodeTemplate::save()" << endl;
    QString fn = locateLocal( "data", QString(kapp->name())+"/templates.xml" );
    kdDebug() << "fn = " << fn << endl;

    QDomDocument doc( "Templates" );
    QAsciiDictIterator<Template> it( m_templates );
    QDomElement root = doc.createElement( "Templates" );
    doc.appendChild( root );
    while( it.current() ){
        Template* templ = it.current();
        QDomElement e = doc.createElement( "Template" );
        e.setAttribute( "name", QString::fromLatin1(it.currentKey()) );
        e.setAttribute( "description", templ->description );
        e.setAttribute( "code", templ->code );
        root.appendChild( e );
        ++it;
    }

    QFile f( fn );
    if( f.open(IO_WriteOnly) ){
        QTextStream stream( &f );
        stream << doc.toString();
        f.close();
    }
}

void KDevCodeTemplate::load()
{
    kdDebug() << "KDevCodeTemplate::save()" << endl;
    QString fn = locate( "data", QString(kapp->name())+"/templates.xml" );
    kdDebug() << "fn = " << fn << endl;
    QFile f( fn );
    if( f.open(IO_ReadOnly) ){
        clearTemplates();

        QDomDocument doc;
        doc.setContent( &f );
        QDomElement root = doc.firstChild().toElement();
        QDomElement e = root.firstChild().toElement();
        while( !e.isNull() ){
            addTemplate( e.attribute("name"),
                         e.attribute("description"),
                         e.attribute("code") );
            e = e.nextSibling().toElement();
        }
        f.close();
    }
}

QAsciiDictIterator<Template> KDevCodeTemplate::templates() const
{
    return QAsciiDictIterator<Template>( m_templates );
}

KDevCodeTemplate* KDevCodeTemplate::self()
{
    if( !m_pSelf ){
        m_pSelf = sd.setObject( m_pSelf, new KDevCodeTemplate() );
    }
    return m_pSelf;
}
