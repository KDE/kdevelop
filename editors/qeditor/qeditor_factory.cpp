/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   Based on KHTML Factory from Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include "qeditor_factory.h"
#include "qeditor_part.h"
#include "qeditor_view.h"

#include <klocale.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <assert.h>

template class QPtrList<QEditorPart>;
template class QPtrList<QEditorView>;

QEditorPartFactory *QEditorPartFactory::s_self = 0;
unsigned long int QEditorPartFactory::s_refcnt = 0;
KInstance *QEditorPartFactory::s_instance = 0;
KAboutData *QEditorPartFactory::s_about = 0;
QPtrList<QEditorPart> *QEditorPartFactory::s_documents = 0;
QPtrList<QEditorView> *QEditorPartFactory::s_views = 0;
KTrader::OfferList *QEditorPartFactory::s_plugins = 0;

extern "C"
{
  void *init_libqeditorpart()
  {
    return new QEditorPartFactory( true );
  }
}

QEditorPartFactory::QEditorPartFactory( bool clone )
{
  if ( clone )
  {
    ref();
    return;
  }
}

QEditorPartFactory::~QEditorPartFactory()
{
  if ( s_self == this )
  {
        assert( !s_refcnt );

        if ( s_instance )
            delete s_instance;

        if ( s_about )
            delete s_about;

        if ( s_documents )
        {
            assert( s_documents->isEmpty() );
            delete s_documents;
        }

        if ( s_views )
        {
            assert( s_views->isEmpty() );
            delete s_views;
        }

	if ( s_plugins )
	  delete s_plugins;

        s_instance = 0;
        s_about = 0;
        s_documents = 0;
        s_views = 0;
	s_plugins = 0;
    }
    else
        deref();
}

void QEditorPartFactory::ref()
{
    if ( !s_refcnt && !s_self )
    {
      s_self = new QEditorPartFactory;
    }

    s_refcnt++;
}

void QEditorPartFactory::deref()
{
    if ( !--s_refcnt && s_self )
    {
        delete s_self;
        s_self = 0;
    }
}

KParts::Part *QEditorPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList & args )
{
  bool bWantSingleView = !( classname == QString("KTextEditor::Document") );
  bool bWantBrowserView = ( classname == QString("Browser/View") );
  bool bWantReadOnly = (bWantBrowserView || ( classname == QString("KParts::ReadOnlyPart") ));

  KParts::ReadWritePart *part = new QEditorPart (
      /*bWantSingleView, bWantBrowserView, bWantReadOnly, */
      parentWidget, widgetName, parent, name, args);
  part->setReadWrite( !bWantReadOnly );

  return part;
}

void QEditorPartFactory::registerDocument ( QEditorPart *doc )
{
    if ( !s_documents )
        s_documents = new QPtrList<QEditorPart>;

    if ( !s_documents->containsRef( doc ) )
    {
        s_documents->append( doc );
        ref();
    }
}

void QEditorPartFactory::deregisterDocument ( QEditorPart *doc )
{
    assert( s_documents );

    if ( s_documents->removeRef( doc ) )
    {
        if ( s_documents->isEmpty() )
        {
            delete s_documents;
            s_documents = 0;
        }

        deref();
    }
}

void QEditorPartFactory::registerView ( QEditorView *view )
{
    if ( !s_views )
        s_views = new QPtrList<QEditorView>;

    if ( !s_views->containsRef( view ) )
    {
        s_views->append( view );
        ref();
    }
}

void QEditorPartFactory::deregisterView ( QEditorView *view )
{
    assert( s_views );

    if ( s_views->removeRef( view ) )
    {
        if ( s_views->isEmpty() )
        {
            delete s_views;
            s_views = 0;
        }

        deref();
    }
}

KTrader::OfferList *QEditorPartFactory::plugins ()
{
  if ( !s_plugins )
   s_plugins = new QValueList<KService::Ptr> (KTrader::self()->query("KTextEditor/Plugin"));

  return s_plugins;
}

KInstance *QEditorPartFactory::instance()
{
    assert( s_self );

    if ( !s_instance )
    {
        s_about = new KAboutData("qeditorpart", I18N_NOOP("QEditor (based on the editor of Qt-Designer)"), "0.1");
	
        s_about->addAuthor("Roberto Raggi", 0, "roberto@kdevelop.org");
        s_about->addAuthor("Trolltech AS", 0, "info@trolltech.com");
        s_about->addAuthor("The Kate authors", 0, "kwrite-devel@kde.org");
	s_about->addAuthor("F@lk Brettschneider", 0, "falk@kdevelop.org");
        s_about->addAuthor("Milo Hoffman", 0, "Milo@NG-Projekt.ORG");

        s_instance = new KInstance( s_about );
    }

    return s_instance;
}

const QPtrList<class QEditorPart>& QEditorPartFactory::documents()
{
    return *s_documents;
}

const QPtrList<class QEditorView>& QEditorPartFactory::views()
{
    return *s_views;
}
#include "qeditor_factory.moc"
